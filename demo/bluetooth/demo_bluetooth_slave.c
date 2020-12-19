/***************
	demo_bluetooth
****************/


#include "string.h"
#include "iot_debug.h"
#include "iot_bluetooth.h"

#define BT_8910_TP_UUID 0xFEE0
#define BT_8910_TP_UUID_CHAR 0xFEE1
#define BT_8910_FEEDBACK_CHAR 0xFEE2

//GATT Characteristic attribute
#define ATT_CHARA_PROP_BROADCAST           0x01
#define ATT_CHARA_PROP_READ         0x02
#define ATT_CHARA_PROP_WWP          0x04    // WWP short for "write without response"
#define ATT_CHARA_PROP_WRITE               0x08
#define ATT_CHARA_PROP_NOTIFY              0x10
#define ATT_CHARA_PROP_INDICATE     0x20
#define ATT_CHARA_PROP_ASW          0x40    // ASW short for "Authenticated signed write"
#define ATT_CHARA_PROP_EX_PROP      0x80

//GATT Characteristic Descriptors
#define ATT_UUID_CHAR_EXT           0x2900
#define ATT_UUID_CHAR_USER      0x2901
#define ATT_UUID_CLIENT         0x2902
#define ATT_UUID_SERVER         0x2903
#define ATT_UUID_CHAR_FORMAT        0x2904
#define ATT_UUID_CHAR_AGGREGATE 0x2905
#define ATT_UUID_EXTERNAL_REF       0x2907
#define ATT_UUID_REPORT_RE      0x2908

HANDLE ble_test_handle = NULL;

typedef struct ble_report_info
{
    unsigned char  eventid;
    char    state;
    UINT8   bleRcvBuffer[BLE_MAX_DATA_COUNT];
    UINT8    len;
    UINT16 uuid;
    UINT16 handle;
    UINT8 long_uuid[BLE_LONG_UUID_FLAG];
    UINT8 uuid_flag;
} ble_report_info_t;


typedef struct Ble_add_characteristic
{
    UINT8 *uuid_c; //特征uuid  
    UINT8 uuid_len;//uuid长度
    UINT8  type; //特征属性
    UINT16 *descriptor; //特征描述
    UINT8 count;//描述数量
} ble_add_characteristic_t;

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
    ble_report_info_t *msg = NULL;
    msg = iot_os_malloc(sizeof(ble_report_info_t));
	msg->eventid = result->id;
	msg->state  =  result->state;
	switch(result->id)
	{
		case OPENAT_BLE_RECV_DATA:
			if((result->len != 0) && (result->dataPtr != NULL))
			{
                msg->len  =  result->len;
                msg->uuid_flag  =  result->uuid_flag;
                msg->uuid  =  result->uuid;
                memcpy(msg->long_uuid, result->long_uuid, sizeof(result->long_uuid));
                memset(msg->bleRcvBuffer,0,sizeof(msg->bleRcvBuffer));
				memcpy(msg->bleRcvBuffer, result->dataPtr,result->len);  
                iot_os_send_message(ble_test_handle,msg);
			}
			break;
        case OPENAT_BLE_CONNECT_IND:
            msg->handle  =  result->handle;
            iot_os_send_message(ble_test_handle,msg);
            break;
        case OPENAT_BLE_DISCONNECT_IND:
            iot_debug_print("[bluetooth]bt disconnect");
            iot_os_free(msg);
            break;
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
/*先添加服务，在依次添加特征，若特征下包含特性描述，则添加特征后接着添加描述*/
BOOL AddService()
{
    int i,j;
    UINT8 bt_tp_data_uuid[BLE_LONG_UUID_FLAG] = {BT_8910_TP_UUID >> 8, BT_8910_TP_UUID & 0xff};//服务uuid
    UINT8 bt_tp_data_char[BLE_LONG_UUID_FLAG] = {BT_8910_TP_UUID_CHAR >> 8, BT_8910_TP_UUID_CHAR & 0xff};//特征uuid
    UINT8 bt_feedback_data_char[BLE_LONG_UUID_FLAG] = {BT_8910_FEEDBACK_CHAR >> 8, BT_8910_FEEDBACK_CHAR & 0xff};//特征uuid
    UINT16 bt_descriptor[1] = {ATT_UUID_CLIENT};//特征描述
    ble_add_characteristic_t param[2] = {{bt_tp_data_char,2,ATT_CHARA_PROP_READ | ATT_CHARA_PROP_WRITE,NULL,0},
                                        {bt_feedback_data_char,2,ATT_CHARA_PROP_READ | ATT_CHARA_PROP_NOTIFY | ATT_CHARA_PROP_INDICATE,bt_descriptor,1}};   
    
    iot_ble_iotctl(BLE_ADD_SERVICE,(U_OPENAT_BT_IOTCTL_PARM*)bt_tp_data_uuid,strlen(bt_tp_data_uuid),0);//添加服务
    for(i = 0;i < sizeof(param)/sizeof(ble_add_characteristic_t);i ++)
    {
        iot_ble_iotctl(BLE_ADD_CHARACTERISTIC,(U_OPENAT_BT_IOTCTL_PARM*)param[i].uuid_c,param[i].uuid_len,param[i].type);//添加特征
        if(param[i].descriptor != NULL)
        {
            for(j = 0;j < param[i].count;j ++)
            {
                UINT8 ble_descriptor[2] = {param[i].descriptor[j] >> 8, param[i].descriptor[j] & 0xff};
                iot_ble_iotctl(BLE_ADD_DESCRIPTOR,(U_OPENAT_BT_IOTCTL_PARM*)ble_descriptor,0,0);//添加描述
            }
        }
    }

    return TRUE;
}

BOOL advertising(VOID)
{
    //UINT8 advdata[BLE_MAX_ADV_MUBER] = {0x02,0x01,0x06,0x04,0xff,0x01,0x02,0x03};//广播包数据
    //UINT8 scanrspdata[BLE_MAX_ADV_MUBER] = {0x02,0x0a,0x04};//响应包数据
    iot_debug_print("[bluetooth]bt advertising");
    iot_ble_iotctl(BLE_SET_NAME,(U_OPENAT_BT_IOTCTL_PARM*)"Luat_Air724UG",strlen("Luat_Air724UG"),0);//设置广播名称
    //iot_ble_iotctl(BLE_SET_ADV_DATA,(U_OPENAT_BT_IOTCTL_PARM*)advdata,strlen(advdata),0);//设置广播包数据
    //iot_ble_iotctl(BLE_SET_SCANRSP_DATA,(U_OPENAT_BT_IOTCTL_PARM*)scanrspdata,strlen(scanrspdata),0);//设置响应包数据
    //AddService();//添加自定义蓝牙服务
    iot_os_sleep(1000);
    iot_ble_iotctl(BLE_SET_ADV_ENABLE,(U_OPENAT_BT_IOTCTL_PARM*)1,0,0);//打开广播
    return TRUE;
}

BOOL ble_data_trans(VOID)
{
    ble_report_info_t *msg = NULL;
    T_OPENAT_BLE_UUID uuid = {0};
    uuid.uuid_short = BT_8910_FEEDBACK_CHAR;
    uuid.uuid_type = UUID_SHORT;
    char *bleRcvBuffer = NULL;
    /*链接成功*/
    while(1)
    {
        
        iot_os_wait_message(ble_test_handle,&msg);//等待接收到数据
        if(msg->eventid == OPENAT_BLE_RECV_DATA)
        {
            bleRcvBuffer = iot_os_malloc(BLE_MAX_DATA_COUNT*2+1);                                                 
            iot_debug_print("[bluetooth]bt recv uuid %x",msg->uuid);
            iot_debug_print("[bluetooth]bt recv data len %d",msg->len);

            AppConvertBinToHex(msg->bleRcvBuffer,msg->len,bleRcvBuffer);
            bleRcvBuffer[msg->len*2] = '\0';
            iot_debug_print("[bluetooth]bt recv data %s",bleRcvBuffer);

            iot_ble_write(connect_handle,uuid,msg->bleRcvBuffer,msg->len);  
            //test 当接收到"close"时主动断开蓝牙连接
            if(memcmp(msg->bleRcvBuffer,"close",sizeof("close")) == 0)
            {
                iot_ble_disconnect(connect_handle);
            }
            iot_os_free(bleRcvBuffer);
            bleRcvBuffer = NULL;
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
        iot_os_free(msg);
        msg = NULL;
        //2.广播蓝牙
        advertising();
        iot_os_wait_message(ble_test_handle,&msg);//等待连接成功
        if(msg->eventid == OPENAT_BLE_CONNECT_IND)
        {     
            //3. 数据传输
            connect_handle = msg->handle;//连接句柄
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
