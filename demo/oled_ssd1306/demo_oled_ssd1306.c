/***************
	demo_oled
****************/

#include "iot_debug.h"
#include "iot_os.h"
#include "am_openat.h"
#include "oled.h"

static void demo_oled(PVOID pParameter)
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
    OLED_ShowString(0, 24, "OLED_SSD1306", 16);
    OLED_ShowString(0, 40, "DATE 2020/7/26", 12);
    OLED_ShowString(0, 52, "ASCII:", 12);
    OLED_ShowString(64, 52, "CODE:", 12);
    uint8 t = 0;
    while (1)
    {
        OLED_ShowChar(36, 52, t, 12, 1); //显示ASCII字符
        OLED_ShowNum(94, 52, t, 3, 12);  //显示ASCII字符的码值
        OLED_Refresh_Gram();             //更新显示到OLED
        t++;
        iot_os_sleep(1000);
        iot_debug_print("[oled]demo_oled run!");
    }
}

int appimg_enter(void *param)
{
    iot_os_sleep(2000);
    iot_debug_print("[oled]appimg_enter");

    iot_os_create_task(demo_oled, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "oled");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[oled]appimg_exit");
}
