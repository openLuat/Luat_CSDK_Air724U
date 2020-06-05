#ifndef __OPENAT_OTA_H__
#define __OPENAT_OTA_H__

#include "am_openat.h"

E_OPENAT_OTA_RESULT openat_otaInit(void);

E_OPENAT_OTA_RESULT openat_otaProcess(char* data, unsigned int len, unsigned int total);

E_OPENAT_OTA_RESULT openat_otaDone(void);


#endif
