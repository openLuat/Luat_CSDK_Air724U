/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    fcntl.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/11/27
 *
 * Description:
 * 
 **************************************************************************/

#ifndef __FCNTL_H__
#define __FCNTL_H__

#define O_ACCMODE 0x0000003
#define O_RDONLY 0x0000000
#define O_WRONLY 0x0000001
#define O_RDWR   0x0000002
#define O_SHARE  0x0004000

#if 1
#define O_CREAT 0x0000100  
#endif

#if 1 
#define O_EXCL 0x0000200  
#endif

#if 1 
#define O_TRUNC 0x0001000  
#endif

#if 1
#define O_APPEND 0x0002000
#endif

#if 0/*USES_NOR_FLASH*/
#define	_FOPEN		(-1)	/* from sys/file.h, kernel use only */
#define	_FREAD		0x0001	/* read enabled */
#define	_FWRITE		0x0002	/* write enabled */
#define	_FAPPEND	0x0008	/* append (writes guaranteed at the end) */
#define	_FMARK		0x0010	/* internal; mark during gc() */
#define	_FDEFER		0x0020	/* internal; defer for next gc pass */
#define	_FASYNC		0x0040	/* signal pgrp when data ready */
#define	_FSHLOCK	0x0080	/* BSD flock() shared lock present */
#define	_FEXLOCK	0x0100	/* BSD flock() exclusive lock present */
#define	_FCREAT		0x0200	/* open with file create */
#define	_FTRUNC		0x0400	/* open with truncation */
#define	_FEXCL		0x0800	/* error on open if file exists */
#define	_FNBIO		0x1000	/* non blocking I/O (sys5 style) */
#define	_FSYNC		0x2000	/* do all writes synchronously */
#define	_FNONBLOCK	0x4000	/* non blocking I/O (POSIX style) */
#define	_FNDELAY	_FNONBLOCK	/* non blocking I/O (4.2 style) */
#define	_FNOCTTY	0x8000	/* don't assign a ctty on this open */

#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)
#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	_FAPPEND
#define	O_CREAT		_FCREAT
#define	O_TRUNC		_FTRUNC
#define	O_EXCL		_FEXCL
#define O_SYNC		_FSYNC

#endif


#endif //__FCNTL_H__