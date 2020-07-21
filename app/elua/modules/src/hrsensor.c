
/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    hrsensor.c
 * Author:  
 * Version: V0.1
 * Date:    2015.11.17
 *
 * Description:
 *          lua.hrsensor hrsensor∑√Œ ø‚
 **************************************************************************/
#ifdef HRD_SENSOR_SUPPORT
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_hrsensor.h"


static int hrsensor_open(lua_State *L) {
    PUB_TRACE("hrsensor_open");

    platform_hrsensor_start();
    return 1; 
}

static int hrsensor_get(lua_State *L) {
    int ret = 0;

    ret = platform_hrsensor_getrate();
    
    PUB_TRACE("hrsensor_get rate = %d",ret);

    lua_pushinteger(L, ret);

    return 1; 
}

static int hrsensor_close(lua_State *L) {    
    PUB_TRACE("hrsensor_close");
    
    platform_hrsensor_close();
   
    return 1; 
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE hrsensor_map[] =
{ 
  { LSTRKEY( "open" ),  LFUNCVAL( hrsensor_open ) },
  { LSTRKEY( "get" ),  LFUNCVAL( hrsensor_get ) },
  { LSTRKEY( "close" ),  LFUNCVAL( hrsensor_close ) },
  
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_hrsensorcore( lua_State *L )
{
    luaL_register( L, AUXLIB_HRSENSOR, hrsensor_map );

    return 1;
}  
#endif

