#ifndef _BLOCKDEVICE_H_
#define _BLOCKDEVICE_H_
#include"Buffer.h"
#include"Bitmap.h"

class BlockDevice
{
public:
	BlockDevice(){};
	~BlockDevice(){};
public:
	void initialize();
	void strategy(Buf* bp);

private:
	void devStart();
	void fileRead(Buf * bp);
	void fileWrite(Buf* bp);
public:
	static const int ALL_SECTOR_SIZE = 4085;
	Bitmap* dbitmapPtr;
	int	d_active;			/*controller busy flag*/
	Buf* d_actf;				/*head of I/O queue*/
	Buf* d_actl;				/*tail of I/O queue*/
};

#endif