/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    tcpipsock.c
 * Author:  liulean
 * Version: V0.1
 * Date:    2015/1/22
 *
 * Description:
 *          lua.tcpipsock ∑√Œ ø‚
 **************************************************************************/

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_socket.h"


static char* getFiledString(lua_State *L, int index, const char *key, char* defval)
{
    lua_getfield(L, index, key);
    return luaL_optstring(L, -1, defval);
}


static int l_pdp_activate(lua_State *L) {
    size_t  len      = 0;
    char* apn;
     char* username;
     char* password;
    
    apn      = (char*)luaL_checklstring(L, 1, &len);
    username = (char*)luaL_checklstring(L, 2, &len);
    password = (char*)luaL_checklstring(L, 3, &len);
    
    lua_pushinteger(L, platform_activate_pdp(apn, username, password));    
    return 1; 
}


static int l_pdp_deactivate(lua_State *L) {
    lua_pushinteger(L, platform_deactivate_pdp());   
    return 1; 
}


static int l_sock_conn(lua_State *L) {
    size_t len      = 0;
    int sock_type;
    int port;
    char* addr;
    int socket_id;
    
    sock_type  = luaL_checkinteger(L, 1);    
    addr       = (char*)luaL_checklstring(L, 2, &len);
    port       = luaL_checkinteger(L, 3);
    

    if(sock_type == SOC_SOCK_STREAM_SSL)
    {
      mthl_socket_cert cert;
      if(lua_type(L, 4) == LUA_TTABLE)
      {
        cert.hostName = getFiledString(L, 4, "hostName", NULL);
        cert.serverCacert = getFiledString(L, 4, "caCert", NULL);
        cert.clientCacert = getFiledString(L, 4, "clientCert", NULL);
        cert.clientKey = getFiledString(L, 4, "clientKey", NULL);

        socket_id = platform_socket_open( 
                                                    (openSocketType)sock_type,
                                                    port,
                                                    addr,
                                                    &cert);
      }
      else
      {
        socket_id = platform_socket_open( 
                                                  (openSocketType)sock_type,
                                                  port,
                                                  addr,
                                                  NULL);
      }
      
    }
    else
    {
      socket_id = platform_socket_open( 
                                                  (openSocketType)sock_type,
                                                  port,
                                                  addr,
                                                  NULL);
    }

    
                                                    
    if(socket_id < 0)
    {
      lua_pushnil(L);
    }
    else
    {
      lua_pushinteger(L, socket_id);   
    }
    return 1; 
}

static int l_sock_conn_ext(lua_State *L) {
    size_t len      = 0;
    int sock_type;
    int port;
    char* addr;
    int socket_id;
    
    sock_type  = luaL_checkinteger(L, 1);    
    addr       = (char*)luaL_checklstring(L, 2, &len);
    port       = luaL_checkinteger(L, 3);
    

    if(sock_type == SOC_SOCK_STREAM_SSL)
    {
      mthl_socket_cert cert;
      if(lua_type(L, 4) == LUA_TTABLE)
      {
        cert.hostName = getFiledString(L, 4, "hostName", NULL);
        cert.serverCacert = getFiledString(L, 4, "caCert", NULL);
        cert.clientCacert = getFiledString(L, 4, "clientCert", NULL);
        cert.clientKey = getFiledString(L, 4, "clientKey", NULL);

        socket_id = platform_socket_open( 
                                                    (openSocketType)sock_type,
                                                    port,
                                                    addr,
                                                    &cert);
      }
      else
      {
        socket_id = platform_socket_open( 
                                                  (openSocketType)sock_type,
                                                  port,
                                                  addr,
                                                  NULL);
      }
      
    }
    else
    {
      socket_id = platform_socket_open( 
                                                  (openSocketType)sock_type,
                                                  port,
                                                  addr,
                                                  NULL);
    }

    
                                                    
    lua_pushinteger(L, socket_id);   

    return 1; 
}

static int l_sock_send(lua_State *L) {
    size_t len      = 0;
    int sock_index;
    char* buf;

    sock_index = luaL_checkinteger(L, 1);
  
    luaL_checktype( L, 2, LUA_TSTRING );
    
    buf = (char*)lua_tolstring( L, 2, &len );
 
    lua_pushinteger(L, platform_socket_send(sock_index, buf, len));
    return 1;
}

static int l_sock_close(lua_State *L) {
    int sock_index;

    sock_index     = luaL_checkinteger(L, 1);

    lua_pushinteger(L, platform_socket_close(sock_index));
    return 1;
}


static int l_sock_recv(lua_State *L) {
    int sock_index;
    int total_len = 0; 
    luaL_Buffer b;
    int count ;
    int ret;
    char* buf;
    int recved_len = 0;
    
    sock_index     = luaL_checkinteger(L, 1);
    total_len      = luaL_checkinteger(L, 2);
    count          = total_len / LUAL_BUFFERSIZE;
    
    luaL_buffinit( L, &b );

    while(count-- > 0)
    {
      buf = luaL_prepbuffer(&b);
      ret = platform_socket_recv(sock_index, buf, LUAL_BUFFERSIZE);
      if(ret >= 0)
      {
        recved_len += ret;
        luaL_addsize(&b, ret);
      }
      else
      { 
        break;
      }
    }

    if(total_len % LUAL_BUFFERSIZE)
    {
        buf = luaL_prepbuffer(&b);
        ret = platform_socket_recv(sock_index, buf, total_len % LUAL_BUFFERSIZE);
        if(ret >= 0)
        {
          recved_len += ret;
          luaL_addsize(&b, ret);
        }
    }
    
    luaL_pushresult( &b );

    platform_socket_on_recv_done(sock_index, recved_len);
    
    return 1;
} 

static int l_sock_destroy(lua_State *L) {
    int sock_index;

    sock_index     = luaL_checkinteger(L, 1);

    lua_pushinteger(L, platform_socket_destroy(sock_index));
    return 1;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE tcpipsock_map[] =
{ 
  { LSTRKEY( "sock_conn" ),  LFUNCVAL( l_sock_conn ) },
  { LSTRKEY( "sock_conn_ext" ),  LFUNCVAL( l_sock_conn_ext ) },
  { LSTRKEY( "sock_send" ),  LFUNCVAL( l_sock_send ) },
  { LSTRKEY( "sock_close" ),  LFUNCVAL( l_sock_close ) },
  { LSTRKEY( "sock_recv" ),  LFUNCVAL( l_sock_recv ) },
  { LSTRKEY( "sock_destroy" ),  LFUNCVAL( l_sock_destroy ) },

 
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_tcpipsock( lua_State *L )
{
    luaL_register( L, AUXLIB_TCPIPSOCK, tcpipsock_map );
    platform_lua_socket_init();
    return 1;
}  
