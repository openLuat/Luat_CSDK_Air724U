// Module for interfacing with the I2C interface

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "platform_i2c.h"
#include "auxmods.h"
#include "lrotable.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef LUA_GPIO_I2C
/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/
static int i2c_gSensorParam_get(lua_State *L)
{
  unsigned int id = luaL_checkinteger( L, 1 );
  unsigned int slave_addr = 0;
  unsigned int slave_id = 0;

  platform_i2c_gSensorParam_get(id, (u8 *)&slave_addr, (u8 *)&slave_id);

  OPENAT_print("i2c_gSensorParam_get slave_addr %x slave_id %x", slave_addr, slave_id);
  
  lua_pushinteger( L, slave_addr);
  lua_pushinteger( L, slave_id);
  return 2;
}
/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/

// Lua: speed = i2c.setup( id, speed, slaveaddr )
static int i2c_setup( lua_State *L )
{
    unsigned id = luaL_checkinteger( L, 1 );
    T_AMOPENAT_GPIO_I2C_PARAM i2cParam;

    i2cParam.slaveAddr= ( u16 )luaL_checkinteger( L, 2 );

    i2cParam.sda_port = ( u32 )luaL_checkinteger( L, 3 );
    i2cParam.scl_port = (u32) luaL_checkinteger(L, 4);

    MOD_CHECK_ID( i2c, id );
    lua_pushinteger( L, platform_gpio_i2c_setup( id, &i2cParam ) );
    return 1;
}

// Lua: wrote = i2c.write( id, [slave,] reg, data )
// data can be either a string, a table or an 8-bit number
static int i2c_write( lua_State *L )
{
    u8 arg_index = 1;
    unsigned id = luaL_checkinteger( L, arg_index++ );
    u16 slave_addr = I2C_NULL_SLAVE_ADDR;

    u8 regAddr;
    u32 wrote = 0;

    if(lua_gettop(L) == 4){
        slave_addr = luaL_checkinteger(L, arg_index++);
    }
    regAddr = (u8)luaL_checkinteger(L, arg_index++);

    MOD_CHECK_ID( i2c, id );

    switch(lua_type(L, arg_index))
    {
    case LUA_TNUMBER:
        {
            u8 numdata = (u8) luaL_checkinteger(L, arg_index);

            wrote = platform_gpio_i2c_send_data(id, slave_addr, &regAddr, &numdata, 1);
        }
        break;

    case LUA_TSTRING:
        {
            const char *pdata;
            u32 slen;

            pdata = luaL_checklstring(L, arg_index, &slen);
            wrote = platform_gpio_i2c_send_data(id, slave_addr, &regAddr, pdata, slen);
        }
        break;

    case LUA_TTABLE:
        {
            size_t datalen = lua_objlen(L, arg_index);
            size_t i;
            u8 *pBuff;

            pBuff = L_MALLOC(datalen);

            for(i = 0; i < datalen; i++)
            {
                lua_rawgeti(L, arg_index, i+1);
                pBuff[i] = (u8)luaL_checkinteger(L, -1);
            }

            wrote = platform_gpio_i2c_send_data(id, slave_addr, &regAddr, pBuff, datalen);

            L_FREE(pBuff);
        }
        break;

    default:
        return luaL_error(L, "i2c.write: data must be number,string,table");
        break;
    }

    lua_pushinteger( L, wrote );
    return 1;
}

// Lua: read = i2c.read( id, [slave,] reg, size )
static int i2c_read( lua_State *L )
{
    u8 arg_index = 1;
    unsigned id = luaL_checkinteger( L, arg_index++ );
    u16 slave_addr = I2C_NULL_SLAVE_ADDR;

    u8 regAddr;
    u32 size;
    luaL_Buffer b;

    if(lua_gettop(L) == 4){
        slave_addr = luaL_checkinteger(L, arg_index++);
    }
    regAddr = (u8)luaL_checkinteger(L, arg_index++);
    size = (u32)luaL_checkinteger( L, arg_index++ );

    MOD_CHECK_ID( i2c, id );
    if( size == 0 )
        return 0;
    
    if(size >= LUAL_BUFFERSIZE)
        return luaL_error(L, "i2c.read: size must < %d", LUAL_BUFFERSIZE);

    luaL_buffinit( L, &b );

    platform_gpio_i2c_recv_data(id, slave_addr, &regAddr, b.p, size);
    b.p += size;

    luaL_pushresult( &b );

    return 1;
}

// Module function map
#define MIN_OPT_LEVEL   2
#include "lrodefs.h"
const LUA_REG_TYPE gpio_i2c_map[] = 
{
  { LSTRKEY( "setup" ),  LFUNCVAL( i2c_setup ) },
  { LSTRKEY( "write" ), LFUNCVAL( i2c_write ) },
  { LSTRKEY( "read" ), LFUNCVAL( i2c_read ) },

/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/
  { LSTRKEY( "gsensorParam_get" ),  LFUNCVAL( i2c_gSensorParam_get ) },
/*-\NEW\zhuwangbin\2016.4.6\兼容不同版本的g_sensor, 添加获取设备id和设备地址接口*/
  
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_gpio_i2c( lua_State *L )
{
#if LUA_OPTIMIZE_MEMORY > 0
  return 0;
#else // #if LUA_OPTIMIZE_MEMORY > 0
  luaL_register( L, AUXLIB_GPIO_I2C, gpio_i2c_map );
#endif
  
  return 1;
}
#endif

