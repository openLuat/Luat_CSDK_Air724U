/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_stubs.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/11/29
 *
 * Description:
 *   实现newlib/stubs.c中需要平台支持的一些stdio.c的接口
 * History:
 *     panjun 2015.04.30 Add an 'platform_separator_strrpl()' API to replace a sub-string.
 **************************************************************************/
#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "stdarg.h"
#include "utils.h"

static char iobuf_temp[2048];

extern int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap);

int platform_vfprintf(FILE *fp, const char *fmt, va_list ap)
{
    int len;

    len = vsnprintf(iobuf_temp, sizeof(iobuf_temp), fmt, ap);

    ASSERT(len < sizeof(iobuf_temp));

    return (*fp->_write)(fp->_cookie, iobuf_temp, len);
}

void platform_separator_strrpl(char* pDstOut, char* pSrcIn, const char* pSrcRpl, const char* pDstRpl)
{ 
   char* pi = pSrcIn; 
   char* po = pDstOut; 
   
   int nSrcRplLen = strlen(pSrcRpl); 
   int nDstRplLen = strlen(pDstRpl); 
   
   char *p = NULL; 
   int nLen = 0; 
   
   do 
   {
      p = strstr(pi, pSrcRpl); 
      
      if(p != NULL) 
      { 
         nLen = p - pi; 
         memcpy(po, pi, nLen);
         
         memcpy(po + nLen, pDstRpl, nDstRplLen); 
      } 
      else 
      { 
         strcpy(po, pi); 
         
         break;
      } 
      
      pi = p + nSrcRplLen; 
      po = po + nLen + nDstRplLen;    
   } while (p != NULL); 
}

