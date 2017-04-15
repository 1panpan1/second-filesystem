#include"inode.h"
#include<stdio.h>
#include<cstring>
#include"FileSystem.h"
#include"message.h"
#include"BufferManager.h"
#include<iostream>

using namespace std;
extern BufferManager g_bufferManager;
extern FileSystem		g_fileSystem;
extern OpenFileTable	g_openFileTable;
extern InodeTable		g_inodeTable;

DiskInode::DiskInode()
{
	/*所有成员设置为无效值*/
	this->d_flag = 0;
	this->d_nlink = 0;
	this->d_size = 0;
	this->d_gid = -1;
	this->d_uid = -1;
	this->d_atime = 0;
	this->d_mtime = 0;
	for (int i = 0; i < 10; i++)
	{
		d_addr[i] = 0;
	}
}

Inode::Inode()
{
	this->i_flag = 0;
	this->i_nlink = 0;
	this->i_count = 0;
	this->i_number = -1;

	this->i_uid = -1;
	this->i_gid = -1;

	this->i_size = 0;
	for (int i = 0; i < 10; i++)
		this->i_addr[i] = 0;
}

void Inode::copyInode(int diskInodeNo)
{
	Buf *bp = NULL;
	DiskInode diskInode_tmp;
	bp = g_bufferManager.bread(FileSystem::INODE_ZONE_START_SECTOR + diskInodeNo / FileSystem::INODE_NUMBER_PER_SECTOR);

	memcpy((unsigned char *)&diskInode_tmp, bp->b_addr + (diskInodeNo % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode), sizeof(DiskInode));
	
	g_bufferManager.brelease(bp);
	/*i_number设置不在此处*/
	this->i_flag = diskInode_tmp.d_flag;
	this->i_gid = diskInode_tmp.d_gid;
	this->i_uid = diskInode_tmp.d_uid;
	this->i_nlink = diskInode_tmp.d_nlink;
	this->i_size = diskInode_tmp.d_size;
	for (int i = 0; i < 10; i++)
		this->i_addr[i] = diskInode_tmp.d_addr[i];
}

/*此处只考虑目录文件中存放的目录项不会用完b_addr[0] ~ b_addr[5]*/
int Inode::getDirEntry(const char *dirEntryName)
{
	if (dirEntryName == NULL)
	{
		Message::out("<Inode::getDirName> invalid dirEntryName");
		return -1;
	}
	DirectoryEntry dirEntryTmp[16];	/*一个扇区最多存16个dirEntry*/
	Buf *bp = NULL;

	/*只考虑b_addr[0] ~ b_addr[5]*/
	for (int i = 0; i < 6; i++)
	{
		if (this->i_addr[i] != 0)
		{
			bp = g_bufferManager.bread(this->i_addr[i]);
			if (bp == NULL)
			{
				Message::out("<Inode::getDirEntry> bufferManager bread failed");
				return -1;
			}
			memcpy((unsigned char *)dirEntryTmp, bp->b_addr, 512);
			for (int j = 0; j < 16 && dirEntryTmp[j].m_name[0] != 0; j++)
			{
				if (strcmp(dirEntryTmp[j].m_name, dirEntryName) == 0)
					return dirEntryTmp[j].m_inode;
			}
			memset(dirEntryTmp, 0, 512);
			g_bufferManager.brelease(bp);
		}
	}
	return -1;
}


int Inode::bmap(int lbn)
{
	Buf* pFirstBuf = NULL;
	Buf* pSecondBuf = NULL;
	int phyBlkno =0;	/* 转换后的物理盘块号 */
	int* iTable = NULL;	/* 用于访问索引盘块中一次间接、两次间接索引表 */
	int index = 0;
	if (lbn > 128 * 128 * 2 + 128 * 2 + 6)
	{
		Message::out("<Inode::bmap> file is too large");
	}
	if (lbn < 6)
	{
		phyBlkno = this->i_addr[lbn];

		if (phyBlkno == 0 && (pFirstBuf = g_fileSystem.diskSectorAlloc()) != NULL)
		{
			phyBlkno = pFirstBuf->b_blkno;
			this->i_addr[lbn] = phyBlkno;
			g_bufferManager.brelease(pFirstBuf);
			g_inodeTable.updateInode(this);
			return phyBlkno;
		}
		return phyBlkno;
	}
	else
	{
		if (lbn < 128 * 2 + 6)	//大型文件
			index = (lbn - 6) / 128 + 6;
		else //巨型文件
			index = (lbn - 128 * 2 - 6) / (128 * 128) + 8;
		phyBlkno = this->i_addr[index];
		if (phyBlkno == 0)
		{
			if ((pFirstBuf = g_fileSystem.diskSectorAlloc()) == NULL)
			{
				return 0;
			}
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else
		{
			pFirstBuf = g_bufferManager.bread(phyBlkno);
		}
		iTable = (int *)pFirstBuf->b_addr;
		if (index >= 8)
		{
			index = ((lbn - 128 * 2 - 6) / 128) % 128;
			phyBlkno = iTable[index];
			if (phyBlkno == 0)
			{
				if ((pSecondBuf = g_fileSystem.diskSectorAlloc()) == NULL)
				{
					g_bufferManager.brelease(pFirstBuf);
					return 0;
				}
				iTable[index] = pSecondBuf->b_blkno;
				g_bufferManager.bwrite(pFirstBuf);
			}
			else
			{
				g_bufferManager.brelease(pFirstBuf);
				pSecondBuf = g_bufferManager.bread(phyBlkno);
			}
			pFirstBuf = pSecondBuf;
			iTable = (int *)pSecondBuf->b_addr;
		}
		if (lbn < (128 * 2 + 6))
			index = (lbn - 6) % 128;
		else
			index = (lbn - 128 * 2 - 6) % 128;
		if ((phyBlkno = iTable[index]) == 0 && ((pSecondBuf = g_fileSystem.diskSectorAlloc()) != NULL))
		{
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			g_bufferManager.brelease(pSecondBuf);
			g_bufferManager.bwrite(pFirstBuf);
			g_bufferManager.brelease(pFirstBuf);
		}
		else
		{
			g_bufferManager.brelease(pFirstBuf);
		}
		return phyBlkno;
	}
}