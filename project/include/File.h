#ifndef _FILE_H_
#define _FILE_H_
#include"inode.h"

class File
{
public:
	enum FileFlags
	{
		FREAD = 0x1,							/* 读请求类型 */
		FWRITE = 0x2,						/* 写请求类型 */
	};

public:
	File();
	~File();

	unsigned int f_flag;						/* 对打开文件的读、写操作要求 */
	int		f_count;							/* 当前引用该文件控制块的进程数量 */
	Inode*	f_inode;							/* 指向打开文件的内存Inode指针 */
	int		f_offset;						/* 文件读写位置指针 */
	int		f_parInode;						/*打开文件父目录的diskinode号*/
};


class OpenFileTable
{
public:
	static const int NFILE = 100;				/* 打开文件控制块File结构的数量 */
public:
	OpenFileTable() {};
	~OpenFileTable() {};
	int fAlloc();								/*申请一个打开文件控制块*/
	void ffree(int fd);						/*释放一个打开文件控制块，free时要调用inodetable->freeinode()释放inode*/
public:
	File m_File[NFILE];						/* 系统打开文件表*/
};

class InodeTable
{
public:
	static const int NINODE = 100;				/* 内存Inode的数量 */
public:
	InodeTable() {};
	~InodeTable() {};
	Inode *getInode(int inodeno);				/*返回外存inode区编号为ibnodeno的inode送入内存*/
	void updateInode(Inode *pinode);			/*将内存inode更新到外存*/
	void freeInode(Inode *pinode);				/*释放内存inode节点*/
private:
	Inode *getFreeInode();					/*获得一个新的inode*/
public:
	Inode m_Inode[NINODE];					/* 内存Inode数组，每个打开文件都会占用一个内存Inode */
};

#endif