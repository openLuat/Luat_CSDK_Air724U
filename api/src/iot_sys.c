#include "iot_sys.h"


/**ota设置new app的文件，用来告知底层需要从文件读取升级新的程序
*@param		newAPPFile:		新程序文件 
*@return	TRUE: 成功   FALSE: 失败
**/
BOOL iot_ota_newapp(              
                    CONST char* newAPPFile
               )
{
    //return IVTBL(flash_set_newapp)(newAPPFile);
    return FALSE;
}


/**将char类型转换为WCHAR，结果用来作为iot_fs_open_file等接口的文件名参数
*@param     dst:        转换输出结果
*@param     src:        等待转换的字符串
*@return    返回dst首地址
**/ 
WCHAR* iot_strtows(WCHAR* dst, const char* src)
{
   WCHAR* rlt = dst;
   while(*src)
   {
       *dst++ = *src++;
   }
   *dst = 0;
   
   return (rlt);
}


