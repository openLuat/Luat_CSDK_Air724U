#include "at.h"
#include "at_cmd.h"
#include "at_baseCmd.h"

/** @defgroup AT_BASECMD_Functions
  * @{
  */
#define AT_CMD_TABLE_END(atFunTable)      \
	(((atFunTable).at_cmdName == NULL &&  \
	  (atFunTable).at_cmdLen == 0 &&      \
	  (atFunTable).at_testCmd == NULL &&  \
	  (atFunTable).at_queryCmd == NULL && \
	  (atFunTable).at_setupCmd == NULL && \
	  (atFunTable).at_exeCmd == NULL)     \
		 ? TRUE                           \
		 : FALSE)

/**
  * @brief  Query and localization one commad.
  * @param  cmdLen: received length of command
  * @param  pCmd: point to received command 
  * @retval the id of command
  *   @arg -1: failure
  */
static int16_t ICACHE_FLASH_ATTR at_cmdSearch(int8_t cmdLen, uint8_t *pCmd, at_funcationType *atFunTable)
{
	int16_t i;
	/*+/NEW WIFI-10/ZWB/2014.11.18/兼容AT指令大小写*/
	uint8_t *temp_pCmd = pCmd;

	for (i = 0; i < cmdLen; i++, temp_pCmd++)
	{
		if (*temp_pCmd >= 'a' && *temp_pCmd <= 'z')
		{
			*temp_pCmd = *temp_pCmd - ('a' - 'A');
		}
	}
	/*-/NEW WIFI-10/ZWB/2014.11.18/兼容AT指令大小写*/

	if (cmdLen == 0)
	{
		return 0;
	}
	else if (cmdLen > 0)
	{
		for (i = 0;; i++)
		{
			if (AT_CMD_TABLE_END(atFunTable[i]))
			{
				break;
			}

			if (cmdLen == atFunTable[i].at_cmdLen)
			{

				if (os_memcmp(pCmd, atFunTable[i].at_cmdName, cmdLen) == 0) //think add cmp len first
				{
					return i;
				}
			}
		}
	}
	return -1;
}

/**
  * @brief  Get the length of commad.
  * @param  pCmd: point to received command 
  * @retval the length of command
  *   @arg -1: failure
  */
static int8_t ICACHE_FLASH_ATTR at_getCmdLen(uint8_t *pCmd)
{
	uint8_t n, i;

	n = 0;
	i = 128;

	while (i--)
	{
		if ((*pCmd == '\r') || (*pCmd == '=') || (*pCmd == '?') || ((*pCmd >= '0') && (*pCmd <= '9')))
		{
			return n;
		}
		else
		{
			pCmd++;
			n++;
		}
	}
	return -1;
}

/**
  * @brief  Distinguish commad and to execution.
  * @param  pAtRcvData: point to received (command) 
  * @retval None
  */
at_cmdResult at_cmdProcess(uint8_t *pAtRcvData, at_funcationType *atFunTable)
{
	char tempStr[32];

	int16_t cmdId;
	int8_t cmdLen;
	uint16_t i;
	at_cmdResult result = cmdResultOk;

	T_AT_CONTEXT *atContextP = at_ptrAtContext();

	uint8_t *excludeAtHead = pAtRcvData + 2;

	atContextP->atState = at_statProcess;

	cmdLen = at_getCmdLen(excludeAtHead);

	if (cmdLen != -1)
	{
		if (cmdLen == 0)
		{
			if (atContextP->echoMode)
			{
				at_uart_send(pAtRcvData, strlen(pAtRcvData));
			}
			return cmdResultOk;
		}
		else
		{
			cmdId = at_cmdSearch(cmdLen, excludeAtHead, atFunTable);
		}
	}
	else
	{
		cmdId = -1;
	}
	if (cmdId != -1)
	{
		if (atContextP->echoMode)
		{
			at_uart_send(pAtRcvData, strlen(pAtRcvData));
		}
		at_debug("[at_engine] AT < %s\r\n", atFunTable[cmdId].at_cmdName);
		if (atFunTable[cmdId].at_cmdNameCore == NULL)
		{
			at_putNewLine();
			excludeAtHead += cmdLen;
			if (*excludeAtHead == '\r')
			{
				if (atFunTable[cmdId].at_exeCmd)
				{
					result = atFunTable[cmdId].at_exeCmd(cmdId);
				}
				else
				{
					return cmdResultNotSupport;
				}
			}
			else if (*excludeAtHead == '?' && (excludeAtHead[1] == '\r'))
			{
				if (atFunTable[cmdId].at_queryCmd)
				{
					result = atFunTable[cmdId].at_queryCmd(cmdId);
				}
				else
				{
					return cmdResultNotSupport;
				}
			}
			else if ((*excludeAtHead == '=') && (excludeAtHead[1] == '?') && (excludeAtHead[2] == '\r'))
			{
				if (atFunTable[cmdId].at_testCmd)
				{
					result = atFunTable[cmdId].at_testCmd(cmdId);
				}
				else
				{
					return cmdResultNotSupport;
				}
			}
			else if (((*excludeAtHead >= '0') && (*excludeAtHead <= '9')) || (*excludeAtHead == '='))
			{
				if (atFunTable[cmdId].at_setupCmd)
				{
					atFunTable[cmdId].at_setupCmd(cmdId, excludeAtHead);
				}
				else
				{
					return cmdResultNotSupport;
				}
			}
			else
			{
				return cmdResultNotSupport;
			}
		}
		else
		{
			uint8_t *AtCMDHead = excludeAtHead + atFunTable[cmdId].at_cmdLen;
			char *buf1 = "AT%s%s";
			char buf2[100] = {0};
			uint8 len = os_sprintf(buf2, buf1, atFunTable[cmdId].at_cmdNameCore, AtCMDHead);
			at_debug("[at_engine] buf2:%s len:%d\r\n", buf2, len);
			iot_vat_send_cmd(buf2, len);

			return cmdResultReplace;
		}
	}
	else
	{
		result = cmdResultNotFound;
	}

	return result;
}

void at_cmdResponse(at_cmdResult result)
{
	T_AT_CONTEXT *atContextP = at_ptrAtContext();

	atContextP->atState = at_statIdle;
	switch (result)
	{
	case cmdResultError:
	case cmdResultNotSupport:
		at_backError;
		break;
	case cmdResultProcessAgain:
	case cmdResultNull:
	case cmdResultNotFound:
	case cmdResultReplace:
		break;
	case cmdResultOk:
		at_putNewLine();
		at_backOk;
		break;
	case cmdResultProcessing:
		atContextP->atState = at_statProcess;
		break;
	default:
		ASSERT(0);
		break;
	}
}

/**
  * @}
  */
