#ifndef _FILE_H_
#define _FILE_H_
#include"inode.h"

class File
{
public:
	enum FileFlags
	{
		FREAD = 0x1,							/* ���������� */
		FWRITE = 0x2,						/* д�������� */
	};

public:
	File();
	~File();

	unsigned int f_flag;						/* �Դ��ļ��Ķ���д����Ҫ�� */
	int		f_count;							/* ��ǰ���ø��ļ����ƿ�Ľ������� */
	Inode*	f_inode;							/* ָ����ļ����ڴ�Inodeָ�� */
	int		f_offset;						/* �ļ���дλ��ָ�� */
	int		f_parInode;						/*���ļ���Ŀ¼��diskinode��*/
};


class OpenFileTable
{
public:
	static const int NFILE = 100;				/* ���ļ����ƿ�File�ṹ������ */
public:
	OpenFileTable() {};
	~OpenFileTable() {};
	int fAlloc();								/*����һ�����ļ����ƿ�*/
	void ffree(int fd);						/*�ͷ�һ�����ļ����ƿ飬freeʱҪ����inodetable->freeinode()�ͷ�inode*/
public:
	File m_File[NFILE];						/* ϵͳ���ļ���*/
};

class InodeTable
{
public:
	static const int NINODE = 100;				/* �ڴ�Inode������ */
public:
	InodeTable() {};
	~InodeTable() {};
	Inode *getInode(int inodeno);				/*�������inode�����Ϊibnodeno��inode�����ڴ�*/
	void updateInode(Inode *pinode);			/*���ڴ�inode���µ����*/
	void freeInode(Inode *pinode);				/*�ͷ��ڴ�inode�ڵ�*/
private:
	Inode *getFreeInode();					/*���һ���µ�inode*/
public:
	Inode m_Inode[NINODE];					/* �ڴ�Inode���飬ÿ�����ļ�����ռ��һ���ڴ�Inode */
};

#endif