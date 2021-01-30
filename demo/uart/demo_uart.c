#include <stdio.h>
#include "string.h"
#include "iot_debug.h"
#include "iot_uart.h"
#include "iot_os.h"

HANDLE uart_task_handle;

#define uart_print iot_debug_print
#define UART_PORT1 OPENAT_UART_1
#define UART_PORT2 OPENAT_UART_2
#define UART_USB   OPENAT_UART_USB
#define UART_RECV_TIMEOUT (5 * 1000) // 2S

typedef enum
{
    UART_RECV_MSG = 1,

}TASK_MSG_ID;

typedef struct
{
    TASK_MSG_ID id;
    UINT32 len;
    VOID *param;
}TASK_MSG;


VOID uart_msg_send(HANDLE hTask, TASK_MSG_ID id, VOID *param, UINT32 len)
{
    TASK_MSG *msg = NULL;

    msg = (TASK_MSG *)iot_os_malloc(sizeof(TASK_MSG));
    msg->id = id;
    msg->param = param;
    msg->len = len;

    iot_os_send_message(hTask, msg);
}

//中断方式读串口1数据
//注: 中断中有复杂的逻辑,要发送消息到task中处理
void uart_recv_handle(T_AMOPENAT_UART_MESSAGE* evt)
{
	INT8 *recv_buff = NULL;
    int32 recv_len;
    int32 dataLen = evt->param.dataLen;
	if(dataLen)
	{
		recv_buff = iot_os_malloc(dataLen + 1);
		memset(recv_buff, 0, dataLen + 1);
		if(recv_buff == NULL)
		{
			iot_debug_print("uart_recv_handle_0 recv_buff malloc fail %d", dataLen);
		}	
		switch(evt->evtId)
		{
		    case OPENAT_DRV_EVT_UART_RX_DATA_IND:

		        recv_len = iot_uart_read(UART_PORT2, (UINT8*)recv_buff, dataLen , UART_RECV_TIMEOUT);
		        iot_debug_print("uart_recv_handle_1:recv_len %d", recv_len);
				uart_msg_send(uart_task_handle, UART_RECV_MSG, recv_buff, recv_len);
		        break;

		    case OPENAT_DRV_EVT_UART_TX_DONE_IND:
		        iot_debug_print("uart_recv_handle_2 OPENAT_DRV_EVT_UART_TX_DONE_IND");
		        break;
		    default:
		        break;
		}
	}
}

VOID uart_write(VOID)
{
    char write_buff[] = "uart hello world";
    int32 write_len;
  
    write_len = iot_uart_write(UART_PORT2, (UINT8*)write_buff, strlen(write_buff));
    write_len = iot_uart_write(UART_USB, (UINT8*)write_buff, strlen(write_buff));
    iot_debug_print("[uart] uart_write_1 len %d, buff %s", write_len, write_buff);
}

VOID uart_open(VOID)
{
    BOOL err;
    T_AMOPENAT_UART_PARAM uartCfg;
    
    memset(&uartCfg, 0, sizeof(T_AMOPENAT_UART_PARAM));
    uartCfg.baud = OPENAT_UART_BAUD_115200; //波特率
    uartCfg.dataBits = 8;   //数据位
    uartCfg.stopBits = 1; // 停止位
    uartCfg.parity = OPENAT_UART_NO_PARITY; // 无校验
    uartCfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE; //无流控
    uartCfg.txDoneReport = TRUE; // 设置TURE可以在回调函数中收到OPENAT_DRV_EVT_UART_TX_DONE_IND
    uartCfg.uartMsgHande = uart_recv_handle; //回调函数

    // 配置uart1 使用中断方式读数据
    err = iot_uart_open(UART_PORT2, &uartCfg);
	iot_debug_print("[uart] uart_open_2 err: %d", err);

	uartCfg.txDoneReport = FALSE;
	uartCfg.uartMsgHande = NULL;
	err = iot_uart_open(UART_USB, &uartCfg);
	iot_debug_print("[uart] uart_open_usb err: %d", err);
}

VOID uart_close(VOID)
{
    iot_uart_close(UART_PORT2);
    iot_uart_close(UART_USB);
    iot_debug_print("[uart] uart_close_1");
}

VOID uart_init(VOID)
{   
    uart_open(); // 打开串口1和串口2 (串口1中断方式读数据, 串口2轮训方式读数据)
}

static VOID uart_task_main(PVOID pParameter)
{
	TASK_MSG *msg = NULL;
	
	while(1)
	{
		iot_os_wait_message(uart_task_handle, (PVOID*)&msg);
		switch(msg->id)
	    {
	        case UART_RECV_MSG:    
				iot_debug_print("[uart] uart_task_main_1 recv_len %s", msg->param);
	            break;
	        default:
	            break;
	    }

	    if(msg)
	    {
	        if(msg->param)
	        {
	            iot_os_free(msg->param);
	            msg->param = NULL;
	        }
	        iot_os_free(msg);
	        msg = NULL;
			iot_debug_print("[uart] uart_task_main_2 uart free");
	    }
		uart_write(); //串口2 写数据
	}
}

static VOID usb_task_main(PVOID param)
{
	INT8 *recv_buff = iot_os_malloc(32);
	CHAR buff[64];
	UINT32 len = 0;
	while (1)
	{
		len = iot_uart_read(UART_USB, (UINT8 *)recv_buff, 32, UART_RECV_TIMEOUT);
		if (len == 0)
		{
			iot_os_sleep(10);
		}
		else
		{
			snprintf(buff, len, "%s", recv_buff);
			iot_debug_print("[uart] usb_task_main %s", buff);
		}
	}
}

int appimg_enter(void *param)
{    
    iot_debug_print("[uart] appimg_enter");

    uart_init();
	uart_task_handle =  iot_os_create_task(uart_task_main, NULL, 4096, 1, OPENAT_OS_CREATE_DEFAULT, "uart_task");
	iot_os_create_task(usb_task_main, NULL, 4096, 1, OPENAT_OS_CREATE_DEFAULT, "usb_task");

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[uart] appimg_exit");
}

