/**************************************************************************
 * TTPCom Software Copyright (c) 1997-2005 TTPCom Ltd
 * Licensed to TTP_CLIENT_NAME
 **************************************************************************
 *   $Id: //central/releases/simcom/branch_release_16_0_4_B_simcom/tplgsm/utinc/ut_md5.h#1 $
 *   $Revision: #1 $
 *   $DateTime: 2006/11/27 07:18:36 $
 **************************************************************************
 * File Description :
 *
 * Header for MD5 algorithm, based on example in RFC1321.  RSA disclaimer:
 *
 *
 * Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 * rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 **************************************************************************/

#if !defined (UT_MD5_H)
#define       UT_MD5_H

/******************************************************************************
 * Include Files
 *****************************************************************************/



/******************************************************************************
 * Types
 *****************************************************************************/

/* MD5 context. */
typedef struct Md5ContextTag
{
    unsigned int state[4];        /* state (ABCD) */
    unsigned int count[2];        /* number of bits, modulo 2^64 (lsb first) */
    unsigned char  buffer[64];      /* input buffer */
}
Md5Context;

/* Length of a digest */
#define MD5_DIGEST_LENGTH 16

#define MD5_BLOCK_LENGTH 64

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

void Md5Init   (Md5Context *context);
void Md5Update (Md5Context *context, unsigned char *input, unsigned int inputLength);
void Md5Final  (unsigned char *digest, Md5Context *context);

void sd_md5(unsigned char *data, unsigned int datalen, unsigned char *result);
#ifdef __cplusplus
}
#endif
#endif

/* END OF FILE */
