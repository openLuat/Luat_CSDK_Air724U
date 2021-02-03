/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    adc.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/6/6
 *
 * Description:
 *          lua.adc adc访问库
 **************************************************************************/

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_pmd.h"

// adc.open(id)
/*+\bug3689\zhuwangbin\2020.11.25\adc添加可选参数scale*/
static int adc_open(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
	int scale =  luaL_optinteger(L, 2, 0);
    int ret;
	
    MOD_CHECK_ID(adc, id);

    ret = platform_adc_open(id, 0, scale);

    lua_pushinteger(L, ret);

    return 1; 
}
/*-\bug3689\zhuwangbin\2020.11.25\adc添加可选参数scale*/

// adc.read(id)
static int adc_read(lua_State *L) {    
    int id = luaL_checkinteger(L,1);
    int adc, volt;

    MOD_CHECK_ID(adc, id);

    platform_adc_read(id, &adc, &volt);

    lua_pushinteger(L, adc);
    lua_pushinteger(L, volt);
   
    return 2; 
}

static int adc_close(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int ret;

    MOD_CHECK_ID(adc, id);

    ret = platform_adc_close(id);

    lua_pushinteger(L, ret);

    return 1; 
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE adc_map[] =
{ 
  { LSTRKEY( "open" ),  LFUNCVAL( adc_open ) },
  { LSTRKEY( "read" ),  LFUNCVAL( adc_read ) },
  { LSTRKEY( "close" ),  LFUNCVAL( adc_close ) },

  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_adc( lua_State *L )
{
    luaL_register( L, AUXLIB_ADC, adc_map );

	/*+\bug3689\zhuwangbin\2020.11.25\adc添加可选参数scale*/
	MOD_REG_NUMBER(L, "SCALE_1V250", PLATFORM_ADC_SCALE_1V250);
    MOD_REG_NUMBER(L, "SCALE_2V444", PLATFORM_ADC_SCALE_2V444);
    MOD_REG_NUMBER(L, "SCALE_3V233", PLATFORM_ADC_SCALE_3V233);
    MOD_REG_NUMBER(L, "SCALE_5V000", PLATFORM_ADC_SCALE_5V000);
	/*-\bug3689\zhuwangbin\2020.11.25\adc添加可选参数scale*/
	
    return 1;
}  
