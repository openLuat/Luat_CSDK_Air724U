/***************
	demo_at_engine
****************/


#include "iot_debug.h"
#include "iot_os.h"


int appimg_enter(void *param)
{
    iot_debug_print("[hello]appimg_enter");

    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);

    iot_vat_send_cmd("AT^TRACECTRL=0,1,3\r\n", sizeof("AT^TRACECTRL=0,1,3\r\n"));
    at_task_init();//初始化AT引引擎
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[hello]appimg_exit");
}
