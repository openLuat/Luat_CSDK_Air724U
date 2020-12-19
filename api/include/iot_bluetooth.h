#ifndef __IOT_BLUETOOTH_H__
#define __IOT_BLUETOOTH_H__

#include "iot_os.h"
#include "am_openat_bluetooth.h"

void bluetooth_callback(T_OPENAT_BLE_EVENT_PARAM *result);
/**
 * @defgroup iot_sdk_bluetooth 蓝牙接口
 * @{
 */
/**@example bluetooth/demo_bluetooth.c
* bluetooth接口示例
*/

/**打开蓝牙
*@param  mode:          蓝牙打开模式
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_bt_open(                                        
                        E_OPENAT_BT_MODE mode
                );

/**关闭蓝牙
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_bt_close(                                        
                        VOID
                );

/**写蓝牙
*@param  handle:      连接句柄  
*@param  uuid:        写入特征uuid
*@param  data:      写入数据内容
*@param  len:        写入数据长度

*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_write(    
                        UINT16 handle,   
                        T_OPENAT_BLE_UUID uuid,                                 
                        char *data , 
                        UINT8 len         
                );

/**关闭蓝牙
*@param  cmd:        功能cmd
*@param  parm:        广播参数
*@param  len:        数据长度
*@param  handle:      连接句柄  
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_iotctl(                                        
                        E_OPENAT_BT_CMD cmd,
                        U_OPENAT_BT_IOTCTL_PARM  *parm,
                        UINT8 len,
                        UINT16 handle
                );

/**断开蓝牙连接
* *@param  handle:      连接句柄  
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_disconnect(                                        
                        UINT16 handle
                );

/**蓝牙连接
* *@param  addr:      连接蓝牙地址
* *@param  addr_type: 连接蓝牙地址类型
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_connect(    
                        char *addr ,                                    
                        UINT8 addr_type
                        
                );

#endif