#ifndef __AT_CMD_H
#define __AT_CMD_H
#include "at.h"

at_cmdResult at_cmdProcess(uint8_t *pAtRcvData, at_funcationType* atFunTable);
void at_cmdResponse(at_cmdResult result);

extern at_funcationType at_fun[];

#endif
