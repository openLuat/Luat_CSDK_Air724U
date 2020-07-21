/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_hrsensor.c
 * Author:  
 * Version: V0.1
 * Date:    2015/11/17
 *
 * Description:
 *           
 * History:
 *
 **************************************************************************/
#ifdef HRD_SENSOR_SUPPORT
#include "string.h"
#include "malloc.h"

#include "assert.h"
#include "lplatform.h"


VOID platform_hrsensor_start(void)
{
    IVTBL(hrd_sensor_start)();
}

VOID platform_hrsensor_close(void)
{
    IVTBL(hrd_sensor_close)();
}

int platform_hrsensor_getrate(void)
{
    return IVTBL(hrd_sensor_getrate)();
}

#endif




