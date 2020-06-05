#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"

#define datetime_print iot_debug_print

VOID demo_set_system_datetime()
{
	T_AMOPENAT_SYSTEM_DATETIME* pDatetime;

	pDatetime = iot_os_malloc(sizeof(T_AMOPENAT_SYSTEM_DATETIME));

	pDatetime->nYear = 2019;
	pDatetime->nMonth = 8;
	pDatetime->nDay = 21;
	pDatetime->nHour = 10;
	pDatetime->nMin = 33;
	pDatetime->nSec = 55;

	iot_os_set_system_datetime(pDatetime);

	iot_os_free(pDatetime);
}

VOID demo_get_system_datetime()
{
	T_AMOPENAT_SYSTEM_DATETIME* pDatetime;
	
  	pDatetime = iot_os_malloc(sizeof(T_AMOPENAT_SYSTEM_DATETIME));

  	iot_os_get_system_datetime(pDatetime);
	datetime_print("[datetime] time: %02d/%02d/%02d,%02d:%02d:%02d",
		pDatetime->nYear,pDatetime->nMonth,pDatetime->nDay,pDatetime->nHour,pDatetime->nMin,pDatetime->nSec);
  	iot_os_free(pDatetime);	
	
}

VOID demo_system_datetime(VOID)
{
    //1. 设置系统时间
    demo_set_system_datetime();

    //2. 获取系统时间
    demo_get_system_datetime();
}

int appimg_enter(void *param)
{    
    datetime_print("[datetime] app_main");

    demo_system_datetime();

    return 0;
}

void appimg_exit(void)
{
    datetime_print("[datetime] appimg_exit");
}