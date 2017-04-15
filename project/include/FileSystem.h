#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_ 
#include"BufferManager.h"
#include"File.h"
#include<string>
#include"Bitmap.h"

#define IMAGENAME "fs.img"

class DirectoryEntry
{
public:
	static const int DIRSIZ = 28;
public:
	/* Constructors */
	DirectoryEntry();
	/* Destructors */
	~DirectoryEntry() {};
	/* Members */
public:
	int m_inode;		/* 目录项中Inode编号部分 */
	char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};


class SuperBlock
{
	/* Functions */
public:
	/* Constructors */
	SuperBlock(){};
	/* Destructors */
	~SuperBlock(){};
	
	/* Members */
	void initialize();
public:
	int		s_available_size;										/* 可用盘快数 */
	int		s_all_size;											/* 盘块总数 */
															
	int		s_superblk_start;										/*超级块起始扇区号*/
	int		s_imap_start;											/*管理inode区位图的起始扇区号*/
	int		s_dmap_start;											/*管理data区位图的起始扇区号*/
	int		s_inode_start;										/*inode区起始扇区号*/
	int		s_inode_size;											/*inode区所占扇区数目*/
	int		s_data_start;											/*data区起始扇区号*/
	int		s_data_size;											/*data区所占扇区数目*/
	
	int		s_fmod;												/* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
	int		s_ronly;												/* 本文件系统只能读出 */
	int		s_time;												/* 最近一次更新时间 */
	int		padding[244];											/* 填充使SuperBlock块大小等于1024字节，占据2个扇区 */
};

class FileSystem
{
public:
	static const int SUPERBLOCK_START_SECTOR = 1;
	static const int INODE_BITMAP_START_SECTOR = 3;
	static const int DATA_BITMAP_START_SECTOR = 4;
	static const int INODE_ZONE_START_SECTOR = 5;
	static const int INODE_NUMBER_PER_SECTOR = 8;
	static const int INODE_ZONE_SIZE = 16;
	static const int DATA_ZONE_START_SECTOR = 21;
	static const int DATA_ZONE_SIZE = 4064;
	static const int ROOT_DIR_INODE_NUMBER = 0;
public:
	FileSystem() {};
	~FileSystem() {};
	void		initialize();
	void		loadSuperBlock();
	void		updateSuperBlock();
	void		loadBitmap();
	void		updateBitmap();
	void		createImage();											/*生成一个新的虚拟磁盘*/
	void		formatImage();											/*格式化虚拟磁盘*/

	int			writeDir(const char *filename , const int fileNodeNo);	/*将名为filename的文件加入到其父目录目录项中,返回值为父目录的diskinode号*/
	void		removeDir(const char *filename);						/*将名为filename的文件从父目录目录项中移除，用于删除文件*/
	int			getParInode(const char *filename);						/*返回文件filename对应的父目录的inode号*/

	void		iTrunc(Inode *pinode);									/*释放文件pinode占用的磁盘空间*/

	Inode*		diskDirInodeAlloc();									/*在外存diskinode区为新目录分配一个diskinode并返回分配到的diskinode对应的内存inode地址*/
	Inode*		diskInodeAlloc();										/*在外存diskinode区为新文件分配一个diskinode并返回分配的diskinode对应的内存的inode地址*/
	void		diskInodeFree(Inode *pinode);							/*释放外存的inode节点*/
	Buf*		diskSectorAlloc();										/*在外存data区申请一个空闲盘块，并返回对应的缓存块*/
	void		diskSectorFree(int diskN0);								/*释放外存diskNo扇区*/
	int			getFileSize(int fd);									/*获得文件描述符为fd的文件的大小*/

	std::string getCurPath();											/*返回当前工作空间的路径*/
	Inode*		pathSrch(const char *path);								/*返回指向内存Inode节点的指针，没找到文件返回null*/
	void		setWorkDir(Inode *newWorkDir);							/*设置当前动作目录*/
	Inode*		getCurWorkDir();										/*获得当前工作目录所对应的内存inode节点*/
	void		updateDirInfo(int fd ,int sizeChange);					/*更新文件x对应的一系列父目录的大小，用于文件写操作时*/
	int			createDir(const char *filename);						/*新建目录*/

	/*文件操作API*/
	int			openf(const char *filename, File::FileFlags flag);		/*以flag方式打开文件filename*/
	void		closef(int fd);											/*关闭文件描述符为fd的文件*/
	void		seekf(int fd, int position);							/*重定位文件读写指针到位置position*/
	int			deletef(const char *filename);							/*delete时一定要将父目录中对应的目录项清空，归零*/
	int			readf(int fd, char *buffer, int length);				/*读取文件描述符为fd的文件中length字节的数据存到缓冲区buffer中*/
	int			writef(int fd, char *buffer, int length);				/*往文件描述符为fd的文件中写入以buffer为起始地址的length字节的数据*/
	int			createf(const char * filename);							/*新建文件filename，若文件已经存在则先清空，返回0表示失败，返回1表示成功*/

private:
	BufferManager * 	bufferManagerPtr;								/*指向全局高速缓存对象的指针*/
	SuperBlock *		superBlockPtr;									/*指向全局超级块的指针*/	
	Bitmap	*			ibitmapPtr;										/*指向全局inode区位图的指针*/
	Bitmap	*			dbitmapPtr;										/*指向全局data区位图的指针*/
	OpenFileTable*		openFileTablePtr;								/*指向全局打开文件描述表的指针*/
	InodeTable *		inodeTablePtr;									/*指向全局内存inode表的指针*/

	Inode * 			rootDirInode;									/*指向文件系统根目录的指针*/
	Inode * 			curWorkDirInode;								/*指向当前工作路径的指针*/
};

#endif