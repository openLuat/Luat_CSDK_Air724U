/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_fs.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/11/27
 *
 * Description:
 * 
 **************************************************************************/

#ifndef __PLATFORM_FS_H__
#define __PLATFORM_FS_H__
#include <devman.h>
#include "am_openat_fs.h"

/*+\new\wj\2020.9.1\完善mount，unmount，format接口*/
typedef enum
{
	E_PLATFROM_FLASH_INTERNAL, // mount 内部的flash区域
    E_PLATFROM_FLASH_EXTERN_PINLCD, // mount 外部的flash区域，使用LCD pin脚复用  V_LCD供电
    E_PLATFROM_FLASH_EXTERN_PINGPIO,// mount 外部的flash区域，使用GPIO pin脚复用 V_PAD_1V8供电
}PLATFORM_FLASH_TYPE;


typedef struct
{
	char *path;    //mount的文件系统根目录 长度>=5
	UINT32 offset; //flash 地址偏移量
	UINT32 size;  //文件系统的大小
	PLATFORM_FLASH_TYPE exFlash;
	UINT8 clkDiv; //外部flash分频范围2-255. clk=166M/clkDiv	
}PLATFORM_FS_MOUNT_PARAM;

/*-\new\wj\2020.9.1\完善mount，unmount，format接口*/
const DM_DEVICE* platform_fs_init(void);

int platformfs_removedir_r( const char *path );

int platformfs_changedir_r( const char *path );

int platformfs_removedir_rec_r( const char *path );


int platformfs_findfirst_r( const char *path, PAMOPENAT_FS_FIND_DATA find_data);


int platformfs_findnext_r(int find_id, PAMOPENAT_FS_FIND_DATA find_data);


int platformfs_findclose_r(int find_id);

int platformfs_makedir_r( const char *path, int mode );

_ssize_t platformfs_get_size_r(const char *path);
/*+\BUG\wangyuan\2020.06.11\将sdcard挂载、卸载、格式化操作放到io库中*/
/*+\new\wj\2020.9.1\完善mount，unmount，format接口*/
BOOL platform_sdcard_fsMount(void);

BOOL platform_sdcard_fsUMount(void);

BOOL platformfs_Format_sdcard(void);
/*-\new\wj\2020.9.1\完善mount，unmount，format接口*/
/*-\BUG\wangyuan\2020.06.11\将sdcard挂载、卸载、格式化操作放到io库中*/
#endif //__PLATFORM_FS_H__

/*+\bug2991\zhuwangbin\2020.06.11\增加lua otp接口*/
BOOL platformfs_otp_erase(UINT16 address, UINT16 size);

BOOL platformfs_otp_write(UINT16 address, char * data, UINT32 size);

BOOL platformfs_otp_read(UINT16 address, char * data, UINT32 size);

BOOL platformfs_otp_lock(UINT16 address, UINT16 size);
/*-\bug2991\zhuwangbin\2020.06.11\增加lua otp接口*/
