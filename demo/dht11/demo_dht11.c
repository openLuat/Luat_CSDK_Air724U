/***************
    demo_dht11
****************/

#include "iot_debug.h"
#include "iot_os.h"
#include "am_openat.h"
#include "oled.h"
#include "dht11.h"

char HumStr[10] ={ 0 };
char TemStr[10] ={ 0 };

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
    OLED_ShowString(0, 24, "DEMO DHT11", 16);
    OLED_ShowString(0, 40, "Humidity:", 12);
    OLED_ShowString(0, 52, "Tempera:", 12);

    uint8 t = 0;
    while (1)
    {
        OLED_ShowString(60, 40, HumStr, 12); //显示ASCII字符
        OLED_ShowString(60, 52, TemStr, 12); //显示ASCII字符
        OLED_Refresh_Gram();                 //更新显示到OLED
        iot_os_sleep(1000);
        iot_debug_print("[oled]demo_oled run!");
    }
}

static void dht11_task(PVOID pParameter)
{
    iot_os_sleep(3000);
    while (1)
    {
        if (DHT11_GetData_String(7, &HumStr, &TemStr) == 0)
        {
            iot_debug_print("[dht11]HumStr: %s,TemStr: %s", HumStr, TemStr);
        }
        iot_os_sleep(3000);
    }
}

int appimg_enter(void *param)
{
    iot_os_sleep(2000);
    iot_debug_print("[oled]appimg_enter");

    iot_os_create_task(oled_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "oled_task");
    iot_os_create_task(dht11_task, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "dht11_task");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[oled]appimg_exit");
}
