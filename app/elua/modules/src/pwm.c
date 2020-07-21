/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    adc.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/6/6
 *
 * Description:
 *          lua.pwm pwm访问库
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
/*+\NEW\RUFEI\2015.8.27\Add adc fuction*/
static int pwm_open(lua_State *L) {
    int id = luaL_checkinteger(L, 1);
    int ret;

    ret = platform_pwm_open(id);

    lua_pushinteger(L, ret);

    return 1; 
}

static int pwm_close(lua_State *L) {
    int ret;
    int id = luaL_checkinteger(L, 1);

    ret = platform_pwm_close(id);

    lua_pushinteger(L, ret);

    return 1; 
}
/*-\NEW\RUFEI\2015.8.27\Add adc fuction*/
// adc.read(id)
/*+\bug\wj\2020.4.30\lua添加pwm接口*/
static int pwm_set(lua_State *L) {    
    int id = luaL_checkinteger(L,1);
    int param0 = luaL_checkinteger(L, 2);
    int param1 = luaL_checkinteger(L, 3);
    
    platform_pwm_set(id, param0, param1);
   
    return 1; 
}
/*-\bug\wj\2020.4.30\lua添加pwm接口*/

#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE pwm_map[] =
{ 
  { LSTRKEY( "open" ),  LFUNCVAL( pwm_open ) },
  { LSTRKEY( "set" ),  LFUNCVAL( pwm_set ) },
  { LSTRKEY( "close" ),  LFUNCVAL( pwm_close ) },
  
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pwm( lua_State *L )
{
    luaL_register( L, AUXLIB_PWM, pwm_map );

    return 1;
}  
