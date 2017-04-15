#include"FileSystem.h"
#include"message.h"
#include<iostream>
#include<fstream>
#include<cstring>
#include<stdlib.h>
#include"func.h"
#define MAX_COMMAND_LENGTH 500
using namespace std;

extern FileSystem		g_fileSystem;
extern InodeTable		g_inodeTable;
extern BufferManager	g_bufferManager;
extern bool			g_shutdown;

void help()
{
	cout << "welcome to use FS file system" << endl;
	cout << "cmd supported:" << endl;
	cout << "ls			display the files and subdirectories in current directory" << endl;
	cout << "cd	[argv0]		change current dirctory to argv0" << endl;
	cout << "vi	[argv0]		edit the file argv0" << endl;
	cout << "mkdir	[argv0]		create a new directory named argv0" << endl;
	cout << "cat	[argv0]		display the content of file argv0" << endl;
	cout << "rm	[argv0]		delete the file named argv0" << endl;
	cout << "load	[argv0]	[argv1]	dump file from PC to virtual disk, argv0 is the distination path, argv1 is the source path" << endl;
	cout << "write	[argv0]	[argv1] dump file from virtual disk to PC, argv0 is the distination path, argv1 is the source path" << endl;
	cout << "shutdown		the file system will shutdown" << endl;
}	

void ls()
{
	Inode * pinode = g_fileSystem.getCurWorkDir();
	Inode * dirInode = NULL;
	DirectoryEntry dirEntryTmp[16];	/*一个扇区最多存16个dirEntry*/
	Buf *bp = NULL;


	/*只考虑b_addr[0] ~ b_addr[5]*/
	for (int i = 0; i < 6; i++)
	{
		if (pinode->i_addr[i] != 0)
		{
			bp = g_bufferManager.bread(pinode->i_addr[i]);
			if (bp == NULL)
			{
				Message::out("<ls> bufferManager bread failed");
				return;
			}
			memcpy((unsigned char *)dirEntryTmp, bp->b_addr, 512);
			for (int j = 0; j < 16; j++)
			{
				if (dirEntryTmp[j].m_name[0] != 0)
				{
					dirInode = g_inodeTable.getInode(dirEntryTmp[j].m_inode);
					printf("%10dB	   %d	%d	", dirInode->i_size, dirInode->i_gid, dirInode->i_uid);
					if (dirInode->i_flag & DiskInode::ISDIR)
						printf("DIR	");
					else
						printf("FILE	");
					printf("%s\n", dirEntryTmp[j].m_name);
				}
			}
			memset(dirEntryTmp, 0, 512);
			g_bufferManager.brelease(bp);
		}
	}
}

void cd(char *argv)
{
	Inode *pinode = NULL;
	pinode = g_fileSystem.pathSrch(argv);
	if (pinode == NULL)
	{
		Message::out("invalid path: " + string(argv));
		return;
	}
	else
	{
		g_fileSystem.setWorkDir(pinode);
	}
}
/*
void vi(const char * filename)
{
	char *buf = NULL;
	int ptr = 0;
	int size = 1000;
	buf = (char *)malloc(size);
	if (buf == NULL)
	{
		Message::out("vi erro");
		return;
	}
	cout << "press \"shift + `\" to exit inputting" << endl;
	while (true)
	{
		if (ptr == size - 1)
		{
			size += 1000;
			realloc(buf, size);
		}
		buf[ptr] = getche();
		if (buf[ptr] == '~')
			break;
		ptr++;
	}
	if (ptr == size - 1)
		realloc(buf, size + 10);
	buf[ptr] = '\0';
	
	int fd =g_fileSystem.openf(filename, File::FWRITE);
	g_fileSystem.writef(fd, buf, ptr);
	g_fileSystem.closef(fd);
	cout << endl;
}*/


bool isEnd(const char *buf , int size)
{
	if(buf != NULL)
	{
		for (int i = 0; i < size; i++)
			if (buf[i] == '~')
				return true;
	}
	return false;
}

void vi(const char *filename)
{
	int fd = -1;
	int len = 0;
	int ptr = 0;
	int size = 1000;
	char input[1000];
	char* buf = NULL;
	bool endFlag = false;

	buf = (char *)malloc(size);
	if (buf == NULL)
	{
		Message::out("vi erro");
		return;
	}
	cout << "press \"shift + `\" to exit inputting" << endl;
	while (true && !endFlag)
	{
		cin.getline(input, 1000);
		input[strlen(input) + 1] = '\0';
		input[strlen(input)] = '\n';
		len = strlen(input);
		endFlag = isEnd(input, len);
		/*包含结束符，写入文件时不要写入结束符*/
		if (endFlag)
		{
			len -= 2;
		}
		/*缓冲区空间不够，申请新空间*/
		if (ptr >= size - len - 1)
		{
			size += 1000;
			buf = (char *)realloc(buf, size);
		}
		strncpy(buf + ptr, input, len);
		ptr += len;
	}
	fd = g_fileSystem.openf(filename, File::FWRITE);
	g_fileSystem.writef(fd, buf, ptr);
	g_fileSystem.closef(fd);
	free(buf);
}

void mkdir(const char *filename)
{
	if (g_fileSystem.createDir(filename) == 0)
	{
		Message::out("invalid path");
	}
	return;
}

void cat(const char * filename)
{
	char *buf = NULL;
	int fd = g_fileSystem.openf(filename, File::FREAD);
	int size = g_fileSystem.getFileSize(fd);
	buf = new char[size + 1];
	if (buf == NULL)
	{
		Message::out("vi erro");
		g_fileSystem.closef(fd);
		return;
	}
	g_fileSystem.readf(fd, buf, size);
	g_fileSystem.closef(fd);
	buf[size] = '\0';
	cout << buf << endl;
	delete[] buf;
	return;
}

void load(const char *dst_name, const char * src_name)
{
	ifstream file(src_name,ios::in|ios::binary);
	if (!file.is_open())
	{
		cout << "打开文件" << src_name << "失败" << endl;
		return;
	}
	int length = 0;
	file.seekg(0, ios::end);
	length = file.tellg();
	char *buf = new char[length+1];
	file.seekg(0, ios::beg);
	file.read(buf, length);
	file.close();
	buf[length] = '\0';
	cout << "loading..." << endl;
	int fd = g_fileSystem.openf(dst_name, File::FWRITE);
	g_fileSystem.writef(fd, buf, length);
	g_fileSystem.closef(fd);
	delete[] buf;
	cout << "load successfully" << endl;
}

void write(const char *dst_filename, const char *src_filename)
{
	ofstream file(dst_filename, ios::out | ios::binary);
	if (!file.is_open())
	{
		cout << "打开文件" << dst_filename << "失败" << endl;
		return;
	}
	cout << "writing..." << endl;
	int fd = g_fileSystem.openf(src_filename, File::FREAD);
	int length = g_fileSystem.getFileSize(fd);
	char *buf = new char[length + 1];
	g_fileSystem.readf(fd, buf, length);
	g_fileSystem.closef(fd);
	file.write(buf, length);
	file.close();
	delete[] buf;
	cout << "write successfully" << endl;
}

void shutdown()
{
	g_shutdown = true;
}

void rm(const char *filename)
{
	char get;
	cout << "Are you sure to delete file \"" << filename << "\"[y/n]: ";
	cin >> get;
	if(get == 'y' || get == 'Y')
		g_fileSystem.deletef(filename);
	fflush(stdin);
}

void mycmd()
{
	char command[MAX_COMMAND_LENGTH];
	char cmd[20];				/*命令最长20个字符*/
	char argv[10][50];		/*每个命令最多10个参数，每个参数最多50个字符*/
	cout << "[root@localhost " << g_fileSystem.getCurPath() << "]$ ";
	cin.getline(command, MAX_COMMAND_LENGTH);
	int gPtr = 0;
	int cmdPtr = 0;
	int argc = 0;
	/*获得命令*/
	while (command[gPtr] != ' '&& gPtr < strlen(command))
		cmd[cmdPtr++] = command[gPtr++];
	cmd[cmdPtr] = '\0';
	/*获得参数*/
	while (gPtr < strlen(command))
	{
		int arPtr = 0;
		while (command[gPtr] == ' ' && gPtr < strlen(command))
			gPtr++;
		if (gPtr == strlen(command))
			break;
		while (command[gPtr] != ' '&& gPtr < strlen(command))
			argv[argc][arPtr++] = command[gPtr++];
		argv[argc][arPtr] = '\0';
		argc++;
	}
	excute(cmd, argc, argv);
}

void excute(char *cmd , int argc, char argv[][50])
{
	if (strcmp(cmd, "ls") == 0)
	{
		if (argc != 0)
		{
			Message::out("wrong argument");
			return;
		}
		else
			ls();
	}
	else if (strcmp(cmd, "cd") == 0)
	{
		if (argc != 1)
		{
			Message::out("wrong argument");
			return;
		}
		else
			cd(argv[0]);
	}
	else if (strcmp(cmd, "vi") == 0)
	{
		if (argc != 1)
		{
			Message::out("wrong argument");
			return;
		}
		else
			vi(argv[0]);
	}
	else if (strcmp(cmd, "cat") == 0)
	{
		if (argc != 1)
		{
			Message::out("wrong argument");
			return;
		}
		else
			cat(argv[0]);
	}
	else if (strcmp(cmd, "shutdown") == 0)
	{
		if (argc != 0)
		{
			Message::out("wrong argument");
			return;
		}
		else
			shutdown();
	}
	else if (strcmp(cmd, "rm") == 0)
	{
		if (argc != 1)
		{
			Message::out("wrong argument");
			return;
		}
		else
			rm(argv[0]);
	}
	else if (strcmp(cmd, "load") == 0)
	{
		if (argc != 2)
		{
			Message::out("wrong argument");
			return;
		}
		else
			load(argv[0] , argv[1]);
	}
	else if (strcmp(cmd, "write") == 0)
	{
		if (argc != 2)
		{
			Message::out("wrong argument");
			return;
		}
		else
			write(argv[0], argv[1]);
	}
	else if (strcmp(cmd, "mkdir") == 0)
	{
		if (argc != 1)
		{
			Message::out("wrong argument");
			return;
		}
		else
			mkdir(argv[0]);
	}
	else if (strcmp(cmd, "help") == 0)
	{
		if (argc != 0)
		{
			Message::out("wrong argument");
			return;
		}
		else
			help();
	}
	else
	{
		Message::out("invalid command");
		return;
	}
}
