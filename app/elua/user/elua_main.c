/***************
	demo_elua
****************/

#include "string.h"

#include "osi_log.h"
#include "osi_api.h"
#include "cs_types.h"
/*******************************************
  definition of some functions of libc
 *******************************************/

int fstat(int fd, struct stat *st)
{
    return vfs_fstat(fd, st);
}

ssize_t read(int fd, void *buf, size_t count)
{
    return vfs_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    return vfs_write(fd, buf, count);
}
ssize_t close(int fd)
{
    return vfs_close(fd);
}
long lseek(int fd, long offset, int whence)
{
    return vfs_lseek(fd, offset, whence);
}

int isatty(int fd)
{
    return 0;
}

//main函数
int appimg_enter(void *param)
{
    extern void luatEluaInit();
    luatEluaInit();
    return 0;
}

//退出提示
void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
