/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: liangjian
  Description: AMOPENAT 开放平台
  Others:
  History: 
    Version： Date:       Author:   Modification:
    V0.1      2020.09.05  liangjain     创建文件
*********************************************************/
#ifndef AM_OPENAT_BULETOOTH_H
#define AM_OPENAT_BULETOOTH_H

#include "am_openat_common.h"
//#include "drv_bluetooth.h"

#define BLE_MAX_DATA_COUNT  244
#define BLE_MAX_ADV_MUBER  31
#define BLE_LONG_UUID_FLAG 16
#define BLE_SHORT_UUID_FLAG 2

typedef struct 
{
    unsigned char id;         ///< event identifier
    char state;               ///< 返回状态
    UINT8 len;                ///< 返回的数据长度
    unsigned char * dataPtr;  ///< 返回的数据指针
    UINT16 uuid;   
    UINT16 handle;
    UINT8 long_uuid[16];
    UINT8 uuid_flag;
}T_OPENAT_BLE_EVENT_PARAM;

typedef struct 
{
   UINT16 AdvMin;            ///< 最小广播间隔
   UINT16 AdvMax;            ///< 最大广播间隔
   UINT8 AdvType;           ///< 广播类型
   UINT8 OwnAddrType;       ///< 广播本地地址类型
   UINT8 DirectAddrType;    ///< 定向地址类型
   char *DirectAddr;        ///< 定向地址
   UINT8 AdvChannMap;       ///< 广播channel map,3个bit，分别对应37， 38， 39信道 
   UINT8 AdvFilter;         ///< 广播过滤策略
}T_OPENAT_BLE_ADV_PARAM;

typedef struct 
{
    UINT8 scanType;        //  扫描类型   
    UINT16 scanInterval;   //  扫描间隔
    UINT16 scanWindow;     //  扫描窗口
    UINT8 filterPolicy;    //  扫描过滤策略
    UINT8 own_addr_type;   //  本地地址类型
} T_OPENAT_BLE_SCAN_PARAM;

typedef enum
{
    BLE_SET_NAME = 0x01,    ///< 设置BLE 广播名称
    BLE_SET_ADV_PARAM,		///< 设置BLE 广播广播参数
    BLE_SET_ADV_DATA,		///< 设置BLE 广播广播包数据
    BLE_SET_SCANRSP_DATA,	///< 设置BLE 广播响应包数据
	BLE_SET_ADV_ENABLE,		///< 是否使能广播
	BLE_SET_SCAN_ENABLE,		///< 是否使能扫描
	BLE_READ_STATE,			///< 读BLE 是否使能
    BLE_ADD_SERVICE,        ///< 添加服务
    BLE_ADD_CHARACTERISTIC, ///< 添加特征
    BLE_ADD_DESCRIPTOR,     ///< 添加描述
    BLE_FIND_SERVICE,       ///< 发现服务
    BLE_FIND_CHARACTERISTIC,///< 发现特征
    BLE_OPEN_NOTIFICATION,  ///< 打开通知
    BLE_CLOSE_NOTIFICATION,  ///< 关闭通知
    BLE_GET_ADDR,           ///< 获取蓝牙MAC地址
    BLE_SET_BEACON_DATA,     ///< 设置beacon数据
    BLE_SET_SCAN_PARAM,		///< 设置BLE扫描参数
} E_OPENAT_BT_CMD;

typedef enum
{
    BLE_SLAVE = 0,    ///< 设置BLE从模式
    BLE_MASTER,	      ///< 设置BLE主模式
} E_OPENAT_BT_MODE;

typedef enum
{
    UUID_SHORT = 0,    // 16位uuid
    UUID_LONG,	       // 128位uuid
} E_OPENAT_BLE_UUID_FLAG;

typedef struct 
{
    E_OPENAT_BLE_UUID_FLAG uuid_type;
    union {
        UINT16 uuid_short;
        UINT8 uuid_long[16];
    };
}T_OPENAT_BLE_UUID;

typedef struct 
{
    T_OPENAT_BLE_UUID uuid;
    UINT8   attvalue;//属性值
    UINT16  permisssion;//权限
}T_OPENAT_BLE_CHARACTERISTIC_PARAM;

typedef struct 
{
    T_OPENAT_BLE_UUID uuid;
    UINT8   value[255];//属性
    UINT16  configurationBits;//属性
}T_OPENAT_BLE_DESCRIPTOR_PARAM;

typedef struct 
{
    UINT8 		data[BLE_MAX_ADV_MUBER]; 
    UINT8       len;
}T_OPENAT_BLE_ADV_DATA;

typedef struct 
{
    UINT8 uuid[16];
    UINT16 major;
    UINT16 minor; 
}T_OPENAT_BLE_BEACON_DATA;

typedef union {
    T_OPENAT_BLE_ADV_PARAM  *advparam;   ///< 设置BLE 广播参数
    T_OPENAT_BLE_SCAN_PARAM  *scanparam;   ///< 设置BLE 扫描参数
    T_OPENAT_BLE_ADV_DATA   *advdata;    ///< 广播包数据、响应包数据
    UINT8 	*data;    ///< 设置BLE 广播名称、获取蓝牙MAC地址
    UINT8       advEnable;          ///< 是否使能广播、使能扫描
    T_OPENAT_BLE_UUID  *uuid;   ///< 添加服务、发现特征、打开通知、关闭通知
    T_OPENAT_BLE_CHARACTERISTIC_PARAM  *characteristicparam;   ///< 添加特征
    T_OPENAT_BLE_DESCRIPTOR_PARAM   *descriptorparam;   ///< 添加描述
    T_OPENAT_BLE_BEACON_DATA   *beacondata;   ///< 设置beacon数据
}U_OPENAT_BT_IOTCTL_PARAM;

typedef enum{
    OPENAT_BT_ME_ON_CNF = 1,
    OPENAT_BT_ME_OFF_CNF,
    OPENAT_BT_VISIBILE_CNF,
    OPENAT_BT_HIDDEN_CNF,
    OPENAT_BT_SET_LOCAL_NAME_RES,
    OPENAT_BT_SET_LOCAL_ADDR_RES,
    OPENAT_BT_INQ_DEV_NAME,
    OPENAT_BT_INQ_COMP_CNF,
    OPENAT_BT_INQUIRY_CANCEL,
    OPENAT_BT_DEV_PAIR_COMPLETE,
    OPENAT_BT_DELETE_DEVICE_RES,
    OPENAT_BT_DEV_PIN_REQ,
    OPENAT_BT_SSP_NUM_IND,
    OPENAT_BT_SPP_CONNECT_IND_will_del,
    OPENAT_BT_SPP_DISCONNECT_IND_will_del,
    OPENAT_BT_SPP_DATA_RECIEVE_IND,
    OPENAT_BLE_SET_PUBLIC_ADDR = 51,
    OPENAT_BLE_SET_RANDOM_ADDR,
    OPENAT_BLE_ADD_WHITE_LIST,
    OPENAT_BLE_REMOVE_WHITE_LIST,
    OPENAT_BLE_CLEAR_WHITE_LIST,
    OPENAT_BLE_CONNECT,
    OPENAT_BLE_DISCONNECT,
    OPENAT_BLE_UPDATA_CONNECT,
    OPENAT_BLE_SET_ADV_PARA,
    OPENAT_BLE_SET_ADV_DATA,
    OPENAT_BLE_SET_ADV_ENABLE,
    OPENAT_BLE_SET_ADV_SCAN_RSP,
    OPENAT_BLE_SET_SCAN_PARA,
    OPENAT_BLE_SET_SCAN_ENABLE,
    OPENAT_BLE_SET_SCAN_DISENABLE,
    OPENAT_BLE_SET_SCAN_REPORT,
    OPENAT_BLE_CONNECT_IND,
    OPENAT_BLE_DISCONNECT_IND,
    OPENAT_BLE_FIND_CHARACTERISTIC_IND,
    OPENAT_BLE_FIND_SERVICE_IND,
    OPENAT_BLE_FIND_CHARACTERISTIC_UUID_IND,
    OPENAT_BLE_RECV_DATA = 100,
}E_OPENAT_BT_EVENT;

typedef VOID (*F_BT_CB)(VOID* param);
void OPENAT_SetBLECallback(F_BT_CB handler);
BOOL OPENAT_OpenBT(E_OPENAT_BT_MODE mode);
BOOL OPENAT_CloseBT(void);
BOOL OPENAT_WriteBLE(UINT16 handle,T_OPENAT_BLE_UUID uuid,char *data,UINT8 len);
BOOL OPENAT_IotctlBLE(UINT16 handle,E_OPENAT_BT_CMD cmd,U_OPENAT_BT_IOTCTL_PARAM param);
BOOL OPENAT_DisconnectBLE(UINT16 handle);
BOOL OPENAT_ConnectBLE(UINT8 addr_type, char *addr);



#endif /* AM_OPENAT_FS_H */


