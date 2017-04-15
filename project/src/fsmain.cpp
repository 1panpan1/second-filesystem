#include"FileSystem.h"
#include"func.h"
#include<iostream>
using namespace std;

/*全局变量*/
FileSystem		g_fileSystem;
BufferManager		g_bufferManager;
BlockDevice		g_blockDevice;
SuperBlock		g_superBlock;
Bitmap			g_ibitmap;
Bitmap			g_dbitmap;
OpenFileTable		g_openFileTable;
InodeTable		g_inodeTable;
bool				g_shutdown;

void globalInitialize()
{
	g_superBlock.initialize();
	g_blockDevice.initialize();
	g_bufferManager.initialize();
	g_fileSystem.initialize();
}


int main()
{
	char buf[] = "abcdefg";
	char bf[100] = { 0 };
	char bg[10] = { 0 };
	globalInitialize();
	cout << "use help cmd to get more help" << endl;
	while (!g_shutdown)
	{
		mycmd();
	}

	return 0;
}