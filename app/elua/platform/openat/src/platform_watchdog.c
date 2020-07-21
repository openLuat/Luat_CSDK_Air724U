#include "lplatform.h"
#include "platform_watchdog.h"

/*+\NEW\brezen\2016.03.03\增加watchdog使能接口*/
int platform_watchdog_open(watchdog_info_t *info){
    return PLATFORM_OK;
}

int platform_watchdog_close(void){
    return PLATFORM_OK;
}
/*-\NEW\brezen\2016.03.03\增加watchdog使能接口*/

int platform_watchdog_kick(void){
    return PLATFORM_ERR;
}

