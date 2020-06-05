#ifndef __IOT_PMD_H__
#define __IOT_PMD_H__

#include "iot_os.h"

/**
 * @defgroup iot_sdk_pmd 电源管理接口
 * @{
 */

/**充电初始化
*@param		chrMode:		充电方式
*@param		cfg:		    配置信息
*@param		pPmMessage:		消息回调函数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pmd_init(     
                    E_AMOPENAT_PM_CHR_MODE chrMode,     
                    T_AMOPENAT_PMD_CFG*    cfg,       
                    PPM_MESSAGE            pPmMessage  
            );


/**正常开机
*@param		simStartUpMode:		开启SIM卡方式
*@param		nwStartupMode:		开启协议栈方式
*@return	TRUE: 	            成功
*           FALSE:              失败
**/
BOOL iot_pmd_poweron_system(                                     
                    E_AMOPENAT_STARTUP_MODE simStartUpMode,
                    E_AMOPENAT_STARTUP_MODE nwStartupMode
                  );

/**正常关机
*@note 正常关机 包括关闭协议栈和供电
**/
VOID iot_pmd_poweroff_system(VOID);

/**打开LDO
*@param		ldo:		    ldo通道
*@param		level:		    0-7 0:关闭 1~7电压等级
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_pmd_poweron_ldo(                                       
                    E_AMOPENAT_PM_LDO    ldo,
                    UINT8                level          
               );

/**进入睡眠
**/
VOID iot_pmd_enter_deepsleep(VOID);

/**退出睡眠
**/
VOID iot_pmd_exit_deepsleep(VOID);                               

/**获取开机原因值
*@return	E_AMOPENAT_POWERON_REASON: 	   返回开机原因值
**/
E_AMOPENAT_POWERON_REASON iot_pmd_get_poweronCasue(VOID);

/** @}*/

#endif

