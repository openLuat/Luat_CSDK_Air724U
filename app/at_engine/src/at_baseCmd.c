#include "at.h"
#include "at_baseCmd.h"
//#include "gsm.h"
//执行AT命令
//存储数据
//上传

at_cmdResult at_NEW_Cmdtest(uint8_t id)
{
	char buf[] = "AT+NEW TEST";
	at_uart_send(buf, sizeof(buf) - 1);
	return cmdResultOk;
}

at_cmdResult at_NEW_Cmdquery(uint8_t id)
{
	char buf[] = "AT+NEW QUERY";
	at_uart_send(buf, sizeof(buf) - 1);
	return cmdResultOk;
}

at_cmdResult at_NEW_Cmdsetup(uint8_t id, char *pPara)
{
	char *data = pPara + 1;
	char buf[] = "AT+NEW SETUP pPara is: ";
	at_uart_send(buf, sizeof(buf) - 1);
	at_uart_send(data, strlen(data) - 2);
	return cmdResultOk;
}

at_cmdResult at_NEW_Cmdexe(uint8_t id)
{
	char buf[] = "AT+NEW EXE";
	at_uart_send(buf, sizeof(buf) - 1);
	return cmdResultOk;
}
