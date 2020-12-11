#ifndef __AT_BASECMD_H
#define __AT_BASECMD_H


at_cmdResult at_NEW_Cmdtest(uint8_t id);

at_cmdResult at_NEW_Cmdquery(uint8_t id);

at_cmdResult at_NEW_Cmdsetup(uint8_t id, char *pPara);

at_cmdResult at_NEW_Cmdexe(uint8_t id);

#endif