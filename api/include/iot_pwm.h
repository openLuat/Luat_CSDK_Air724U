#ifndef __IOT_PWM_H__
#define __IOT_PWM_H__

#include "iot_os.h"

/**
 * @ingroup iot_sdk_device 外设接口
 * @{
 */
/**
 * @defgroup iot_sdk_pwm pwm接口
 * @{
 */

/**打开pwm功能 
*@param		port:		端口
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_open(E_AMOPENAT_PWM_PORT port);

/**设置pwm功能
*@param		pwm_cfg:		pwm_cfg
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_set(T_AMOPENAT_PWM_CFG * pwm_cfg);

/**关闭pwm功能
*@param		port:		端口
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_close(E_AMOPENAT_PWM_PORT port);


/** @}*/
/** @}*/





#endif

