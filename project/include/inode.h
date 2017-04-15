#ifndef _INODE_H_
#define _INODE_H_

/*
* 外存索引节点(DiskINode)的定义
* 外存Inode位于文件存储设备上的
* 外存Inode区中。每个文件有唯一对应
* 的外存Inode，其作用是记录了该文件
* 对应的控制信息。
* 外存Inode中许多字段和内存Inode中字段
* 相对应。外存INode对象长度为64字节，
* 每个磁盘块可以存放512/64 = 8个外存Inode
*/
class DiskInode
{
public:
	DiskInode();
	~DiskInode() {};

public:
	enum INodeFlag
	{
		ILOCK = 0x1,							/* 索引节点上锁 */
		IUPD = 0x2,							/* 内存inode被修改过，需要更新相应外存inode */
		IACC = 0x4,							/* 内存inode被访问过，需要修改最近一次访问时间 */
		ISDIR = 0x4000						/* 文件类型：目录文件 */
	};
public:
	unsigned int d_flag;						/* 状态的标志位，定义见enum INodeFlag */
	int		d_nlink;							/* 文件联结计数，即该文件在目录树中不同路径名的数量 */

	short	d_uid;							/* 文件所有者的用户标识数 */
	short	d_gid;							/* 文件所有者的组标识数 */

	int		d_size;							/* 文件大小，字节为单位 */
	int		d_addr[10];						/* 用于文件逻辑块好和物理块好转换的基本索引表 */

	int		d_atime;							/* 最后访问时间 */
	int		d_mtime;							/* 最后修改时间 */
};

class Inode
{
public:
	Inode();
	~Inode() {};
	int bmap(int lbn);						/*求inode的逻辑盘块号为lbn的物理盘块地址*/
	void copyInode(int diskInodeNo);			/*将外存编号为diskInodeNo的inode拷贝到内存inode中*/
	int getDirEntry(const char *dirEntryName);	/*返回对应direntry项的inode编号，没找到对应direntry项返回-1*/
public:
	unsigned int i_flag;						/* 状态的标志位，定义见enum INodeFlag */
	int		i_count;							/*引用计数，当此值为0，代表此inode空闲*/
	int		i_nlink;							/* 文件联结计数，即该文件在目录树中不同路径名的数量 */
	int		i_number;						/* 外存inode区中的编号 */
	short	i_uid;							/* 文件所有者的用户标识数 */
	short	i_gid;							/* 文件所有者的组标识数 */
	int		i_size;							/* 文件大小，字节为单位 */
	int		i_addr[10];						/* 用于文件逻辑块好和物理块好转换的基本索引表 */
};



#endif