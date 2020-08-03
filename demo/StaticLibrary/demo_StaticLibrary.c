/***************
	demo_StaticLibrary
****************/


#include "iot_debug.h"
#include "iot_os.h"

#ifdef HelloBuild
#include "HelloWorld.h"
#endif
HANDLE demo_hello_task;

static void demo_hello(PVOID pParameter)
{
    #ifdef HelloBuild
        HelloFunc();
    #endif
    iot_os_delete_task(demo_hello_task);
}

int appimg_enter(void *param)
{
    iot_debug_print("[hello]appimg_enter");

	demo_hello_task = iot_os_create_task(demo_hello, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "hello");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[hello]appimg_exit");
}
