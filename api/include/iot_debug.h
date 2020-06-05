#ifndef __IOT_DEBUG_H__
#define __IOT_DEBUG_H__

#include "iot_os.h"
#include "stdarg.h"

/**
 * @defgroup iot_sdk_debug 调试接口
 * @{
 */

/**调试信息打印
**/
VOID iot_debug_print(     CHAR *fmt, ...);

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
              );

/** @}*/

#endif
