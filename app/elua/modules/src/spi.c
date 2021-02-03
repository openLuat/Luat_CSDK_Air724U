
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_pmd.h"
#include "auxmods.h"
#include "lrotable.h"
#include "common.h"
#include "sermux.h"
#include "platform_conf.h"
#include "lrodefs.h"
#include "platform_spi.h"

static int spi_setup(lua_State *L)
{
    int chpa, cpol,dataBits,clock,duplex_mode, withCS, result;
    int spiId = luaL_checkinteger(L, 1);

    
    chpa = luaL_checkinteger(L, 2);
    cpol = luaL_checkinteger(L, 3);
    dataBits = luaL_checkinteger(L, 4);
    clock = luaL_checkinteger(L, 5);
    duplex_mode = luaL_optinteger(L, 6, 0);
    withCS  = luaL_optinteger(L, 7, 1);
    
    result = platform_spi_setup(spiId, clock, cpol, chpa, dataBits, duplex_mode, withCS);
    lua_pushinteger(L, result);

    return 1;
}

static int spi_send(lua_State *L)
{
    int spiId = luaL_checkinteger(L, 1);
    int luaType = lua_type( L, 2 );
    int len;
    u8* buf;
    
    if(luaType == LUA_TSTRING )
    {
        buf = lua_tolstring( L, 2, &len );
        
        len = platform_spi_send(spiId, buf, len);
    }
    else if(luaType == LUA_TLIGHTUSERDATA)
    {         
         buf = lua_touserdata(L, 2);
         len = lua_tointeger( L, 3);
         
         printf("spi_send userdata %x %d", buf, len);

         len = platform_spi_send(spiId, buf, len);
    }
    
    lua_pushinteger(L, len);

    return 1;
}


static int spi_recv(lua_State *L)
{
    int spiId = luaL_checkinteger(L, 1);
    int len = luaL_checkinteger( L, 2 );
    u8* recvData;
    int readlen=0;
    
    readlen = platform_spi_recv(spiId, &recvData, len);
    
    if(readlen >= 0)
    {
        luaL_Buffer b;
        
        luaL_buffinit( L, &b );
        luaL_addlstring(&b, recvData, readlen);
        luaL_pushresult( &b );
        
        free(recvData);
    }

    return 1;
}


static int spi_send_recv(lua_State *L)
{
    int spiId = luaL_checkinteger(L, 1);
    int luaType = lua_type( L, 2 );
    int len;
    int readlen=0;
    const char *sendBuf;
    u8* recvData;
    
    if(luaType == LUA_TSTRING )
    {
        sendBuf = lua_tolstring( L, 2, &len );
        
        //OPENAT_print("lua spi send/rev address:%p/%p",sendBuf,recvData);
        
        //OPENAT_print("lua spi send len:%d",len);
        //OPENAT_print("luaspi send:%x,%x,%x,%x\r\n",sendBuf[0],sendBuf[1],sendBuf[2],sendBuf[3]);
        readlen = platform_spi_send_recv(spiId, sendBuf, &recvData, len);
        //OPENAT_print("lua spi read len:%d",readlen);
        //OPENAT_print("lua spi receive:%x,%x,%x,%x\r\n",*(recvData),*(recvData+1),*(recvData+2),*(recvData+3));
    }
    else if(luaType == LUA_TLIGHTUSERDATA)
    {         
         sendBuf = lua_touserdata(L, 2);
         len = lua_tointeger( L, 3);
         
         printf("spi_send_recv userdata %d", len);

         readlen = platform_spi_send_recv(spiId, sendBuf, &recvData, len);
    }
    
    if(readlen >= 0)
    {
        luaL_Buffer b;
        
        luaL_buffinit( L, &b );
        luaL_addlstring(&b, recvData, readlen);
        luaL_pushresult( &b );
        
        free(recvData);
    }
    
    return 1;
}

static int spi_close(lua_State *L)
{
    int spiId = luaL_checkinteger(L, 1);    
    int result = platform_spi_close(spiId);
    lua_pushinteger(L, result);

    return 1;
}


// Module function map
const LUA_REG_TYPE spi_map[] =
{ 
  { LSTRKEY( "setup" ),  LFUNCVAL(spi_setup ) },
  { LSTRKEY( "send" ),  LFUNCVAL( spi_send ) },
  { LSTRKEY( "recv" ), LFUNCVAL( spi_recv ) },
  { LSTRKEY( "send_recv" ), LFUNCVAL( spi_send_recv ) },
  { LSTRKEY( "close" ), LFUNCVAL( spi_close ) },

  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_spi( lua_State *L )
{
  luaL_register( L, AUXLIB_SPI, spi_map );
  MOD_REG_NUMBER(L, "SPI_1", PLATFORM_SPI_1);
  MOD_REG_NUMBER(L, "SPI_2", PLATFORM_SPI_2);
  return 1;
}  

