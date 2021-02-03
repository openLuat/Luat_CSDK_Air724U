/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    tcpipsock.c
 * Author:  liulean
 * Version: V0.1
 * Date:    2015/1/22
 *
 * Description:
 *          lua.tcpipsock 访问库
 **************************************************************************/

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_socket.h"
#include "platform_malloc.h"

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
	/*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
	kal_bool res = 0;
	/*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
    sock_index = luaL_checkinteger(L, 1);
  
    luaL_checktype( L, 2, LUA_TSTRING );
    
    buf = (char*)lua_tolstring( L, 2, &len );

	/*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
 	res = platform_socket_send(sock_index, buf, len);
    lua_pushinteger(L, res);
	lua_pushinteger(L, res ? OPENAT_SOCKET_SUCCESS:platform_socket_error(sock_index));
    return 2;
	/*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
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
	/*+\wj\2020.1.22\BUG4307 UDP数据收不全*/
	int recv_left = 0;
    sock_index     = luaL_checkinteger(L, 1);
    total_len      = luaL_checkinteger(L, 2);
    
    
    luaL_buffinit( L, &b );
	char *readBuf = platform_malloc(total_len);
	if(readBuf == NULL){
		luaL_pushresult( &b );
		return 1;
	}
		
	ret = platform_socket_recv(sock_index, readBuf, total_len);

	if(ret > 0)
	{
		count 	= ret / LUAL_BUFFERSIZE;
		while(count-- > 0)
    	{
    		
      		buf = luaL_prepbuffer(&b);
      		//ret = platform_socket_recv(sock_index, buf, READ_BUF_LEN);
      		memcpy(buf,readBuf+recv_left,LUAL_BUFFERSIZE);
			luaL_addsize(&b, LUAL_BUFFERSIZE);
			recv_left += LUAL_BUFFERSIZE;
		}
		if(ret % LUAL_BUFFERSIZE)
    	{	
			buf = luaL_prepbuffer(&b);
			memcpy(buf,readBuf+recv_left,LUAL_BUFFERSIZE);
			luaL_addsize(&b, ret % LUAL_BUFFERSIZE);
		}
    }
	
	platform_free(readBuf);
	#if 0
    while(count-- > 0)
    {
      buf = luaL_prepbuffer(&b);
      ret = platform_socket_recv(sock_index, buf, READ_BUF_LEN);
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

    if(total_len % READ_BUF_LEN)
    {
        buf = luaL_prepbuffer(&b);
        ret = platform_socket_recv(sock_index, buf, total_len % READ_BUF_LEN);
        if(ret >= 0)
        {
          recved_len += ret;
          luaL_addsize(&b, ret);
        }
    }
    #endif	
    luaL_pushresult( &b );


    /*-\wj\2020.1.22\BUG4307 UDP数据收不全*/
    return 1;
} 

static int l_sock_destroy(lua_State *L) {
    int sock_index;

    sock_index     = luaL_checkinteger(L, 1);

    lua_pushinteger(L, platform_socket_destroy(sock_index));
    return 1;
}

/*+\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/
static int l_sock_setopt(lua_State *L) {
    int sock_index;
	int level;
	int optname;
	int optval;
	int setret = 0;

    sock_index = luaL_checkinteger(L, 1);
	level = luaL_checkinteger(L, 2);
	optname = luaL_checkinteger(L, 3);
	if(lua_type(L, 4) == LUA_TNUMBER)
	{
		optval = luaL_checkinteger(L, 4);
	}else
	{
		/**目前对结构体类型的参数设置没做处理**/
		setret = -1;
	}

	/* level目前只支持SOL_SOCKET跟 IPPROTO_TCP   name 支持SO_KEEPALIVE，TCP_KEEPIDLE，TCP_KEEPINTVL，TCP_KEEPCNT，SO_REUSEADDR */
	if(sock_index < 0 
		|| (level != OPENAT_SOCKET_SOL_SOCKET && level != OPENAT_SOCKET_IPPROTO_TCP)
		|| (optname != OPENAT_SOCKET_SO_KEEPALIVE && optname != OPENAT_SOCKET_TCP_KEEPIDLE && optname != OPENAT_SOCKET_TCP_KEEPINTVL 
		    && optname != OPENAT_SOCKET_TCP_KEEPCNT && optname != OPENAT_SOCKET_SO_REUSEADDR))
	{
		setret = -1;
	}

	if(setret == 0)
	{
		setret = platform_socket_setopt(sock_index,level,optname,&optval,sizeof(optval));
	}
	
    lua_pushinteger(L, setret);
    return 1;
}
/*-\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/

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
  /*+\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/
  { LSTRKEY( "sock_setopt" ),  LFUNCVAL( l_sock_setopt ) },
  /*-\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/

 
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_tcpipsock( lua_State *L )
{
    luaL_register( L, AUXLIB_TCPIPSOCK, tcpipsock_map );
	/*+\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/
	MOD_REG_NUMBER(L, "SOL_SOCKET", OPENAT_SOCKET_SOL_SOCKET);
	MOD_REG_NUMBER(L, "SO_KEEPALIVE", OPENAT_SOCKET_SO_KEEPALIVE);
	MOD_REG_NUMBER(L, "SO_REUSEADDR", OPENAT_SOCKET_SO_REUSEADDR);
	MOD_REG_NUMBER(L, "IPPROTO_TCP", OPENAT_SOCKET_IPPROTO_TCP);
	MOD_REG_NUMBER(L, "TCP_KEEPIDLE", OPENAT_SOCKET_TCP_KEEPIDLE);
	MOD_REG_NUMBER(L, "TCP_KEEPINTVL", OPENAT_SOCKET_TCP_KEEPINTVL);
	MOD_REG_NUMBER(L, "TCP_KEEPCNT", OPENAT_SOCKET_TCP_KEEPCNT);
	/*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
	MOD_REG_NUMBER(L, "ERR_UNKOWN", -1);
	MOD_REG_NUMBER(L, "ERR_OK", OPENAT_SOCKET_SUCCESS);
	MOD_REG_NUMBER(L, "ERR_ENOMEM", OPENAT_SOCKET_ENOMEM);
	MOD_REG_NUMBER(L, "ERR_ENOBUFS", OPENAT_SOCKET_ENOBUFS);
	MOD_REG_NUMBER(L, "ERR_ETIMEOUT", OPENAT_SOCKET_ETIMEOUT);
	MOD_REG_NUMBER(L, "ERR_ERTE", OPENAT_SOCKET_ERTE);
	MOD_REG_NUMBER(L, "ERR_EINPROGRESS", OPENAT_SOCKET_EINPROGRESS);
	MOD_REG_NUMBER(L, "ERR_EWOULDBLOCK", OPENAT_SOCKET_EWOULDBLOCK);
	MOD_REG_NUMBER(L, "ERR_EADDRINUSE", OPENAT_SOCKET_EADDRINUSE);
	MOD_REG_NUMBER(L, "ERR_EALREADY", OPENAT_SOCKET_EALREADY);
	MOD_REG_NUMBER(L, "ERR_EISCONN", OPENAT_SOCKET_EISCONN);
	MOD_REG_NUMBER(L, "ERR_ENOTCONN", OPENAT_SOCKET_ENOTCONN);
	MOD_REG_NUMBER(L, "ERR_EIF", OPENAT_SOCKET_EIF);
	MOD_REG_NUMBER(L, "ERR_EABORT", OPENAT_SOCKET_EABORT);
	MOD_REG_NUMBER(L, "ERR_ERST", OPENAT_SOCKET_ERST);
	MOD_REG_NUMBER(L, "ERR_ECLSD", OPENAT_SOCKET_ECLSD);
	MOD_REG_NUMBER(L, "ERR_EARG", OPENAT_SOCKET_EARG);
	MOD_REG_NUMBER(L, "ERR_EIO", OPENAT_SOCKET_EIO);
	MOD_REG_NUMBER(L, "ERR_SSL_EHANDESHAK", OPENAT_SOCKET_SSL_EHANDESHAK);
	MOD_REG_NUMBER(L, "ERR_SSL_EINIT", OPENAT_SOCKET_SSL_EINIT);
	MOD_REG_NUMBER(L, "ERR_SSL_EINVALID", OPENAT_SOCKET_SSL_EINVALID);
	/*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
	/*-\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/
    platform_lua_socket_init();
    return 1;
}  
