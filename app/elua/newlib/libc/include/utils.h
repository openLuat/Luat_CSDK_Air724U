#ifndef __UTILS_H__
#define __UTILS_H__

#include "compiler.h"

#ifdef __CS_TYPE_H__
#define	_SIZE_T_DEFINED_
#endif

#ifndef _PTRDIFF_T_DEFINED
typedef int ptrdiff_t;
#define PTRDIFF(p1, p2, type)	((p1) - (p2))
#define _PTRDIFF_T_DEFINED
#endif

#ifndef	_SIZE_T_DEFINED_
#define	_SIZE_T_DEFINED_
typedef	unsigned int    size_t;
#endif

#undef NULL
#if defined(__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#undef offsetof
#define offsetof(TYPE, MEMBER)          ((unsigned int) &((TYPE *)0)->MEMBER)

#ifndef __int64
#define __int64 long long  
#endif

#ifndef Bool
#define Bool unsigned char
#endif


#endif