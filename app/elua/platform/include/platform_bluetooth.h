/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_bluetooth.h
 * Author:  liangjian
 * Version: V0.1
 * Date:    2020/09/08
 *

 **************************************************************************/


#ifndef __PLATFORM_BLUETOOTH_H__
#define __PLATFORM_BLUETOOTH_H__

#ifdef LUA_BLUETOOTH_LIB


#include "type.h"

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
} plat_advparam_t;

typedef struct 
{
    UINT8 scanType;            
    UINT16 scanInterval;      
    UINT16 scanWindow;     
    UINT8 filterPolicy; 
    UINT8 own_addr_type;
} plat_scanparam_t;

typedef struct _plat_bt_addr
{
    UINT8 addr[6];
} plat_bt_addr;

/*+\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/

typedef struct _plat_ble_recv_buff
{
	UINT8 uuid_flag;
	UINT16 uuid; ///characteristic
	UINT8 long_uuid[16];
	UINT8 len;
	unsigned char *dataPtr; ///< 返回的数据指针
} plat_ble_recv_buff;
/*-\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/

BOOL platform_ble_open(u8 mode);
BOOL platform_ble_close(void);
BOOL platform_ble_send(u8* data, int len, u16 uuid_c,u16 handle);
BOOL platform_ble_send_string(u8* data, int len, u8*  uuid_c,u16 handle);

/*+\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/
int platform_ble_recv(plat_ble_recv_buff *data);
/*-\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

BOOL platform_ble_set_name(u8* data);
BOOL platform_ble_set_adv_param(plat_advparam_t *param);
BOOL platform_ble_set_adv_data(u8* data,int len);
BOOL platform_ble_set_scanrsp_data(u8* data,int len);
BOOL platform_ble_set_adv_enable(u8 data);
BOOL platform_ble_set_scan_param(plat_scanparam_t *param);
BOOL platform_ble_set_scan_enable(u8 data);
BOOL platform_ble_disconnect(UINT16 handle);
BOOL platform_ble_connect(UINT8 addr_type, u8 *addr);
BOOL platform_ble_add_service(u16 uuid_s);
BOOL platform_ble_add_service_string(u8 *uuid_s);
BOOL platform_ble_add_characteristic(u16 uuid_c,u8 type,u16 permission);
BOOL platform_ble_add_characteristic_string(u8 *uuid_c,u8 type,u16 permission);
BOOL platform_ble_add_descriptor(u16 uuid_d,u8 *value,u16 configurationBits);
BOOL platform_ble_add_descriptor_string(u8 *uuid_d,u8 *value,u16 configurationBits);
BOOL platform_ble_find_characteristic(u16 uuid_s,u16 handle);
BOOL platform_ble_find_characteristic_string(u8* uuid_s,u16 handle);
BOOL platform_ble_find_service(u16 handle);

BOOL platform_ble_open_notification(u16 uuid_c,u16 handle);
BOOL platform_ble_open_notification_string(u8* uuid_c,u16 handle);
BOOL platform_ble_close_notification(u16 uuid_c,u16 handle);
BOOL platform_ble_close_notification_string(u8* uuid_c,u16 handle);
BOOL platform_ble_get_addr(u8* addr);
BOOL platform_ble_set_beacon_data(u8* uuid,u16 major,u16 minor);

BOOL platform_ble_read_state();
#endif

#endif //__PLATFORM_BLUETOOTH_H__


