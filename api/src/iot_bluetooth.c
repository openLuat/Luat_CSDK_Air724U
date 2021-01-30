#include "iot_bluetooth.h"
/****************************** BLUETOOTH ******************************/


/**打开蓝牙
*@param  mode:          蓝牙打开模式
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_bt_open(                                        
                        E_OPENAT_BT_MODE mode
                )
{
    IVTBL(SetBLECallback)(bluetooth_callback);
    return IVTBL(OpenBT)(mode);
}

/**关闭蓝牙
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_bt_close(                                        
                        VOID
                )
{
    return IVTBL(CloseBT)();
}

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
                )
{
    return IVTBL(WriteBLE)(handle,uuid,data,len);
}

/**蓝牙其他操作
*@param  handle:      连接句柄  
*@param  cmd:        功能cmd
*@param  parm:        参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_iotctl(      
                        UINT16 handle,                                  
                        E_OPENAT_BT_CMD cmd,
                        U_OPENAT_BT_IOTCTL_PARAM  param
         
                )
{
    return IVTBL(IotctlBLE)(handle,cmd,param);
}

/**断开蓝牙连接
*@param  handle:      连接句柄  
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_disconnect(                                        
                        UINT16 handle
                )
{
    return IVTBL(DisconnectBLE)(handle);
}

/**蓝牙连接
* *@param  addr:      连接蓝牙地址
* *@param  addr_type: 连接蓝牙地址类型
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_ble_connect(    
                        char *addr ,                                    
                        UINT8 addr_type
                        
                )
{
    return IVTBL(ConnectBLE)(addr_type,addr);
}

