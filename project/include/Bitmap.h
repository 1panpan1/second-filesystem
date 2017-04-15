#ifndef _BITMAP_H_
#define _BITMAP_H_
#define OFFSET 5
#define BITMAP_ERRO -1

class Bitmap
{
public:
	Bitmap();
	Bitmap(int start , int size);
	~Bitmap() {};
	int get(int pos_index);			/*返回位置pos_index对应的元素的值：0或1*/
	int set(int pos_index);			/*将位置pos_index对应的元素的值置1*/
	int reset(int pos_index);			/*将位置pos_index对应的元素的值置0*/
	int alloc();						/*获得第一个空闲的元素*/
private:
	/*总共大小占据一个扇区*/
	int bsize;						/*位图容量*/
	int start;						/*位图有效区域起始下标*/
	int bmap[512 / sizeof(int) - 2];	/*位图*/
};

#endif