#ifndef _FUNC_H_
#define _FUNC_H_

void ls();													/*列出当前目录下的文件和子目录*/
void cd(char *argv);											/*将工作目录转到argv*/
void mycmd();													/*解析命令*/
void vi(const char * filename);								/*编辑文件filename*/
void cat(const char * filename);								/*查看文件filename*/
void rm(const char *filename);									/*删除文件filename*/
void shutdown();												/*关闭文件系统*/
void excute(char *cmd, int argc, char argv[][50]);				/*执行命令*/
void mkdir(const char *filename);								/*新建目录*/

/*filename只能是相对路径，且在工程文件夹下*/
void load(const char *dst_filename , const char *src_filename);	/*将宿主机的文件src_filename拷贝到虚拟磁盘上并命名为dst_filename（dst_filename可以是文件在虚拟磁盘的路径）*/
void write(const char *dst_filename , const char *src_filename);	/*将虚拟磁盘上的文件src_filename（src_filename可以是文件在虚拟磁盘上的路径）拷贝到宿主机*/

void help();													/*显示帮助信息*/
#endif