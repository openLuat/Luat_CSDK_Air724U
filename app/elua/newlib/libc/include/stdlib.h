/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    stdlib.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/9/15
 *
 * Description:
 * 
 **************************************************************************/
#ifdef LUA_USE_NEWLIBC
#ifndef __STDLIB_H__
#define __STDLIB_H__

#include "cdefs.h"
#include "string.h"

__BEGIN_DECLS

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

//lua libc exit
#undef exit
#define exit lualibc_exit
extern void lualibc_exit(int);

extern long strtol(const char *, char **, int);

//system strtoul
extern unsigned long strtoul(const char *, char **, int);

//abs
#ifndef abs
#define abs(x)              ((x<0)?(-(x)):(x))
#endif

/*+\NEW\liweiqiang\2013.10.25\支持环境变量访问 */
#undef getenv
#define getenv lualibc_getenv
char *lualibc_getenv(const char *name);
/*-\NEW\liweiqiang\2013.10.25\支持环境变量访问 */

#undef system
#define system lualibc_system
int lualibc_system(const char *__string);

#undef abort
#define abort lualibc_abort
void lualibc_abort(void);

#undef strtod
#define strtod lualibc_strtod
double lualibc_strtod(const char *, char **);

#define RAND_MAX    0x7fffffff

#undef rand
#define rand lualibc_rand
int lualibc_rand (void);

#undef srand
#define srand lualibc_srand
void lualibc_srand(unsigned int seed);

__END_DECLS

#endif //__STDLIB_H__

#endif
