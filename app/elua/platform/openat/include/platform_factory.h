

#ifndef _PLATFORM_FACTOTY_H_
#define _PLATFORM_FACTOTY_H_

#include "am_openat.h"

typedef struct
{
    UINT32 magic;
    UINT32 md5key[4];
    UINT32 uid;
}T_LUA_ENC_DATA;

BOOL platform_set_key(T_LUA_ENC_DATA* enc_data);

void platform_decode(UINT32* v, INT32 n);

UINT32 platform_get_uid();

boolean platform_factory_chkcalib(void);

#endif /* _PLATFORM_FACTOTY_H_ */

