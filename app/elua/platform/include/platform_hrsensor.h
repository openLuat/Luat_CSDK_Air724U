/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_hrsensor.h
 * Author:  
 * Version: V0.1
 * Date:    2015/11/17
 *
 * Description:
 *           
 * History:
 *
 **************************************************************************/
#ifndef _PLATFORM_HRSENSOR_H_
#define _PLATFORM_HRSENSOR_H_

#ifdef HRD_SENSOR_SUPPORT

#include "string.h"
#include "malloc.h"

#include "assert.h"
#include "lplatform.h"


VOID platform_hrsensor_start(void);

VOID platform_hrsensor_close(void);

int platform_hrsensor_getrate(void);

#endif

#endif



