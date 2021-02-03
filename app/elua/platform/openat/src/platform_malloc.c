
#include "string.h"
#include "assert.h"
/*+\NEW\zhuwangbin\2019.12.10\添加lua 编译不过*/
#include "lplatform.h"
#include "platform_malloc.h"
#include "platform_rtos.h"
/*-\NEW\zhuwangbin\2019.12.10\添加lua 编译不过*/
#include "am_openat.h"

void* platform_malloc1( size_t size, const char* fun, UINT32 line)
{
    return OPENAT_malloc1( size, fun, line);
}

void* platform_calloc( size_t nelem, size_t elem_size )
{
    void *p;

    ASSERT(nelem*elem_size);

    p = (void*)OPENAT_malloc(nelem*elem_size);

    memset(p, 0, nelem*elem_size);

    return p;
}

void platform_free( void* ptr )
{
    OPENAT_free( ptr );
}

void* platform_realloc( void* ptr, size_t size )
{
    return OPENAT_realloc( ptr, size );
}

