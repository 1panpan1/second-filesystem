#ifndef _FUNC_H_
#define _FUNC_H_

void ls();													/*�г���ǰĿ¼�µ��ļ�����Ŀ¼*/
void cd(char *argv);											/*������Ŀ¼ת��argv*/
void mycmd();													/*��������*/
void vi(const char * filename);								/*�༭�ļ�filename*/
void cat(const char * filename);								/*�鿴�ļ�filename*/
void rm(const char *filename);									/*ɾ���ļ�filename*/
void shutdown();												/*�ر��ļ�ϵͳ*/
void excute(char *cmd, int argc, char argv[][50]);				/*ִ������*/
void mkdir(const char *filename);								/*�½�Ŀ¼*/

/*filenameֻ�������·�������ڹ����ļ�����*/
void load(const char *dst_filename , const char *src_filename);	/*�����������ļ�src_filename��������������ϲ�����Ϊdst_filename��dst_filename�������ļ���������̵�·����*/
void write(const char *dst_filename , const char *src_filename);	/*����������ϵ��ļ�src_filename��src_filename�������ļ�����������ϵ�·����������������*/

void help();													/*��ʾ������Ϣ*/
#endif