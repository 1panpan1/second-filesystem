## 程序名称

FStm二级文件系统

## 运行环境

Windows7/8/8.1/10/Linux

## 可用空间

2000KB

## 说明

在windows和linux测试环境中切换时切记要先删除/project/obj目录中的链接文件后再编译，否则会出错，linux下支持make clean命令。

***

## API

`int	openf(const char *path, File::FileFlags flag)`

打开文件，将路径path对应的文件以flag的方式打开，以便后面的处理。
flag的值有两种：File::READ和File::WRITE，分别以读和写的方式打开文件,打开文件成功后API
返回值为打开的文件的文件描述符。



`void	closef(int fd)`

关闭文件，将文件描述符fd对应的文件关闭，文件在读写操作完成后一定要调用此API关闭。



`void	seekf(int fd , int position)`

移动文件读写指针，为了更加灵活的进行文件读写操作，我们有时需要从文件的某个特定位置开始读写，这时候就需要利用到此API。此API的功能是将文件描述符fd对应的文件的读写指针移动到position位置，position的值位于0~X之间，其中X为文件的大小。



`int 	deletef(const char *path)`

删除文件，利用此API可以删除路径path对应的文件，返回值为0或1,0代表删除失败，1代表删除成功。



`int	readf(int fd , char *buffer , int length)`

读取文件，利用此API可以读取文件描述符fd对应的文件中的内容。使用时先以File::READ方式打开文件，然后使用此API读取长度为length的内容到缓冲区buffer中，若还没有读取到length长度就遇到文件结尾则不再读取。返回值为读取到的数据长度。



`int	write(int fd , char *buffer , int length)`

写文件，利用此API可以向文件描述符fd对应的文件中写入数据。使用时先以File::WRITE方式打开文件然后使用此API向文件中写入buffer中长度为length的数据。写入的起始位置是当前文件读写指针指向的位置，返回值为写入的数据的长大衣。



`int 	createf(const char *path)`

新建文件，新建一个文件，文件路径位path 

***

## 命令

基于上述的API，我为FStm文件系统编写了如下几个命令，利用这些命令可以操作该文件系统

`root@localhost /home]$ help`

用法：	help
功能：	获得命令的使用方法，功能同本文档



`[root@localhost /home]$ ls`

用法：	ls
功能：	罗列出当前目录下的所有文件以及子目录信息



`[root@localhost /home]$ cd subDir`

用法：	cd [路径]
功能：	当前工作路径改为[路径]
说明：	cd .	表示转到当前目录

​		cd ..	表示转到上一层目录

​		cd /xxx	表示该文件系统中的绝对路径

​		cd xxx	表示该文件系统中的相对路径

​		cd /.或cd /..可以直接跳转到根目录



`[root@localhost /home]$ vi new.txt`

用法：	vi [路径]
功能：	编辑一个文件，该文件的路径是[路径]
说明：	如果文件不存在，则新建一个，如果文件已经存在，则文件会先被清空，再进行编辑
【注意】结束输入时在末尾输入'~'字符然后回车



`[root@localhost /home]$ cat new.txt`

用法：	cat [路径]
功能：	查看文件的内容，文件的路径位[路径]



`[root@localhost /home]$ rm new.txt`

用法：	rm [路径]
功能：	删除路径[路径]对应的文件
说明：	删除文件时，系统会让用户确认是否删除，即输入y或n，分别表示确认和不确认
【注意】此命令在linux系统下会由于清空缓冲区无效导致报错invalid command，但是不会影响删除操作



`[root@localhost /home]$ mkdir dirs`

用法：	mkdir [路径]
功能：	新建一个目录，目录的路径位[路径]
说明：	如果该目录已经存在，则不能再新建，否则可以新建



`[root@localhost /home]$ load /dirs/in.jpg   D:\img\test.jpg`

用法：	load 【目的路径】 【源路径】
功能：	将宿主机中路径[源路径]对应的文件拷贝到虚拟磁盘中，在虚拟磁盘中的路径为[目的路径]
说明：	【源路径】必须是宿主机路径，【目的路径】必须是虚拟磁盘中的路径



`[root@localhost /home]$ write D:\img\out.jpg  /dirs/in.jpg`

用法：	write 【目的路】【源路径】
功能：	将虚拟磁盘中路径[源路径]对应的文件拷贝到宿主机中，宿主机中文件的路径是[目的路径]
说明：	【目的路径】必须是宿主机路径，【源路径】必须是虚拟磁盘中的路径



`[root@localhost /home]$ shutdown`

用法：	shutdown
功能：	系统关闭

***

## 备注

在 project/test/目录下，有两个测试文件，test.jpg和test.txt。