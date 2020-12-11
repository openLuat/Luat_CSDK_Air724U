

#ifndef __AT_BASETABLE_H
#define __AT_BASETABLE_H

#include "at.h"
#include "at_baseCmd.h"


at_funcationType at_fun[] = {
	{NULL, "+NEW", 4, at_NEW_Cmdtest, at_NEW_Cmdquery, at_NEW_Cmdsetup, at_NEW_Cmdexe},
	{"I", "+REPI", 5, NULL, NULL, NULL, NULL},
	{"+WIMEI", "+REPWIMEI", 9, NULL, NULL, NULL, NULL},
	{NULL, NULL, 0, NULL, NULL, NULL, NULL}};

#endif
