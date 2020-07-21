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
#ifdef LUA_QRENCODE_SUPPORT

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_pmd.h"
#include "qrencode.h"

// adc.open(id)
/*+\NEW\RUFEI\2015.8.27\Add adc fuction*/

extern QRcode* qrencode(const unsigned char *intext, int length);

extern  void put_qr_code_buff(unsigned char* buff, int width, int dispHeight);

static int qr_encode_show(lua_State *L) {
    size_t  len      = 0;
    int width;
    int disp_height;
    
    char* url_string = (char*)luaL_checklstring(L, 1, &len);
    
    QRcode* code;

    disp_height = luaL_optint(L, 2, 0);
    
    code  = qrencode(url_string, len);

    put_qr_code_buff(code->data, code->width, disp_height);

    qrencode_free(code);
    
    return 0; 
}

/*+\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */
static int l_qr_create(lua_State *L) {
    size_t  len      = 0;
    int width;
    int disp_height;
    int idx;
    
    char* url_string = (char*)luaL_checklstring(L, 1, &len);
    
    QRcode* code;

    disp_height = luaL_optint(L, 2, 0);
    
    code  = qrencode(url_string, len);

    lua_pushinteger(L, code->width);
    lua_pushinteger(L, code->width);
    lua_newtable(L);

    
    for( idx = 1; idx <= code ->width * code->width; idx ++ )
    {
      lua_pushinteger( L, ((code->data[idx - 1] & 1) ? 0xff : 0x00));
      lua_rawseti( L, -2, idx );
    }
    
    qrencode_free(code);
    
    return 3; 
}

static int l_qr_encode(lua_State *L) {
    size_t len = 0;
    QRcode* code;
    char* s = (char*)luaL_checklstring(L, 1, &len);
    int qr_version = luaL_optint(L, 2, 0);
    int qr_level = luaL_optint(L, 3, 0);
    
    code  = QRcode_encodeData(len, s, qr_version, qr_level);

    lua_pushinteger(L, code->width);
    lua_pushlstring(L, code->data, code->width*code->width);
    QRcode_free(code);
    
    return 2; 
}
/*-\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */

#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE qr_encode_map[] =
{ 
  { LSTRKEY( "show" ),  LFUNCVAL( qr_encode_show ) },

  /*+\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */	  
  { LSTRKEY( "build" ),  LFUNCVAL( l_qr_create ) },
  { LSTRKEY("encode"), LFUNCVAL(l_qr_encode)},
  /*-\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */	

  
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_qr_encode( lua_State *L )
{
    luaL_register( L, AUXLIB_QRENCODE, qr_encode_map );

    return 1;
}  
#endif

