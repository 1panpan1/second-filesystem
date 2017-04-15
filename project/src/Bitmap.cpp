#include"Bitmap.h"
#include"FileSystem.h"
#include<cstring>
#include<iostream>
using namespace std;

extern FileSystem g_fileSystem;

Bitmap::Bitmap()
{
	bsize = (512 - sizeof(int) * 2) * 8;
	memset((unsigned char *)bmap, 0, 512 - sizeof(int) * 2);
}

Bitmap::Bitmap(int _start ,int size) :start(_start) , bsize(size)
{
	memset((unsigned char *)bmap, 0, 512 - sizeof(int) *2);
}

int  Bitmap::get(int pos_index)
{
	//pos_index -= 1;
	if ((pos_index >> OFFSET) > bsize)
		return BITMAP_ERRO;
	else
		return (bmap[pos_index >> OFFSET] & ( 0x1 << (pos_index & 0x1F))) == 0 ? 0 : 1;
}

int Bitmap::set(int pos_index)
{
	//pos_index -= 1;
	int ret = 0;
	if ((pos_index >> OFFSET) > bsize)
		ret = BITMAP_ERRO;
	else
		ret = bmap[pos_index >> OFFSET] |= (0X1 << (pos_index & 0X1F));
	g_fileSystem.updateBitmap();
	return ret;
}

int Bitmap::reset(int pos_index)
{
	//pos_index -= 1;
	int ret = 0;
	if ((pos_index >> OFFSET) > bsize)
		ret = BITMAP_ERRO;
	else
		ret = bmap[pos_index >> OFFSET] &= ~(0X1 << (pos_index & 0x1F));
	g_fileSystem.updateBitmap();
	return ret;
}

int Bitmap::alloc()
{
	int ret = 0;
	int i = 0;
	for (i = this->start; i < this->bsize; i++)
	{
		if ((this->get(i)) == 0)
		{
			ret = i;
			this->set(i);
			break;
		}
	}
	if(i == this->bsize)
		ret = BITMAP_ERRO;
	g_fileSystem.updateBitmap();
	return ret;
}

