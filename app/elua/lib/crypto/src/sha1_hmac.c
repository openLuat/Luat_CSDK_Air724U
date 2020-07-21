/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:   sha1_hmac.c
 * Author:  zhutianhua
 * Date:    2018/5/15
 *
 * Description:
 *          sha1_hmac.c
 **************************************************************************/
#include "crypto.h"
#include "assert.h"
#include "sha1_hmac.h"

sha1_hmac_context ctxhmac;

/*
 * SHA-1 HMAC context setup
 */
void sha1_hmac_starts( sha1_context *ctx, const unsigned char *key, size_t keylen )
{
    size_t i;
    unsigned char sum[20];

    if( keylen > 64 )
    {
        sha1( key, keylen, sum );
        keylen = 20;
        key = sum;
    }


    memset( ctxhmac.ipad, 0x36, 64 );
    memset( ctxhmac.opad, 0x5C, 64 );
	

    for( i = 0; i < keylen; i++ )
    {
        ctxhmac.ipad[i] = (unsigned char)( ctxhmac.ipad[i] ^ key[i] );
        ctxhmac.opad[i] = (unsigned char)( ctxhmac.opad[i] ^ key[i] );
    }

    sha1_starts( ctx );
    sha1_update( ctx, ctxhmac.ipad, 64 );

    memset( sum, 0, sizeof( sum ) );
}

/*
 * SHA-1 HMAC process buffer
 */
void sha1_hmac_update( sha1_context *ctx, const unsigned char *input, size_t ilen )
{
    sha1_update( ctx, input, ilen );
}

/*
 * SHA-1 HMAC final digest
 */
void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] )
{
    unsigned char tmpbuf[20];

    sha1_finish( ctx, tmpbuf );
    sha1_starts( ctx );
    sha1_update( ctx, ctxhmac.opad, 64 );
    sha1_update( ctx, tmpbuf, 20 );
    sha1_finish( ctx, output );

    memset( tmpbuf, 0, sizeof( tmpbuf ) );
}

/*
 * output = HMAC-SHA-1( hmac key, input buffer )
 */
void sha1_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[20] )
{
    sha1_context ctx;

    sha1_hmac_starts( &ctx, key, keylen );
    sha1_hmac_update( &ctx, input, ilen );
    sha1_hmac_finish( &ctx, output );

    memset( &ctx, 0, sizeof( sha1_context ) );
}

