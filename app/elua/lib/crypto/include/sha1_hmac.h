#ifndef _I_SHA1_HMAC_H_
#define _I_SHA1_HMAC_H_

//#include <stdint.h>
#include "type.h"

typedef struct
{
    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */
}
sha1_hmac_context;

/**
 * \brief          SHA-1 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void sha1_hmac_starts( sha1_context *ctx, const unsigned char *key, size_t keylen );

/**
 * \brief          SHA-1 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sha1_hmac_update( sha1_context *ctx, const unsigned char *input, size_t ilen );

/**
 * \brief          SHA-1 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SHA-1 HMAC checksum result
 */
void sha1_hmac_finish( sha1_context *ctx, unsigned char output[20] );

/**
 * \brief          SHA-1 HMAC context reset
 *
 * \param ctx      HMAC context to be reset
 */
void sha1_hmac_reset( sha1_context *ctx );

/**
 * \brief          Output = HMAC-SHA-1( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SHA-1 result
 */
void sha1_hmac( const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char output[20] );

#endif //_AES_H_