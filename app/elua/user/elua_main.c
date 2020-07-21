/***************
	demo_elua
****************/

#include "string.h"

#include "osi_log.h"
#include "osi_api.h"

//main函数
int appimg_enter(void *param)
{
    extern void luatEluaInit();
    luatEluaInit();
    return 0;
}

//退出提示
void appimg_exit(void)
{
    OSI_LOGI(0, "application image exit");
}
