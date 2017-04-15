#include"BlockDevice.h"
#include"message.h"
#include"FileSystem.h"
#include<fstream>
#include<iostream>

extern Bitmap g_dbitmap;

void BlockDevice::initialize()
{
	this->dbitmapPtr = &g_dbitmap;
	d_active = 0;
	d_actf = NULL;
	d_actl = NULL;
}

void BlockDevice::strategy(Buf *bp)
{
	if (bp->b_blkno >= BlockDevice::ALL_SECTOR_SIZE)
	{
		Message::out("<BlockDevice::strategy(Buf*)> blockno out of device size");
		return;
	}
	bp->nextBuf = NULL;
	if (this->d_actf == NULL)
	{
		this->d_actf = bp;
	}
	else
	{
		this->d_actl->nextBuf = bp;
	}
	this->d_actl = bp;
	if (this->d_active == 0)		/* 磁盘空闲 */
	{
		this->devStart();
	}
	return;
}

void BlockDevice::devStart()
{
	Buf *bp;
	if ((bp = this->d_actf) == NULL)
	{
		Message::out("<BlockDevice::devStart> IO queue is empty");
		return;
	}
	this->d_active++;
	this->d_actf = bp->nextBuf;
	if ((bp->b_flags&Buf::B_READ) == Buf::B_READ)
	{
		this->fileRead(bp);
	}
	else
	{
		this->fileWrite(bp);
	}
	this->d_active--;
}

void BlockDevice::fileRead(Buf* bp)
{
	ifstream file;
	file.open(IMAGENAME,ios::in|ios::binary);
	if (!file.is_open())
	{
		Message::out("<BlockDevice::fileRead> open image failed");
		return;
	}
	file.seekg(bp->b_blkno * 512, ios::beg);
	file.read((char*)bp->b_addr, 512);
	file.close();
}

void BlockDevice::fileWrite(Buf *bp)
{
	ofstream file;
	file.open(IMAGENAME,ios::in|ios::out|ios::binary|ios::ate);	/*加上ios::in以去除掉隐含的ios::trunc标识*/
	if (!file.is_open())
	{
		Message::out("<BlockDevice::fileWrite> open image failed");
		return;
	}
	file.seekp(bp->b_blkno * 512, ios::beg);
	file.write((char *)bp->b_addr, 512);
	file.close();
}

