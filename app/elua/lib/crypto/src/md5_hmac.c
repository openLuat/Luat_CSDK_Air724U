/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:   md5_hmac.c
 * Author:  zhutianhua
 * Date:    2018/5/15
 *
 * Description:
 *          md5_hmac.c
 **************************************************************************/
#include "crypto.h"
#include "assert.h"
#include "md5_hmac.h"

md5_hmac_context ctxhmac;

void md5_hmac_starts( md5_context *ctx, const unsigned char *key, size_t keylen )
{
    size_t i;
    unsigned char sum[16];

    if( keylen > 64 )
    {
        md5( key, keylen, sum );
        keylen = 16;
        key = sum;
    }

    memset( ctxhmac.ipad, 0x36, 64 );
    memset( ctxhmac.opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        ctxhmac.ipad[i] = (unsigned char)( ctxhmac.ipad[i] ^ key[i] );
        ctxhmac.opad[i] = (unsigned char)( ctxhmac.opad[i] ^ key[i] );
    }
    md5_starts( ctx );
    md5_update( ctx, ctxhmac.ipad, 64 );

    memset( sum, 0, sizeof( sum ) );

}

/*
 * MD5 HMAC process buffer
 */
void md5_hmac_update( md5_context *ctx, const unsigned char *input, size_t ilen )
{
    md5_update( ctx, input, ilen );
}

/*
 * MD5 HMAC final digest
 */
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] )
{
    unsigned char tmpbuf[16];

    md5_finish( ctx, tmpbuf );
    md5_starts( ctx );
    md5_update( ctx, ctxhmac.opad, 64 );
    md5_update( ctx, tmpbuf, 16 );
    md5_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-MD5( hmac key, input buffer )
 */
void md5_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] )
{
    md5_context ctx;

    md5_hmac_starts( &ctx, key, keylen );
    md5_hmac_update( &ctx, input, ilen );
    md5_hmac_finish( &ctx, output );

    memset( &ctx, 0, sizeof( md5_context ) );
}

