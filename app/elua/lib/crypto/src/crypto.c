/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:   crypto.c
 * Author:  zhutianhua
 * Date:    2018/5/15
 *
 * Description:
 *          lua.crypto库
 **************************************************************************/
#include "crypto.h"
#include "assert.h"
#include "openat_crypto.h"

#define MD5_DIGEST_SIZE 16
#define META_NAME                 "crypto"
#define crypto_check( L )      ( md5_context* )luaL_checkudata( L, 1, META_NAME )

static INT8 I_crypto_common_hb2hex(UINT8 hb)
{
    hb = hb & 0xF;
    return (INT8) (hb < 10 ? '0' + hb : hb - 10 + 'A');
}

static void I_cropto_md5_zeroize(md5_context *ctx) 
{
    unsigned int n = sizeof(md5_context);
    if(ctx==NULL)
        return;
    volatile unsigned char *p = ctx; while( n-- ) *p++ = 0;
}

static int l_crypto_base64_encode(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);
    u8 *outputData = NULL;
    u32 outputLen = 0;
    u32 outputLenMax = (inputLen/3+1)*4;

    luaL_Buffer b;
    luaL_buffinit( L, &b );
    
    if(outputLenMax > LUAL_BUFFERSIZE)
    {
        outputData = malloc(outputLenMax+1);
        memset(outputData,0,outputLenMax+1);
        aliyun_iot_common_base64encode(inputData, inputLen, outputLenMax, outputData, &outputLen);
        luaL_addlstring(&b,outputData,outputLen);
        free(outputData);
        outputData = NULL;
    }
    else
    {
        aliyun_iot_common_base64encode(inputData, inputLen, LUAL_BUFFERSIZE, b.p, &outputLen);
        b.p += outputLen;
    }
    
    luaL_pushresult( &b );
    return 1;
}


static int l_crypto_base64_decode(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);
    u8 *outputData = NULL;
    u32 outputLen = 0;
    u32 outputLenMax = inputLen*3/4+1;

    luaL_Buffer b;
    luaL_buffinit( L, &b );
    
    if(outputLenMax > LUAL_BUFFERSIZE)
    {
        outputData = malloc(outputLenMax+1);
        memset(outputData,0,outputLenMax+1);
        aliyun_iot_common_base64decode(inputData, inputLen, outputLenMax, outputData, &outputLen);
        luaL_addlstring(&b,outputData,outputLen);
        free(outputData);
        outputData = NULL;
    }
    else
    {
        aliyun_iot_common_base64decode(inputData, inputLen, LUAL_BUFFERSIZE, b.p, &outputLen);
        b.p += outputLen;
    }
    
    luaL_pushresult( &b );
    return 1;
}

static int l_crypto_hmac_md5(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    unsigned int inputLen = luaL_checkinteger(L, 2);
    const char *signKey = luaL_checkstring(L,3);
    unsigned int signKeyLen = luaL_checkinteger(L, 4);  
    unsigned char digest[MD5_DIGEST_SIZE];
    int i;
    
    luaL_Buffer b;
    luaL_buffinit( L, &b );    
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    md5_hmac( signKey, signKeyLen,
               inputData, inputLen,
               b.p);    
    memcpy(digest,b.p,MD5_DIGEST_SIZE);
    
    for (i = 0; i < MD5_DIGEST_SIZE; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(digest[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(digest[i]);
    }
    b.p += strlen(b.buffer);
    
    luaL_pushresult( &b );
    return 1;
}
static int l_crypto_flow_md5(lua_State *L)
{
    md5_context* ctx= (md5_context*)lua_newuserdata(L, sizeof(md5_context));
    memset(ctx,0,sizeof(md5_context));
    md5_starts(ctx);
    luaL_getmetatable( L, META_NAME );
    lua_setmetatable( L, -2 );
    return 1;
}

static int l_crypto_md5_update(lua_State *L)
{
    md5_context* ctx = (md5_context*)crypto_check(L);
    size_t inputLen;
    const char *inputData = lua_tolstring(L,2,&inputLen);
    md5_update( ctx,(unsigned char *)inputData, inputLen);

    return 0;
}
static int l_crypto_md5_hexdigest(lua_State *L)
{
    luaL_Buffer b;
    md5_context* ctx = (md5_context*)crypto_check(L);
    luaL_buffinit( L, &b );    
       
    memset(b.buffer,0,LUAL_BUFFERSIZE);

    UINT8 out[MD5_DIGEST_SIZE];
	UINT32 i = 0;
    memset(out, 0, MD5_DIGEST_SIZE);   
    md5_finish( ctx, out);
    I_cropto_md5_zeroize( ctx );
    for (i = 0; i < MD5_DIGEST_SIZE; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
    }
    b.p += strlen(b.buffer);
    luaL_pushresult( &b );
    return 1;

}

static void I_crypto_common_file_md5(const INT8 *filePath, INT8 *result)
{
    UINT32 i = 0;
    md5_context ctx;
    FILE *fp;
    
    memset(&ctx,0,sizeof(md5_context));
    md5_starts( &ctx );
    if((fp = fopen((UINT8 *)filePath, "rb")) != NULL)
    {
        UINT32 bufSize = 4096;
        UINT32 readLen = 0;
        UINT8 *pBuffer = malloc(bufSize);
        if(pBuffer != NULL)
        {
            while(1)
            {
                readLen = fread(pBuffer, 1, bufSize, fp);
                //printf("md5 readLen=%d\n", readLen);
                if(readLen==0)
                {
                    fclose(fp);
                    free(pBuffer);
                    pBuffer = NULL;
                    break;
                }
                else
                {
                    md5_update( &ctx, pBuffer, readLen );
                }
            }               
        }
        else
        {
            printf("md5 malloc error\n");
        }
    }
    else
    {
        printf("md5 open file error: %s\n", filePath);
    }       
    md5_finish( &ctx, result );
    I_cropto_md5_zeroize( &ctx );
}

static int l_crypto_md5(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    unsigned char out[MD5_DIGEST_SIZE+1];
    int i,result;
    switch(lua_type(L, 2))
    {
        case LUA_TNUMBER:
        {
            int inputLen = luaL_checkinteger(L, 2);
            luaL_Buffer b;
            luaL_buffinit( L, &b );    
        
            memset(b.buffer,0,LUAL_BUFFERSIZE);
            md5(inputData, inputLen,out);
            for (i = 0; i < MD5_DIGEST_SIZE; ++i)
            {
                b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
                b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
            }
            b.p += strlen(b.buffer);
            
            luaL_pushresult( &b );
        }
        break;

        case LUA_TSTRING:
        {
            const char *pData;
            u32 sLen;
            
            pData = luaL_checklstring(L, 2, &sLen);
            if (strcmp(pData,"file")==0)
            {
                luaL_Buffer b;
                luaL_buffinit( L, &b );    
            
                memset(b.buffer,0,LUAL_BUFFERSIZE);
                I_crypto_common_file_md5(inputData, out);
                for (i = 0; i < MD5_DIGEST_SIZE; ++i)
                {
                    b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
                    b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
                }
                b.p += strlen(b.buffer);
                
                luaL_pushresult( &b );
            }
            else
            {
                luaL_error(L, "crypto.md5: the senond parameter must be \"file\"");
            }
        
        }
        break;    
    default:
        return luaL_error(L, "crypto.md5: data must be number,string");
        break;
    }
    return 1;
}
static int l_crypto_hmac_sha1(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);
    const char *signKey = luaL_checkstring(L,3);
    int signKeyLen = luaL_checkinteger(L, 4);    
    unsigned char out[20];
    int i;

    luaL_Buffer b;
    luaL_buffinit( L, &b );    

    memset(b.buffer,0,LUAL_BUFFERSIZE);
    sha1_hmac(signKey, signKeyLen,inputData, inputLen,out);
    for (i = 0; i < 20; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
    }
    b.p += strlen(b.buffer);
    
    luaL_pushresult( &b );
	
	return 1;
}

static int l_crypto_hmac_sha2(lua_State *L)
{
#if !defined(LUAT_TTSFLOAT_SUPPORT)
    int inputLen,signKeyLen;
    const char *inputData = lua_tolstring( L, 1, &inputLen );
    const char *signKey = lua_tolstring(L, 2, &signKeyLen);
    unsigned char out[32];
    int i;

    luaL_Buffer b;
    luaL_buffinit( L, &b );    

    memset(b.buffer,0,LUAL_BUFFERSIZE);
    sha2_hmac(signKey, signKeyLen,inputData, inputLen,out,0);
    for (i = 0; i < 32; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
    }
    b.p += strlen(b.buffer);
    
    luaL_pushresult( &b );
#endif
	return 1;
}

static int l_crypto_sha1(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);
    int i;
    unsigned char out[20];

    luaL_Buffer b;
    luaL_buffinit( L, &b );    

    memset(b.buffer,0,LUAL_BUFFERSIZE);
    sha1(inputData, inputLen, out);
    for (i = 0; i < 20; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
    }
 
    b.p += strlen(b.buffer);
    
    luaL_pushresult( &b );

    return 1;
}

static int l_crypto_sha256(lua_State *L)
{
#if !defined(LUAT_TTSFLOAT_SUPPORT)
    uint inputLen;
    const u8 *inputData = lua_tolstring(L,1,&inputLen);
    luaL_Buffer b;
    luaL_buffinit( L, &b );
    unsigned char out[32];
    int i;
 
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    sha2(inputData, inputLen, out ,0);
    for (i = 0; i < 32; ++i)
    {
        b.p[i * 2] = I_crypto_common_hb2hex(out[i] >> 4);
        b.p[i * 2 + 1] = I_crypto_common_hb2hex(out[i]);
    }
   /*+\BUG\WANGJIAN\2019.4.10\修改crypto.sha256 返回32个字节*/ 
    b.p += strlen(b.buffer);
   /*-\BUG\WANGJIAN\2019.4.10\修改crypto.sha256 返回32个字节*/ 
    luaL_pushresult( &b );
#endif
    return 1;
}

static int l_crypto_crc16(lua_State *L)
{   
    int inputLen;
    const char  *inputmethod = luaL_checkstring(L, 1);
    const u8 *inputData = lua_tolstring(L,2,&inputLen);
    u16 poly = luaL_optnumber(L,3,0x0000);
    u16 initial = luaL_optnumber(L,4,0x0000);
    u16 finally = luaL_optnumber(L,5,0x0000);
    u8 inReverse = luaL_optnumber(L,6,0);
    u8 outReverse = luaL_optnumber(L,7,0);
   
    lua_pushinteger(L, calcCRC16(inputData, inputmethod,inputLen,poly,initial,finally,inReverse,outReverse));
    return 1;
}

static int l_crypto_crc16_modbus(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);

    lua_pushinteger(L, calcCRC16_modbus(inputData, inputLen));
    return 1;
}

static int l_crypto_crc32(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);

    lua_pushinteger(L, calcCRC32(inputData, inputLen));
    return 1;
}

static int l_crypto_crc8(lua_State *L)
{
    const char *inputData = luaL_checkstring(L,1);
    int inputLen = luaL_checkinteger(L, 2);

    lua_pushinteger(L, calcCRC8(inputData, inputLen));
    return 1;
}

#define AES_BLOCK_LEN 16
static void DeletePaddingBuf(luaL_Buffer *B, u8 *pPadding, size_t nBufLen, u8 *pBuf)
{
    u8 nPadLen;
    if((strcmp(pPadding, "PKCS5")==0) || (strcmp(pPadding, "PKCS7")==0))
    {
        nPadLen = *(pBuf+nBufLen-1);
        //printf("aes DeletePaddingBuf length=%d\n", nPadLen);
        if((AES_BLOCK_LEN-nPadLen) >= 0)
        {
            luaL_addlstring(B, pBuf, nBufLen-nPadLen);
        }
    }
    else if(strcmp(pPadding, "ZERO")==0)
    {                        
        u8 *pEnd = pBuf+nBufLen-1;
        nPadLen = 0;
        while(1)
        {
            if(*pEnd == 0)
            {
                nPadLen++;
                if(nPadLen == AES_BLOCK_LEN)
                {
                    break;
                }
                pEnd--;
            }
            else
            {
                break;
            }
        }
        //printf("aes DeletePaddingBuf length=%d\n", nPadLen);
        if((AES_BLOCK_LEN-nPadLen) >= 0)
        {
            luaL_addlstring(B, pBuf, nBufLen-nPadLen);
        }
    }
    else
    {
        luaL_addlstring(B, pBuf, nBufLen);
    }
}

static int l_crypto_aes128_ecb_encrypt(lua_State *L)
{
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 1, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 3, &nPswdLen);

    int nPadLen = AES_BLOCK_LEN-(nBufLen%AES_BLOCK_LEN);
    u8 pPadBuf[AES_BLOCK_LEN];
    u8 *pInBuf = NULL;

    //检查参数合法性
    if(nPswdLen!=16)
    {
        return luaL_error(L, "invalid password length=%d, only support AES128", nPswdLen);
    }
   
    //构造填充数据
    memset(pPadBuf, 0, sizeof(pPadBuf));
    
    //加密
    {       
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );

         //原始数据和填充数据拼接在一起
        pInBuf = malloc(nBufLen+nPadLen);
        if(pInBuf == NULL)
        {
            printf("aes_encrypt malloc error!!!\n");
            luaL_pushresult( &b );
            return 1;
        }
        memcpy(pInBuf, pBuf, nBufLen);
        memcpy(pInBuf+nBufLen, pPadBuf, nPadLen); 
        nBufLen += nPadLen;
        nRmnLen = nBufLen;
       
        //开始分组加密，每16字节一组
        while(nRmnLen>0)
        {
            AES_Encrypt("ECB", nRmnLen, pInBuf+nBufLen-nRmnLen, nPswdLen, pPassword, "");
            luaL_addlstring(&b, pInBuf+nBufLen-nRmnLen, AES_BLOCK_LEN);
            nRmnLen -= AES_BLOCK_LEN;
        }

        free(pInBuf);
        pInBuf = NULL;

        luaL_pushresult( &b );
        return 1;
    }    
}


static int l_crypto_aes128_ecb_decrypt(lua_State *L)
{    
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 1, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 3, &nPswdLen);

    //检查参数合法性
    if(nPswdLen!=16)
    {
        return luaL_error(L, "invalid password length=%d, only support AES128", nPswdLen);
    }
    
    
    //解密
    {       
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );

        nRmnLen = nBufLen;

        //开始分组解密，每16字节一组
        while(nRmnLen>0)
        {
            AES_Decrypt("ECB", nRmnLen, pBuf+nBufLen-nRmnLen, nPswdLen, pPassword, "");

            //删除填充数据
            if(nRmnLen==AES_BLOCK_LEN)
            {
                DeletePaddingBuf(&b, "ZERO", AES_BLOCK_LEN, pBuf+nBufLen-nRmnLen);
            }
            else
            {
                luaL_addlstring(&b, pBuf+nBufLen-nRmnLen, AES_BLOCK_LEN);
            }
            nRmnLen -= AES_BLOCK_LEN;
        }
        luaL_pushresult( &b );
        return 1;
    }    
}

static int l_crypto_aes_encrypt(lua_State *L)
{    
    u8 *pMode = luaL_checkstring(L, 1);
    u8 *pPadding = luaL_checkstring(L, 2);
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 3, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 4, &nPswdLen);
    size_t nIVLen = 0;
    u8 *pIV =  lua_tolstring(L, 5, &nIVLen);

    int nPadLen = AES_BLOCK_LEN-(nBufLen%AES_BLOCK_LEN);
    u8 pPadBuf[AES_BLOCK_LEN];
    u8 *pInBuf = NULL;

    //检查参数合法性
    if((nPswdLen!=16) && (nPswdLen!=24) && (nPswdLen!=32))
    {
        return luaL_error(L, "invalid password length=%d, only support AES128,AES192,AES256", nPswdLen);
    }
    if((strcmp(pMode, "ECB")!=0) && (strcmp(pMode, "CBC")!=0) && (strcmp(pMode, "CTR")!=0))
    {
        return luaL_error(L, "invalid mode=%s, only support ECB,CBC,CTR", pMode);
    }
    if((strcmp(pPadding, "NONE")!=0) && (strcmp(pPadding, "PKCS5")!=0) && (strcmp(pPadding, "PKCS7")!=0) && (strcmp(pPadding, "ZERO")!=0))
    {
        return luaL_error(L, "invalid padding=%s, only support NONE,PKCS5,PKCS7,ZERO", pPadding);
    }
    if(((strcmp(pMode, "CBC")==0) || (strcmp(pMode, "CTR")==0)) && (nIVLen!=16))
    {
        return luaL_error(L, "invalid iv length=%d, only support 16", nIVLen);
    }
    
    //构造填充数据
    if((strcmp(pPadding, "PKCS5")==0) || (strcmp(pPadding, "PKCS7")==0))
    {
        memset(pPadBuf, nPadLen, sizeof(pPadBuf));
    }
    else if(strcmp(pPadding, "ZERO")==0)
    {
        memset(pPadBuf, 0, sizeof(pPadBuf));
    }   
    
    //加密
    {       
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );

         //原始数据和填充数据拼接在一起
        if (strcmp(pPadding, "NONE")!=0)
        {
            pInBuf = malloc(nBufLen+nPadLen);
            if(pInBuf == NULL)
            {
                printf("aes_encrypt malloc error!!!\n");
                luaL_pushresult( &b );
                return 1;
            }
            memcpy(pInBuf, pBuf, nBufLen);
            memcpy(pInBuf+nBufLen, pPadBuf, nPadLen); 
            nBufLen += nPadLen;
            nRmnLen = nBufLen;
        }
        else
        {
            pInBuf =  malloc(nBufLen);
            if(pInBuf == NULL)
            {
                printf("aes_encrypt malloc error!!!\n");
                luaL_pushresult( &b );
                return 1;
            }
            memcpy(pInBuf, pBuf, nBufLen);
        }

        if(strcmp(pMode, "ECB") == 0)
        {
            //开始分组加密，每16字节一组
            while(nRmnLen>0)
            {
                AES_Encrypt(pMode, nRmnLen, pInBuf+nBufLen-nRmnLen, nPswdLen, pPassword, pIV);
                luaL_addlstring(&b, pInBuf+nBufLen-nRmnLen, AES_BLOCK_LEN);
                nRmnLen -= AES_BLOCK_LEN;
            }
        }
        else if((strcmp(pMode, "CBC") == 0) || (strcmp(pMode, "CTR") == 0))
        {
            //待加密数据一次性传入
            AES_Encrypt(pMode, nBufLen, pInBuf, nPswdLen, pPassword, pIV);
            luaL_addlstring(&b, pInBuf, nBufLen);
        }

        if(pInBuf != NULL)
        {
            free(pInBuf);
            pInBuf = NULL;
        }

        luaL_pushresult( &b );
        return 1;
    }    
}


static int l_crypto_aes_decrypt(lua_State *L)
{    
    u8 *pMode = luaL_checkstring(L, 1);
    u8 *pPadding = luaL_checkstring(L, 2);
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 3, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 4, &nPswdLen);
    size_t nIVLen = 0;
    u8 *pIV =  lua_tolstring(L, 5, &nIVLen);

    //检查参数合法性
    if((nPswdLen!=16) && (nPswdLen!=24) && (nPswdLen!=32))
    {
        return luaL_error(L, "invalid password length=%d, only support AES128,AES192,AES256", nPswdLen);
    }
    if((strcmp(pMode, "ECB")!=0) && (strcmp(pMode, "CBC")!=0) && (strcmp(pMode, "CTR")!=0))
    {
        return luaL_error(L, "invalid mode=%s, only support ECB,CBC,CTR", pMode);
    }
    if((strcmp(pPadding, "NONE")!=0) && (strcmp(pPadding, "PKCS5")!=0) && (strcmp(pPadding, "PKCS7")!=0) && (strcmp(pPadding, "ZERO")!=0))
    {
        return luaL_error(L, "invalid padding=%s, only support NONE,PKCS5,PKCS7,ZERO", pPadding);
    }
    if(((strcmp(pMode, "CBC")==0) || (strcmp(pMode, "CTR")==0)) && (nIVLen!=16))
    {
        return luaL_error(L, "invalid iv length=%d, only support 16", nIVLen);
    }    
    
    
    //解密
    {       
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );

        nRmnLen = nBufLen;

        if(strcmp(pMode, "ECB") == 0)
        {
            //开始分组解密，每16字节一组
            while(nRmnLen>0)
            {
                AES_Decrypt(pMode, nRmnLen, pBuf+nBufLen-nRmnLen, nPswdLen, pPassword, pIV);

                //删除填充数据
                if(nRmnLen==AES_BLOCK_LEN)
                {
                    DeletePaddingBuf(&b, pPadding, AES_BLOCK_LEN, pBuf+nBufLen-nRmnLen);
                }
                else
                {
                    luaL_addlstring(&b, pBuf+nBufLen-nRmnLen, AES_BLOCK_LEN);
                }
                nRmnLen -= AES_BLOCK_LEN;
            }
        }
        else if((strcmp(pMode, "CBC") == 0) || (strcmp(pMode, "CTR") == 0))
        {
            //待解密数据一次性传入
            AES_Decrypt(pMode, nBufLen, pBuf, nPswdLen, pPassword, pIV);
            DeletePaddingBuf(&b, pPadding, nBufLen, pBuf);
        }

        luaL_pushresult( &b );
        return 1;
    }    
}

#if 0
static int l_crypto_aes_encrypt(lua_State *L)
{    
    u8 *pMode = luaL_checkstring(L, 1);
    u8 *pPadding = luaL_checkstring(L, 2);
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 3, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 4, &nPswdLen);
    size_t nIVLen = 0;
    u8 *pIV =  lua_tolstring(L, 5, &nIVLen);
    int nPadLen = AES_BLOCK_LEN-(nBufLen%AES_BLOCK_LEN);
    u8 pPadBuf[AES_BLOCK_LEN];
    u8 *pInBuf = NULL;
    unsigned char *pOutBuff = NULL;
    unsigned char nPassWord = NULL;

    //检查参数合法性
    if((nPswdLen!=16) && (nPswdLen!=24) && (nPswdLen!=32))
    {
        return luaL_error(L, "invalid password length=%d, only support AES128,AES192,AES256", nPswdLen);
    }
    if((strcmp(pMode, "ECB")!=0) && (strcmp(pMode, "CBC")!=0) && (strcmp(pMode, "CTR")!=0))
    {
        return luaL_error(L, "invalid mode=%s, only support ECB,CBC,CTR", pMode);
    }
    if((strcmp(pPadding, "NONE")!=0) && (strcmp(pPadding, "PKCS5")!=0) && (strcmp(pPadding, "PKCS7")!=0) && (strcmp(pPadding, "ZERO")!=0))
    {
        return luaL_error(L, "invalid padding=%s, only support NONE,PKCS5,PKCS7,ZERO", pPadding);
    }
    if(((strcmp(pMode, "CBC")==0) || (strcmp(pMode, "CTR")==0)) && (nIVLen!=16))
    {
        return luaL_error(L, "invalid iv length=%d, only support 16", nIVLen);
    }
    
    //构造填充数据
    if((strcmp(pPadding, "PKCS5")==0) || (strcmp(pPadding, "PKCS7")==0))
    {
        memset(pPadBuf, nPadLen, sizeof(pPadBuf));
    }
    else if(strcmp(pPadding, "ZERO")==0)
    {
        memset(pPadBuf, 0, sizeof(pPadBuf));
    } 
    memset(nPassWord,0,32);
    if(nPswdLen ==16)
    {
        nPswdLen = 128;
        nPassWord = malloc(16);
        memset(nPassWord,pPassword,16);
    }
    else if (nPswdLen ==24)
    {
        nPswdLen = 192;
        nPassWord = malloc(24);
        memset(nPassWord,pPassword,24);
    }
    else if (nPswdLen ==32)
    {
        nPswdLen = 256; 
        nPassWord = malloc(32);
        memset(nPassWord,pPassword,32);
    }
    //加密
      
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );
        aes_context ctx;
        size_t count = 0;
        unsigned char output[16];
        unsigned char input[16];
        unsigned char nonce_counter[16];

         //原始数据和填充数据拼接在一起
        pInBuf = malloc(nBufLen+nPadLen);
        if(pInBuf == NULL)
        {
            printf("aes_encrypt malloc error!!!\n");
            luaL_pushresult( &b );
            return 1;
        }
        memset(&ctx,0,sizeof(aes_context));
        memcpy(pInBuf, pBuf, nBufLen);
        memcpy(pInBuf+nBufLen, pPadBuf, nPadLen); 
        nBufLen += nPadLen;
        nRmnLen = nBufLen;

        if(strcmp(pMode, "ECB") == 0)
        {
            //开始分组加密，每16字节一组
            while(nRmnLen>0)
            {
                aes_setkey_enc(&ctx,nPassWord,nPswdLen);
                memset(input,pInBuf+nBufLen-nRmnLen,AES_BLOCK_LEN);
                aes_crypt_ecb(&ctx,AES_ENCRYPT,input,output);
                memset(&ctx,0,sizeof(aes_context));
                luaL_addlstring(&b, output, AES_BLOCK_LEN);
                nRmnLen -= AES_BLOCK_LEN;
            }
        }
        else if( strcmp(pMode, "CBC") == 0 )
        {
            //待加密数据一次性传入
            pOutBuff = malloc(nBufLen);
            aes_setkey_enc(&ctx,nPassWord,nPswdLen);
            aes_crypt_cbc(&ctx,
                    AES_ENCRYPT,
                    nBufLen,
                    pIV,
                    pInBuf,
                    pOutBuff);
            luaL_addlstring(&b,pOutBuff,nBufLen);
        }
        else if(strcmp(pMode, "CTR") == 0) 
        {
            memset(&nonce_counter,0,16);
            pOutBuff = malloc(nBufLen);
            aes_setkey_enc(&ctx,nPassWord,nPswdLen);
            aes_crypt_ctr(&ctx,
                       nBufLen,
                       &count,
                       nonce_counter,
                       pIV,
                       pInBuf,
                       pOutBuff);
            luaL_addlstring(&b,pOutBuff,nBufLen);
        }
        luaL_pushresult( &b );
        return 1;
 
}

static int l_crypto_aes_decrypt(lua_State *L)
{    
    u8 *pMode = luaL_checkstring(L, 1);
    u8 *pPadding = luaL_checkstring(L, 2);
    size_t nBufLen = 0;
    u8 *pBuf = lua_tolstring(L, 3, &nBufLen);
    size_t nPswdLen = 0;
    u8 *pPassword = lua_tolstring(L, 4, &nPswdLen);
    size_t nIVLen = 0;
    u8 *pIV =  lua_tolstring(L, 5, &nIVLen);
    unsigned char *pOutBuff = NULL;
    unsigned char nPassWord[32];
    //检查参数合法性
    if((nPswdLen!=16) && (nPswdLen!=24) && (nPswdLen!=32))
    {
        return luaL_error(L, "invalid password length=%d, only support AES128,AES192,AES256", nPswdLen);
    }
    if((strcmp(pMode, "ECB")!=0) && (strcmp(pMode, "CBC")!=0) && (strcmp(pMode, "CTR")!=0))
    {
        return luaL_error(L, "invalid mode=%s, only support ECB,CBC,CTR", pMode);
    }
    if((strcmp(pPadding, "NONE")!=0) && (strcmp(pPadding, "PKCS5")!=0) && (strcmp(pPadding, "PKCS7")!=0) && (strcmp(pPadding, "ZERO")!=0))
    {
        return luaL_error(L, "invalid padding=%s, only support NONE,PKCS5,PKCS7,ZERO", pPadding);
    }
    if(((strcmp(pMode, "CBC")==0) || (strcmp(pMode, "CTR")==0)) && (nIVLen!=16))
    {
        return luaL_error(L, "invalid iv length=%d, only support 16", nIVLen);
    }    
    memset(nPassWord,0,32);
    if(nPswdLen ==16)
    {
        nPswdLen = 128;
        memset(nPassWord,pPassword,16);
    }
    else if (nPswdLen ==24)
    {
        nPswdLen = 192;
        memset(nPassWord,pPassword,24);
    }
    else if (nPswdLen ==32)
    {
        nPswdLen = 256; 
        memset(nPassWord,pPassword,32);  
    }       
    //解密
    {       
        luaL_Buffer b;
        u32 nRmnLen;
        memset(b.buffer,0,LUAL_BUFFERSIZE);
        luaL_buffinit( L, &b );
        aes_context ctx;
        size_t count = 0;
        unsigned char input[16];
        unsigned char output[16];
        unsigned char nonce_counter[16];

        nRmnLen = nBufLen;

        if(strcmp(pMode, "ECB") == 0)
        {
            //开始分组解密，每16字节一组           
            while(nRmnLen>0)
            {
                aes_setkey_dec(&ctx,nPassWord,nPswdLen);
                memset(input,pBuf+nBufLen-nRmnLen,AES_BLOCK_LEN);
                aes_crypt_ecb(&ctx,
                    AES_DECRYPT,
                    input,
                    output);
                //删除填充数据
                if(nRmnLen==AES_BLOCK_LEN)
                {
                    DeletePaddingBuf(&b, pPadding, AES_BLOCK_LEN, pBuf+nBufLen-nRmnLen);
                }
                else
                {
                    luaL_addlstring(&b, output, AES_BLOCK_LEN);
                }
                nRmnLen -= AES_BLOCK_LEN;
            }
        }
        else if(strcmp(pMode, "CBC") == 0 )
        {
            //待解密数据一次性传入
            pOutBuff = malloc(nBufLen);
            aes_setkey_dec(&ctx,nPassWord,nPswdLen);
            aes_crypt_cbc(&ctx,
                    AES_DECRYPT,
                    nBufLen,
                    pIV,
                    pBuf,
                    pOutBuff);
//          AES_Decrypt(pMode, nBufLen, pBuf, nPswdLen, pPassword, pIV);
            DeletePaddingBuf(&b, pPadding, nBufLen, pOutBuff);
        }
        else if(strcmp(pMode, "CTR") == 0)
        {
            pOutBuff = malloc(nBufLen);
            aes_setkey_dec(&ctx,nPassWord,nPswdLen);
            aes_crypt_ctr(&ctx,
                       nBufLen,
                       &count,
                       nonce_counter,
                       pIV,
                       pBuf,
                       pOutBuff); 
            DeletePaddingBuf(&b, pPadding, nBufLen, pOutBuff);
        }
        luaL_pushresult( &b );
        return 1;
    }    
}
#endif

/*+\NEW\wangyuan\2020.04.14\ BUG_1445:rsa加解密接口不可用*/
INT32 crypto_rsa_encrypt(UINT8 *pKeyMode, UINT8 *pKeyBuf, INT32 nKeyLen, UINT8 *pPswd, INT32 nPswdLen, 
										UINT8 *pEncryptMode, UINT8 *pInBuf, INT32 nInbufLen, UINT8 *pOutBuf)
{
    E_AMOPENAT_RSA_KEY_MODE nKeyMode = OPENAT_RSA_PUBLIC_KEY;
    E_AMOPENAT_RSA_CRYPT_MODE nEncryptMode = OPENAT_RSA_PUBLIC_KEY_CRYPT;
 
    if(strcmp(pKeyMode, "PRIVATE_KEY")==0)
    {
        nKeyMode = OPENAT_RSA_PRIVATE_KEY;
    }
    if(strcmp(pEncryptMode, "PRIVATE_CRYPT")==0)
    {
        nEncryptMode = OPENAT_RSA_PRIVATE_KEY_CRYPT;
    }
    return openat_rsa_encrypt(nKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, nEncryptMode, pInBuf, nInbufLen, pOutBuf);
}

INT32 crypto_rsa_decrypt(UINT8 *pKeyMode, UINT8 *pKeyBuf, INT32 nKeyLen, UINT8 *pPswd, INT32 nPswdLen, 
										UINT8 *pDecryptMode, UINT8 *pInBuf, INT32 nInbufLen, UINT8 *pOutBuf, UINT16 *pOutLen, UINT16 nOutBufSize)
{
    E_AMOPENAT_RSA_KEY_MODE nKeyMode = OPENAT_RSA_PUBLIC_KEY;
    E_AMOPENAT_RSA_CRYPT_MODE nDecryptMode = OPENAT_RSA_PUBLIC_KEY_CRYPT;
 
    if(strcmp(pKeyMode, "PRIVATE_KEY")==0)
    {
        nKeyMode = OPENAT_RSA_PRIVATE_KEY;
    }
    if(strcmp(pDecryptMode, "PRIVATE_CRYPT")==0)
    {
        nDecryptMode = OPENAT_RSA_PRIVATE_KEY_CRYPT;
    }
    return openat_rsa_decrypt(nKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, nDecryptMode, pInBuf, nInbufLen, pOutBuf, pOutLen, nOutBufSize);
}

INT32 crypto_rsa_sha256_sign(UINT8 *pKeyMode, UINT8 *pKeyBuf, INT32 nKeyLen, UINT8 *pPswd, INT32 nPswdLen, 
										UINT8 *pEncryptMode, UINT8 *pInBuf, INT32 nInbufLen, UINT8 *pOutBuf)
{
    E_AMOPENAT_RSA_KEY_MODE nKeyMode = OPENAT_RSA_PUBLIC_KEY;
    E_AMOPENAT_RSA_CRYPT_MODE nEncryptMode = OPENAT_RSA_PUBLIC_KEY_CRYPT;
 
    if(strcmp(pKeyMode, "PRIVATE_KEY")==0)
    {
        nKeyMode = OPENAT_RSA_PRIVATE_KEY;
    }
    if(strcmp(pEncryptMode, "PRIVATE_CRYPT")==0)
    {
        nEncryptMode = OPENAT_RSA_PRIVATE_KEY_CRYPT;
    }
    return openat_rsa_sha256_sign(nKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, nEncryptMode, pInBuf, nInbufLen, pOutBuf);
}

INT32 crypto_rsa_sha256_verify(UINT8 *pKeyMode, UINT8 *pKeyBuf, INT32 nKeyLen, UINT8 *pPswd, INT32 nPswdLen, 
										UINT8 *pDecryptMode, UINT8 *pInBuf, INT32 nInbufLen, UINT8 *pInPlainBuf, UINT16 nInPlainLen)
{
    E_AMOPENAT_RSA_KEY_MODE nKeyMode = OPENAT_RSA_PUBLIC_KEY;
    E_AMOPENAT_RSA_CRYPT_MODE nDecryptMode = OPENAT_RSA_PUBLIC_KEY_CRYPT;
 
    if(strcmp(pKeyMode, "PRIVATE_KEY")==0)
    {
        nKeyMode = OPENAT_RSA_PRIVATE_KEY;
    }
    if(strcmp(pDecryptMode, "PRIVATE_CRYPT")==0)
    {
        nDecryptMode = OPENAT_RSA_PRIVATE_KEY_CRYPT;
    }
    return openat_rsa_sha256_verify(nKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, nDecryptMode, pInBuf, nInbufLen, pInPlainBuf, nInPlainLen);
}

static int l_crypto_rsa_encrypt(lua_State *L)
{    
    u8 *pKeyMode = luaL_checkstring(L, 1);
    size_t nKeyLen = 0;
    u8 *pKeyBuf = lua_tolstring(L, 2, &nKeyLen);
    u32 nKeyBits = lua_tonumber(L, 3);
    u8 *pEncryptMode = luaL_checkstring(L, 4);
    size_t nInbufLen = 0;
    u8 *pInBuf = lua_tolstring(L, 5, &nInbufLen);
    size_t nPswdLen = 0;
    u8 *pPswd = NULL;
    int n = lua_gettop(L);
    u8 *pOutBuf = NULL;

    luaL_Buffer b;
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    luaL_buffinit( L, &b );

    if(n>=6)
    {
        pPswd = lua_tolstring(L, 6, &nPswdLen);
    }

    if((strcmp(pKeyMode, "PUBLIC_KEY")!=0) && (strcmp(pKeyMode, "PRIVATE_KEY")!=0))
    {
        return luaL_error(L, "invalid keyMode=%s, only support PUBLIC_KEY,PRIVATE_KEY", pKeyMode);
    }
    if((strcmp(pEncryptMode, "PUBLIC_CRYPT")!=0) && (strcmp(pEncryptMode, "PRIVATE_CRYPT")!=0))
    {
        return luaL_error(L, "invalid cryptMode=%s, only support PUBLIC_CRYPT,PRIVATE_CRYPT", pEncryptMode);
    }
    if(nKeyBits%8 != 0)
    {
        return luaL_error(L, "invalid nKeyBits=%d", nKeyBits);
    }

    pOutBuf = malloc(nKeyBits/8);
    if(pOutBuf==NULL)
    {
        printf("l_crypto_rsa_encrypt malloc error!!!\n");
        luaL_pushresult( &b );
        return 1;
    }

    if(crypto_rsa_encrypt(pKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, pEncryptMode, pInBuf, nInbufLen, pOutBuf) == 0)
    {
        luaL_addlstring(&b, pOutBuf, nKeyBits/8);
    }
  
    if(pOutBuf != NULL)
    {
        free(pOutBuf);
        pOutBuf = NULL;
    }
        
    luaL_pushresult( &b );
    return 1;
}

static int l_crypto_rsa_decrypt(lua_State *L)
{    
    u8 *pKeyMode = luaL_checkstring(L, 1);
    size_t nKeyLen = 0;
    u8 *pKeyBuf = lua_tolstring(L, 2, &nKeyLen);
    u32 nKeyBits = lua_tonumber(L, 3);
    u8 *pDecryptMode = luaL_checkstring(L, 4);
    size_t nInbufLen = 0;
    u8 *pInBuf = lua_tolstring(L, 5, &nInbufLen);
    size_t nPswdLen = 0;
    u8 *pPswd = NULL;
    int n = lua_gettop(L);
    u8 *pOutBuf = NULL;
    u16 nOutLen = 0;

    luaL_Buffer b;
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    luaL_buffinit( L, &b );

    if(n>=6)
    {
        pPswd = lua_tolstring(L, 6, &nPswdLen);
    }

    if((strcmp(pKeyMode, "PUBLIC_KEY")!=0) && (strcmp(pKeyMode, "PRIVATE_KEY")!=0))
    {
        return luaL_error(L, "invalid keyMode=%s, only support PUBLIC_KEY,PRIVATE_KEY", pKeyMode);
    }
    if((strcmp(pDecryptMode, "PUBLIC_CRYPT")!=0) && (strcmp(pDecryptMode, "PRIVATE_CRYPT")!=0))
    {
        return luaL_error(L, "invalid cryptMode=%s, only support PUBLIC_CRYPT,PRIVATE_CRYPT", pDecryptMode);
    }
    if(nKeyBits%8 != 0)
    {
        return luaL_error(L, "invalid nKeyBits=%d", nKeyBits);
    }

    pOutBuf = malloc(nKeyBits/8);
    if(pOutBuf==NULL)
    {
        printf("l_crypto_rsa_decrypt malloc error!!!\n");
        luaL_pushresult( &b );
        return 1;
    }

    if(crypto_rsa_decrypt(pKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, pDecryptMode, pInBuf, nInbufLen, pOutBuf, &nOutLen, nKeyBits/8) == 0)
    {
        luaL_addlstring(&b, pOutBuf, nOutLen);
    }

    if(pOutBuf != NULL)
    {
        free(pOutBuf);
        pOutBuf = NULL;
    }
        
    luaL_pushresult( &b );
    return 1;
}

static int l_crypto_rsa_sha256_sign(lua_State *L)
{    
    u8 *pKeyMode = luaL_checkstring(L, 1);
    size_t nKeyLen = 0;
    u8 *pKeyBuf = lua_tolstring(L, 2, &nKeyLen);
    u32 nKeyBits = lua_tonumber(L, 3);
    u8 *pEncryptMode = luaL_checkstring(L, 4);
    size_t nInbufLen = 0;
    u8 *pInBuf = lua_tolstring(L, 5, &nInbufLen);
    size_t nPswdLen = 0;
    u8 *pPswd = NULL;
    int n = lua_gettop(L);
    u8 *pOutBuf = NULL;

    luaL_Buffer b;
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    luaL_buffinit( L, &b );

    if(n>=6)
    {
        pPswd = lua_tolstring(L, 6, &nPswdLen);
    }

    if((strcmp(pKeyMode, "PUBLIC_KEY")!=0) && (strcmp(pKeyMode, "PRIVATE_KEY")!=0))
    {
        return luaL_error(L, "l_crypto_rsa_sha256_sign invalid keyMode=%s, only support PUBLIC_KEY,PRIVATE_KEY", pKeyMode);
    }
    if((strcmp(pEncryptMode, "PUBLIC_CRYPT")!=0) && (strcmp(pEncryptMode, "PRIVATE_CRYPT")!=0))
    {
        return luaL_error(L, "l_crypto_rsa_sha256_sign invalid cryptMode=%s, only support PUBLIC_CRYPT,PRIVATE_CRYPT", pEncryptMode);
    }
    if(nKeyBits%8 != 0)
    {
        return luaL_error(L, "l_crypto_rsa_sha256_sign invalid nKeyBits=%d", nKeyBits);
    }

    pOutBuf = malloc(nKeyBits/8);
    if(pOutBuf==NULL)
    {
        printf("l_crypto_rsa_sha256_sign malloc error!!!\n");
        luaL_pushresult( &b );
        return 1;
    }

    if(crypto_rsa_sha256_sign(pKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, pEncryptMode, pInBuf, nInbufLen, pOutBuf) == 0)
    {
        luaL_addlstring(&b, pOutBuf, nKeyBits/8);
    }

    if(pOutBuf != NULL)
    {
        free(pOutBuf);
        pOutBuf = NULL;
    }
        
    luaL_pushresult( &b );
    return 1;
}


static int l_crypto_rsa_sha256_verify(lua_State *L)
{
    #define SHA256_SUM_LEN 32
    u8 *pKeyMode = luaL_checkstring(L, 1);
    size_t nKeyLen = 0;
    u8 *pKeyBuf = lua_tolstring(L, 2, &nKeyLen);
    u32 nKeyBits = lua_tonumber(L, 3);
    u8 *pDecryptMode = luaL_checkstring(L, 4);
    size_t nInbufLen = 0;
    u8 *pInBuf = lua_tolstring(L, 5, &nInbufLen);
    size_t pInPlainLen = 0;
    u8 *pInPlainBuf = lua_tolstring(L, 6, &pInPlainLen);
    size_t nPswdLen = 0;
    u8 *pPswd = NULL;
    int n = lua_gettop(L);

    luaL_Buffer b;
    memset(b.buffer,0,LUAL_BUFFERSIZE);
    luaL_buffinit( L, &b );

    if(n>=7)
    {
        pPswd = lua_tolstring(L, 7, &nPswdLen);
    }

    if((strcmp(pKeyMode, "PUBLIC_KEY")!=0) && (strcmp(pKeyMode, "PRIVATE_KEY")!=0))
    {
        return luaL_error(L, "l_crypto_rsa_sha256_verify invalid keyMode=%s, only support PUBLIC_KEY,PRIVATE_KEY", pKeyMode);
    }
    if((strcmp(pDecryptMode, "PUBLIC_CRYPT")!=0) && (strcmp(pDecryptMode, "PRIVATE_CRYPT")!=0))
    {
        return luaL_error(L, "l_crypto_rsa_sha256_verify invalid cryptMode=%s, only support PUBLIC_CRYPT,PRIVATE_CRYPT", pDecryptMode);
    }
    if(nKeyBits%8 != 0)
    {
        return luaL_error(L, "l_crypto_rsa_sha256_verify invalid nKeyBits=%d", nKeyBits);
    }

    lua_pushboolean(L, crypto_rsa_sha256_verify(pKeyMode, pKeyBuf, nKeyLen, pPswd, nPswdLen, pDecryptMode, pInBuf, nInbufLen, pInPlainBuf, pInPlainLen) == 0);

    return 1;
}
/*-\NEW\wangyuan\2020.04.14\ BUG_1445:rsa加解密接口不可用*/

#if 1//def AM_XXTEA_SUPPORT 
#include "xxtea.h"
static int l_crypto_xxtea_encrypt(lua_State *L) {
	unsigned char *result;
	const char *data, *key;
	size_t data_len, key_len, out_len;

	data = luaL_checklstring(L, 1, &data_len);
	key  = luaL_checklstring(L, 2, &key_len);
	result = xxtea_encrypt(data, data_len, key, &out_len);

	if(result == NULL){
		lua_pushnil(L);
	}else{
		lua_pushlstring(L, (const char *)result, out_len);
		free(result);
	}

	return 1;
}

static int l_crypto_xxtea_decrypt(lua_State *L) {
	unsigned char *result;
	const char *data, *key;
	size_t data_len, key_len, out_len;

	data = luaL_checklstring(L, 1, &data_len);
	key  = luaL_checklstring(L, 2, &key_len);
	result = xxtea_decrypt(data, data_len, key, &out_len);

	if(result == NULL){
		lua_pushnil(L);
	}else{
		lua_pushlstring(L, (const char *)result, out_len);
		free(result);
	}

	return 1;
}
#endif



#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE crypto_map[] =
{
    { LSTRKEY( "base64_encode" ),  LFUNCVAL( l_crypto_base64_encode ) },
    { LSTRKEY( "base64_decode" ),  LFUNCVAL( l_crypto_base64_decode ) },
    { LSTRKEY( "hmac_md5" ),  LFUNCVAL( l_crypto_hmac_md5 ) },
    { LSTRKEY( "md5" ),  LFUNCVAL( l_crypto_md5 ) },
    { LSTRKEY( "hmac_sha1" ),  LFUNCVAL( l_crypto_hmac_sha1 ) },
    { LSTRKEY( "hmac_sha256" ),  LFUNCVAL( l_crypto_hmac_sha2 ) },
    { LSTRKEY( "sha1" ),  LFUNCVAL( l_crypto_sha1 ) },
    { LSTRKEY( "sha256" ),  LFUNCVAL( l_crypto_sha256 ) },
    { LSTRKEY( "crc16" ),  LFUNCVAL( l_crypto_crc16 ) },
    { LSTRKEY( "crc16_modbus" ),  LFUNCVAL( l_crypto_crc16_modbus ) },
    { LSTRKEY( "crc32" ),  LFUNCVAL( l_crypto_crc32 ) },
    { LSTRKEY( "crc8" ),  LFUNCVAL( l_crypto_crc8 ) },
    { LSTRKEY( "aes128_ecb_encrypt" ),  LFUNCVAL( l_crypto_aes128_ecb_encrypt ) },
    { LSTRKEY( "aes128_ecb_decrypt" ),  LFUNCVAL( l_crypto_aes128_ecb_decrypt ) },
    { LSTRKEY( "aes_encrypt" ),  LFUNCVAL( l_crypto_aes_encrypt ) },
    { LSTRKEY( "aes_decrypt" ),  LFUNCVAL( l_crypto_aes_decrypt ) },
    #if 1//def AM_XXTEA_SUPPORT
    { LSTRKEY( "xxtea_encrypt" ),  LFUNCVAL( l_crypto_xxtea_encrypt ) },
    { LSTRKEY( "xxtea_decrypt" ),  LFUNCVAL( l_crypto_xxtea_decrypt ) },
    #endif
	/*+\NEW\wangyuan\2020.04.14\ BUG_1445:rsa加解密接口不可用*/
	{ LSTRKEY( "rsa_encrypt" ),  LFUNCVAL( l_crypto_rsa_encrypt ) },
	{ LSTRKEY( "rsa_decrypt" ),  LFUNCVAL( l_crypto_rsa_decrypt ) },
	{ LSTRKEY( "rsa_sha256_sign" ),  LFUNCVAL( l_crypto_rsa_sha256_sign ) },
	{ LSTRKEY( "rsa_sha256_verify" ),  LFUNCVAL( l_crypto_rsa_sha256_verify ) },
	/*-\NEW\wangyuan\2020.04.14\ BUG_1445:rsa加解密接口不可用*/
	
    { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE crypto_fwmd5_map[] =
{
    { LSTRKEY( "flow_md5" ),  LFUNCVAL( l_crypto_flow_md5 ) },


    { LNILKEY, LNILVAL }
};
const LUA_REG_TYPE crypto_fwmd5_mt_map[] =
{
    { LSTRKEY( "update" ),  LFUNCVAL( l_crypto_md5_update ) },
    { LSTRKEY( "hexdigest" ),  LFUNCVAL( l_crypto_md5_hexdigest ) },

    { LNILKEY, LNILVAL }
};
int luaopen_crypto( lua_State *L )
{
    luaL_register( L, AUXLIB_CRYPTO, crypto_map );

    luaL_newmetatable( L, META_NAME );
    lua_pushvalue(L,-1);
    lua_setfield(L, -2, "__index");
    luaL_register( L, NULL, crypto_fwmd5_mt_map );
    luaL_register( L, AUXLIB_CRYPTO, crypto_fwmd5_map );
    return 1;
}

