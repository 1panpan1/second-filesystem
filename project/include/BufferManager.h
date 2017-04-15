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
	void initialize();						/*��ʼ���������ģ��*/
	Buf *getBlk(int blkno);					/*����һ�����л����*/
	Buf *bread(int blkno);					/*��ȡ���̿���������Ϊbklno�����ݵ�������*/
	void bwrite(Buf *bp);						/*������bp�е�����д��������*/
	void brelease(Buf *bp);					/*�ͷŻ����bp*/
private:
	BlockDevice * blockDevicePtr;				/*ָ��ȫ�ֿ��豸����ģ���ָ��*/
    Buf m_buf[NBUF];							/*�����������*/
    unsigned char buffer[NBUF][BUFFER_SIZE];	/*����������*/
};

#endif