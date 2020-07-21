/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    ttsply.c
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/03/17
 *
 * Description:
 *          ttsply.c
 *
 * History:
 *     panjun 2016.03.17 Add TTS's API according to MTK.
 **************************************************************************/


#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_ttsply.h"
#include "lrodefs.h"  
/*+\new\wj\2019.12.27\添加TTS功能*/
#if defined(__AM_LUA_TTSPLY_SUPPORT__)

#define MIN_OPT_LEVEL 2

static int l_ttsply_initEngine(lua_State *L) {
#if 0
    ttsPlyParam param;

    param.volume = cast(s16, luaL_checkinteger(L, 1));
    param.speed = cast(s16, luaL_checkinteger(L, 2));
    param.pitch = cast(s16, luaL_checkinteger(L, 3));
    param.codepage = cast(u16, luaL_checkinteger(L, 4));
    param.digit_mode = cast(u8, luaL_checkinteger(L, 5));
    param.punc_mode = cast(u8, luaL_checkinteger(L, 6));
    param.tag_mode = cast(u8, luaL_checkinteger(L, 7));
    param.wav_format = cast(u8, luaL_checkinteger(L, 8));
    param.eng_mode = cast(u8, luaL_checkinteger(L, 9));
#endif
    lua_pushboolean(L, platform_ttsply_initEngine() == 1);

    return 1;
}

static int l_ttsply_setParam(lua_State *L) {
    u16 plyParam = cast(u16, luaL_checkinteger(L, 1));
    s16 value = cast(u16, luaL_checkinteger(L, 2));

    lua_pushboolean(L, platform_ttsply_setParam(plyParam, value) == 1);

    return 1;
}

#if 0
static int l_ttsply_getParam(lua_State *L) {
    u16 plyParam = cast(u16, luaL_checkinteger(L, 1));

    lua_pushinteger(L, platform_ttsply_getParam(plyParam));

    return 1;
}
#endif

static int l_ttsply_play(lua_State *L) {
    ttsPly param;
    param.text = cast(char*, luaL_checkstring(L,1));	
    lua_pushboolean(L, platform_ttsply_play(&param) == 1);
    return 1;
}
#if 0
static int l_ttsply_pause(lua_State *L) {
    /*
    ttsPly param;

    param.text = luaL_checkstring(L,1);
    param.text_size = cast(u32, luaL_checkinteger(L, 2));
    param.spk_vol = cast(u8, luaL_checkinteger(L, 3));
  
    lua_pushinteger(L, platform_ttsply_pause(&param));
    */

    lua_pushinteger(L, platform_ttsply_pause());
    
    return 1;
}
#endif
static int l_ttsply_stop(lua_State *L) {
    /*
    ttsPly param;

    param.text = luaL_checkstring(L,1);
    param.text_size = cast(u32, luaL_checkinteger(L, 2));
    param.spk_vol = cast(u8, luaL_checkinteger(L, 3));
  
    lua_pushinteger(L, platform_ttsply_stop(&param));
    */
    lua_pushboolean(L, platform_ttsply_stop() == 1);
    
    return 1;
}

// Module function map
const LUA_REG_TYPE ttsplycore_map[] =
{ 
  { LSTRKEY( "initEngine" ),  LFUNCVAL( l_ttsply_initEngine ) },
  { LSTRKEY( "setParm" ),  LFUNCVAL( l_ttsply_setParam ) },
 // { LSTRKEY( "getParam" ),  LFUNCVAL( l_ttsply_getParam ) },
  { LSTRKEY( "play" ),  LFUNCVAL( l_ttsply_play ) },
//  { LSTRKEY( "pause" ),  LFUNCVAL( l_ttsply_pause ) },
  { LSTRKEY( "stop" ),  LFUNCVAL( l_ttsply_stop ) },

  { LNILKEY, LNILVAL }
};
/*-\new\wj\2019.12.27\添加TTS功能*/
LUALIB_API int luaopen_ttsplycore( lua_State *L )
{
    luaL_register( L, AUXLIB_TTSPLYCORE, ttsplycore_map );

    return 1;
}

#else

// Module function map
const LUA_REG_TYPE ttsplycore_map[] =
{ 
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_ttsplycore( lua_State *L )
{
    luaL_register( L, AUXLIB_TTSPLYCORE, ttsplycore_map );

    return 1;
}

#endif //__AM_LUA_TTSPLY_SUPPORT__

