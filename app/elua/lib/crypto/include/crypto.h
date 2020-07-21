/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:   crypto.h
 * Author:  zhutianhua
 * Date:    2013/4/17
 *
 * Description:
 *          lua平台层crypto库接口
 **************************************************************************/

#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include"type.h"
#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "auxmods.h"

#include "aliyun_iot_common_base64.h"
#include "md5.h"
#include "I_aes.h"
#include "crc.h"
#include "sha1.h"
#include "compat-1.3.h"

#include "aes.h"

typedef unsigned char u8;
typedef unsigned int uint;

#endif
