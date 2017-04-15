#ifndef _INODE_H_
#define _INODE_H_

/*
* ��������ڵ�(DiskINode)�Ķ���
* ���Inodeλ���ļ��洢�豸�ϵ�
* ���Inode���С�ÿ���ļ���Ψһ��Ӧ
* �����Inode���������Ǽ�¼�˸��ļ�
* ��Ӧ�Ŀ�����Ϣ��
* ���Inode������ֶκ��ڴ�Inode���ֶ�
* ���Ӧ�����INode���󳤶�Ϊ64�ֽڣ�
* ÿ�����̿���Դ��512/64 = 8�����Inode
*/
class DiskInode
{
public:
	DiskInode();
	~DiskInode() {};

public:
	enum INodeFlag
	{
		ILOCK = 0x1,							/* �����ڵ����� */
		IUPD = 0x2,							/* �ڴ�inode���޸Ĺ�����Ҫ������Ӧ���inode */
		IACC = 0x4,							/* �ڴ�inode�����ʹ�����Ҫ�޸����һ�η���ʱ�� */
		ISDIR = 0x4000						/* �ļ����ͣ�Ŀ¼�ļ� */
	};
public:
	unsigned int d_flag;						/* ״̬�ı�־λ�������enum INodeFlag */
	int		d_nlink;							/* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */

	short	d_uid;							/* �ļ������ߵ��û���ʶ�� */
	short	d_gid;							/* �ļ������ߵ����ʶ�� */

	int		d_size;							/* �ļ���С���ֽ�Ϊ��λ */
	int		d_addr[10];						/* �����ļ��߼���ú�������ת���Ļ��������� */

	int		d_atime;							/* ������ʱ�� */
	int		d_mtime;							/* ����޸�ʱ�� */
};

class Inode
{
public:
	Inode();
	~Inode() {};
	int bmap(int lbn);						/*��inode���߼��̿��Ϊlbn�������̿��ַ*/
	void copyInode(int diskInodeNo);			/*�������ΪdiskInodeNo��inode�������ڴ�inode��*/
	int getDirEntry(const char *dirEntryName);	/*���ض�Ӧdirentry���inode��ţ�û�ҵ���Ӧdirentry���-1*/
public:
	unsigned int i_flag;						/* ״̬�ı�־λ�������enum INodeFlag */
	int		i_count;							/*���ü���������ֵΪ0�������inode����*/
	int		i_nlink;							/* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */
	int		i_number;						/* ���inode���еı�� */
	short	i_uid;							/* �ļ������ߵ��û���ʶ�� */
	short	i_gid;							/* �ļ������ߵ����ʶ�� */
	int		i_size;							/* �ļ���С���ֽ�Ϊ��λ */
	int		i_addr[10];						/* �����ļ��߼���ú�������ת���Ļ��������� */
};



#endif