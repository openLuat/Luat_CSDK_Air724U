/***************
	demo_hello_Debug
****************/

#include "iot_debug.h"
#include "iot_os.h"

HANDLE demo_hello_Debug_task;
int demo_hello_Debug_num = 0;
static void demo_hello_Debug(PVOID pParameter)
{
    iot_debug_print("[debug]demo_hello_Debug");
    //关闭看门狗，死机不会重启。默认打开
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    //打开调试信息，默认关闭
    iot_vat_send_cmd("AT^TRACECTRL=0,1,1\r\n", sizeof("AT^TRACECTRL=0,1,1\r\n"));

    volatile int n = 0;
    for (n = 0; n < 15; n++)
    {
        demo_hello_Debug_num++;
        iot_debug_print("[debug]hello world %d", n);
        iot_os_sleep(1000);
    }
    n++;
    iot_debug_assert(0, __func__, __LINE__);
    iot_os_delete_task(demo_hello_Debug_task);
}

int appimg_enter(void *param)
{
    iot_debug_print("[debug]appimg_enter");

    demo_hello_Debug_task = iot_os_create_task(demo_hello_Debug, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "hello_Debug");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[hello]appimg_exit");
}
