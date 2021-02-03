/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_uart.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/10/15
 *
 * Description:
 *  2013.08.07 liweiqiang       增加uart1支持
 **************************************************************************/
#if 1
#include "string.h"

#include "am_openat.h"
#include "cycle_queue.h"
#include "assert.h"

#include "lplatform.h"
#include "platform_malloc.h"
#include "platform_conf.h"
#include "platform_rtos.h"
//#include "dcl.h"
#include "malloc.h"

//#include "teldef.h"

#define COS_WAIT_FOREVER            OPENAT_OS_SUSPENDED

//vat recv queue
/*+\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */
#define VATC_RX_BUF_SIZE            8192
#define VATC_READ_BUF_SIZE          512
/*-\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */

#define UART_RX_BUF_SIZE            2048

// 临时缓冲区
#define RX_BUFFER_SIZE          256
/*+\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
#define READ_BUFFER_SIZE        126
/*-\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */

#define PHY_PORT(ID)            (uartmap[ID].port)

typedef struct UartPhyContextTag
{
    CycleQueue  rxqueue;
    uint8       temprxbuff[RX_BUFFER_SIZE];
/*+\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
    uint8       readbuf[READ_BUFFER_SIZE];
    uint8       readindex;
    uint8       readsize;
/*-\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
}UartPhyContext;

typedef struct UartMapTag
{
    const E_AMOPENAT_UART_PORT port;
    const PUART_MESSAGE        msg;
}UartMap;
/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
typedef enum{
	UART_NO_EVENT_CONFIG = -1,
	UART_RX_EVENT_CONFIG,
	UART_TX_DONE_EVENT_CONFIG,
	UART_MAX_EVENT_CONFIG
}PLATFORM_UART_EVENT_CONFIG;

typedef struct{
	PLATFORM_UART_EVENT_CONFIG UartConfigEvent;
	int UartConfigId;
}PLATFORM_UART_CONFIG;
/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
typedef struct UartContextTag
{
    uint8 opened;
    uint8 workmode; //uart数据提示方式:1:用户轮询 其他:消息提示 2: host uart ID A2数据透传
    /*+\NEW\zhutianhua\2018.12.27 14:20\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
    uint8 rs485Io;
    uint8 rs485ValidLevel;
	/*+\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/
	uint32 rs485DelayTime;
	/*-\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/
    /*-\NEW\zhutianhua\2018.12.27 14:20\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
    /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
    uint32 uartBand;//存储串口的波特率。485通讯需要计算oe的停止时间
    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
}UartContext;

/*+\NEW\zhutianhua\2018.12.27 15:8\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
#define RS485_INVALID_LEVEL 0xFF
/*-\NEW\zhutianhua\2018.12.27 15:8\新增uart.set_rs485_oe接口，可配置rs485 io使能*/

/*+\NEW\liweiqiang\2014.7.21\修正AM002_LUA项目RAM不够编译不过的问题*/
#if defined(LOW_MEMORY_SUPPORT)
static UartPhyContext uartPhyContext[3]/*openat uart 1 & 2 & host uart*/;
#else
static uint8 uart1RxBuff[UART_RX_BUF_SIZE];
static uint8 uart2RxBuff[UART_RX_BUF_SIZE];
/*+\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
static uint8 uart3RxBuff[UART_RX_BUF_SIZE];
/*-\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/

static UartPhyContext uartPhyContext[3]/*openat uart 1 & 2 & host uart*/ =
{
    // OPENAT_UART_1,
    {
        .rxqueue = {
            uart1RxBuff,
            UART_RX_BUF_SIZE,
            0,
            0,
            1,
            0,
            0,
        },
    },
    // OPENAT_UART_2,
    {
        .rxqueue = {
            uart2RxBuff,
            UART_RX_BUF_SIZE,
            0,
            0,
            1,
            0,
            0,
        },
    },
/*+\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
    // OPENAT_UART_3,
    {
        .rxqueue = {
            uart3RxBuff,
            UART_RX_BUF_SIZE,
            0,
            0,
            1,
            0,
            0,
        },
    },
/*-\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
};
#endif
/*-\NEW\liweiqiang\2014.7.21\修正AM002_LUA项目RAM不够编译不过的问题*/

/*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
HANDLE rs485_oe_task_hand = NULL;

typedef enum
{
  enable_rs485_oe = 0x10,
  disable_rs485_oe,
  close_rs485_oe
}rs485_oe_event;

/*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/

static void uart0_message_handle(T_AMOPENAT_UART_MESSAGE* evt);
static void uart1_message_handle(T_AMOPENAT_UART_MESSAGE* evt);
static void uart2_message_handle(T_AMOPENAT_UART_MESSAGE* evt);
/*+\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
static void uart3_message_handle(T_AMOPENAT_UART_MESSAGE* evt);
/*-\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/

static const UartMap uartmap[NUM_UART] = 
{
    {OPENAT_UART_1,uart1_message_handle},
    {OPENAT_UART_2,uart2_message_handle},
/*+\NEW\zhuwangbin\2019.12.31\添加uart3功能*/
    {OPENAT_UART_3,uart3_message_handle},
/*-\NEW\zhuwangbin\2019.12.31\添加uart3功能*/
/*-\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
};
/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
static PLATFORM_UART_CONFIG gUartConfig = {
	.UartConfigEvent = UART_NO_EVENT_CONFIG,
	.UartConfigId = -1,
};
/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
static UartContext uartContext[NUM_UART];

static HANDLE hAtcReadSem = 0;
static uint8 vatcRxBuff[VATC_RX_BUF_SIZE];
CycleQueue vatcRx_Q = {
    vatcRxBuff,
    VATC_RX_BUF_SIZE,
    0,
    0,
    1,
    0,
    0,
};
/*+\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */
static uint8 vatc_read_buffer[VATC_READ_BUF_SIZE];
static uint16 vatc_read_buf_size;
static uint16 vatc_read_buf_index;
/*-\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */

/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
static HANDLE hUsbReadSem = 0;
static uint8 usbRxBuff[VATC_RX_BUF_SIZE];
CycleQueue usbRx_Q = {
    usbRxBuff,
    VATC_RX_BUF_SIZE,
    0,
    0,
    1,
    0,
    0,
};

static uint8 usb_read_buffer[VATC_READ_BUF_SIZE];
static uint16 usb_read_buf_size;
static uint16 usb_read_buf_index;
u32 usbdata_mode = 0;
/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */

/*+\NEW\liweiqiang\2013.4.7\优化debug口输出*/
static char debugStdoutBuffer[128];//openat接口的print接口buff最大为127字节与其同步
/*-\NEW\liweiqiang\2013.4.7\优化debug口输出*/
static UINT16 debugStdoutCachedCount = 0;
/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
static void sendUartMessage(int uart_id,platform_msg_type event_id)
{
    PlatformMsgData msgData;
   
    msgData.uart_id = uart_id;

    OPENAT_print("uart sendUartMessage uart %d tick %d", uart_id, OPENAT_get_system_tick());

	if(gUartConfig.UartConfigId == uart_id && 
			((UART_TX_DONE_EVENT_CONFIG == gUartConfig.UartConfigEvent && MSG_ID_RTOS_UART_TX_DONE == event_id) 
			|| (UART_RX_EVENT_CONFIG == gUartConfig.UartConfigEvent && MSG_ID_RTOS_UART_RX_DATA == event_id)))
	{
		platform_rtos_send_high_priority(event_id,&msgData);
	}
	else

	{
		platform_rtos_send(event_id, &msgData);
	}
}
/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
/*+\NEW\zhuwangbin\2018.8.10\添加OPENAT_DRV_EVT_UART_TX_DONE_IND上报*/
static void sendUartTxDoneMessage(int uart_id)
{
    PlatformMsgData msgData;
    
    msgData.uart_id = uart_id;

    platform_rtos_send(MSG_ID_RTOS_UART_TX_DONE, &msgData);
}
/*-\NEW\zhuwangbin\2018.8.10\添加OPENAT_DRV_EVT_UART_TX_DONE_IND上报*/

static void uart_message_handle(uint8 id, T_AMOPENAT_UART_MESSAGE* evt)
{
    uint32 length;
    uint8 phyid = uartmap[id].port;
/*+\NEW\liweiqiang\2013.4.7\优化uart/atc数据接收消息提示,避免发消息过于频繁导致系统无法响应 */
    BOOL needMsg = FALSE; // buffer是空的时候,放入数据才需要作提示
    int count = (evt->param.dataLen / RX_BUFFER_SIZE);
    int i = 0;

    if(evt->evtId == OPENAT_DRV_EVT_UART_RX_DATA_IND)
    {
        for(i = 0; i < count; i++)
        {
            length = IVTBL(read_uart)(phyid, uartPhyContext[phyid].temprxbuff, RX_BUFFER_SIZE, 0);
            
            if(length != 0)
            {
                if(!needMsg)
                {
                    needMsg = uartPhyContext[phyid].rxqueue.empty ? TRUE : FALSE;
                }
                // 此处后续最好加上保护,写入缓冲在uart中断,读取数据在lua shell线程
                QueueInsert(&uartPhyContext[phyid].rxqueue, uartPhyContext[phyid].temprxbuff, length);
            }
        }

        if(evt->param.dataLen % RX_BUFFER_SIZE)
        {
            length = IVTBL(read_uart)(phyid, 
                uartPhyContext[phyid].temprxbuff,
                evt->param.dataLen % RX_BUFFER_SIZE, 
                0);
            
            if(length != 0)
            {
                if(!needMsg)
                {
                    needMsg = uartPhyContext[phyid].rxqueue.empty ? TRUE : FALSE;
                }
                // 此处后续最好加上保护,写入缓冲在uart中断,读取数据在lua shell线程
                QueueInsert(&uartPhyContext[phyid].rxqueue, uartPhyContext[phyid].temprxbuff, length);
            }
        }

        if(needMsg)
        {
			/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
            sendUartMessage(id+1,MSG_ID_RTOS_UART_RX_DATA);
			/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
        }   
    }
	/*+\NEW\zhuwangbin\2018.8.10\添加OPENAT_DRV_EVT_UART_TX_DONE_IND上报*/
    else if(evt->evtId == OPENAT_DRV_EVT_UART_TX_DONE_IND)
    {
        /*+\NEW\zhutianhua\2018.12.27 15:33\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
        platform_uart_disable_rs485_oe(id);
        /*-\NEW\zhutianhua\2018.12.27 15:33\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
		/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
        sendUartMessage(id+1,MSG_ID_RTOS_UART_TX_DONE);
		/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
    }
	/*-\NEW\zhuwangbin\2018.8.10\添加OPENAT_DRV_EVT_UART_TX_DONE_IND上报*/
/*-\NEW\liweiqiang\2013.4.7\优化uart/atc数据接收消息提示,避免发消息过于频繁导致系统无法响应 */
}


static void uart1_message_handle(T_AMOPENAT_UART_MESSAGE* evt)
{
    uart_message_handle(0, evt);
}

static void uart2_message_handle(T_AMOPENAT_UART_MESSAGE* evt)
{
    uart_message_handle(1, evt);
}
/*+\NEW\zhuwangbin\2019.12.31\添加uart3功能*/
static void uart3_message_handle(T_AMOPENAT_UART_MESSAGE* evt)
{
    uart_message_handle(2, evt);
}
/*-\NEW\zhuwangbin\2019.12.31\添加uart3功能*/

/*+\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
static void host_uart_recv(UINT8 *data, UINT32 length)
{
    uint8 phyid = OPENAT_UART_3;
    BOOL needMsg = FALSE; // buffer是空的时候,放入数据才需要作提示

    if(length != 0)
    {
        needMsg = uartPhyContext[phyid].rxqueue.empty ? TRUE : FALSE;
        
        QueueInsert(&uartPhyContext[phyid].rxqueue, data, length);
    }

    if(needMsg)
    {
        sendUartMessage(3,MSG_ID_RTOS_UART_RX_DATA);
    }
}
/*-\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */

/****************************************************************************
 *
 * Function: PlatformUartOpen
 *
 * Parameters: 
 *         void
 *
 * Returns: void 
 *
 * Description: 打开串口
 *
 ****************************************************************************/
 /*+\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
static u32 uart_phy_open( unsigned id, u32 baud, int databits, int parity, int stopbits, u32 mode, u32 txDoneReport)
/*-\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
{
    T_AMOPENAT_UART_PARAM uartParam;
    
    if(uartContext[id].opened)
        return baud;

    /*+\NEW\zhutianhua\2018.12.27 15:19\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
    uartContext[id].rs485ValidLevel = RS485_INVALID_LEVEL;
    /*-\NEW\zhutianhua\2018.12.27 15:19\新增uart.set_rs485_oe接口，可配置rs485 io使能*/

/*+\NEW\liweiqiang\2014.7.21\修正AM002_LUA项目RAM不够编译不过的问题*/
    if(!uartPhyContext[PHY_PORT(id)].rxqueue.buf)
    {
        uartPhyContext[PHY_PORT(id)].rxqueue.buf = lualibc_calloc(1, UART_RX_BUF_SIZE);
        uartPhyContext[PHY_PORT(id)].rxqueue.size = UART_RX_BUF_SIZE;
        QueueClean(&uartPhyContext[PHY_PORT(id)].rxqueue);
    }
/*-\NEW\liweiqiang\2014.7.21\修正AM002_LUA项目RAM不够编译不过的问题*/
#if 0
/*+\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
    if(PHY_PORT(id) == uart_port3 && mode == 2)
    {
        if(1)
        {
            uartContext[id].opened = 1;
            uartContext[id].workmode = mode;
            return baud;
        }
        else
        {
            return 0;
        }
    }
/*-\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
#endif
    /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
    uartContext[id].uartBand = baud;
    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
    uartParam.baud  = baud;
    uartParam.dataBits = databits;

    switch(stopbits)
    {
        case PLATFORM_UART_STOPBITS_1:
            uartParam.stopBits = 1;
            break;
            
        case PLATFORM_UART_STOPBITS_2:
            uartParam.stopBits = 2;
            break;

        case PLATFORM_UART_STOPBITS_1_5:
        default:
            goto uart_open_error;
            break;
    }

    switch(parity)
    {
        case PLATFORM_UART_PARITY_EVEN:
            uartParam.parity = OPENAT_UART_EVEN_PARITY;
            break;

        case PLATFORM_UART_PARITY_ODD:
            uartParam.parity = OPENAT_UART_ODD_PARITY;
            break;

        case PLATFORM_UART_PARITY_NONE:
            uartParam.parity = OPENAT_UART_NO_PARITY;
            break;

        default:
            goto uart_open_error;
            break;
    }

/*+\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/
    if(PHY_PORT(id) == OPENAT_UART_3)
    {
        uartParam.flowControl = OPENAT_UART_FLOWCONTROL_NONE;
    }
    else
    {
        uartParam.flowControl = OPENAT_UART_FLOWCONTROL_NONE;
    }
/*-\NEW\liweiqiang\2013.8.31\增加host uart通讯支持*/

    if(platform_get_console_port() == id)
    {
        uartParam.uartMsgHande = NULL;
    }
    else
    {
        if(mode == 1)
        {
            uartParam.uartMsgHande = NULL;
        }
        else
        {
            uartParam.uartMsgHande = uartmap[id].msg;
        }
    }
	
	/*+\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
    uartParam.txDoneReport = txDoneReport;
	/*-\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/

    if(TRUE == IVTBL(config_uart)(PHY_PORT(id), &uartParam))
    {
        uartContext[id].opened = 1;
        uartContext[id].workmode = mode;
        return baud;
    }
    else
    {
        PUB_TRACE("config_uart : error ");
    }

uart_open_error:
    return 0;
}

/*+\NEW\liweiqiang\2013.4.20\增加uart.close接口 */
static u32 uart_phy_close(unsigned id)
{
    u32 ret;

    if(!uartContext[id].opened)
        return PLATFORM_OK;

#if 0
/*+\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
    if(PHY_PORT(id) == uart_port3 && uartContext[id].workmode == 2)
    {
        // host uart此种模式不需要关闭
        return PLATFORM_OK;
    }
/*-\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
#endif    
    ret = IVTBL(close_uart)(PHY_PORT(id)) ? PLATFORM_OK : PLATFORM_ERR;

    uartContext[id].opened = FALSE;

    QueueClean(&uartPhyContext[PHY_PORT(id)].rxqueue);
/*+\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
    uartPhyContext[PHY_PORT(id)].readindex = uartPhyContext[PHY_PORT(id)].readsize = 0;
/*-\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */

    return ret;
}
/*-\NEW\liweiqiang\2013.4.20\增加uart.close接口 */
#if 0

static HANDLE g_uartLoop = NULL;
extern UINT8 uartIrqStatus[];
extern UINT32 uartIrqTick[];
extern UINT32 g_uartTxCount;
extern UINT32 g_uartTxIntCount;
extern UINT32 g_uartRxIntCount;

static void uartTimeout( void* p )
{
  UINT8 i;
  for(i = 0; i < 5; i++)
  {
    PUB_TRACE("uartIrq on %d status[%d] = 0x%x, txCount=%d, txIrqCount=%d rxIrqCount = %d", uartIrqTick[i], i, uartIrqStatus[i], g_uartTxCount, 
        g_uartTxIntCount, g_uartRxIntCount);
  }
  OPENAT_start_timer(g_uartLoop, 1000);
}
#endif
static u32 uart_phy_write(u8 id, uint8 *data_p, uint16 length)
{
    if(!uartContext[id].opened)
        return 0;

    /*+\NEW\zhutianhua\2018.12.27 15:28\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
    platform_uart_enable_rs485_oe(id);
    /*-\NEW\zhutianhua\2018.12.27 15:28\新增uart.set_rs485_oe接口，可配置rs485 io使能*/

#if 0
/*+\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
    if(PHY_PORT(id) == uart_port3 && uartContext[id].workmode == 2)
    {
        //IVTBL(host_send_data)(data_p, length);
        return length;
    }
/*-\NEW\liweiqiang\2014.1.2\host uart ID 0xA2数据透传支持 */
    

    if(!g_uartLoop)
        {
          g_uartLoop = OPENAT_create_timer(uartTimeout, NULL);
        }
        OPENAT_start_timer(g_uartLoop, 1000);
#endif

    return IVTBL(write_uart)(PHY_PORT(id), data_p, length);
}

static u32 uart_phy_read(u8 id, uint8 *data_p, uint16 length, u32 timeout)
{    
    if(!uartContext[id].opened)
        return 0;

    if(uartContext[id].workmode == 1)
    {
        // 用户轮询方式直接从uart接口读取
        return IVTBL(read_uart)(PHY_PORT(id), data_p, length, timeout);
    }
    else
    {
        // 消息提示方式:从环形缓冲区读取
/*+\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
        u32 rcvdlen = 0;
        UartPhyContext *context = &uartPhyContext[PHY_PORT(id)];

        while(rcvdlen < length)
        {
            if(context->readindex >= context->readsize)
            {
            /*+\NEW\liweiqiang\2014.4.12\增加串口缓冲区数据保护 */
                context->readsize = QueueDelete(&context->rxqueue, context->readbuf, READ_BUFFER_SIZE);
            /*-\NEW\liweiqiang\2014.4.12\增加串口缓冲区数据保护 */

                context->readindex = 0;

                if(context->readsize == 0) break;
            }

            data_p[rcvdlen++] = context->readbuf[context->readindex++];
        }
        
        return rcvdlen;
/*-\NEW\liweiqiang\2014.4.12\优化对串口uart库每次只读取一个字节的处理 */
    }
}

/*+\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */
static u32 vatc_read(uint8 *data_p, uint16 length, u32 timeout)
{
    u32 rcvdlen = 0;

    while(rcvdlen < length)
    {
        if(vatc_read_buf_index >= vatc_read_buf_size)
        {
            OPENAT_wait_semaphore(hAtcReadSem, 0);
            vatc_read_buf_size = QueueDelete(&vatcRx_Q, vatc_read_buffer, VATC_READ_BUF_SIZE);
            OPENAT_release_semaphore(hAtcReadSem);
            vatc_read_buf_index = 0;

            if(vatc_read_buf_size == 0) break;
        }

        data_p[rcvdlen++] = vatc_read_buffer[vatc_read_buf_index++];
    }
    
    return rcvdlen;
}
/*-\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */

/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
static u32 usb_read(uint8 *data_p, uint16 length, u32 timeout)
{
    u32 rcvdlen = 0;

    while(rcvdlen < length)
    {
        if(usb_read_buf_index >= usb_read_buf_size)
        {
            OPENAT_wait_semaphore(hUsbReadSem, 0);
            usb_read_buf_size = QueueDelete(&usbRx_Q, usb_read_buffer, VATC_READ_BUF_SIZE);
            OPENAT_release_semaphore(hUsbReadSem);
            usb_read_buf_index = 0;

            if(usb_read_buf_size == 0) break;
        }

        data_p[rcvdlen++] = usb_read_buffer[usb_read_buf_index++];
    }
    
    return rcvdlen;
}
/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */

/*+\NEW\liweiqiang\2013.4.7\优化debug口输出*/
static void debugPortFlush(void)
{
    if(debugStdoutCachedCount != 0)
    {
        debugStdoutBuffer[debugStdoutCachedCount] = '\0';
        OPENAT_lua_print(debugStdoutBuffer);
        
        memset(debugStdoutBuffer, 0, sizeof(debugStdoutBuffer));
        debugStdoutCachedCount = 0;
    }
}

static void debugPortWrite(const u8 *buff, u16 len)
{
    u16 i;
    u8 data;

    for(i = 0; i < len; i++)
    {
        data = buff[i];
        
        if(data == '\r' || data == '\n')
        {
            debugPortFlush();
        }
        else
        {
            if(debugStdoutCachedCount < sizeof(debugStdoutBuffer) - 1)
                debugStdoutBuffer[debugStdoutCachedCount++] = data;
            else
            {
                debugPortFlush();
                debugStdoutBuffer[debugStdoutCachedCount++] = data;
            }
        }
    }
}
/*-\NEW\liweiqiang\2013.4.7\优化debug口输出*/

u32 vatc_mode = 0;




void RILAPI_ReceiveData(void *data, int len)
{
/*+\NEW\liweiqiang\2013.4.7\优化uart/atc数据接收消息提示,避免发消息过于频繁导致系统无法响应 */
    BOOL needMsg = FALSE; // buffer是空的时候,放入数据才需要作提示
    
    OPENAT_wait_semaphore(hAtcReadSem, COS_WAIT_FOREVER);
    needMsg = vatcRx_Q.empty ? TRUE : FALSE;
    QueueInsert(&vatcRx_Q, data, len);
    OPENAT_release_semaphore(hAtcReadSem);

    if(needMsg && (vatc_mode != 1))
    {	
		/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
        sendUartMessage(PLATFORM_UART_ID_ATC,MSG_ID_RTOS_UART_RX_DATA);
		/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
    }
/*-\NEW\liweiqiang\2013.4.7\优化uart/atc数据接收消息提示,避免发消息过于频繁导致系统无法响应 */
}

void platform_setup_vat_queue(void)
{
    QueueClean(&vatcRx_Q);
    hAtcReadSem = OPENAT_create_semaphore(1);
}

/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
u32 Get_usbdata_mode(void)
{
	return usbdata_mode;
}

void USBAPI_ReceiveData(void *data, int len)
{
    BOOL needMsg = FALSE; // buffer是空的时候,放入数据才需要作提示
    
    OPENAT_wait_semaphore(hUsbReadSem, COS_WAIT_FOREVER);
    needMsg = usbRx_Q.empty ? TRUE : FALSE;
    QueueInsert(&usbRx_Q, data, len);
    OPENAT_release_semaphore(hUsbReadSem);

    if(needMsg)
    {
		/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
        sendUartMessage(PLATFORM_PORT_ID_USB,MSG_ID_RTOS_UART_RX_DATA);
		/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
    }
}

/**+\BUG3623\zhuwangbin\2020.11.18\uart.on(uart.USB, "receive" 丢数据**/
BOOL USBAPI_IsEmpty(void)
{
	BOOL isEmpty;

	OPENAT_wait_semaphore(hUsbReadSem, COS_WAIT_FOREVER);
	isEmpty = usbRx_Q.empty ? TRUE : FALSE;
	OPENAT_release_semaphore(hUsbReadSem);

	return isEmpty;
}


u32 USBAPI_FreeSpace(void)
{
	u32 space;
	
	OPENAT_wait_semaphore(hUsbReadSem, COS_WAIT_FOREVER);
	space = QueueGetFreeSpace(&usbRx_Q);
	OPENAT_release_semaphore(hUsbReadSem);

	return space;
}
/**-\BUG3623\zhuwangbin\2020.11.18\uart.on(uart.USB, "receive" 丢数据**/

void platform_setup_usb_queue(void)
{
    QueueClean(&usbRx_Q);
    hUsbReadSem = OPENAT_create_semaphore(1);
}
/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */

/*+\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
u32 platform_uart_setup( unsigned id, u32 baud, int databits, int parity, int stopbits, u32 mode, u32 txDoneReport)
/*-\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
{      
    u32 ret = baud;

    if(PLATFORM_UART_ID_ATC == id)
    {
        vatc_mode = mode;
        /* 在初始化vat之前就初始化缓冲区,不由lua控制初始化,避免漏掉vat的数据 */
    }
    else if(PLATFORM_PORT_ID_DEBUG == id)
    {   
        memset(debugStdoutBuffer, 0, sizeof(debugStdoutBuffer));
        debugStdoutCachedCount = 0;
    }
	/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
	else if(PLATFORM_PORT_ID_USB == id)
    {   
		usbdata_mode = 1;
    }
	/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
    else
    {
/*+\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
        ret = uart_phy_open(id, baud, databits, parity, stopbits, mode, txDoneReport);
/*-\NEW\zhuwangbin\2018.8.31\添加参数判断是否上报UART TXDONE*/
	}

    return ret;
}

/*+\NEW\zhutianhua\2018.12.27 14:54\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
void platform_uart_enable_rs485_oe( unsigned char id)
{
    if(uartContext[id].opened && uartContext[id].rs485ValidLevel != RS485_INVALID_LEVEL)
    {
    /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
       //IVTBL(set_gpio)(platform_pio_get_gpio_port(uartContext[id].rs485Io), uartContext[id].rs485ValidLevel);
       //OPENAT_print("platform_uart_enable_rs485_oe id=%d, realIO=%d, level=%d\n", id, platform_pio_get_gpio_port(uartContext[id].rs485Io), uartContext[id].rs485ValidLevel);
        OPENAT_send_message(rs485_oe_task_hand, enable_rs485_oe, (void *)&id, sizeof(id));
    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
	}
}

void platform_uart_disable_rs485_oe( unsigned char id)
{
    if(uartContext[id].opened && uartContext[id].rs485ValidLevel != RS485_INVALID_LEVEL)
    {
    /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
       //IVTBL(set_gpio)(platform_pio_get_gpio_port(uartContext[id].rs485Io), (uartContext[id].rs485ValidLevel==0) ? 1 : 0);
       //OPENAT_print("platform_uart_disable_rs485_oe id=%d, realIO=%d, level=%d\n", id, platform_pio_get_gpio_port(uartContext[id].rs485Io), (uartContext[id].rs485ValidLevel==0) ? 1 : 0);
        OPENAT_send_message(rs485_oe_task_hand, disable_rs485_oe, (void *)&id, sizeof(id));
    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
    }
}

void platform_uart_close_rs485_oe( unsigned char id)
{
    if(uartContext[id].opened && uartContext[id].rs485ValidLevel != RS485_INVALID_LEVEL)
    {
    /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
        //platform_uart_disable_rs485_oe(id);
        //OPENAT_print("platform_uart_close_rs485_oe id=%d, realIO=%d\n", id, platform_pio_get_gpio_port(uartContext[id].rs485Io));
        //IVTBL(close_gpio)(platform_pio_get_gpio_port(uartContext[id].rs485Io));        
        //uartContext[id].rs485ValidLevel = RS485_INVALID_LEVEL;
        OPENAT_send_message(rs485_oe_task_hand, close_rs485_oe, (void *)&id, sizeof(id));
    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
    }
}

/*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
void rs485_oe_task(void* p)
{
    char *oe_pin = NULL;
	int msgId;
    while(1)
    {
        OPENAT_wait_message(rs485_oe_task_hand, &msgId, &oe_pin, OPENAT_OS_SUSPENDED);
        switch (msgId)
        {
        case enable_rs485_oe:
            IVTBL(set_gpio)(platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io), uartContext[*oe_pin].rs485ValidLevel);
            OPENAT_print("platform_uart_enable_rs485_oe id=%d, realIO=%d, level=%d\n", *oe_pin, platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io), uartContext[*oe_pin].rs485ValidLevel);
            break;
        case disable_rs485_oe:
            // Modbus 通讯时规定主机发送完一组命令必须间隔3.5个字符再发送下一组新命令，这个3.5字符主要用来告诉其他设备这次命令（数据）已结束
            //1*1000*1000是一秒钟时间。
            //(1 * 1000 * 1000 / oe_band)代表传输一个位需要多少ms
            //比如9600bps，意思就是说每1秒（也就是1000毫秒）传输9600个位，
            //osiDelayUS(4 * 10 * (1 * 1000 * 1000 / uartContext[*oe_pin].uartBand));
            //这个地方按理说应该写4，但是等任务调度过来。实际已经过去一段时间了，为了提高传输效率。做相应的减少
            /*+\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/
            if (uartContext[*oe_pin].rs485DelayTime)
            {
				osiDelayUS(uartContext[*oe_pin].rs485DelayTime);
			}
			else
			{
				osiDelayUS(5 * (1 * 1000 * 1000 / uartContext[*oe_pin].uartBand));
            }
			/*-\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/
			IVTBL(set_gpio)(platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io), (uartContext[*oe_pin].rs485ValidLevel==0) ? 1 : 0);
            OPENAT_print("platform_uart_disable_rs485_oe id=%d, realIO=%d, level=%d\n", *oe_pin, platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io), (uartContext[*oe_pin].rs485ValidLevel==0) ? 1 : 0);
            break;
        case close_rs485_oe:
            IVTBL(set_gpio)(platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io), (uartContext[*oe_pin].rs485ValidLevel==0) ? 1 : 0);
            OPENAT_print("platform_uart_close_rs485_oe id=%d, realIO=%d\n", *oe_pin, platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io));
            IVTBL(close_gpio)(platform_pio_get_gpio_port(uartContext[*oe_pin].rs485Io));        
            uartContext[*oe_pin].rs485ValidLevel = RS485_INVALID_LEVEL;
            break;
        default:
            break;
        }
		/*+\bug_3639\rww\2020.11.18\内存泄漏死机*/
		if (oe_pin != NULL)
		{
			OPENAT_free(oe_pin);
		}
		/*-\bug_3639\rww\2020.11.18\内存泄漏死机*/
    }
}

/*+\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/
/*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
u32 platform_uart_setup_rs485_oe(unsigned id, u32 rs485IO, u32 rs485ValidLevel, u32 rs485DelayTime)
{      
    u32 ret = PLATFORM_ERR;

    if(id < NUM_UART) //非特殊端口才可以配置
    {
        if(uartContext[id].opened)
        {
            T_AMOPENAT_GPIO_CFG cfg;
            cfg.mode = OPENAT_GPIO_OUTPUT;
            cfg.param.defaultState = (rs485ValidLevel==0) ? 1 : 0;
            IVTBL(config_gpio)(platform_pio_get_gpio_port(rs485IO), &cfg);
            
            uartContext[id].rs485Io = rs485IO;
            uartContext[id].rs485ValidLevel = rs485ValidLevel;
			uartContext[id].rs485DelayTime = rs485DelayTime;
            OPENAT_print("platform_uart_setup_rs485_oe id=%d, io=%d, level=%d\n", id, rs485IO, cfg.param.defaultState);
            /*+\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
			if(rs485_oe_task_hand == NULL)
            {
                OPENAT_create_task(&rs485_oe_task_hand, rs485_oe_task, NULL, NULL,
                                            2 * 1024, 30,
                                            0,
                                            0,
                                            "rs485_oe_task");
            }
		    /*-\NEW\czm\2020.9.11\bug:2929 485发送1200波特率下最后一字节错误*/
            ret = PLATFORM_OK;
        }
    }

    return ret;
}
/*-\NEW\zhutianhua\2018.12.27 14:54\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
/*-\bug4024\zhuwangbin\2020.12.25\uart.set_rs485_oe添加可选参数,用来配置485延迟时间*/

/*+\NEW\liweiqiang\2013.4.20\增加uart.close接口 */
u32 platform_uart_close( unsigned id )
{
    u32 ret = PLATFORM_ERR;
    
    if(id < NUM_UART) //非特殊端口才可以关闭
    {
        /*+\NEW\zhutianhua\2018.12.27 15:8\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
        platform_uart_close_rs485_oe(id);
        /*-\NEW\zhutianhua\2018.12.27 15:8\新增uart.set_rs485_oe接口，可配置rs485 io使能*/
        return uart_phy_close(id);
    }
	/*+\BUG\shenyuanyuan\2020.4.10\usb at口应该可以在at和data功能功能之间灵活切换*/
	else if( id == PLATFORM_PORT_ID_USB)
	{
		usbdata_mode = 0;
		return PLATFORM_OK;
	}
	/*-\BUG\shenyuanyuan\2020.4.10\usb at口应该可以在at和data功能功能之间灵活切换*/

    return ret;
}
/*-\NEW\liweiqiang\2013.4.20\增加uart.close接口 */

u32 platform_s_uart_send( unsigned id, u8 data )
{
    u32 ret = 1;

    
    if(PLATFORM_UART_ID_ATC == id)
    {
        IVTBL(send_at_command)(&data, 1);
    }
    else if(PLATFORM_PORT_ID_DEBUG == id)
    {
/*+\NEW\liweiqiang\2013.4.7\优化debug口输出*/
        debugPortWrite(&data, 1);
/*-\NEW\liweiqiang\2013.4.7\优化debug口输出*/
    }
    else
    {
        return uart_phy_write(id, &data, 1);
    }

    return ret;
}

/*+\NEW\liweiqiang\2013.4.7\修改uart数据发送为buffer方式 */
/*+\BUG\wangyuan\2020.04.03\usb虚拟串口功能不能使用*/
//#include "at_engine.h"
//extern atDevice_t *gAtDevice;
/*-\BUG\wangyuan\2020.04.03\usb虚拟串口功能不能使用*/
u32 platform_s_uart_send_buff( unsigned id, const u8 *buff, u16 len )
{
    u32 ret = len;

    
    if(PLATFORM_UART_ID_ATC == id)
    {
        IVTBL(send_at_command)((UINT8*)buff, len);
    }
    else if(PLATFORM_PORT_ID_DEBUG == id)
    {
        debugPortWrite(buff, len);
    }
	/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
	// else if(PLATFORM_PORT_ID_USB == id)
    // {  
    // 	int    plen = 0;
	// 	char   *pbuf = NULL;
	//     if(buff != NULL && gAtDevice != NULL)
	//     {
    //         /*+\BUG\hedonghao\2019.9.2\使用usbdata的demo，pc端串口工具通过usb枚举出的usb at口和模块通信，串口工具发送一些特殊字符结尾的数据(最后再加上回车换行)，模块会死机，必现 */
	//         plen = len+3;
	//         pbuf = (char *)malloc(plen * sizeof(char));
	//         ASSERT(pbuf != NULL);
    //         sprintf(pbuf, "%s\r\n",buff);
    //         pbuf[plen - 1] = '\0';
	// 		/*+\BUG\wangyuan\2020.04.03\usb虚拟串口功能不能使用*/
	// 		atDeviceWrite(gAtDevice, buff, len);
	// 		/*-\BUG\wangyuan\2020.04.03\usb虚拟串口功能不能使用*/
    //         /*-\BUG\hedonghao\2019.9.2\使用usbdata的demo，pc端串口工具通过usb枚举出的usb at口和模块通信，串口工具发送一些特殊字符结尾的数据(最后再加上回车换行)，模块会死机，必现 */
	// 		free(pbuf);
	// 	}
    // }
	/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
    else
    {
        OPENAT_print("uart phy write:%d %d", id,len);
        return uart_phy_write(id, (uint8 *)buff, len);
    }

    return ret;
}
/*-\NEW\liweiqiang\2013.4.7\修改uart数据发送为buffer方式 */

/* 兼容旧版本的sleep接口 */
void platform_os_sleep(u32 ms)
{
    IVTBL(sleep)(ms);
}

int platform_s_uart_recv( unsigned id, s32 timeout )
{
    uint8 rcvdBuf[1];
    int rcvdLength = 0;
    
    if(PLATFORM_UART_ID_ATC == id)
    {
/*+\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */
        rcvdLength = vatc_read(rcvdBuf, 1, timeout);
/*-\NEW\liweiqiang\2014.4.12\优化对虚拟AT每次只读取一个字节的处理 */
    }
	/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
	else if(PLATFORM_PORT_ID_USB == id)
    {
        rcvdLength = usb_read(rcvdBuf, 1, timeout);
    }
	/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
    else if(PLATFORM_PORT_ID_DEBUG == id)
    {
        // debug口读取直接堵塞挂起
        HANDLE hDebugPortReadSem = IVTBL(create_semaphore)(0);
        PUB_TRACE("[platform_s_uart_recv]: read from debug port, stop!");
        IVTBL(wait_semaphore)(hDebugPortReadSem, COS_WAIT_FOREVER);
        return -1;
    }
    else
    {
        rcvdLength = uart_phy_read(id, rcvdBuf, 1, timeout);
    }
    
    return rcvdLength ? rcvdBuf[0] : -1;
}

int platform_s_uart_set_flow_control( unsigned id, int type )
{
  return PLATFORM_ERR;
}
/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
int platform_uart_config_event(int uartId, PLATFORM_UART_EVENT_CONFIG event)
{
	gUartConfig.UartConfigEvent = event;
	gUartConfig.UartConfigId = uartId;
	return 1;
}
/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
#endif
