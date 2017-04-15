#include"File.h"
#include"FileSystem.h"
#include"message.h"
#include"BufferManager.h"
#include<iostream>
#include<cstring>

extern BufferManager g_bufferManager;
extern InodeTable		g_inodeTable;

File::File()
{
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	this->f_inode = NULL;
	this->f_parInode = -1;
}

File::~File()
{
	//nothing to do
}

/*返回打开文件描述符fd*/
int OpenFileTable::fAlloc()
{
	for (int i = 0; i < OpenFileTable::NFILE; i++)
	{
		if (this->m_File[i].f_count == 0)
		{
			this->m_File[i].f_count++;
			return i;
		}
	}
	return -1;
}

void OpenFileTable::ffree(int fd)
{
	this->m_File[fd].f_count = 0;
	this->m_File[fd].f_flag = 0;
	g_inodeTable.freeInode(this->m_File[fd].f_inode);	/*同时释放占用的inode*/
	this->m_File[fd].f_inode = NULL;
	this->m_File[fd].f_offset = 0;
	this->m_File[fd].f_parInode = -1;
}

Inode * InodeTable::getFreeInode()
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		if (this->m_Inode[i].i_count == 0)
			return &m_Inode[i];
	}
	return NULL;
}

Inode* InodeTable::getInode(int inodeNo)
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		if (this->m_Inode[i].i_number == inodeNo)
		{
			this->m_Inode[i].i_count++;
			return &m_Inode[i];
		}
	}
	Inode * pinode = NULL;
	pinode = getFreeInode();
	if (pinode == NULL)
	{
		Message::out("<InodeTable::getInode> no free inode available");
		return NULL;
	}

	pinode->i_count++;
	pinode->i_number = inodeNo;
	pinode->copyInode(inodeNo);
	return pinode;
}

void InodeTable::freeInode(Inode *pinode)
{
	if (pinode->i_count == 1)
	{
		pinode->i_count = 0;
		pinode->i_number = -1;
		pinode->i_nlink = 0;
		pinode->i_size = 0;
		pinode->i_flag = 0;
		for (int i = 0; i < 10; i++)
			pinode->i_addr[i] = 0;
	}
	else
	{
		pinode->i_count--;
	}
	return;
}

void InodeTable::updateInode(Inode * pinode)
{
	int dinodeNo = pinode->i_number;
	char buf[512];
	DiskInode dinodeTmp;
	dinodeTmp.d_atime = dinodeTmp.d_mtime = 0;
	dinodeTmp.d_gid = pinode->i_gid;
	dinodeTmp.d_uid = pinode->i_uid;
	dinodeTmp.d_nlink = pinode->i_nlink;
	dinodeTmp.d_size = pinode->i_size;
	dinodeTmp.d_flag = pinode->i_flag;
	for (int i = 0; i < 10; i++)
		dinodeTmp.d_addr[i] = pinode->i_addr[i];
	/*现将外存中的内容读进内存*/
	Buf *bp = g_bufferManager.bread(pinode->i_number / 8 + FileSystem::INODE_ZONE_START_SECTOR);
	memcpy(buf, bp->b_addr, 512);
	g_bufferManager.brelease(bp);

	memcpy(buf + (pinode->i_number % 8) * sizeof(DiskInode), (unsigned char *)&dinodeTmp, sizeof(DiskInode));
	/*修改相应inode的信息后存回外存*/
	bp = g_bufferManager.getBlk(pinode->i_number / 8 + FileSystem::INODE_ZONE_START_SECTOR);
	memcpy(bp->b_addr, buf, 512);
	g_bufferManager.bwrite(bp);
	g_bufferManager.brelease(bp);
	return;
}