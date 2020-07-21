
/*
 * Author: 
 *	Guido Draheim <guidod@gmx.de>
 *      Mike Nordell <tamlin-@-algonet-se>
 *
 * Copyright (c) Guido Draheim, use under copyleft (LGPL,MPL)
 */

#include <zzip/lib.h>
#include <zzip/plugin.h>
#include <zlib_pal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef DEBUG
#include <stdio.h>
#endif

#include <zzip/file.h>
#include <zzip/format.h>



zzip_off_t
zzip_filesize(int fd)
{
	int cur;
	int filelen;
	cur = _zzip_lseek(fd,0L,SEEK_CUR);
	_zzip_lseek(fd,0L,SEEK_SET);
	filelen = _zzip_lseek(fd,0L,SEEK_END);
	_zzip_lseek(fd,cur,SEEK_SET);
	return filelen;
}

static const struct zzip_plugin_io default_io = {
	_open_r,
	_close_r,
    _zzip_read,
    _zzip_lseek,
    zzip_filesize,
    1,
	1,
    _zzip_write
};

/** => zzip_init_io
 * This function returns a zzip_plugin_io_t handle to static defaults
 * wrapping the posix io file functions for actual file access. The
 * returned structure is shared by all threads in the system.
 */
zzip_plugin_io_t
zzip_get_default_io(void)
{
    return (zzip_plugin_io_t) & default_io;
}

/**
 * This function initializes the users handler struct to default values 
 * being the posix io functions in default configured environments.
 *
 * Note that the target io_handlers_t structure should be static or 
 * atleast it should be kept during the lifetime of zzip operations.
 */
int
zzip_init_io(zzip_plugin_io_handlers_t io, int flags)
{
    if (! io)
    {
        return ZZIP_ERROR;
    }
    memcpy(io, &default_io, sizeof(default_io));
    io->fd.sys = flags;
    return 0;
}

/* 
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
