#include"FileSystem.h"
#include"BlockDevice.h"
#include"Bitmap.h"
#include"inode.h"
#include<cstring>
#include<fstream>
#include"message.h"
#include<stdlib.h>
#define min(x , y) (x < y ? x : y)
#define max(x , y) (x > y ? x : y)
using std::string;
using std::cout;
using std::endl;

extern BufferManager g_bufferManager;
extern SuperBlock	   g_superBlock;
extern Bitmap			g_ibitmap;
extern Bitmap			g_dbitmap;
extern OpenFileTable	g_openFileTable;
extern InodeTable		g_inodeTable;
extern bool			g_shutdown;

DirectoryEntry::DirectoryEntry()
{
	this->m_inode = 0;
	memset(this->m_name, 0, DirectoryEntry::DIRSIZ);
}

void SuperBlock::initialize()
{
	memset(this, 0, sizeof(SuperBlock));
}

void FileSystem::initialize()
{
	this->bufferManagerPtr = &g_bufferManager;
	this->superBlockPtr = &g_superBlock;
	this->ibitmapPtr = &g_ibitmap;
	this->dbitmapPtr = &g_dbitmap;
	this->openFileTablePtr = &g_openFileTable;
	this->inodeTablePtr = &g_inodeTable;

	ifstream file;
	file.open(IMAGENAME, ios::in);
	if (!file.is_open())
	{
		/*镜像文件不存在，则创建并格式化*/
		file.close();
		this->createImage();
		this->formatImage();
	}
	/*载入超级块和位图*/
	this->loadSuperBlock();
	this->loadBitmap();

	/*初始化根目录和工作目录*/
	this->rootDirInode = this->inodeTablePtr->getInode(0);
	this->curWorkDirInode = this->rootDirInode;
	g_shutdown = false;
	return;
}

void FileSystem::loadSuperBlock()
{
	Buf *bp = NULL;
	for (int i = 0; i < 2; i++)
	{
		bp = this->bufferManagerPtr->bread(FileSystem::SUPERBLOCK_START_SECTOR + i);
		memcpy((unsigned char *)this->superBlockPtr + 512 * i, bp->b_addr, 512);
		bufferManagerPtr->brelease(bp);
	}
}

void FileSystem::loadBitmap()
{
	Buf *bp = NULL;
	for (int i = 0; i < 2; i++)
	{
		bp = this->bufferManagerPtr->bread(FileSystem::INODE_BITMAP_START_SECTOR + i);
		if (i == 0)
			memcpy((unsigned char *)this->ibitmapPtr, bp->b_addr, 512);
		else
			memcpy((unsigned char *)this->dbitmapPtr, bp->b_addr, 512);
		bufferManagerPtr->brelease(bp);
	}
}

void FileSystem::updateSuperBlock()
{
	Buf *bp = NULL;
	for (int i = 0; i < 2; i++)
	{
		bp = bufferManagerPtr->getBlk(i + FileSystem::SUPERBLOCK_START_SECTOR);
		if (bp == NULL)
		{
			Message::out("<FileSystem::updateSuperBlock> get block failed");
			return;
		}
		memcpy(bp->b_addr, ((unsigned char *)this->superBlockPtr) + i * 512, 512);
		bufferManagerPtr->bwrite(bp);
		bufferManagerPtr->brelease(bp);
	}
}

void FileSystem::updateBitmap()
{
	Buf *bp = NULL;
	for (int i = 0; i < 2; i++)
	{
		bp = bufferManagerPtr->getBlk(i + FileSystem::INODE_BITMAP_START_SECTOR);
		if (bp == NULL)
		{
			Message::out("<FileSystem::uopdateBitmap get block failed>");
			return;
		}
		if (i == 0)
			memcpy(bp->b_addr, (unsigned char *)this->ibitmapPtr, 512);
		else
			memcpy(bp->b_addr, (unsigned char *)this->dbitmapPtr, 512);
		bufferManagerPtr->bwrite(bp);
		bufferManagerPtr->brelease(bp);
	}
}

void FileSystem::createImage()
{

    char *tmp = (char *)malloc(BlockDevice::ALL_SECTOR_SIZE * 512);
    memset(tmp , 0 , BlockDevice::ALL_SECTOR_SIZE * 512);
    if(tmp == NULL) 
    {
        Message::out("<FileSystem::createImage> create image failed for malloc erro");
		return;
    }
    fstream file;
	file.open(IMAGENAME, ios::binary | ios::out);
	if (!file.is_open())
	{
		Message::out("<FileSystem::createImage> create image failed");
		return;
	}
	file.write(tmp, BlockDevice::ALL_SECTOR_SIZE * 512);
	file.close();
	free(tmp);
	return;
}

void FileSystem::formatImage()
{
	SuperBlock sblk_tmp;
	Buf * bp = NULL;
	/* 建立超级块*/
	sblk_tmp.s_all_size = BlockDevice::ALL_SECTOR_SIZE;
	sblk_tmp.s_available_size = FileSystem::DATA_ZONE_SIZE - 6;
	sblk_tmp.s_data_size = FileSystem::DATA_ZONE_SIZE;
	sblk_tmp.s_data_start = FileSystem::DATA_ZONE_START_SECTOR;
	sblk_tmp.s_dmap_start = FileSystem::DATA_BITMAP_START_SECTOR;
	sblk_tmp.s_fmod = 0;
	sblk_tmp.s_imap_start = FileSystem::INODE_BITMAP_START_SECTOR;
	sblk_tmp.s_inode_size = FileSystem::INODE_ZONE_SIZE;
	sblk_tmp.s_inode_start = FileSystem::INODE_ZONE_START_SECTOR;
	sblk_tmp.s_ronly = 0;
	sblk_tmp.s_superblk_start = FileSystem::SUPERBLOCK_START_SECTOR;
	sblk_tmp.s_time = 0;
	memset(sblk_tmp.padding, 0, 244 * sizeof(int));
	for (int i = 0; i < 2; i++)
	{
		bp = bufferManagerPtr->getBlk(i+FileSystem::SUPERBLOCK_START_SECTOR);
		if (bp == NULL)
		{
			Message::out("<FileSystem::format> get block failed");
			return;
		}
		memcpy(bp->b_addr, ((unsigned char *)&sblk_tmp) + i * 512, 512);
		bufferManagerPtr->bwrite(bp);
		bufferManagerPtr->brelease(bp);
	}

	/*something need to be done ...inode zone   data zone   bitmap*/
	/*建立bitmap区*/

	Bitmap ibitmap(0,FileSystem::INODE_ZONE_SIZE * 8);
	Bitmap dbitmap(21,FileSystem::DATA_ZONE_SIZE);

	/*设置bitmap前6个bit为已用，分别对应文件系统初始的目录*/

	for (int i = 0; i < 6; i++)
	{
		ibitmap.set(i);
	}

	for (int i = 0; i < 27; i++)
	{
		dbitmap.set(i);
	}
	for (int i = 0;i < 2; i++)
	{
		bp = bufferManagerPtr->getBlk(i + FileSystem::INODE_BITMAP_START_SECTOR);
		if (bp == NULL)
		{
			Message::out("<FileSystem::format> get block failed");
			return;
		}
		if (i == 0)
			memcpy(bp->b_addr, &ibitmap, 512);
		else
			memcpy(bp->b_addr, &dbitmap, 512);
		bufferManagerPtr->bwrite(bp);
		bufferManagerPtr->brelease(bp);
	}

	/*建立inode区*/
	DiskInode inode_rootDir;
	inode_rootDir.d_addr[0] = 21;	/*系统根目录文件在21号扇区*/
	inode_rootDir.d_uid = 0;
	inode_rootDir.d_gid = 0;
	inode_rootDir.d_flag |= DiskInode::ISDIR;
	inode_rootDir.d_size = 0;
	DiskInode inode_bin;
	inode_bin.d_addr[0] = 22;
	inode_bin.d_uid = 0;
	inode_bin.d_gid = 0;
	inode_bin.d_flag |= DiskInode::ISDIR;
	inode_bin.d_size = 0;
	DiskInode inode_home;
	inode_home.d_addr[0] = 23;
	inode_home.d_uid = 0;
	inode_home.d_gid = 0;
	inode_home.d_flag |= DiskInode::ISDIR;
	inode_home.d_size = 0;
	DiskInode inode_usr;
	inode_usr.d_addr[0] = 24;
	inode_usr.d_uid = 0;
	inode_usr.d_gid = 0;
	inode_usr.d_flag |= DiskInode::ISDIR;
	inode_usr.d_size = 0;
	DiskInode inode_dev;
	inode_dev.d_addr[0] = 25;
	inode_dev.d_uid = 0;
	inode_dev.d_gid = 0;
	inode_dev.d_flag |= DiskInode::ISDIR;
	inode_dev.d_size = 0;
	DiskInode inode_etc;
	inode_etc.d_addr[0] = 26;
	inode_etc.d_uid = 0;
	inode_etc.d_gid = 0;
	inode_etc.d_flag |= DiskInode::ISDIR;
	inode_etc.d_size = 0;

	bp = bufferManagerPtr->getBlk(FileSystem::INODE_ZONE_START_SECTOR);
	memcpy(bp->b_addr + sizeof(DiskInode) * 0, &inode_rootDir, sizeof(DiskInode));
	memcpy(bp->b_addr + sizeof(DiskInode) * 1, &inode_bin, sizeof(DiskInode));
	memcpy(bp->b_addr + sizeof(DiskInode) * 2, &inode_home, sizeof(DiskInode));
	memcpy(bp->b_addr + sizeof(DiskInode) * 3, &inode_usr, sizeof(DiskInode));
	memcpy(bp->b_addr + sizeof(DiskInode) * 4, &inode_dev, sizeof(DiskInode));
	memcpy(bp->b_addr + sizeof(DiskInode) * 5, &inode_etc, sizeof(DiskInode));
	memset(bp->b_addr + sizeof(DiskInode) * 6, 0, sizeof(DiskInode) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*建立data区*/
	/*rootDir扇区*/
	DirectoryEntry dirs[7];
	dirs[0].m_inode = 0;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	dirs[2].m_inode = 1;
	sprintf(dirs[2].m_name, "%s", "bin");
	dirs[3].m_inode = 2;
	sprintf(dirs[3].m_name, "%s", "home");
	dirs[4].m_inode = 3;
	sprintf(dirs[4].m_name, "%s", "usr");
	dirs[5].m_inode = 4;
	sprintf(dirs[5].m_name, "%s", "dev");
	dirs[6].m_inode = 5;
	sprintf(dirs[6].m_name, "%s", "etc");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 7);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 7, 0, 512 - sizeof(DirectoryEntry) * 7);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*bin扇区*/
	dirs[0].m_inode = 1;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR + 1);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 2);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 2, 0, 512 - sizeof(DirectoryEntry) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*home扇区*/
	dirs[0].m_inode = 2;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR + 2);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 2);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 2, 0, 512 - sizeof(DirectoryEntry) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*usr扇区*/
	dirs[0].m_inode = 3;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR + 3);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 2);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 2, 0, 512 - sizeof(DirectoryEntry) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*dev扇区*/
	dirs[0].m_inode = 4;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR + 4);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 2);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 2, 0, 512 - sizeof(DirectoryEntry) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	/*etc扇区*/
	dirs[0].m_inode = 5;
	sprintf(dirs[0].m_name, "%s", ".");
	dirs[1].m_inode = 0;
	sprintf(dirs[1].m_name, "%s", "..");
	bp = bufferManagerPtr->getBlk(FileSystem::DATA_ZONE_START_SECTOR + 5);
	memcpy(bp->b_addr, dirs, sizeof(DirectoryEntry) * 2);
	memset(bp->b_addr + sizeof(DirectoryEntry) * 2, 0, 512 - sizeof(DirectoryEntry) * 2);
	bufferManagerPtr->bwrite(bp);
	bufferManagerPtr->brelease(bp);
	return;
}

Inode *FileSystem::pathSrch(const char *path)
{
	Inode *pinode = NULL;
	int pathPtr = 0;		/*字符读取指针，在逐个往后移动*/
	char split[DirectoryEntry::DIRSIZ];	/*存放路径分量*/
	if (path == NULL)
	{
		Message::out("<FileSystem::pathSrch> erro: path ==NULL");
		return NULL;
	}
	pinode = this->curWorkDirInode;
	if (path[0] == '/')
		/*从根目录开始搜索*/ 
		pinode = this->rootDirInode;
	else
		/*从当前工作目录开始搜索*/
		pinode = this->curWorkDirInode;

	while (path[pathPtr] != '\0')
	{
		int sPtr = 0;
		while (path[pathPtr] == '/')
			pathPtr++;
		if (path[pathPtr] == '\0')
			break;
		while (path[pathPtr] != '/' && path[pathPtr] != '\0')
			split[sPtr++] = path[pathPtr++];
		split[sPtr] = '\0';
		int inodeNo = pinode->getDirEntry(split);
		if (inodeNo == -1)
		{
			//Message::out("<FileSystem::pathSrch> erro: inodeNo == -1");
			pinode = NULL;
			break;
		}
		pinode = this->inodeTablePtr->getInode(inodeNo);
	}
	return pinode;
}

string FileSystem::getCurPath()
{
	string ret = "";
	Inode * pinode = this->curWorkDirInode;
	while (pinode->i_number != 0)
	{
		int parNo = pinode->getDirEntry("..");
		int childNo = pinode->i_number;
		pinode = this->inodeTablePtr->getInode(parNo);

		DirectoryEntry dirEntryTmp[16];	/*一个扇区最多存16个dirEntry*/
		Buf *bp = NULL;

		/*只考虑b_addr[0] ~ b_addr[5]*/
		for (int i = 0; i < 6; i++)
		{
			if (pinode->i_addr[i] != 0)
			{
				bp = g_bufferManager.bread(pinode->i_addr[i]);
				if (bp == NULL)
				{
					Message::out("<Inode::getDirEntry> bufferManager bread failed");
					ret = "/erro/";
				}
				memcpy((unsigned char *)dirEntryTmp, bp->b_addr, 512);
				for (int j = 0; j < 16 && dirEntryTmp[j].m_name[0] != 0; j++)
				{
					if (dirEntryTmp[j].m_inode == childNo)
					{
						string tmp(dirEntryTmp[j].m_name);
						ret = "/" + tmp + ret;
						break;
					}
				}
				memset(dirEntryTmp, 0, 512);
				g_bufferManager.brelease(bp);
			}
		}
	}
	return ret;
}

void FileSystem::setWorkDir(Inode * newWorkDir)
{
	this->curWorkDirInode = newWorkDir;
	return;
}

Inode *FileSystem::getCurWorkDir()
{
	return this->curWorkDirInode;
}

Inode* FileSystem::diskDirInodeAlloc()
{
	int newNodeNo = this->ibitmapPtr->alloc();
	if (newNodeNo == -1)
	{
		Message::out("<FileSystem::diskDirInodeAlloc> alloc inode failed");
		return NULL;
	}
	Inode *pinode = this->inodeTablePtr->getInode(newNodeNo);
	if (pinode == NULL)
	{
		Message::out("<FileSystem::diskInodeAlloc> get inode failed");
		return NULL;
	}
	pinode->i_flag |= DiskInode::ISDIR;			/*设置文件标志位为目录*/
	pinode->i_gid = pinode->i_uid = 0;
	pinode->i_size = 0;
	pinode->i_nlink = 0;
	for (int i = 0; i < 10; i++)
		pinode->i_addr[i] = 0;
	return pinode;
}

Inode *FileSystem::diskInodeAlloc()
{
	int newNodeNo = this->ibitmapPtr->alloc();
	if (newNodeNo == -1)
	{
		Message::out("<FileSystem::diskInodeAlloc> alloc inode failed");
		return NULL;
	}
	Inode *pinode = this->inodeTablePtr->getInode(newNodeNo);
	if (pinode == NULL)
	{
		Message::out("<FileSystem::diskInodeAlloc> get inode failed");
		return NULL;
	}
	pinode->i_flag &= ~(DiskInode::ISDIR);			/*设置文件标志位为文件而不是目录*/
	pinode->i_gid = pinode->i_uid = 0;
	pinode->i_size = 0;
	pinode->i_nlink = 0;
	for (int i = 0; i < 10; i++)
		pinode->i_addr[i] = 0;
	/*更新外存中的inode,归零，清除之前的inode的信息*/
	this->inodeTablePtr->updateInode(pinode);
	return pinode;
}

void FileSystem::diskInodeFree(Inode *pinode)
{
	int inodeNo = pinode->i_number;
	this->ibitmapPtr->reset(inodeNo);
	this->inodeTablePtr->freeInode(pinode);
}

Buf* FileSystem::diskSectorAlloc()
{
	int newSectorNo = this->dbitmapPtr->alloc();
	if (newSectorNo == -1)
	{
		Message::out("<FileSystem::diskSectorAlloc> alloc sector failed");
		return NULL;
	}
	this->superBlockPtr->s_available_size--;
	this->updateSuperBlock();
	Buf *bp = this->bufferManagerPtr->getBlk(newSectorNo);
	return bp;
}

void FileSystem::diskSectorFree(int blkno)
{
	this->dbitmapPtr->reset(blkno);
	this->superBlockPtr->s_available_size++;
	this->updateSuperBlock();
	return;
}

int FileSystem::writeDir(const char * filename, const int newFileNodeNo)
{
	char filenameTmp[100];
	int ret = 0;
	Inode *retInode = NULL;
	DirectoryEntry dirEntryTmp[16];	/*一个扇区最多存16个dirEntry*/
	Inode *pinode = NULL;
	Buf *bp = NULL;
	int sigCount = 0;

	if (filename == NULL)
	{
		Message::out("<FileSystem::writeDir> invalid filename");
		return -1;
	}

	int ptr = strlen(filename);
	int i = 0;
	/*统计/字符的的个数，若为零，代表父目录就是当前工作目录*/
	while (filename[i])
	{
		if (filename[i] == '/') sigCount++;
		i++;
	}
	/*获得父目录的inode*/
	if (sigCount == 0)
	{
		pinode = this->curWorkDirInode;
		ptr = -1;
	}
	else
	{
		ptr--;							/*指向filename最后一个字符*/
		while (filename[ptr] != '/' && ptr >= 0)
			ptr--;
		strncpy(filenameTmp, filename, ptr + 1);
		filenameTmp[ptr + 1] = '\0';

		/*pinode指向新建文件的父目录*/
		pinode = this->pathSrch(filenameTmp);
	}

	if (pinode == NULL)
	{
		Message::out("<FileSystem::writeDir> pathsrch failed");
		return -1;
	}

	retInode = pinode;
	/*读出将要新建的文件所在的父目录的目录项,并在第一个空位处加入新建的文件*/
	for (int i = 0; i < 6; i++)
	{
		if (pinode->i_addr[i] != 0)
		{
			int j = 0;
			bp = this->bufferManagerPtr->bread(pinode->i_addr[i]);
			if (bp == NULL)
			{
				Message::out("<FileSystem::writeDir> bufferManager bread failed");
				return -1;
			}
			memcpy((unsigned char *)dirEntryTmp, bp->b_addr, 512);
			for (j = 0; j < 16 && dirEntryTmp[j].m_name[0] != 0; j++);
			/*找到的第一个空的目录项*/
			if (dirEntryTmp[j].m_name[0] == 0)
			{
				Buf *bpp = this->bufferManagerPtr->getBlk(pinode->i_addr[i]);
				dirEntryTmp[j].m_inode = newFileNodeNo;
				strcpy(dirEntryTmp[j].m_name, filename + ptr + 1);
				memcpy(bpp->b_addr ,(unsigned char *)dirEntryTmp, 512);
				this->bufferManagerPtr->bwrite(bpp); /*出问题*/
				this->bufferManagerPtr->brelease(bpp);
				this->bufferManagerPtr->brelease(bp);
				break;
			}
			memset(dirEntryTmp, 0, 512);
			this->bufferManagerPtr->brelease(bp);
		}
	}
	/*释放inode*/
	ret = retInode->i_number;
	if(pinode != this->curWorkDirInode) 
		this->inodeTablePtr->freeInode(pinode);
	return ret;
}

void FileSystem::removeDir(const char *filename)
{
	char filenameTmp[100];
	DirectoryEntry dirEntryTmp[16];	/*一个扇区最多存16个dirEntry*/
	Inode *pinode = NULL;
	Buf *bp = NULL;
	int sigCount = 0;

	if (filename == NULL)
	{
		Message::out("<FileSystem::writeDir> invalid filename");
		return ;
	}

	int ptr = strlen(filename);
	int i = 0;
	/*统计/字符的的个数，若为零，代表父目录就是当前工作目录*/
	while (filename[i])
	{
		if (filename[i] == '/') sigCount++;
		i++;
	}
	/*获得父目录的inode*/
	if (sigCount == 0)
	{
		pinode = this->curWorkDirInode;
		ptr = -1;
	}
	else
	{
		ptr--;							/*指向filename最后一个字符*/
		while (filename[ptr] != '/' && ptr >= 0)
			ptr--;
		strncpy(filenameTmp, filename, ptr + 1);
		filenameTmp[ptr + 1] = '\0';

		/*pinode指向文件的父目录*/
		pinode = this->pathSrch(filenameTmp);
	}
	if (pinode == NULL)
	{
		Message::out("<FileSystem::writeDir> pathsrch failed");
		return ;
	}
	for (int i = 0; i < 6; i++)
	{
		if (pinode->i_addr[i] != 0)
		{
			int j = 0;
			bp = this->bufferManagerPtr->bread(pinode->i_addr[i]);
			if (bp == NULL)
			{
				Message::out("<FileSystem::writeDir> bufferManager bread failed");
				return ;
			}
			memcpy((unsigned char *)dirEntryTmp, bp->b_addr, 512);
			for (j = 0; j < 16; j++)
			{
				if (strcmp(dirEntryTmp[j].m_name, filename + ptr + 1) == 0)
					break;
			}
			/*找到文件在父目录中对应的目录项后删除之*/
			if (j != 16)
			{
				dirEntryTmp[j].m_inode = 0;
				memset(dirEntryTmp[j].m_name, 0, 28);
				Buf *bpp = this->bufferManagerPtr->getBlk(pinode->i_addr[i]);
				memcpy(bpp->b_addr, (unsigned char *)dirEntryTmp, 512);
				this->bufferManagerPtr->bwrite(bpp); /*出问题*/
				this->bufferManagerPtr->brelease(bpp);
				this->bufferManagerPtr->brelease(bp);
				break;
			}
			memset(dirEntryTmp, 0, 512);
			this->bufferManagerPtr->brelease(bp);
		}
	}
}

int FileSystem::getParInode(const char *filename)
{
	char filenameTmp[100];
	Inode *retInode = NULL;
	int ret = 0;
	Inode *pinode = NULL;

	int sigCount = 0;

	if (filename == NULL)
	{
		Message::out("<FileSystem::writeDir> invalid filename");
		return -1;
	}

	int ptr = strlen(filename);
	int i = 0;
	/*统计/字符的的个数，若为零，代表父目录就是当前工作目录*/
	while (filename[i])
	{
		if (filename[i] == '/') sigCount++;
		i++;
	}
	/*获得父目录的inode*/
	if (sigCount == 0)
	{
		pinode = this->curWorkDirInode;
		ptr = -1;
	}
	else
	{
		ptr--;							/*指向filename最后一个字符*/
		while (filename[ptr] != '/' && ptr >= 0)
			ptr--;
		strncpy(filenameTmp, filename, ptr + 1);
		filenameTmp[ptr + 1] = '\0';

		/*pinode指向新建文件的父目录*/
		pinode = this->pathSrch(filenameTmp);
	}

	if (pinode == NULL)
	{
		Message::out("<FileSystem::writeDir> pathsrch failed");
		return -1;
	}
	retInode = pinode;
	ret = retInode->i_number;
	this->inodeTablePtr->freeInode(pinode);
	return ret;
}

/*返回打开文件描述符fd*/
int FileSystem::openf(const char *filename, File::FileFlags fileflag)
{
	int retFd = -1;
	Inode *pinode = this->pathSrch(filename);

	/*未找到路径filename对应的文件，即该文件不存在*/
	if (pinode == NULL)
	{
		if (fileflag == File::FREAD)
		{
			Message::out("<open file: >" + string(filename) + " failed, it doesn't exist");
			return -1;
		}
		else if (fileflag == File::FWRITE)	/*以写方式打开文件，若文件不存在要新建*/
		{
			/*在外存新申请一个diskinode,newInode对应该diskinode的内存拷贝*/
			Inode *newInode = this->diskInodeAlloc();
			/*关联File类中的inode成员*/
			retFd = this->openFileTablePtr->fAlloc();	/*获得一个空闲的file文件控制块*/
			this->openFileTablePtr->m_File[retFd].f_flag |= File::FWRITE;
			this->openFileTablePtr->m_File[retFd].f_count++;
			this->openFileTablePtr->m_File[retFd].f_inode = newInode;
			this->openFileTablePtr->m_File[retFd].f_offset = 0;
			int parInode = this->writeDir(filename, newInode->i_number);
			this->openFileTablePtr->m_File[retFd].f_parInode = parInode;
			return retFd;
		}
		else
		{
			Message::out("open file flag erro");
			return -1;
		}
	}
	else
	{
		retFd = this->openFileTablePtr->fAlloc();
		this->openFileTablePtr->m_File[retFd].f_flag |= fileflag;
		this->openFileTablePtr->m_File[retFd].f_count++;
		this->openFileTablePtr->m_File[retFd].f_offset = 0;
		this->openFileTablePtr->m_File[retFd].f_inode = pinode;
		int parInode = this->getParInode(filename);
		this->openFileTablePtr->m_File[retFd].f_parInode = parInode;
		return retFd;
	}
}

void FileSystem::closef(int fd)
{
	this->openFileTablePtr->ffree(fd);	/*释放对应的File和inode*/
}

int FileSystem::readf(int fd, char *buffer, int length)
{
	Buf *bp = NULL;
	/*ptr指向打开文件的inode中的i_addr数组*/
	int *ptr = this->openFileTablePtr->m_File[fd].f_inode->i_addr;
	/*已经读取的长度*/
	int readAready = 0;
	while (length > 0 && (this->openFileTablePtr->m_File[fd].f_offset < this->openFileTablePtr->m_File[fd].f_inode->i_size))
	{
		/*求当前文件读写指针f_offset在文件的逻辑盘块位置*/
		int logAddrOffset = this->openFileTablePtr->m_File[fd].f_offset / 512;
		int logBufOffset = this->openFileTablePtr->m_File[fd].f_offset % 512;
		int toRead = min(this->openFileTablePtr->m_File[fd].f_inode->i_size - readAready, min(512 - logBufOffset, length));

		if (logAddrOffset < 6)
		{
			bp = this->bufferManagerPtr->bread(ptr[logAddrOffset]);
			if (bp == NULL)
			{
				Message::out("<FileSystem::readf> bread erro 1");
				return -1;
			}

			memcpy(buffer + readAready, bp->b_addr + logBufOffset, toRead);
			/*移动文件读写指针*/
			this->openFileTablePtr->m_File[fd].f_offset += toRead;
			/*移动buffer中的读写指针*/
			readAready += toRead;
			length -= toRead;
			this->bufferManagerPtr->brelease(bp);
		}
		else if (logAddrOffset < 262)
		{
			int indexBlk[128];
			int phyAddr = 0;
			bp = this->bufferManagerPtr->bread(ptr[(logAddrOffset - 6) / 128 + 6]);
			memcpy((unsigned char *)indexBlk, bp->b_addr, 512);
			this->bufferManagerPtr->brelease(bp);
			phyAddr = indexBlk[(logAddrOffset - 6) % 128];

			bp = this->bufferManagerPtr->bread(phyAddr);
			if (bp == NULL)
			{
				Message::out("<FileSystem::readf> bread erro 2");
				return -1;
			}

			memcpy(buffer + readAready, bp->b_addr + logBufOffset, toRead);
			/*移动文件读写指针*/
			this->openFileTablePtr->m_File[fd].f_offset += toRead;
			/*移动buffer中的读写指针*/
			readAready += toRead;
			length -= toRead;
			this->bufferManagerPtr->brelease(bp);
		}
		else if (logAddrOffset < 33029)
		{
			int index_1 = 0;
			int index_2 = 0;
			int index_3 = 0;
			int phyAddr = 0;
			int indexBlk[128];
			index_1 = (logAddrOffset - 262) / (128 * 128) + 8;
			index_2 = (logAddrOffset - 262) / 128;
			index_3 = (logAddrOffset - 262) % 128;
			bp = this->bufferManagerPtr->bread(ptr[index_1]);
			memcpy((unsigned char *)indexBlk, bp->b_addr, 512);
			this->bufferManagerPtr->brelease(bp);
			bp = this->bufferManagerPtr->bread(indexBlk[index_2]);
			memcpy((unsigned char *)indexBlk, bp->b_addr, 512);
			this->bufferManagerPtr->brelease(bp);
			phyAddr = indexBlk[index_3];

			bp = this->bufferManagerPtr->bread(phyAddr);
			if (bp == NULL)
			{
				Message::out("<FileSystem::readf> bread erro 2");
				return -1;
			}

			memcpy(buffer + readAready, bp->b_addr + logBufOffset, toRead);
			/*移动文件读写指针*/
			this->openFileTablePtr->m_File[fd].f_offset += toRead;
			/*移动buffer中的读写指针*/
			readAready += toRead;
			length -= toRead;
			this->bufferManagerPtr->brelease(bp);
		}
		else
		{
			Message::out("<FileSystem::readf> file too large");
			return  -1;
		}
	}
	return readAready;
}


int FileSystem::writef(int fd, char *buffer, int length)
{
	Buf *bp = NULL;
	char buftmp[512];
	int lengthTmp = length;

	Inode *pinode = this->openFileTablePtr->m_File[fd].f_inode;
	/*已经写的长度*/
	int writeAready = 0;
	int phyBlkNo = 0;
	int offsetTmp = this->openFileTablePtr->m_File[fd].f_offset;

	while (length > 0)
	{
		/*求当前文件读写指针f_offset在文件的逻辑盘块位置*/
		int logAddrOffset = this->openFileTablePtr->m_File[fd].f_offset / 512;
		int logBufOffset = this->openFileTablePtr->m_File[fd].f_offset % 512;

		if ((phyBlkNo = pinode->bmap(logAddrOffset)) != -1)
		{
			bp = this->bufferManagerPtr->bread(phyBlkNo);
			if (bp != NULL)
			{
				memcpy(buftmp, bp->b_addr, 512);
				this->bufferManagerPtr->brelease(bp);
			}
			else
				break;
			bp = this->bufferManagerPtr->getBlk(phyBlkNo);
			memcpy(bp->b_addr, buftmp, 512);
			memcpy(bp->b_addr + logBufOffset, buffer + writeAready, min(length, 512 - logBufOffset));
			this->bufferManagerPtr->bwrite(bp);
			/*移动文件读写指针*/
			this->openFileTablePtr->m_File[fd].f_offset += min(512 - logBufOffset, length);
			/*移动buffer中的读写指针*/
			writeAready += min(512 - logBufOffset, length);
			length -= min(512 - logBufOffset, length);
			this->bufferManagerPtr->brelease(bp);
		}

	}
	/*第二个参数是新写入内容的大小与原来文件大小的差值加上文件指针值*/
	/*用于更新路径上对应目录的大小变化(目录原大小加上此差值）*/
	this->updateDirInfo(fd, lengthTmp - (this->openFileTablePtr->m_File[fd].f_inode->i_size) + offsetTmp);
	return writeAready;
}

void FileSystem::updateDirInfo(int fd, int sizeChange)
{
	int nodeNo = 0;
	Inode *pinode = NULL;
	/*更新当前文件的大小*/
	pinode = this->openFileTablePtr->m_File[fd].f_inode;
	nodeNo = this->openFileTablePtr->m_File[fd].f_parInode;
	/*更新当前文件的大小*/
	pinode->i_size += sizeChange;
	pinode->i_size = max(pinode->i_size, 0);
	/*更新外存文件大小*/
	this->inodeTablePtr->updateInode(pinode);
	this->inodeTablePtr->freeInode(pinode);
	/*更新一系列父目录的大小*/
	do
	{
		pinode = this->inodeTablePtr->getInode(nodeNo);
		nodeNo = pinode->getDirEntry("..");
		/*更新当前文件的大小*/
		pinode->i_size += sizeChange;
		pinode->i_size = max(pinode->i_size, 0);
		/*更新外存文件大小*/
		this->inodeTablePtr->updateInode(pinode);
		this->inodeTablePtr->freeInode(pinode);
	} while (pinode->i_number != 0);
}

void FileSystem::seekf(int fd, int pos)
{
	/*设置的文件读写指针必须位于0~文件末尾之间*/
	this->openFileTablePtr->m_File[fd].f_offset = min(max(0, pos), this->openFileTablePtr->m_File[fd].f_inode->i_size);
}

int FileSystem::createf(const char *filename)
{
	Inode *pinode = this->pathSrch(filename);
	/*文件已经存在，则先删除*/
	if (pinode != NULL)
	{
		this->iTrunc(pinode);
		this->inodeTablePtr->freeInode(pinode);
	}
	else
	{
		/*在外存新申请一个diskinode,newInode对应该diskinode的内存拷贝*/
		Inode *newInode = this->diskInodeAlloc();
		if (newInode == NULL)
		{
			Message::out("FileSystem::createf> create new file failed");
			return 0;
		}
		/*更新父目录的目录项*/
		this->writeDir(filename, newInode->i_number);
	}
	return 1;
}

int FileSystem::createDir(const char *filename)
{
	Buf *bp = NULL;
	Inode *pinode = this->pathSrch(filename);
	DirectoryEntry *itable = NULL;
	/*若目录已经存在，则创建失败*/
	if (pinode != NULL)
	{
		this->inodeTablePtr->freeInode(pinode);
		return 0;
	}
	else
	{
		/*在外存新申请一个diskinode,newInode对应该diskinode的内存拷贝*/
		Inode *newInode = this->diskDirInodeAlloc();
		if (newInode == NULL)
		{
			Message::out("FileSystem::createDir> create new directory failed");
			return 0;
		}
		/*更新父目录目录项*/
		this->writeDir(filename, newInode->i_number);

		bp = this->diskSectorAlloc();
		itable = (DirectoryEntry *)bp->b_addr;
		int parInodeNo = this->getParInode(filename);

		newInode->i_addr[0] = bp->b_blkno;
		/*更新新建目录的目录项*/
		itable[0].m_inode = newInode->i_number;
		strcpy(itable[0].m_name, ".");
		itable[1].m_inode = parInodeNo;
		strcpy(itable[1].m_name, "..");

		/*将新建的目录的inode更新到外存中*/
		this->inodeTablePtr->updateInode(newInode);
		this->bufferManagerPtr->bwrite(bp);
		this->bufferManagerPtr->brelease(bp);
		return 1;
	}
}

void FileSystem::iTrunc(Inode *pinode)
{
	for (int i = 9; i >= 0; i--)
	{
		/* 如果i_addr[]中第i项存在索引 */
		if (pinode->i_addr[i] != 0)
		{
			/* 如果是i_addr[]中的一次间接、两次间接索引项 */
			if (i >= 6 && i <= 9)
			{
				/* 将间接索引表读入缓存 */
				Buf* pFirstBuf = this->bufferManagerPtr->bread(pinode->i_addr[i]);
				/* 获取缓冲区首址 */
				int* pFirst = (int *)pFirstBuf->b_addr;
				/* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
				for (int j = 128 - 1; j >= 0; j--)
				{
					if (pFirst[j] != 0)	/* 如果该项存在索引 */
					{
						/*
						* 如果是两次间接索引表，i_addr[8]或i_addr[9]项，
						* 那么该字符块记录的是128个一次间接索引表存放的磁盘块号
						*/
						if (i >= 8 && i <= 9)
						{
							Buf* pSecondBuf = this->bufferManagerPtr->bread(pFirst[j]);
							int* pSecond = (int *)pSecondBuf->b_addr;
							for (int k = 128 - 1; k >= 0; k--)
							{
								if (pSecond[k] != 0)
								{
									/* 释放指定的磁盘块 */
									this->diskSectorFree(pSecond[k]);
								}
							}
							this->bufferManagerPtr->brelease(pSecondBuf);
						}
						this->diskSectorFree(pFirst[j]);
					}
				}
				this->bufferManagerPtr->brelease(pFirstBuf);
			}
			this->diskSectorFree(pinode->i_addr[i]);
			pinode->i_addr[i] = 0;
		}
	}
	pinode->i_size = 0;
	this->inodeTablePtr->updateInode(pinode);
}

int FileSystem::deletef(const char * filename)
{
	//Inode *pinode = this->pathSrch(filename);
	int fd = this->openf(filename, File::FREAD);
	if (fd != -1)
	{
		this->updateDirInfo(fd, -1 * this->openFileTablePtr->m_File[fd].f_inode->i_size);
		//cout << "this->openFileTablePtr->m_File[fd].f_inode->ino=" << this->openFileTablePtr->m_File[fd].f_inode->i_number << endl;
		Inode * pinode = this->openFileTablePtr->m_File[fd].f_inode;
		this->iTrunc(pinode);
		this->removeDir(filename);
		this->closef(fd);
		this->diskInodeFree(pinode);
		return 1;
	}
	else
		return 0;
}

int FileSystem::getFileSize(int fd)
{
	return this->openFileTablePtr->m_File[fd].f_inode->i_size;
}