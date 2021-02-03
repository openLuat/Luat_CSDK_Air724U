


#include "string.h"
#include "am_openat.h"
#include "devman.h"
#include "assert.h"
#include "btea.h"
#include "platform_factory.h"

#define LUA_ENC_MAGIC 0x55447ab9

static UINT32 k[4] = {0};

static BOOL key_setted = FALSE;

BOOL platform_set_key(T_LUA_ENC_DATA* enc_data)
{
    UINT32 uid;
    char* encRawData = (char*)enc_data;

    const UINT32 enc_code[4] = {
    	0x23456600,
    	0x4acc6600,
    	0x8a000000,
    	0x9b123689
    };

    btea((UINT32*)encRawData, -((sizeof(T_LUA_ENC_DATA) - sizeof(UINT32))/4), enc_code);

    if(enc_data->magic != LUA_ENC_MAGIC)
    {
        OPENAT_print("magic number invalid %08x", enc_data->magic);
        return FALSE;
    }

	/*
    uid = pmd_GetEncryptUid();
    uid = uid<<2;
    uid = ~uid;
    uid++;
    if(encData.uid != uid)
    {
        OPENAT_Print("uid invalid %08x", encData.uid);
        return FALSE;
    }
	*/

    memcpy(k, enc_data->md5key, sizeof(enc_data->md5key));

	key_setted = TRUE;

    return TRUE;

}

void platform_decode(UINT32* v, INT32 n)
{
    if(!key_setted)
    {
        OPENAT_sleep(1000);
        OPENAT_assert(FALSE, __FUNCTION__, __LINE__);
    }
    btea(v,n,k);
}

UINT32 platform_get_uid()
{
	UINT32 uid;
    ASSERT(drvEfuseRead(FALSE, 24/* LOW_INDEX */, &uid));
	return uid;
}

boolean platform_factory_chkcalib(void)
{
    return TRUE;
}

