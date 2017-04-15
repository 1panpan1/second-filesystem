#ifndef _BUFFERMANAGER_H_
#define _BUFFERMANAGER_H_
#include"Buffer.h"
#include"BlockDevice.h"
class BufferManager
{
public:
    static const int NBUF = 15;
    static const int BUFFER_SIZE = 512;
public:
	BufferManager() {};
	~BufferManager() {};
	void initialize();						/*初始化缓存管理模块*/
	Buf *getBlk(int blkno);					/*返回一个空闲缓存块*/
	Buf *bread(int blkno);					/*读取磁盘块上扇区号为bklno的数据到缓存中*/
	void bwrite(Buf *bp);						/*将缓存bp中的数据写到磁盘上*/
	void brelease(Buf *bp);					/*释放缓存块bp*/
private:
	BlockDevice * blockDevicePtr;				/*指向全局块设备管理模块的指针*/
    Buf m_buf[NBUF];							/*缓存管理数组*/
    unsigned char buffer[NBUF][BUFFER_SIZE];	/*缓冲区数组*/
};

#endif