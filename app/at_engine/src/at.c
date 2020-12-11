#include "at.h"
#include "at_cmd.h"
#include "iot_vat.h"
#include "at_process.h"
#include "at_baseTable.h"

#define AT_CLEAR_UART_CHAR()                  \
	do                                        \
	{                                         \
		HANDLE cr;                            \
		cr = iot_os_enter_critical_section(); \
		atContextP->uartDataRecivedLen = 0;   \
		iot_os_exit_critical_section(cr);     \
	} while (0)

static HANDLE g_s_atRecvTask = NULL;
static HANDLE g_s_atProcTask = NULL;

static T_AT_CONTEXT g_s_atContext = {0};

uint8 g_s_at_resultBuff[AM_AT_RESULT_BUF_SIZE];

static VOID vatIndHandle(UINT8 *pData, UINT16 length)
{
	T_AT_CONTEXT *atContextP = at_ptrAtContext();
	uint16 len;

	at_debug("[at_engine] vat ind  toPort = %d, data %s", atContextP->vatIndToUartPort, pData);
	if (atContextP->vatIndToUartPort)
	{
		len = at_uart_send(pData, length);
		at_debug("[at_engine] send len = %d", len);
	}
	else
	{
		at_message(pData, length);
	}
}

static void ICACHE_FLASH_ATTR at_procTask()
{
	T_AT_CONTEXT *atContextP = at_ptrAtContext();

	os_event_t *events;
	char *cmdString;
	at_cmdResult result;

	while (1)
	{
		iot_os_wait_message(g_s_atProcTask, (PVOID *)&events);

		cmdString = (char *)events->par;

		at_debug("[at_engine] process status=%d, at %s", atContextP->atState, cmdString);

		result = at_cmdProcess(cmdString, at_fun);

		if (result == cmdResultNotFound || result == cmdResultProcessAgain)
		{
			iot_vat_send_cmd(cmdString, strlen(cmdString));
		}
		at_cmdResponse(result);

		iot_os_free((PVOID)events->par);
		iot_os_free(events);
	}
}

static void at_recvTask(PVOID param)
{

	uint8_t *pCmdLine, *pStartCmdLine;

	uint32 index;
	uint8_t temp;

	uint32 cmdIndex = 0;
	uint32 cmdLenght = 0;
	T_AT_CONTEXT *atContextP = at_ptrAtContext();
	os_event_t *events = NULL;
	while (1)
	{
		iot_os_wait_message(g_s_atRecvTask, (PVOID)&events);
		at_debug("[at_engine] recv sig %d", events->sig);
		switch (events->sig)
		{
		case UART_EVENT_RECV_DATA:
		{

			pStartCmdLine = atContextP->uartDataBuffer;
			pCmdLine = atContextP->uartDataBuffer + cmdIndex;
			cmdLenght = atContextP->uartDataRecivedLen - cmdIndex;

			at_debug("[at_engine] uart recv len=%d data %s", atContextP->uartDataRecivedLen,
					 atContextP->uartDataBuffer);
			at_debug("[at_engine] atState=%d", atContextP->atState);
			if (atContextP->uartDataRecivedLen == 0)
			{
				break;
			}
			{
				for (index = 0; index < atContextP->uartDataRecivedLen; index++)
				{
					temp = pStartCmdLine[index];
					switch (atContextP->atState)
					{
					case at_statIdle: //serch "AT" head

						if (temp == '\r')
						{
							if ((os_memcmp(pStartCmdLine, "AT", 2) == 0) || (os_memcmp(pStartCmdLine, "at", 2) == 0) || os_memcmp(pStartCmdLine, "At", 2) == 0 || os_memcmp(pStartCmdLine, "aT", 2) == 0)
							{
								char *cmdString = iot_os_malloc(index + 2 + 1);
								os_memcpy(cmdString, pStartCmdLine, index);
								cmdString[index++] = '\r';
								cmdString[index++] = '\n';
								cmdString[index] = 0;
								system_os_post(at_procTaskPrio, 0, (os_param_t)cmdString);
								goto uartBufferFree;
							}
							else
							{
								at_backError;
								atContextP->atState = at_statIdle;
								goto uartBufferFree;
							}
						}
						else if (atContextP->uartDataRecivedLen >= at_cmdLenMax - 1)
						{
							atContextP->atState = at_statIdle;
							goto uartBufferFree;
						}
						break;

					case at_statProcess: //process data
						if (temp == '\r')
						{
							AM_AT_SEND_RESULT(AM_ERR_DEVICE_BUSY);
							goto uartBufferFree;
						}
						break;
					case at_statIpSended: //send data
						if (temp == '\r')
						{
							AM_AT_SEND_RESULT(AM_ERR_DEVICE_BUSY);
							goto uartBufferFree;
						}
						break;

					default:
						ASSERT(0);
						break;
					}
				}
			}
			break;
		}
		default:
			break;
		}

		iot_os_free(events);

		continue;
	uartBufferFree:
		AT_CLEAR_UART_CHAR();
		cmdIndex = 0;
		iot_os_free(events);
	}
}

bool system_os_post(uint8 prio, os_signal_t sig, os_param_t par)
{
	os_event_t *evt = iot_os_malloc(sizeof(os_event_t));
	evt->par = par;
	evt->sig = sig;
	switch (prio)
	{
	case at_recvTaskPrio:
		return iot_os_send_message(g_s_atRecvTask, (PVOID)evt);
	case at_procTaskPrio:
		return iot_os_send_message(g_s_atProcTask, (PVOID)evt);
	default:
		iot_os_free(evt);
		return FALSE;
	}
}

T_AT_CONTEXT *at_ptrAtContext(void)
{
	return &g_s_atContext;
}

void at_task_init(void)
{

	g_s_atRecvTask = iot_os_create_task((PTASK_MAIN)at_recvTask, NULL,
										4096, at_recvTaskPrio, OPENAT_OS_CREATE_DEFAULT, "at_recvTask");
	if (g_s_atRecvTask == NULL)
	{
		iot_debug_assert(0, __func__, __LINE__);
	}
	g_s_atProcTask = iot_os_create_task((PTASK_MAIN)at_procTask, NULL,
										4096, at_procTaskPrio, OPENAT_OS_CREATE_DEFAULT, "atProc");
	if (g_s_atProcTask == NULL)
	{
		iot_debug_assert(0, __func__, __LINE__);
	}

    at_uart_init();
    at_vat_init();
    at_init();
}

void at_vat_init()
{
	memset(&g_s_atContext, 0, sizeof(g_s_atContext));
	g_s_atContext.vatIndToUartPort = TRUE;
	g_s_atContext.echoMode = TRUE;
	iot_vat_init(vatIndHandle);
}
