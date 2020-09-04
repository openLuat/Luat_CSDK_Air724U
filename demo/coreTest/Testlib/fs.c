/***************
	demo_hello
****************/

#include "iot_debug.h"
#include "iot_os.h"
#include "iot_fs.h"

#define DEMO_FS_FILE_PATH_SDCARD "/sdcard0/demo_file"

VOID demo_fs_write(char *file)
{
    INT32 fd;
    char *write_buff = "hello world";
    INT32 write_len;

    fd = iot_fs_open_file(file, FS_O_RDWR);
    if (fd < 0)
    {
        iot_debug_print("[coreTest-False-fs]: demo_fs_write open false :%d", fd);
        return;
    }

    write_len = iot_fs_write_file(fd, (UINT8 *)write_buff, strlen(write_buff));

    if (write_len < 0)
    {
        iot_debug_print("[coreTest-False-fs]: iot_fs_write_file false");
        return;
    }

    iot_debug_print("[coreTest-fs]:write_len %d, write_buff %s", write_len, write_buff);

    fd = iot_fs_close_file(fd);
    if (fd < 0)
    {
        iot_debug_print("[coreTest-False-fs]: iot_fs_close_file open false :%d", fd);
        return;
    }
}
VOID demo_fs_read(char *file)
{
    INT32 fd;
    UINT8 read_buff[64] = {0};
    INT32 read_len;
    fd = iot_fs_open_file(file, FS_O_RDONLY);

    if (fd < 0)
    {
        iot_debug_print("[coreTest-False-fs]: demo_fs_read open false :%d", fd);
        return;
    }
    read_len = iot_fs_read_file(fd, read_buff, sizeof(read_buff));
    if (read_len < 0)
    {
        iot_debug_print("[coreTest-False-fs]: demo_fs_read false");
        return;
    }
    iot_debug_print("[coreTest-fs]:readlen %d, read_buff %s", read_len, read_buff);
    fd = iot_fs_close_file(fd);
    if (fd < 0)
    {
        iot_debug_print("[coreTest-False-fs]: iot_fs_close_file open false :%d", fd);
        return;
    }
}
static void demo_fs_ls(char *dirName)
{
    AMOPENAT_FS_FIND_DATA findResult;
    INT32 iFd = -1;
    iFd = iot_fs_find_first(dirName, &findResult);
    iot_debug_print("[coreTest-fs]: %s ls:", dirName);
    iot_debug_print("[coreTest-fs]: \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode & E_FS_ATTR_ARCHIVE) ? "FILE" : "DIR"), findResult.st_name, findResult.st_size, findResult.mtime);
    while (iot_fs_find_next(iFd, &findResult) == 0)
        iot_debug_print("[coreTest-fs]: \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode & E_FS_ATTR_ARCHIVE) ? "FILE" : "DIR"), findResult.st_name, findResult.st_size, findResult.mtime);
    if (iFd >= 0)
    {
        iot_fs_find_close(iFd);
    }
}

BOOL demo_fs_create(char *file)
{
    INT32 fd;
    fd = iot_fs_open_file(file, FS_O_RDONLY);
    if (fd >= 0) //DEMO_FS_FILE_PATH文件存在
    {
        iot_debug_print("[coreTest-fs]: DEMO_FS_FILE_PATH File exists");
        fd = iot_fs_close_file(fd);
        if (fd < 0)
        {
            iot_debug_print("[coreTest-False-fs]: iot_fs_close_file open false :%d", fd);
            return FALSE;
        }
        return FALSE;
    }
    // 创建文件DEMO_FS_FILE_PATH
    fd = iot_fs_create_file(file);
    if (fd < 0)
    {
        iot_debug_print("[coreTest-False-fs]: iot_fs_create_file  false :%d", fd);
        return FALSE;
    }
    iot_debug_print("[coreTest-fs]: create demo_file");
    iot_fs_close_file(fd);
    return TRUE;
}

BOOL demo_fs_delete(char *file)
{
    INT32 err = iot_fs_delete_file(file);
    if (err < 0)
    {
        iot_debug_print("[coreTest-False-fs]: iot_fs_delete_file  :%d", err);
        return FALSE;
    }
    iot_debug_print("[coreTest-fs]:delete demo_file");
    return TRUE;
}
bool fsTest(char *file)
{
    T_AMOPENAT_FILE_INFO fileInfo = {0};
    if (iot_fs_get_fs_info(E_AMOPENAT_FS_INTERNAL, &fileInfo) < 0) //获取文件信息
    {
        iot_debug_print("[coreTest-fs]:iot_fs_get_fs_info false");
        return FALSE;
    }
    iot_debug_print("[coreTest-fs]:iot_fs_get_fs_info->totalSize:%d,usedSize:%d", fileInfo.totalSize, fileInfo.usedSize);

    char pCurDirUniLe[20] = {0};
    if (iot_fs_get_current_dir(pCurDirUniLe, sizeof(pCurDirUniLe)) < 0) //获取当前目录信息
    {
        iot_debug_print("[coreTest-fs]:iot_fs_get_current_dir false");
        return FALSE;
    }
    iot_debug_print("[coreTest-fs]:iot_fs_get_current_dir : pCurDirUniLe:%s", pCurDirUniLe);

    demo_fs_ls("/");
    //文件不存, 创建成功, 写数据读数据
    if (demo_fs_create(file))
    {
        demo_fs_write(file); // 写文件
        demo_fs_read(file);  // 读文件
    }
    demo_fs_ls("/");
    demo_fs_delete(file); //删除文件
    demo_fs_ls("/");
    return TRUE;
}
