/***************
	demo_bluetooth
****************/


#include "string.h"
#include "iot_debug.h"
#include "iot_bluetooth.h"

HANDLE  ble_test_handle = NULL;

typedef struct BT_ADDRESS
{
    uint8 addr[6];
} BT_ADDRESS_t;

typedef struct _ble_scan_report_info
{
    UINT8 name_length;
    UINT8 name[BLE_MAX_ADV_MUBER+1];
    UINT8 addr_type;
    BT_ADDRESS_t bdAddress;
    UINT8 event_type;
    UINT8 data_length;
    UINT8 manu_data[BLE_MAX_ADV_MUBER+1];
    UINT8 manu_len;
    UINT8 raw_data[BLE_MAX_ADV_MUBER+1];
    UINT8 rssi;
} ble_scan_report_info;

typedef struct ble_report_info
{
    unsigned char  eventid;
    char    state;
    UINT8   bleRcvBuffer[BLE_MAX_DATA_COUNT];
    UINT8    len;
    UINT8 enable;
    UINT16 uuid;
    UINT16 handle;
    UINT8 long_uuid[BLE_LONG_UUID_FLAG];
    UINT8 uuid_flag;
} ble_report_info_t;

UINT16 connect_handle = 0xff;//连接句柄

static void AppConvertBinToHex(
    UINT8 *bin_ptr, // in: the binary format string
    UINT16 length,  // in: the length of binary string
    UINT8 *hex_ptr  // out: pointer to the hexadecimal format string
)
{
    UINT8 semi_octet = 0;
    int32 i = 0;

    for (i = 0; i < length; i++)
    {
        // get the high 4 bits
        semi_octet = (UINT8)((bin_ptr[i] & 0xF0) >> 4);
        if (semi_octet <= 9) //semi_octet >= 0
        {
            *hex_ptr = (UINT8)(semi_octet + '0');
        }
        else
        {
            *hex_ptr = (UINT8)(semi_octet + 'A' - 10);
        }

        hex_ptr++;

        // get the low 4 bits
        semi_octet = (UINT8)(bin_ptr[i] & 0x0f);
        if (semi_octet <= 9) // semi_octet >= 0
        {
            *hex_ptr = (UINT8)(semi_octet + '0');
        }
        else
        {
            *hex_ptr = (UINT8)(semi_octet + 'A' - 10);
        }
        hex_ptr++;
    }
}

void bluetooth_callback(T_OPENAT_BLE_EVENT_PARAM *result)
{
    char scanaddr[20] = {0};
    ble_scan_report_info *ble_scanInfo = NULL;
    ble_report_info_t *msg = NULL;
    U_OPENAT_BT_IOTCTL_PARAM  param = {0};
    msg = iot_os_malloc(sizeof(ble_report_info_t));
	msg->eventid = result->id;
	msg->state  =  result->state;
    char manu_data[BLE_MAX_ADV_MUBER*2+1] = {0};
    char raw_data[BLE_MAX_ADV_MUBER*2+1] = {0};
    char long_uuid[BLE_LONG_UUID_FLAG*2] = {0};
	switch(result->id)
	{
		case OPENAT_BLE_RECV_DATA:
			if((result->len != 0) && (result->dataPtr != NULL))
			{
                msg->len  =  result->len;
                msg->uuid_flag  =  result->uuid_flag;//1:128位uuid 0:16位uuid
                msg->uuid  =  result->uuid;//16位uuid
                memcpy(msg->long_uuid, result->long_uuid, sizeof(result->long_uuid));//128位uuid
                memset(msg->bleRcvBuffer,0,sizeof(msg->bleRcvBuffer));
				memcpy(msg->bleRcvBuffer, result->dataPtr,result->len);  
                iot_os_send_message(ble_test_handle,msg);
			}
			break;
        case OPENAT_BLE_CONNECT:
            msg->handle  =  result->handle;
            iot_os_send_message(ble_test_handle,msg);
            break;
        case OPENAT_BLE_DISCONNECT:
            iot_debug_print("[bluetooth]bt disconnect");
            iot_os_free(msg);
            break;
        case OPENAT_BT_ME_ON_CNF:
        case OPENAT_BLE_SET_SCAN_ENABLE:
        case OPENAT_BLE_FIND_CHARACTERISTIC_IND:
            iot_os_send_message(ble_test_handle,msg);
		    break;
        case OPENAT_BLE_SET_SCAN_REPORT:
            ble_scanInfo = (ble_scan_report_info *)result->dataPtr;
            iot_debug_print("[bluetooth]bt scan name %s",ble_scanInfo->name);//蓝牙名称
            iot_debug_print("[bluetooth]bt scan rssi %d",(int8)ble_scanInfo->rssi);//信号强度
            sprintf(scanaddr, "%02x:%02x:%02x:%02x:%02x:%02x", ble_scanInfo->bdAddress.addr[0], ble_scanInfo->bdAddress.addr[1], ble_scanInfo->bdAddress.addr[2], ble_scanInfo->bdAddress.addr[3], ble_scanInfo->bdAddress.addr[4], ble_scanInfo->bdAddress.addr[5]);
            iot_debug_print("[bluetooth]bt scan addr %s",scanaddr);//蓝牙地址
            iot_debug_print("[bluetooth]bt scan addr_type %d",ble_scanInfo->addr_type);//地址种类

            AppConvertBinToHex(ble_scanInfo->manu_data,ble_scanInfo->manu_len,manu_data);
            manu_data[ble_scanInfo->manu_len*2] = '\0';
            iot_debug_print("[bluetooth]bt scan manu_data %s",manu_data);//厂商数据
            AppConvertBinToHex(ble_scanInfo->raw_data,ble_scanInfo->data_length,raw_data);
            raw_data[ble_scanInfo->data_length*2] = '\0';
            iot_debug_print("[bluetooth]bt scan raw_data %s",raw_data);//广播原始数据
            
            if(memcmp(ble_scanInfo->name,"Luat_Air724UG",strlen("Luat_Air724UG")) == 0) //连接的蓝牙名称
            {
                param.advEnable = 0;
                iot_ble_iotctl(0,BLE_SET_SCAN_ENABLE,param);//关闭扫描
                iot_ble_connect(scanaddr,ble_scanInfo->addr_type);//连接蓝牙
            }
            iot_os_free(msg);
		    break;
        case OPENAT_BLE_FIND_SERVICE_IND:
            iot_debug_print("[bluetooth]bt service uuid %x",result->uuid);
            iot_os_free(msg);
            break;
        case OPENAT_BLE_FIND_CHARACTERISTIC_UUID_IND:
            if(result->uuid_flag == UUID_SHORT)//1:128位uuid 0:16位uuid
            {
                iot_debug_print("[bluetooth]bt characteriatic uuid %x",result->uuid);
            }
            else if(result->uuid_flag == UUID_LONG)
            {
                AppConvertBinToHex(result->long_uuid,sizeof(result->long_uuid),long_uuid);
                iot_debug_print("[bluetooth]bt characteriatic uuid %s",long_uuid);
            }
            iot_os_free(msg);
            break;
		default:
		    break;
	}
}

BOOL ble_poweron(VOID)
{
    iot_debug_print("[bluetooth]bt poweron");
    iot_bt_open(BLE_MASTER);//打开蓝牙
    return TRUE;
}

BOOL scan(VOID)
{
    int i;
    //U_OPENAT_BT_IOTCTL_PARAM  param1 = {0};
    U_OPENAT_BT_IOTCTL_PARAM  param2 = {0};
    ble_report_info_t *msg = NULL;
    UINT8 connect_addr_type = 0;
    char connect_scanaddr[20] = {0};
    //T_OPENAT_BLE_SCAN_PARAM scanparam = {1,0x30,0x06,0,0};//广播参数
    iot_debug_print("[bluetooth]bt scan");
    param2.advEnable = 1;
/*
    param1.scanparam = iot_os_malloc(sizeof(T_OPENAT_BLE_SCAN_PARAM));
    memcpy(param1.scanparam,&scanparam,sizeof(T_OPENAT_BLE_SCAN_PARAM));
    iot_ble_iotctl(0,BLE_SET_SCAN_PARAM,param1);//设置广播参数
    if(param1.scanparam != NULL)
        iot_os_free(param1.scanparam);
    param1.scanparam = NULL;
*/
    iot_ble_iotctl(0,BLE_SET_SCAN_ENABLE,param2);//打开扫描
    iot_os_wait_message(ble_test_handle,&msg);
    if(msg->eventid != OPENAT_BLE_SET_SCAN_ENABLE)//等待扫描使能成功
    {
        if(msg != NULL)
            iot_os_free(msg);
        msg = NULL;
        return FALSE;
    }
    if(msg != NULL)
        iot_os_free(msg);
    msg = NULL;
    return TRUE;
}


BOOL ble_data_trans(VOID)
{
    T_OPENAT_BLE_UUID uuid = {0};
    T_OPENAT_BLE_UUID uuid_s = {0};
    T_OPENAT_BLE_UUID uuid_c = {0};
    ble_report_info_t *msg = NULL;
    char *bleRcvBuffer = NULL;
    U_OPENAT_BT_IOTCTL_PARAM  param1 = {0};
    U_OPENAT_BT_IOTCTL_PARAM  param2 = {0};
    U_OPENAT_BT_IOTCTL_PARAM  param3 = {0};
    
    uuid.uuid_short = 0xfee1;
    uuid.uuid_type = UUID_SHORT;
    uuid_s.uuid_short = 0xfee0;
    uuid_s.uuid_type = UUID_SHORT;
    uuid_c.uuid_short = 0xfee2;
    uuid_c.uuid_type = UUID_SHORT;
    //UINT8 uuid_s[16] = {0xF2, 0xC3, 0xF0, 0xAE, 0xA9, 0xFA, 0x15, 0x8C, 0x9D, 0x49, 0xAE, 0x73, 0x71, 0x0A, 0x81, 0xE7};
    //UINT8 uuid_c[16] = {0x9F, 0x9F, 0x00, 0xC1, 0x58, 0xBD, 0x32, 0xB6, 0x9E, 0x4C, 0x21, 0x9C, 0xC9, 0xD6, 0xF8, 0xBE};
    iot_debug_print("[bluetooth]bt data trans");
    
    /*连接成功，发现包含的服务及特征，并打开通知*/
    iot_ble_iotctl(connect_handle,BLE_FIND_SERVICE,param1);//发现服务
    
    param2.uuid = iot_os_malloc(sizeof(T_OPENAT_BLE_UUID));
    memcpy(param2.uuid,&uuid_s,sizeof(T_OPENAT_BLE_UUID));
    iot_ble_iotctl(connect_handle,BLE_FIND_CHARACTERISTIC,param2);//发现服务内的特征
    if(param2.uuid != NULL)
        iot_os_free(param2.uuid);
    param2.uuid = NULL;
    iot_os_wait_message(ble_test_handle,&msg);
    if(msg->eventid != OPENAT_BLE_FIND_CHARACTERISTIC_IND)//等待发现特征成功
    {
        if(msg != NULL)
            iot_os_free(msg);
        msg = NULL;
        return FALSE;
    }
    if(msg != NULL)
        iot_os_free(msg);
    msg = NULL;
    param3.uuid = iot_os_malloc(sizeof(T_OPENAT_BLE_UUID));
    memcpy(param3.uuid,&uuid_c,sizeof(T_OPENAT_BLE_UUID));
    iot_ble_iotctl(connect_handle,BLE_OPEN_NOTIFICATION,param3);//打开通知
    if(param3.uuid != NULL)
        iot_os_free(param3.uuid);
    param3.uuid = NULL;
    while(1)
    {
        iot_ble_write(connect_handle,uuid,"1234567890",strlen("1234567890"));
        iot_os_sleep(500);
        iot_os_wait_message(ble_test_handle,&msg);//等待接收到数据
        if(msg->eventid == OPENAT_BLE_RECV_DATA)
        {
            bleRcvBuffer = iot_os_malloc(BLE_MAX_DATA_COUNT*2+1);
            iot_debug_print("[bluetooth]bt recv uuid %x",msg->uuid);
            iot_debug_print("[bluetooth]bt recv data len %d",msg->len);  

            AppConvertBinToHex(msg->bleRcvBuffer,msg->len,bleRcvBuffer);
            bleRcvBuffer[msg->len*2] = '\0';
            iot_debug_print("[bluetooth]bt recv data %s",bleRcvBuffer);
            if(bleRcvBuffer != NULL)
                iot_os_free(bleRcvBuffer);
            bleRcvBuffer = NULL;
            if(msg != NULL)
                iot_os_free(msg);
            msg = NULL;
        }
    }
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
        if(msg != NULL)
            iot_os_free(msg);
        msg = NULL;
        //2.扫描蓝牙
        scan();
        iot_os_wait_message(ble_test_handle,&msg);//等待连接成功
        if(msg->eventid == OPENAT_BLE_CONNECT)
        { 
            //3. 数据传输
            connect_handle = msg->handle;//连接句柄
            if(msg != NULL)
                iot_os_free(msg);
            msg = NULL;
            ble_data_trans();
        }
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
