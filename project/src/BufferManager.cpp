#include"BufferManager.h"
#include"message.h"
#include"BlockDevice.h"
#include"stdio.h"
#include<cstring>

extern BlockDevice g_blockDevice;

void BufferManager::initialize()
{
	blockDevicePtr = &g_blockDevice;
	for (int i = 0; i<NBUF; i++)
	{
		this->m_buf[i].b_addr = this->buffer[i];
		this->m_buf[i].nextBuf = NULL;
		this->m_buf[i].b_flags = 0;
	}
	return;
}

Buf * BufferManager::getBlk(int blkno)		/* 申请一块缓存，用于读写设备dev上的字符块blkno。*/
{
	int i = 0;
	if( blkno >= BlockDevice::ALL_SECTOR_SIZE)
	{
		printf("blkno is %d\n", blkno);
		Message::out("<BufferManager::getBlk> get block failed for blkno out of device size");
	}
	for (i = 0; i < NBUF; i++)
	{
		if (m_buf[i].b_blkno == blkno)
			break;
	}
	if (i != NBUF)
	{
		m_buf[i].b_flags = Buf::B_BUSY;
		return &m_buf[i];
	}
	else
	{
		for (i = 0; i < NBUF; i++) 
			if ((m_buf[i].b_flags & Buf::B_BUSY) == 0)
			{
				m_buf[i].b_flags = Buf::B_BUSY;
				m_buf[i].b_blkno = blkno;
				return &m_buf[i];
			}
		if (i == NBUF)
		{
			Message::out("<BufferManager::getBlk> no free block available");
			return NULL;
		}
	}
	return NULL;
}

Buf *BufferManager::bread(int blkno)
{
	Buf *bp = getBlk(blkno);
	bp->b_flags |= Buf::B_READ;
	bp->b_wcount = 512;
	this->blockDevicePtr->strategy(bp);
	bp->b_flags &= ~Buf::B_READ;
	return bp;
}

void BufferManager::bwrite(Buf * bp)
{
	bp->b_flags |= Buf::B_WRITE;
	bp->b_wcount = 512;
	this->blockDevicePtr->strategy(bp);
	bp->b_flags &= ~Buf::B_WRITE;
}

void BufferManager::brelease(Buf *bp)
{
	bp->b_flags &= ~(Buf::B_READ | Buf::B_WRITE | Buf::B_BUSY);
	memset(bp->b_addr, 0, BufferManager::BUFFER_SIZE);		/*释放缓存块时将缓冲区清零*/
	return;
}