/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_sys.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/5/10
 *
 * Description:
 *   实现newlib中需要平台支持的一些system 接口
 **************************************************************************/

#include <string.h>
#include "am_openat.h"

extern WCHAR* strtows(WCHAR* dst, const char* src);

int platform_sys_file_rename(const char *old, const char* new)
{
    return OPENAT_rename_file(old, new);
}


int platform_sys_unlink(const char *path)
{
    int ret;
    #if 0
    int length;
    WCHAR *unicode_path;

    length = strlen(path);

    unicode_path = (WCHAR *)platform_malloc((length+1)*sizeof(WCHAR));
    strtows(unicode_path, path);

    ret = OPENAT_delete_file(unicode_path);

    if(unicode_path)
        OPENAT_free(unicode_path);
    #endif
    ret = OPENAT_delete_file(path);
    return ret;
}

