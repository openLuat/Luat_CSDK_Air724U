/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    auxmods.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/10/8
 *
 * Description:
 * 
 **************************************************************************/
// Auxiliary Lua modules. All of them are declared here, then each platform
// decides what module(s) to register in the src/platform/xxxxx/platform_conf.h file

#ifndef __AUXMODS_H__
#define __AUXMODS_H__

#include "lua.h"

#define AUXLIB_PIO      "pio"
LUALIB_API int ( luaopen_pio )( lua_State *L );

#define AUXLIB_SPI      "spi"
LUALIB_API int ( luaopen_spi )( lua_State *L );

#define AUXLIB_CAN      "can"
LUALIB_API int ( luaopen_can )( lua_State *L );

#define AUXLIB_TMR      "tmr"
LUALIB_API int ( luaopen_tmr )( lua_State *L );

#define AUXLIB_PD       "pd"
LUALIB_API int ( luaopen_pd )( lua_State *L );

#define AUXLIB_UART     "uart"
LUALIB_API int ( luaopen_uart )( lua_State *L );

#define AUXLIB_TERM     "term"
LUALIB_API int ( luaopen_term )( lua_State *L );

#define AUXLIB_PWM      "pwm"
LUALIB_API int ( luaopen_pwm )( lua_State *L );

#define AUXLIB_PACK     "pack"
LUALIB_API int ( luaopen_pack )( lua_State *L );

#define AUXLIB_BIT      "bit"
LUALIB_API int ( luaopen_bit )( lua_State *L );

#define AUXLIB_NET      "net"
LUALIB_API int ( luaopen_net )( lua_State *L );

#define AUXLIB_CPU      "cpu"
LUALIB_API int ( luaopen_cpu )( lua_State* L );

#define AUXLIB_ADC      "adc"
LUALIB_API int ( luaopen_adc )( lua_State *L );

#define AUXLIB_RPC   "rpc"
LUALIB_API int ( luaopen_rpc )( lua_State *L );

#define AUXLIB_BITARRAY "bitarray"
LUALIB_API int ( luaopen_bitarray )( lua_State *L );

#define AUXLIB_I2C  "i2c"
LUALIB_API int ( luaopen_i2c )( lua_State *L );

#ifdef LUA_GPIO_I2C
#define AUXLIB_GPIO_I2C  "gpio_i2c"
LUALIB_API int ( luaopen_gpio_i2c )( lua_State *L );
#endif
#define AUXLIB_RTOS     "rtos"
LUALIB_API int ( luaopen_rtos )( lua_State *L );

#ifdef LUA_DISP_LIB
#define AUXLIB_DISP     "disp"
LUALIB_API int ( luaopen_disp )( lua_State *L );
#endif
#ifndef AM_JSON_NOT_SUPPORT
#define AUXLIB_JSON     "json"
LUALIB_API int ( luaopen_cjson)( lua_State *L );
#endif
#define AUXLIB_CRYPTO     "crypto"
LUALIB_API int ( luaopen_crypto)( lua_State *L );

#define AUXLIB_PMD     "pmd"
LUALIB_API int ( luaopen_pmd )( lua_State *L );

/*+\NEW\liweiqiang\2013.7.16\增加iconv字符编码转换库 */
#define AUXLIB_ICONV     "iconv"
LUALIB_API int ( luaopen_iconv)( lua_State *L );
/*-\NEW\liweiqiang\2013.7.16\增加iconv字符编码转换库 */

#ifdef LUA_AUDIO_LIB
/*+\NEW\liweiqiang\2013.11.4\增加audio.core接口库 */
#define AUXLIB_AUDIOCORE "audiocore"
LUALIB_API int ( luaopen_audiocore)( lua_State *L );
/*-\NEW\liweiqiang\2013.11.4\增加audio.core接口库 */
#endif
/*+\NEW\liweiqiang\2014.2.9\增加zlib库 */
#define AUXLIB_ZLIB "zlib"
LUALIB_API int ( luaopen_zlib)( lua_State *L );
/*-\NEW\liweiqiang\2014.2.9\增加zlib库 */

/*+\NEW\liweiqiang\2014.4.8\watchdog库 */
#define AUXLIB_WATCHDOG      "watchdog"
LUALIB_API int ( luaopen_watchdog )( lua_State *L );
/*-\NEW\liweiqiang\2014.4.8\watchdog库 */
#ifdef LUA_GPS_LIB
/*+\NEW\zhuth\2014.8.6\增加gpscore接口库*/
#define AUXLIB_GPSCORE      "gpscore"
LUALIB_API int ( luaopen_gpscore )( lua_State *L );
/*-\NEW\zhuth\2014.8.6\增加gpscore接口库*/
#endif

#define AUXLIB_TCPIPSOCK      "socketcore"
LUALIB_API int ( luaopen_tcpipsock )( lua_State *L );


/*+\NEW\xiongjunqun\2015.06.11\增加factory接口库*/
#define AUXLIB_FACTORY      "factory"
LUALIB_API int ( luaopen_factorycore )( lua_State * L );
/*-\NEW\xiongjunqun\2015.06.11\增加factory接口库*/

#ifdef LUA_QRENCODE_SUPPORT
#define AUXLIB_QRENCODE      "qrencode"
LUALIB_API int ( luaopen_qr_encode )( lua_State *L );
#endif

#if !defined(LUAT_TTSFLOAT_SUPPORT)
#define AUXLIB_WIFI      "wifi"
LUALIB_API int ( luaopen_wificore )( lua_State *L );
#endif

#ifdef LUA_ZIP_SUPPORT
#define AUXLIB_ZIP	"zip"
LUALIB_API int ( luaopen_zip )(lua_State *L);
#endif

#define AUXLIB_HRSENSOR     "hrsensor"
LUALIB_API int ( luaopen_hrsensorcore )( lua_State * L );

//TTS, panjun 160326.
#define AUXLIB_TTSPLYCORE     "ttsply"
LUALIB_API int ( luaopen_ttsplycore )( lua_State * L );

/*+\NEW\zhutianhua\2018.3.7 14:8\支持PBC库*/
#if defined(AM_PBC_SUPPORT)
#define AUXLIB_PBC     "protobuf.c"
LUALIB_API int ( luaopen_protobuf_c)( lua_State *L );
#endif
/*-\NEW\zhutianhua\2018.3.7 14:8\支持PBC库*/

// Helper macros
#define MOD_CHECK_ID( mod, id )\
  if( !platform_ ## mod ## _exists( id ) )\
    return luaL_error( L, #mod" %d does not exist", ( unsigned )id )

#define MOD_CHECK_RES_ID( mod, id, resmod, resid )\
  if( !platform_ ## mod ## _check_ ## resmod ## _id( id, resid ) )\
    return luaL_error( L, #resmod" %d not valid with " #mod " %d", ( unsigned )resid, ( unsigned )id )

#define MOD_REG_NUMBER( L, name, val )\
  lua_pushnumber( L, val );\
  lua_setfield( L, -2, name )

#define MOD_SORT_ID( id ) \
    if(id != PLATFORM_UART_ID_ATC && id != PLATFORM_PORT_ID_DEBUG && id != PLATFORM_PORT_ID_USB) { id = id - 1; }
  
#endif //__AUXMODS_H__

