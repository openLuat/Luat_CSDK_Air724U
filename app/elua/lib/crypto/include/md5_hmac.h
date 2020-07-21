#ifndef _I_MD5_HMAC_H_
#define _I_MD5_HMAC_H_

//#include <stdint.h>
#include "type.h"

typedef struct
{
    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
md5_hmac_context;

/**
 * \brief          MD5 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void md5_hmac_starts( md5_context *ctx,
                      const unsigned char *key, size_t keylen );

/**
 * \brief          MD5 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void md5_hmac_update( md5_context *ctx,
                      const unsigned char *input, size_t ilen );

/**
 * \brief          MD5 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   MD5 HMAC checksum result
 */
void md5_hmac_finish( md5_context *ctx, unsigned char output[16] );
/**
 * \brief          Output = HMAC-MD5( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-MD5 result
 */
void md5_hmac( const unsigned char *key, size_t keylen,
               const unsigned char *input, size_t ilen,
               unsigned char output[16] );

#endif //_AES_H_