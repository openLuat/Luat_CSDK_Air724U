#include "iot_pwm.h"

/**打开pwm功能 
*@param		port:		端口
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_open(E_AMOPENAT_PWM_PORT port)
{
    return OPENAT_pwm_open(port);
}

/**设置pwm功能
*@param		pwm_cfg:		pwm_cfg
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_set(T_AMOPENAT_PWM_CFG * pwm_cfg)
{
    return OPENAT_pwm_set(pwm_cfg);
}

/**关闭pwm功能
*@param		port:		端口
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pwm_close(E_AMOPENAT_PWM_PORT port)
{
    return OPENAT_pwm_close(port);
}

