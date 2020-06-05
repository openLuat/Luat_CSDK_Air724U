/*********************************************************
  Copyright (C), AirM2M Tech. Co., Ltd.
  Author: lifei
  Description: AMOPENAT 开放平台
  Others:
  History: 
    Version： Date:       Author:   Modification:
    V0.1      2012.12.14  lifei     创建文件
*********************************************************/
#ifndef AM_OPENAT_VAT_H
#define AM_OPENAT_VAT_H

#include "am_openat_common.h"


typedef VOID (*PAT_MESSAGE)(UINT8 *pData, UINT16 length);


BOOL OPENAT_vat_init(PAT_MESSAGE resp_cb);
BOOL OPENAT_vat_send_at( const char* pAtCommand, unsigned nLength );

int vat_test_enter(void *param);



#endif /* AM_OPENAT_VAT_H */

