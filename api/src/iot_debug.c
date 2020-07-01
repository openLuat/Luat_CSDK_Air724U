#include "iot_debug.h"
#include "am_openat.h"
#include "string.h"
#include <sys/stdio.h>


extern BOOL g_s_traceflag;
/*******************************************
**                 DEBUG                  **
*******************************************/

/**assert����
*@param		condition:	��������
*@param		func:	    ���Ժ���
*@param		line:	    ����λ��
*@return	TURE: 	    �ɹ�
*           FALSE:      ʧ��
**/
VOID iot_debug_assert(                                          
                        BOOL condition,                  
                        CHAR *func,                     
                        UINT32 line                       
              )
{
    IVTBL(assert)(condition, func, line);
}


/**������Ϣ��ӡ
**/
VOID iot_debug_print(CHAR *fmt, ...)
{
	char buff[256] = {0};
	va_list args;
	va_start(args, fmt);
	vsnprintf(buff, 256, fmt, args);
	if(g_s_traceflag)
		IVTBL(lua_print)("%s", buff);
	else
		IVTBL(print)("%s", buff);
	va_end (args);
}

