/***************
	demo_oled
****************/

#include "iot_debug.h"
#include "iot_os.h"
#include "am_openat.h"
#include "oled.h"
#include "ds18b20.h"

int TempNum = 0;
char TempStr[10] = {0};

static void oled_task(PVOID pParameter)
{

    if (OLED_Init() == FALSE) //初始化OLED
    {
        while (1)
        {
            iot_debug_print("[oled]OLED_Init FALSE!");
            iot_os_sleep(1000);
        }
    }
    OLED_ShowString(0, 0, "LUAT CSDK", 24);
    OLED_ShowString(0, 24, "DEMO DS18B20", 16);
    OLED_ShowString(0, 40, "Tempera:", 12);
    OLED_ShowString(0, 52, "num:", 12);

    while (1)
    {
        OLED_ShowString(50, 40, TempStr, 12); //显示ASCII字符
        OLED_ShowNum(50, 52, TempNum, 6, 12); //显示ASCII字符
        OLED_Refresh_Gram();                  //更新显示到OLED
        iot_os_sleep(1000);
        iot_debug_print("[oled]demo_oled run!");
    }
}

static void ds18b20_task(PVOID pParameter)
{
    iot_os_sleep(3000);
    while (1)
    {
        if (DS18B20_GetTemp_Num(7, &TempNum) == 0)
        {
            iot_debug_print("[ds18b20]DS18B20_GetTemp_Num : %d", TempNum);
        }
        iot_os_sleep(1000);
        if (DS18B20_GetTemp_String(7, &TempStr[0]) == 0)
        {
            iot_debug_print("[ds18b20]DS18B20_GetTemp_String : %s", TempStr);
        }
        iot_os_sleep(1000);
    }
}

int appimg_enter(void *param)
{
    iot_os_sleep(2000);
    iot_debug_print("[oled]appimg_enter");

    iot_os_create_task(oled_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "oled_task");
    iot_os_create_task(ds18b20_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "ds18b20_task");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[oled]appimg_exit");
}
