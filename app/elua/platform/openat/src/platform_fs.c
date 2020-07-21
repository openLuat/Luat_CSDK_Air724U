/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_fs.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/11/27
 *
 * Description:
 * 
 **************************************************************************/

#include "string.h"
#include "am_openat.h"
#include "devman.h"
#include "assert.h"




WCHAR* strtows(WCHAR* dst, const char* src)
{
    while(*src)
    {
        *dst++ = *src++;
    }
    *dst = 0;
    
    return (dst);
}

static int platformfs_open_r( const char *path, int flags, int mode )
{
    int fd;
    fd = IVTBL(open_file)(path, flags, mode);

    return fd;
}


int platformfs_makedir_r( const char *path, int mode )
{
    int fd;
    return IVTBL(make_dir)(path, mode);
}



int platformfs_removedir_r( const char *path )
{
    int fd;
    return IVTBL(remove_dir)(path);
}


int platformfs_removedir_rec_r( const char *path )
{
    int fd;
    return IVTBL(remove_dir_rec)(path);
}



int platformfs_changedir_r( const char *path)
{
    return IVTBL(change_dir)(path);
}

int platformfs_decompress_file_r(const char *Comfilename,const char *Decomdfilename)
{
    return decompress_file(Comfilename,Decomdfilename);
}





static int platformfs_close_r( int fd )
{
    return IVTBL(close_file)(fd);
}

static _ssize_t platformfs_write_r( int fd, const void* ptr, size_t len )
{
    int ret = 0;

    ret = IVTBL(write_file)(fd, (UINT8 *)ptr, len);

    return ret < 0 ? 0 : ret;
}

static _ssize_t platformfs_read_r( int fd, void* ptr, size_t len )
{
    int ret = 0;

    ret = IVTBL(read_file)(fd, ptr, len);

    return ret < 0 ? 0 : ret;
}

static off_t platformfs_lseek_r( int fd, off_t off, int whence )
{
    int ret = 0;

    ret = IVTBL(seek_file)(fd, off, whence);

    return ret < 0 ? -1 : ret;
}

_ssize_t platformfs_get_size_r(const char *path)
{
    return IVTBL(get_file_size(path));
}

static const DM_DEVICE platform_fs_device = 
{
  "/",
  platformfs_open_r,         // open
  platformfs_close_r,        // close
  platformfs_write_r,        // write
  platformfs_read_r,         // read
  platformfs_lseek_r,        // lseek
  NULL,      // opendir
  NULL,      // readdir
  NULL      // closedir
};

const DM_DEVICE* platform_fs_init(void)
{
    return &platform_fs_device;
}

/*+\BUG\wangyuan\2020.06.11\将sdcard挂载、卸载、格式化操作放到io库中*/
#ifdef  LUA_SDCARD_SUPPORT
int platformfs_Mount_sdcard(void)
{
	return IVTBL(fs_mount_sdcard)();
}

int platformfs_uMount_sdcard(void)
{
	return IVTBL(fs_umount_sdcard)();
}

int platformfs_Format_sdcard(void)
{
	return IVTBL(fs_format_sdcard)();
}
#endif
/*-\BUG\wangyuan\2020.06.11\将sdcard挂载、卸载、格式化操作放到io库中*/

