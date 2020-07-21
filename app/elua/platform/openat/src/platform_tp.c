
#include <string.h>

#include "lplatform.h"
#include "platform_conf.h"
#include "am_openat.h"


#if defined(TOUCH_PANEL_SUPPORT)
void platform_tp_sleep_out(void)
{
    OPENAT_TouchScreen_Sleep_Out();
}



void platform_tp_sleep_in(void)
{
    OPENAT_TouchScreen_Sleep_In();
}
#endif
