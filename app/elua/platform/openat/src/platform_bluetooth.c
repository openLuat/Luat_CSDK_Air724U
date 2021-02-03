
#ifdef LUA_BLUETOOTH_LIB
#include <string.h>

#include "malloc.h"

#include "assert.h"
#include "lplatform.h"
#include "platform_bluetooth.h"
#include "am_openat_bluetooth.h"

typedef struct rtos_BT_ADDRESS
{
	uint8 addr[6];
} plat_BT_ADDRESS_t;

typedef struct rtos_ble_scan_report_info
{
	UINT8 name_length;
	UINT8 name[32];
	UINT8 addr_type;
	plat_BT_ADDRESS_t bdAddress;
	UINT8 event_type;
	UINT8 data_length;
	UINT8 manu_data[32];
	UINT8 manu_len;
	UINT8 raw_data[32];
	UINT8 rssi;
} plat_ble_scan_report_info_t;

/*+\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

#define PLA_BLE_RX_BUF_SIZE 2048
static uint8 BLERxBuff[PLA_BLE_RX_BUF_SIZE];
static CycleQueue ble_recv_queue = {
	BLERxBuff,
	PLA_BLE_RX_BUF_SIZE,
	0,
	0,
	1,
	0,
	0,
};
/*-\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

static void bluetooth_callback(T_OPENAT_BLE_EVENT_PARAM *result)
{
	PlatformMsgData rtosmsg = {0};
	unsigned char *bleData = NULL;
	rtosmsg.blueData.eventid = result->id;
	rtosmsg.blueData.state = result->state;
#if 1
	switch (result->id)
	{
	case OPENAT_BLE_RECV_DATA:
		if ((result->len != 0) && (result->dataPtr != NULL))
		{
			/*+\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/

			plat_ble_recv_buff BLERxRecvOne = {0};
			BLERxRecvOne.uuid_flag = result->uuid_flag;
			BLERxRecvOne.uuid = result->uuid;
			memcpy(BLERxRecvOne.long_uuid, result->long_uuid, sizeof(result->long_uuid));
			BLERxRecvOne.len = result->len;
			BLERxRecvOne.dataPtr = malloc(result->len);
			memcpy(BLERxRecvOne.dataPtr, result->dataPtr, result->len);

			/*+\NEW\czm\2020.11.27\代码评审: 插入队列前判断剩余空间，加临界区保护*/
			HANDLE crihand = OPENAT_enter_critical_section();
			//如果队列中剩余空间不够，则抛弃该包数据
			if (QueueGetFreeSpace(&ble_recv_queue) > sizeof(plat_ble_recv_buff))
			{
				QueueInsert(&ble_recv_queue, (uint8 *)&BLERxRecvOne, sizeof(plat_ble_recv_buff));
			}
			else
			{
				if (BLERxRecvOne.dataPtr)
					free(BLERxRecvOne.dataPtr);
				OPENAT_print("DRV_BT:  ble_recv_queue Insufficient queue space!");
			}
			OPENAT_exit_critical_section(crihand);
			/*-\NEW\czm\2020.11.27\代码评审: 插入队列前判断剩余空间，加临界区保护*/

			// rtosmsg.blueData.len = result->len;
			// rtosmsg.blueData.uuid_flag = result->uuid_flag;
			// rtosmsg.blueData.uuid = result->uuid;
			// memcpy(rtosmsg.blueData.long_uuid, result->long_uuid, sizeof(result->long_uuid));
			/*+\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/

			/*+\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

			//QueueInsert(&ble_recv_queue, result->dataPtr, result->len);
			//rtosmsg.blueData.pData = bleData;

			// bleData = OPENAT_malloc(result->len);
			// memcpy(bleData, result->dataPtr, result->len);
			// rtosmsg.blueData.pData = bleData;
			/*-\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/
		}
		break;
	case OPENAT_BLE_SET_SCAN_REPORT:
		if (result->dataPtr != NULL)
		{
			bleData = OPENAT_malloc(sizeof(plat_ble_scan_report_info_t));
			memcpy(bleData, result->dataPtr, sizeof(plat_ble_scan_report_info_t));
			rtosmsg.blueData.pData = (char *)bleData;
		}
		break;
	case OPENAT_BLE_CONNECT_IND:
	case OPENAT_BLE_CONNECT:
		rtosmsg.blueData.handle = result->handle;
		rtosmsg.blueData.pData = result->dataPtr;
		break;
	case OPENAT_BLE_FIND_SERVICE_IND:
		rtosmsg.blueData.uuid = result->uuid;
		rtosmsg.blueData.pData = result->dataPtr;
		break;
	case OPENAT_BLE_FIND_CHARACTERISTIC_UUID_IND:
		rtosmsg.blueData.uuid_flag = result->uuid_flag;
		rtosmsg.blueData.uuid = result->uuid;
		memcpy(rtosmsg.blueData.long_uuid, result->long_uuid, sizeof(result->long_uuid));
		rtosmsg.blueData.pData = result->dataPtr;
		break;
	default:
		rtosmsg.blueData.pData = result->dataPtr;
		break;
	}
#endif
	if ((result->id == OPENAT_BLE_RECV_DATA) && (result->state != 0))
	{
		//IVTBL(print)("DRV_BT:  bluetooth_callback data %p %d", rtosmsg.blueData.pData, *rtosmsg.blueData.pData);
	}

	platform_rtos_send(MSG_ID_RTOS_BLUETOOTH, &rtosmsg);
}
/*+\new\wj\2020.4.26\实现录音接口*/

BOOL platform_ble_open(u8 mode)
{
	IVTBL(SetBLECallback)(bluetooth_callback);
	return IVTBL(OpenBT)(mode);
}
BOOL platform_ble_close(void)
{
	return IVTBL(CloseBT)();
}

BOOL platform_ble_send(u8 *data, int len, u16 uuid_c, u16 handle)
{
	T_OPENAT_BLE_UUID uuid = {0};
	uuid.uuid_short = uuid_c;
	uuid.uuid_type = UUID_SHORT;
	BOOL state = IVTBL(WriteBLE)(handle,uuid,data,len);
	return state;
}
BOOL platform_ble_send_string(u8 *data, int len, u8 *uuid_c, u16 handle)
{
	T_OPENAT_BLE_UUID uuid = {0};
	uuid.uuid_type = UUID_LONG;
	AppConvertHexToBin(uuid_c, strlen(uuid_c), uuid.uuid_long);
	BOOL state = IVTBL(WriteBLE)(handle,uuid,data,len);
    return state;
}

/*+\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/
/*+\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/
int platform_ble_recv(plat_ble_recv_buff *data)
{
	static plat_ble_recv_buff recv_buff = {0}; //保存一次读取的所有数据
	static UINT8 buf_index = 0;				   //保存数据被取走了多少
	if (data == NULL || data->dataPtr == NULL)
	{
		OPENAT_print("DRV_BT:  platform_ble_recv data == NULL || data->dataPtr == NULL");
		return -1;
	}

	if ((buf_index == recv_buff.len) && recv_buff.len != 0) //当前数据包内容全部取完了
	{
		OPENAT_print("DRV_BT:  platform_ble_recv (buf_index == recv_buff.len) && recv_buff.len != 0");
		recv_buff.len = 0;
		buf_index = 0; //重装偏移
		return 0;	   //返回一个0，告诉用户这个数据包已经结束。下一次读取，取到的就是下一个数据包
	}

	if (recv_buff.len == 0)
	{
		if (recv_buff.dataPtr != NULL)
			free(recv_buff.dataPtr);
		memset(&recv_buff, 0, sizeof(plat_ble_recv_buff));
		int status = QueueDelete(&ble_recv_queue, (uint8 *)&recv_buff, sizeof(plat_ble_recv_buff)); //读下一包数据
		buf_index = 0;																				//重装偏移
		if (status == 0)																			//队列中没有数据了
			return 0;
	}

	UINT8 recvlen = 0; //实际能取到的数据包长度

	if ((recv_buff.len - buf_index) > data->len) //如果剩下的数据长度足够
		recvlen = data->len;					 //要多少取多少
	else										 //如果剩下的数据长度不够用户要读取的数据长度
		recvlen = recv_buff.len - buf_index;	 //有多少取多少

	memcpy(data->dataPtr, recv_buff.dataPtr + buf_index, recvlen); //拷贝指定长度的数据
	buf_index += recvlen;										   //指向现在的数据位置

	data->uuid_flag = recv_buff.uuid_flag; //当前数据包的UUID标志位

	if (recv_buff.uuid_flag == 0) //转存当前uuid
		data->uuid = recv_buff.uuid;
	else
		memcpy(data->long_uuid, recv_buff.long_uuid, sizeof(recv_buff.long_uuid));

	OPENAT_print("DRV_BT:  platform_ble_recv QueueDelete recvlen:%d", recvlen);
	return recvlen;
}
/*-\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报，不需要主动上报了*/
/*-\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

BOOL platform_ble_set_name(u8 *data)
{
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_NAME,(U_OPENAT_BT_IOTCTL_PARAM)data);
	return state;
}

BOOL platform_ble_set_adv_param(plat_advparam_t *param)
{
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_ADV_PARAM, (U_OPENAT_BT_IOTCTL_PARAM)((T_OPENAT_BLE_ADV_PARAM *)param));
	return state;
}

BOOL platform_ble_set_scan_param(plat_scanparam_t *param)
{
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_SCAN_PARAM, (U_OPENAT_BT_IOTCTL_PARAM)((T_OPENAT_BLE_SCAN_PARAM *)param));
	return state;
}

BOOL platform_ble_set_adv_data(u8 *data, int len)
{
	T_OPENAT_BLE_ADV_DATA advdata;
	advdata.len = len;
	memcpy(advdata.data,data,len);
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_ADV_DATA, (U_OPENAT_BT_IOTCTL_PARAM)&advdata);
	return state;
}

BOOL platform_ble_set_scanrsp_data(u8 *data, int len)
{
	T_OPENAT_BLE_ADV_DATA scanrspdata;
	scanrspdata.len = len;
	memcpy(scanrspdata.data,data,len);
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_SCANRSP_DATA, (U_OPENAT_BT_IOTCTL_PARAM)&scanrspdata);
	return state;
}

BOOL platform_ble_set_adv_enable(u8 enable)
{
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_ADV_ENABLE, (U_OPENAT_BT_IOTCTL_PARAM)enable);
	return state;
}

BOOL platform_ble_set_scan_enable(u8 enable)
{
	BOOL state = IVTBL(IotctlBLE)(0,BLE_SET_SCAN_ENABLE, (U_OPENAT_BT_IOTCTL_PARAM)enable);
	return state;
}

BOOL platform_ble_read_state()
{
	UINT8 count = 0;
	return IVTBL(IotctlBLE)(0,BLE_READ_STATE,(U_OPENAT_BT_IOTCTL_PARAM)count);
}

BOOL platform_ble_disconnect(UINT16 handle)
{
	return IVTBL(DisconnectBLE)(handle);
}

BOOL platform_ble_connect(UINT8 addr_type, u8 *addr)
{
	return IVTBL(ConnectBLE)(addr_type, addr);
}

BOOL platform_ble_add_service(u16 uuid_s)
{
	T_OPENAT_BLE_UUID uuid;
	uuid.uuid_short = uuid_s;
	uuid.uuid_type = UUID_SHORT;
	return IVTBL(IotctlBLE)(0,BLE_ADD_SERVICE, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}
BOOL platform_ble_add_service_string(u8 *uuid_s)
{
	UINT8 uuid_long[16] = {0};
	T_OPENAT_BLE_UUID uuid;
	
	AppConvertHexToBin(uuid_s, strlen(uuid_s), uuid_long);
	memcpy(uuid.uuid_long,uuid_long,16);
	uuid.uuid_type = UUID_LONG;

	return IVTBL(IotctlBLE)(0,BLE_ADD_SERVICE, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_add_characteristic(u16 uuid_c, u8 type,u16 permission)
{
	T_OPENAT_BLE_CHARACTERISTIC_PARAM uuid;
	uuid.uuid.uuid_short = uuid_c;
	uuid.uuid.uuid_type = UUID_SHORT;
	uuid.attvalue = type;
	uuid.permisssion = permission;
	return IVTBL(IotctlBLE)(0,BLE_ADD_CHARACTERISTIC, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}
BOOL platform_ble_add_characteristic_string(u8 *uuid_c, u8 type,u16 permission)
{

	T_OPENAT_BLE_CHARACTERISTIC_PARAM uuid;
	AppConvertHexToBin(uuid_c, strlen(uuid_c), uuid.uuid.uuid_long);
	uuid.uuid.uuid_type = UUID_LONG;
	uuid.attvalue = type;
	uuid.permisssion = permission;
	return IVTBL(IotctlBLE)(0,BLE_ADD_CHARACTERISTIC, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_add_descriptor(u16 uuid_d,u8 *value,u16 configurationBits)
{
	T_OPENAT_BLE_DESCRIPTOR_PARAM uuid;
	uuid.uuid.uuid_short = uuid_d;
	uuid.uuid.uuid_type = UUID_SHORT;
	if((uuid_d == 0x2901)||(uuid_d == 0x2904))
		memcpy(uuid.value,value,strlen(value));
	else
		uuid.configurationBits = configurationBits;
	return IVTBL(IotctlBLE)(0,BLE_ADD_DESCRIPTOR, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}
BOOL platform_ble_add_descriptor_string(u8 *uuid_d,u8 *value,u16 configurationBits)
{
	T_OPENAT_BLE_DESCRIPTOR_PARAM uuid;
	AppConvertHexToBin(uuid_d, strlen(uuid_d), uuid.uuid.uuid_long);
	uuid.uuid.uuid_type = UUID_SHORT;
	if((uuid_d == 0x2901)||(uuid_d == 0x2904))
		memcpy(uuid.value,value,strlen(value));
	else
		uuid.configurationBits = configurationBits;
	return IVTBL(IotctlBLE)(0,BLE_ADD_DESCRIPTOR, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_find_characteristic(u16 uuid_s, u16 handle)
{
	T_OPENAT_BLE_UUID uuid;
	uuid.uuid_short = uuid_s;
	uuid.uuid_type = UUID_SHORT;
	return IVTBL(IotctlBLE)(handle,BLE_FIND_CHARACTERISTIC, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_find_characteristic_string(u8 *uuid_s, u16 handle)
{
	UINT8 uuid_long[16] = {0};
	T_OPENAT_BLE_UUID uuid;
	AppConvertHexToBin(uuid_s, strlen(uuid_s), uuid_long);
	memcpy(uuid.uuid_long,uuid_long,16);
	uuid.uuid_type = UUID_LONG;
	return IVTBL(IotctlBLE)(handle,BLE_FIND_CHARACTERISTIC, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_find_service(u16 handle)
{
	UINT8 count = 0;
	return IVTBL(IotctlBLE)(handle,BLE_FIND_SERVICE, (U_OPENAT_BT_IOTCTL_PARAM)count);
}

BOOL platform_ble_open_notification(u16 uuid_c, u16 handle)
{
	T_OPENAT_BLE_UUID uuid;
	uuid.uuid_short = uuid_c;
	uuid.uuid_type = UUID_SHORT;
	return IVTBL(IotctlBLE)(handle,BLE_OPEN_NOTIFICATION, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_open_notification_string(u8 *uuid_c, u16 handle)
{
	UINT8 uuid_long[16] = {0};
	T_OPENAT_BLE_UUID uuid;
	AppConvertHexToBin(uuid_c, strlen(uuid_c), uuid_long);
	memcpy(uuid.uuid_long,uuid_long,16);
	uuid.uuid_type = UUID_LONG;
	return IVTBL(IotctlBLE)(handle,BLE_OPEN_NOTIFICATION, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_close_notification(u16 uuid_c, u16 handle)
{
	T_OPENAT_BLE_UUID uuid;
	uuid.uuid_short = uuid_c;
	uuid.uuid_type = UUID_SHORT;
	return IVTBL(IotctlBLE)(handle,BLE_CLOSE_NOTIFICATION, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_close_notification_string(u8 *uuid_c, u16 handle)
{
	UINT8 uuid_long[16] = {0};
	T_OPENAT_BLE_UUID uuid;
	AppConvertHexToBin(uuid_c, strlen(uuid_c), uuid_long);
	memcpy(uuid.uuid_long,uuid_long,16);
	uuid.uuid_type = UUID_LONG;
	return IVTBL(IotctlBLE)(handle,BLE_CLOSE_NOTIFICATION, (U_OPENAT_BT_IOTCTL_PARAM)&uuid);
}

BOOL platform_ble_get_addr(u8* addr)
{
	return IVTBL(IotctlBLE)(0,BLE_GET_ADDR,(U_OPENAT_BT_IOTCTL_PARAM)addr);
}

BOOL platform_ble_set_beacon_data(u8* uuid,u16 major,u16 minor)
{
	UINT8 uuid_long[16] = {0};
	T_OPENAT_BLE_BEACON_DATA beacondata;
	AppConvertHexToBin(uuid, strlen(uuid), uuid_long);
	memcpy(beacondata.uuid,uuid_long,16);
	beacondata.major = major;
	beacondata.minor = minor;
	return IVTBL(IotctlBLE)(0,BLE_SET_BEACON_DATA,(U_OPENAT_BT_IOTCTL_PARAM)&beacondata);
}

#endif