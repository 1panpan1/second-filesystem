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
	int get(int pos_index);			/*����λ��pos_index��Ӧ��Ԫ�ص�ֵ��0��1*/
	int set(int pos_index);			/*��λ��pos_index��Ӧ��Ԫ�ص�ֵ��1*/
	int reset(int pos_index);			/*��λ��pos_index��Ӧ��Ԫ�ص�ֵ��0*/
	int alloc();						/*��õ�һ�����е�Ԫ��*/
private:
	/*�ܹ���Сռ��һ������*/
	int bsize;						/*λͼ����*/
	int start;						/*λͼ��Ч������ʼ�±�*/
	int bmap[512 / sizeof(int) - 2];	/*λͼ*/
};

#endif