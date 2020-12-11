#include "at.h"
#include "at_cmd.h"
#include "iot_uart.h"
/** @defgroup AT_PORT_Defines
  * @{
  */

extern at_funcationType at_fun[];

/**
  * @brief  Uart receive task.
  * @param  events: contain the uart receive data
  * @retval None
  */
/**
  * @brief  Task of process command or txdata.
  * @param  events: no used
  * @retval None
  */

#define AT_ADD_UART_CHAR()                                                                          \
	cr = iot_os_enter_critical_section();                                                           \
	if (atContextP->uartDataRecivedLen >= at_dataLenMax)                                            \
	{                                                                                               \
		break;                                                                                      \
	}                                                                                               \
	len = iot_uart_read(OPENAT_UART_1, atContextP->uartDataBuffer + atContextP->uartDataRecivedLen, \
						at_dataLenMax - atContextP->uartDataRecivedLen, 0);                         \
	if (len > 0)                                                                                    \
	{                                                                                               \
		atContextP->uartDataRecivedLen += len;                                                      \
	}                                                                                               \
	iot_os_exit_critical_section(cr);

void at_uart_handle(T_AMOPENAT_UART_MESSAGE *evt)
{
	HANDLE cr;
	UINT32 len;
	T_AT_CONTEXT *atContextP = at_ptrAtContext();
	switch (evt->evtId)
	{
	case OPENAT_DRV_EVT_UART_RX_DATA_IND:
		AT_ADD_UART_CHAR();
		if (len > 0)
		{
			system_os_post(at_recvTaskPrio, UART_EVENT_RECV_DATA, len);
		}
		break;
	default:
		break;
	}
}
void at_uart_init(void)
{
	T_AMOPENAT_UART_PARAM cfg;

	cfg.baud = OPENAT_UART_BAUD_9600;
	cfg.dataBits = 8;
	cfg.stopBits = 1;
	cfg.parity = OPENAT_UART_NO_PARITY;
	cfg.flowControl = OPENAT_UART_FLOWCONTROL_NONE;
	cfg.txDoneReport = NULL;
	cfg.uartMsgHande = at_uart_handle;

	iot_uart_open(OPENAT_UART_1, &cfg);
}

uint32 at_uart_send(char *data, uint32 len)
{
	return iot_uart_write(OPENAT_UART_1, data, len);
}

/**
  * @}
  */
