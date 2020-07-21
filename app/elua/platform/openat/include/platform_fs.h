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
#ifdef  LUA_SDCARD_SUPPORT
int platform_sdcard_fsMount(void);

int platform_sdcard_fsUMount(void);

int platformfs_Format_sdcard(void);
#endif
/*-\BUG\wangyuan\2020.06.11\将sdcard挂载、卸载、格式化操作放到io库中*/
#endif //__PLATFORM_FS_H__

