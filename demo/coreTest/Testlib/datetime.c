#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"

#define datetime_print iot_debug_print

bool demo_set_system_datetime()
{
	T_AMOPENAT_SYSTEM_DATETIME *pDatetime;
	pDatetime = iot_os_malloc(sizeof(T_AMOPENAT_SYSTEM_DATETIME));
	pDatetime->nYear = 2019;
	pDatetime->nMonth = 8;
	pDatetime->nDay = 21;
	pDatetime->nHour = 10;
	pDatetime->nMin = 33;
	pDatetime->nSec = 55;
	if (iot_os_set_system_datetime(pDatetime) == FALSE)
	{
		datetime_print("[coreTest-False-datetime] iot_os_set_system_datetime false");
		return FALSE;
	}
	iot_os_free(pDatetime);
	return TRUE;
}

bool demo_get_system_datetime()
{
	T_AMOPENAT_SYSTEM_DATETIME *pDatetime;

	pDatetime = iot_os_malloc(sizeof(T_AMOPENAT_SYSTEM_DATETIME));

	if (iot_os_get_system_datetime(pDatetime) == FALSE)
	{
		datetime_print("[coreTest-False-datetime] iot_os_get_system_datetime false");
		return FALSE;
	}
	datetime_print("[coreTest-datetime] :time: %02d/%02d/%02d,%02d:%02d:%02d", pDatetime->nYear, pDatetime->nMonth, pDatetime->nDay, pDatetime->nHour, pDatetime->nMin, pDatetime->nSec);
	iot_os_free(pDatetime);
	return TRUE;
}

bool datetimeTest(void)
{
	//1. 设置系统时间
	demo_set_system_datetime();
	//2. 获取系统时间
	demo_get_system_datetime();
}
