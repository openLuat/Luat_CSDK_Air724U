/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    gps.c
 * Author:  zhutianhua
 * Version: V0.1
 * Date:    2014/8/6
 *
 * Description:
 *          lua.gpscore gpscore∑√Œ ø‚
 **************************************************************************/
#ifdef LUA_GPS_LIB

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_gps.h"

// gpscore.open()
static int l_gps_open(lua_State *L) {
    int op = luaL_checkinteger( L, 1 );
    lua_pushinteger(L, platform_gps_open(op));
    return 1; 
}

// gpscore.close()
static int l_gps_close(lua_State *L) {
    lua_pushinteger(L, platform_gps_close());
    return 1; 
}
static int l_gps_read(lua_State* L)
{
  int total_len = 0; 
  luaL_Buffer b;
  
  total_len      = luaL_checkinteger(L, 1);
  
  luaL_buffinit( L, &b );
  
  {
      b.p += platform_gps_read((UINT8*)b.p);
  }
  luaL_pushresult( &b );    
  return 1;
}

static int l_gps_write(lua_State* L)
{
  size_t len      = 0;
  char* buf;

  luaL_checktype( L, 1, LUA_TSTRING );
  
  buf = (char*)lua_tolstring( L, 1, &len);

  lua_pushinteger(L, platform_gps_write(buf, len));
  return 1;  
}

static int l_gps_cmd(lua_State* L)
{
  int op = luaL_checkinteger( L, 1 );
  lua_pushinteger(L, platform_gps_cmd(op));
  return 1;  
}



#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE gpscore_map[] =
{ 
  { LSTRKEY( "open" ),  LFUNCVAL( l_gps_open ) },
  { LSTRKEY( "close" ),  LFUNCVAL( l_gps_close ) },
  { LSTRKEY( "read" ),  LFUNCVAL( l_gps_read ) },
  { LSTRKEY( "write" ), LFUNCVAL( l_gps_write ) },
  { LSTRKEY( "cmd" ), LFUNCVAL( l_gps_cmd ) },
#if 0
  { LSTRKEY( "WORK_RAW_MODE" ), LNUMVAL( GPS_MODE_RAW_DATA ) },
  { LSTRKEY( "WORK_LOCATION_MODE" ), LNUMVAL( GPS_MODE_LOCATION ) },
  { LSTRKEY( "WORK_LOCATION_QOP_MODE" ), LNUMVAL( GPS_MODE_LOCATION_WITH_QOP ) },
#endif 
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_gpscore( lua_State *L )
{
    luaL_register( L, AUXLIB_GPSCORE, gpscore_map );

    /*open(mode)*/
    MOD_REG_NUMBER( L, "WORK_RAW_MODE", GPS_MODE_RAW_DATA );
    MOD_REG_NUMBER( L, "WORK_LOCATION_MODE", GPS_MODE_LOCATION);
    MOD_REG_NUMBER( L, "WORK_LOCATION_QOP_MODE", GPS_MODE_LOCATION_WITH_QOP);
    
    
    /*ind msg: MSG_GPS_DATA_IND */
    MOD_REG_NUMBER(L, "MSG_GPS_DATA_IND", MSG_ID_GPS_DATA_IND);
    MOD_REG_NUMBER(L, "MSG_GPS_OPEN_IND", MSG_ID_GPS_OPEN_IND);
    /*MSG_GPS_DATA_IND.type*/
    MOD_REG_NUMBER( L, "RAW_DATA", PLATFORM_GPS_PARSER_RAW_DATA);
    MOD_REG_NUMBER( L, "NMEA_GGA", PLATFORM_GPS_PARSER_NMEA_GGA);
    MOD_REG_NUMBER( L, "NMEA_GLL", PLATFORM_GPS_PARSER_NMEA_GLL);
    MOD_REG_NUMBER( L, "NMEA_GSA", PLATFORM_GPS_PARSER_NMEA_GSA);
    MOD_REG_NUMBER( L, "NMEA_GSV", PLATFORM_GPS_PARSER_NMEA_GSV);
    MOD_REG_NUMBER( L, "NMEA_RMC", PLATFORM_GPS_PARSER_NMEA_RMC);
    MOD_REG_NUMBER( L, "NMEA_VTG", PLATFORM_GPS_PARSER_NMEA_VTG);
    MOD_REG_NUMBER( L, "NMEA_GAGSA", PLATFORM_GPS_PARSER_NMEA_GAGSA);
    MOD_REG_NUMBER( L, "NMEA_GAGSV", PLATFORM_GPS_PARSER_NMEA_GAGSV);
    MOD_REG_NUMBER( L, "NMEA_GLGSA", PLATFORM_GPS_PARSER_NMEA_GLGSA);
    MOD_REG_NUMBER( L, "NMEA_GLGSV", PLATFORM_GPS_PARSER_NMEA_GLGSV);
    MOD_REG_NUMBER( L, "NMEA_SENTENCE", PLATFORM_GPS_PARSER_NMEA_SENTENCE);
    MOD_REG_NUMBER( L, "UART_EVENT_VPORT_LOST", PLATFORM_GPS_UART_EVENT_VPORT_LOST);
    MOD_REG_NUMBER( L, "SHOW_AGPS_ICON", PLATFORM_GPS_SHOW_AGPS_ICON);
    MOD_REG_NUMBER( L, "HIDE_AGPS_ICON", PLATFORM_GPS_HIDE_AGPS_ICON);
    MOD_REG_NUMBER( L, "NMEA_ACC", PLATFORM_GPS_PARSER_NMEA_ACC);
    MOD_REG_NUMBER( L, "NMEA_END", PLATFORM_GPS_PARSER_NMEA_END);
    MOD_REG_NUMBER( L, "MA_STATUS", PLATFORM_GPS_PARSER_MA_STATUS);
    MOD_REG_NUMBER( L, "OPEN_IND", PLATFORM_GPS_OPEN_IND);
    MOD_REG_NUMBER( L, "UART_RAW_DATA", PLATFORM_GPS_UART_RAW_DATA);

    /*cmd(cmd)*/

    MOD_REG_NUMBER(L, "CMD_WARM_START", PLATFORM_GPS_UART_GPS_WARM_START);
    MOD_REG_NUMBER(L, "CMD_HOT_START", PLATFORM_GPS_UART_GPS_HOT_START);
    MOD_REG_NUMBER(L, "CMD_COLD_START", PLATFORM_GPS_UART_GPS_COLD_START);
    MOD_REG_NUMBER(L, "CMD_VERSION", PLATFORM_GPS_UART_GPS_VERSION);
    MOD_REG_NUMBER(L, "CMD_ENABLE_DEBUG_INFO", PLATFORM_GPS_UART_GPS_ENABLE_DEBUG_INFO);
    MOD_REG_NUMBER(L, "CMD_SWITCH_MODE_MA", PLATFORM_GPS_UART_GPS_SWITCH_MODE_MA);
    MOD_REG_NUMBER(L, "CMD_SWITCH_MODE_MB", PLATFORM_GPS_UART_GPS_SWITCH_MODE_MB);
	MOD_REG_NUMBER(L, "CMD_SWITCH_MODE_NORMAL", PLATFORM_GPS_UART_GPS_SWITCH_MODE_NORMAL);
    MOD_REG_NUMBER(L, "CMD_QUERY_POS", PLATFORM_GPS_UART_GPS_QUERY_POS);
  	MOD_REG_NUMBER(L, "CMD_QUERY_MEAS", PLATFORM_GPS_UART_GPS_QUERY_MEAS);
  	MOD_REG_NUMBER(L, "CMD_CLEAR_NVRAM", PLATFORM_GPS_UART_GPS_CLEAR_NVRAM);
  	MOD_REG_NUMBER(L, "CMD_AGPS_START", PLATFORM_GPS_UART_GPS_AGPS_START);
  	MOD_REG_NUMBER(L, "CMD_SLEEP", PLATFORM_GPS_UART_GPS_SLEEP);
  	MOD_REG_NUMBER(L, "CMD_STOP", PLATFORM_GPS_UART_GPS_STOP);
  	MOD_REG_NUMBER(L, "CMD_WAKE_UP", PLATFORM_GPS_UART_GPS_WAKE_UP);

    platform_gps_init();
    return 1;
}  
#endif
