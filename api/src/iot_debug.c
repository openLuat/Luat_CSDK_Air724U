#include "iot_debug.h"
#include "am_openat.h"
#include "string.h"

/*******************************************
**                 DEBUG                  **
*******************************************/

/**assert断言
*@param		condition:	断言条件
*@param		func:	    断言函数
*@param		line:	    断言位置
*@return	TURE: 	    成功
*           FALSE:      失败
**/
VOID iot_debug_assert(                                          
                        BOOL condition,                  
                        CHAR *func,                     
                        UINT32 line                       
              )
{
    OPENAT_assert(condition, func, line);
}


/**调试信息打印
**/
VOID iot_debug_print(CHAR *fmt, ...)
{
	char buff[256] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, 256, fmt, args);
	OPENAT_print("%s", buff);
	va_end (args);
}


