/***************
	demo_bluetooth
****************/


#include "string.h"
#include "iot_debug.h"
#include "iot_bluetooth.h"

HANDLE ble_test_handle = NULL;

typedef struct ble_report_info
{
    unsigned char  eventid;
    char    state;
} ble_report_info_t;


void bluetooth_callback(T_OPENAT_BLE_EVENT_PARAM *result)
{
    ble_report_info_t *msg = NULL;
    msg = iot_os_malloc(sizeof(ble_report_info_t));
	msg->eventid = result->id;
	msg->state  =  result->state;
	switch(result->id)
	{
        case OPENAT_BT_ME_ON_CNF:
            iot_os_send_message(ble_test_handle,msg);
		    break;
		default:
            iot_os_free(msg);
		    break;
	}
}

BOOL ble_poweron(VOID)
{
    iot_debug_print("[bluetooth]bt poweron");
    iot_bt_open(BLE_SLAVE);//打开蓝牙
    return TRUE;
}

BOOL advertising(VOID)
{
    U_OPENAT_BT_IOTCTL_PARAM  param1;
    //U_OPENAT_BT_IOTCTL_PARAM  param2;
    U_OPENAT_BT_IOTCTL_PARAM  param3;
    UINT8 uuid[16] = {0xAB,0x81,0x90,0xD5,0xD1,0x1E,0x49,0x41,0xAC,0xC4,0x42,0xF3,0x05,0x10,0xB4,0x08};
    T_OPENAT_BLE_BEACON_DATA beacondata = {0};
    memcpy(beacondata.uuid,uuid,16);
    beacondata.major = 10107;
    beacondata.minor = 50179;
    //T_OPENAT_BLE_ADV_PARAM advparam = {0x80,0xa0,0,0,0,"11:22:33:44:55:66",0x07,0};//广播参数
    iot_debug_print("[bluetooth]bt advertising");

    param1.beacondata = iot_os_malloc(sizeof(T_OPENAT_BLE_BEACON_DATA));
    memcpy(param1.beacondata,&beacondata,sizeof(T_OPENAT_BLE_BEACON_DATA));
    iot_ble_iotctl(0,BLE_SET_BEACON_DATA,param1);//设置beacon数据
    iot_os_free(param1.beacondata);
    param1.beacondata = NULL;
/*
    param2.advparam = iot_os_malloc(sizeof(T_OPENAT_BLE_ADV_PARAM));
    memcpy(param2.advparam,&advparam,sizeof(T_OPENAT_BLE_ADV_PARAM));
    iot_ble_iotctl(0,BLE_SET_ADV_PARAM,param2);//设置广播参数
    iot_os_free(param2.advparam);
    param2.advparam = NULL;
*/
    iot_os_sleep(1000);
    param3.advEnable = 1;
    iot_ble_iotctl(0,BLE_SET_ADV_ENABLE,param3);//打开广播  
    return TRUE;
}

VOID ble_test(VOID)
{
    ble_report_info_t *msg = NULL;
    //1.  打开蓝牙
    ble_poweron();
    iot_os_wait_message(ble_test_handle,&msg);//等待蓝牙打开
    if(msg->eventid == OPENAT_BT_ME_ON_CNF)
    {
        iot_os_free(msg);
        msg = NULL;
        //2.广播蓝牙
        advertising();
    }
}


int appimg_enter(void *param)
{
    iot_debug_print("[bluetooth]appimg_enter");
    ble_test_handle = iot_os_create_task(ble_test, NULL, 4096, 24, OPENAT_OS_CREATE_DEFAULT, "bluetooth");
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[bluetooth]appimg_exit");
}
