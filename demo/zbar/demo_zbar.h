#ifndef __DEMO_ZBAR_H__
#define __DEMO_ZBAR_H__

#include <sys/stdio.h>
#include "string.h"
#include "iot_os.h"
#include "iot_gpio.h"
#include "iot_debug.h"

#include "iot_pmd.h"
#include "iot_zbar.h"
#include "iot_lcd.h"
#include "iot_camera.h"

#define LCD_WIDTH 132
#define LCD_HEIGHT 162
#define LCD_LED (4)

//#define AM_CAM_BF20A6
//#define VGA_SUPPORT

#ifdef VGA_SUPPORT
#define CAM_SENSOR_WIDTH (640)
#define CAM_SENSOR_HEIGHT (480)
#else
#define CAM_SENSOR_WIDTH (320)
#define CAM_SENSOR_HEIGHT (240)
#endif


#define CAM_DISP_WIDTH (132)
#define CAM_DISP_HEIGHT (162)

BOOL cameraInit(PCAMERA_MESSAGE cb);
BOOL lcdInit(void);



#endif

