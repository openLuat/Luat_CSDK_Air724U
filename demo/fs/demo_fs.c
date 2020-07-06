#include "string.h"
#include <stdio.h>
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_fs.h"
#include "iot_flash.h"

#define fs_print iot_debug_print
#define DEMO_FS_FILE_PATH "demo_file"

VOID demo_fs_delete(char* file)
{
    INT32 err;

    err = iot_fs_delete_file(file);

    if (err < 0)
        return;

    fs_print("[fs] delete demo_file");
}

BOOL demo_fs_create(char* file)
{
    INT32 fd;

    fd = iot_fs_open_file(file, FS_O_RDONLY);

    if (fd >= 0) //DEMO_FS_FILE_PATH文件存在
    {
        iot_fs_close_file(fd);
        return FALSE;
    }
    
    // 创建文件DEMO_FS_FILE_PATH
    iot_fs_create_file(file);

    fs_print("[fs] create demo_file");
    iot_fs_close_file(fd);

    return TRUE;
}

VOID demo_fs_read(char* file)
{
    INT32 fd;
    UINT8 read_buff[64] = {0};
    INT32 read_len;
    
    fd = iot_fs_open_file(file, FS_O_RDONLY);
    

    if (fd < 0)
        return;
   
    read_len = iot_fs_read_file(fd, read_buff, sizeof(read_buff));

    if (read_len < 0)
        return;
    
    fs_print("[fs] readlen %d, read_buff %s", read_len, read_buff);

    iot_fs_close_file(fd);
}

VOID demo_fs_write(char* file)
{
    INT32 fd;
    char *write_buff = "hello world";
    INT32 write_len;
    
    fd = iot_fs_open_file(file, FS_O_RDWR);

    if (fd < 0)
        return;
    
    write_len = iot_fs_write_file(fd, (UINT8 *)write_buff, strlen(write_buff));

    if (write_len < 0)
        return;
    
    fs_print("[fs] write_len %d, write_buff %s", write_len, write_buff);

    iot_fs_close_file(fd);
}

VOID demo_fs_init(VOID)
{
    //文件不存, 创建成功, 写数据读数据
    if (demo_fs_create(DEMO_FS_FILE_PATH))  
    {
        demo_fs_write(DEMO_FS_FILE_PATH); // 写文件
        demo_fs_read(DEMO_FS_FILE_PATH); // 读文件
    }
    //文件存在直接读,
    else
    {
        demo_fs_read(DEMO_FS_FILE_PATH); // 读文件
    }
}

static void demo_fs_createDir(void)
{
	int i,j, fd;
	char path[64];
	char dir[16];
	char file[16];
	
	for (i=0; i<4; i++)
	{
		sprintf(dir, "dir_%d", i);
		
		iot_fs_make_dir(dir, 0);
		for (j=0; j<9; j++)
		{
			sprintf(file, "file_%d", j);
			sprintf(path, "%s/%s", dir, file);

			fd = iot_fs_create_file(path);
			iot_fs_write_file(fd, (UINT8 *)path, strlen(path));
			iot_fs_close_file(fd);
		}

		sprintf(dir, "dir_%d/subdir_%d", i, i);
		iot_fs_make_dir(dir, 0);
		for (j=0; j<9; j++)
		{
			sprintf(file, "subfile_%d", j);
			sprintf(path, "%s/%s", dir, file);

			fd = iot_fs_create_file(path);
			iot_fs_write_file(fd, (UINT8 *)path, strlen(path));
			iot_fs_close_file(fd);
		}
		
	}

	
}

static void demo_fs_ls(char *dirName)
{
    AMOPENAT_FS_FIND_DATA findResult;
    INT32 iFd = -1;

    iFd = iot_fs_find_first(dirName, &findResult);


	fs_print("[fs] %s ls:", dirName);
    fs_print("[fs] \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode&E_FS_ATTR_ARCHIVE)? "FILE":"DIR"),    
                                                findResult.st_name,
                                                findResult.st_size,
                                                findResult.mtime);

    while(iot_fs_find_next(iFd, &findResult) == 0)
    {
        fs_print("[fs] \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode&E_FS_ATTR_ARCHIVE)? "FILE":"DIR"),    
                                                findResult.st_name,
                                                findResult.st_size,
                                                findResult.mtime);
    }

    if(iFd >= 0)
    {
        iot_fs_find_close(iFd);
    }
    
}

int appimg_enter(void *param)
{    
    //开机立刻使用文件系统，会看不到打印信息
    INT32 ret;
	
	iot_os_sleep(400);
    fs_print("[fs] appimg_enter");

	/*显示预置文件sffs_file.txt*/
	demo_fs_ls("/");

	/*显示预置文件sffs_dir/sub_sffs_file.txt*/
	demo_fs_ls("/sffs_dir");
	
    demo_fs_init();
	
	/*创建测试目录和文件*/
	demo_fs_createDir();

	/*LS根目录*/
	demo_fs_ls("/");
	/*LS dir_1*/
	demo_fs_ls("dir_1");
	/*DEL dir_1*/
	iot_fs_remove_dir("dir_1");
	/*LS dir_1*/
	demo_fs_ls("dir_1");

	/*LS dir_2*/
	demo_fs_ls("dir_2");

	/*LS dir_2/subdir_2*/
	demo_fs_ls("dir_2/subdir_2");

	/*当前路径设置为dir_2/subdir_2*/
	iot_fs_change_dir("dir_2/subdir_2");

	/*在当前目录下面创建change_subdir*/
	ret = iot_fs_create_file("change_subdir_2");
	iot_fs_write_file(ret, (UINT8 *)"change_subdir_2", strlen("change_subdir_2"));
	iot_fs_close_file(ret);
	/*LS dir_2/subdir_2*/
	demo_fs_ls("./");
	
	return 0;
}

void appimg_exit(void)
{
    fs_print("[fs] appimg_exit");
}

