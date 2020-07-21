#ifdef AM_SLI3108_SUPPORT

#include <string.h>

#include "lplatform.h"
#include "platform_conf.h"
#include "am_openat.h"



void platform_SLI3108_wear_status_callback(void* param)
{
    SLI3108_STATUS sli3108_status = (SLI3108_STATUS)param;
    PlatformMsgData rtosmsg;

    rtosmsg.wearStatusData.wearStatus = (sli3108_status == SLI3108_STATUS_HIGH);
    platform_rtos_send(MSG_ID_RTOS_WEAR_STATUS, &rtosmsg);
}


void platform_SLI3108_init(void)
{
    OPENAT_SLI3108_init(platform_SLI3108_wear_status_callback);
}


void platform_SLI3108_set_work_mode(kal_bool factory_mode)
{
    OPENAT_SLI3108_set_mode(factory_mode);
}

/*-\NEW\zhuwangbin\2016.4.13\添加距离传感器open 和 close 接口*/
void platform_SLI3108_open(void)
{
  OPENAT_SLI3108_open();
}

void platform_SLI3108_close(void)
{
  OPENAT_SLI3108_close();
}
/*-\NEW\zhuwangbin\2016.4.13\添加距离传感器open 和 close 接口*/

#endif
