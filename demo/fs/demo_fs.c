#include "string.h"
#include <stdio.h>
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_fs.h"
#include "iot_flash.h"
#include "iot_pmd.h"

#define fs_print iot_debug_print
#define DEMO_FS_FILE_PATH "demo_file"
#define DEMO_FS_FILE_PATH_SDCARD "/sdcard0/demo_file"

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
	
	
	/*SDcard目录*/
	//文件不存, 创建成功, 写数据读数据
    if (demo_fs_create(DEMO_FS_FILE_PATH_SDCARD))  
    {
        demo_fs_write(DEMO_FS_FILE_PATH_SDCARD); // 写文件
        demo_fs_read(DEMO_FS_FILE_PATH_SDCARD); // 读文件
    }
    //文件存在直接读,
    else
    {
        demo_fs_read(DEMO_FS_FILE_PATH_SDCARD); // 读文件
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

static void demo_mountExternFlash(void)
{
	/*
		以普冉P25Q64H， 8Mflash为例
	*/
	BOOL ret;
	T_AMOPENAT_USER_FSMOUNT param;
	T_AMOPENAT_FILE_INFO fileInfo;

	/*复用lcd的pin脚，需要打开lcd的ldo*/
	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VLCD, 15);

	iot_os_sleep(100);
	
	/*0-4M创建文件系统 /ext1 */
	param.exFlash = E_AMOPENAT_FLASH_EXTERN_PINLCD;
	param.offset = 0;
	param.size = 0x400000;
	param.path = "/ext1";
	param.clkDiv = 4; 
	ret = iot_fs_mount(&param);
	if (!ret)
	{
		ret = iot_fs_format(&param);

		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}

		
		ret = iot_fs_mount(&param);
	
		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}
	}

	demo_fs_create("/ext1/ext1_file");
	demo_fs_write("/ext1/ext1_file");
	demo_fs_ls(param.path);
	iot_fs_get_fs_info(param.path, &fileInfo);
	fs_print("[fs] %s path %s, ret %d, mem Info (%d,%d)", __FUNCTION__, param.path, ret, fileInfo.totalSize, fileInfo.usedSize);
	

	/*4-8M创建文件系统 /ext2 */
	param.exFlash = E_AMOPENAT_FLASH_EXTERN_PINLCD;
	param.offset = 0x400000;
	param.size = 0x400000;
	param.path = "/ext2";
	param.clkDiv = 4;
	ret = iot_fs_mount(&param);
	if (!ret)
	{
		ret = iot_fs_format(&param);

		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}

		ret = iot_fs_mount(&param);
	
		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}
	}

	demo_fs_create("/ext2/ext2_file");
	demo_fs_write("/ext2/ext2_file");
	demo_fs_ls(param.path);
	iot_fs_get_fs_info(param.path, &fileInfo);
	fs_print("[fs] %s path %s, ret %d, mem Info (%d,%d)", __FUNCTION__, param.path, ret, fileInfo.totalSize, fileInfo.usedSize);
	
}

static void demo_mountAppFlash(void)
{
	/*
		应用空间有剩余时， 可以mount成文件系统管理
	*/
	BOOL ret;
	T_AMOPENAT_USER_FSMOUNT param;
	T_AMOPENAT_FILE_INFO fileInfo;
		
	/*0x260000-0X2A0000创建文件系统 /ext1 */
	param.exFlash = E_AMOPENAT_FLASH_INTERNAL;
	param.offset = 0x260000;
	param.size = 0x40000;
	param.path = "/app1";
	param.clkDiv = 2;
	ret = iot_fs_mount(&param);
	if (!ret)
	{
		ret = iot_fs_format(&param);

		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}

		
		ret = iot_fs_mount(&param);
	
		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}
	}

	demo_fs_create("/app1/app1_file");
	demo_fs_write("/app1/app1_file");
	demo_fs_ls(param.path);
	iot_fs_get_fs_info(param.path, &fileInfo);
	fs_print("[fs] %s path %s, ret %d, mem Info (%d,%d)", __FUNCTION__, param.path, ret, fileInfo.totalSize, fileInfo.usedSize);
	

	/*0x2a0000-0x2E0000创建文件系统 /ext2 */
	param.exFlash = E_AMOPENAT_FLASH_INTERNAL;
	param.offset = 0x2a0000;
	param.size = 0x40000;
	param.path = "/app2";
	param.clkDiv = 2;
	ret = iot_fs_mount(&param);
	if (!ret)
	{
		ret = iot_fs_format(&param);

		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}

		
		ret = iot_fs_mount(&param);
	
		if (!ret)
		{
			fs_print("[fs] %s path %s, ret %d fail", __FUNCTION__, param.path, ret);
			return;
		}
	}

	demo_fs_create("/app2/app2_file");
	demo_fs_write("/app2/app2_file");
	demo_fs_ls(param.path);
	iot_fs_get_fs_info(param.path, &fileInfo);
	fs_print("[fs] %s path %s, ret %d, mem Info (%d,%d)", __FUNCTION__, param.path, ret, fileInfo.totalSize, fileInfo.usedSize);
	
}


int appimg_enter(void *param)
{    
    //开机立刻使用文件系统，会看不到打印信息
    INT32 ret;
	
	iot_os_sleep(1000);
    fs_print("[fs] appimg_enter");	
	
	/*显示预置文件sffs_file.txt*/
	demo_fs_ls("/");
	demo_fs_ls("/sdcard0");

	/*显示预置文件sffs_dir/sub_sffs_file.txt*/
	demo_fs_ls("/sffs_dir");
	
    demo_fs_init();
	
	/*创建测试目录和文件*/
	demo_fs_createDir();

	/*LS根目录*/
	demo_fs_ls("/");
	/*LS SDcard目录*/
	demo_fs_ls("/sdcard0");
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
	
	/*APP 剩余区域mount成文件系统*/
	demo_mountAppFlash();

	/*外挂flash上面mount文件系统*/
	demo_mountExternFlash();
	return 0;
}

void appimg_exit(void)
{
    fs_print("[fs] appimg_exit");
}

