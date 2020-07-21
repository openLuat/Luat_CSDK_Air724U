/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_audio.c
 * Author:  liulean
 * Version: V0.1
 * Date:    2015/1/19
 *
 * Description:
 *          TCPIP SOCKET接口
 **************************************************************************/
#include "cs_types.h"
#include "cycle_queue.h"
#include "lua.h"
#include "lualib.h"
#include "elua_int.h"
#include "am_openat.h"
#include "platform_rtos.h"
#include "platform_socket.h"
#include "am_openat_tls.h"
#include "am_openat_socket.h"



#define LUA_MAX_SOCKET_SUPPORT 10

#define LUA_INVALID_SOKCET_ID (-1)

#define LUA_INVALID_SOCKET_INDEX (0xffffffff)

#define LUA_INVALID_PARAM  (-1)

#define SSL_SOCKET_RX_BUF_SIZE (2048)


//#define LUA_SOCKET_TEST

typedef struct 
{
    BOOL    connected;
    openSocketType sock_type;
    kal_uint16    port;
    struct openSocketAddrSin sock_addr; 
    kal_char    addr[MAX_SOCK_ADDR_LEN + 1];  
    int        sock_id; 
    /*+\Bug 183\zhutianhua\2018.12.18 15:56\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
    BOOL remoteCloseDelayed;
    HANDLE remoteCloseDelayTimerId;
    /*-\Bug 183\zhutianhua\2018.12.18 15:56\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
    /*+\Bug 271\zhutianhua\2019.1.24 17:46\tcp ssl发送一次数据，会收到两次send cnf*/
    kal_uint32 totalSendLen;
    kal_uint32 sentLen;
    /*-\Bug 271\zhutianhua\2019.1.24 17:46\tcp ssl发送一次数据，会收到两次send cnf*/
	u8   ssl_ctx_id; 
	mthl_socket_cert soc_cert;
	CycleQueue sslRxQ;
}lua_socket_info_struct;

typedef struct
{
    kal_char  apn[ MAX_APN_LEN + 1];
    kal_char  user_name[CUSTOM_DTCNT_PROF_MAX_USER_LEN+1];     /* User string of a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_USER_LEN) */
    kal_char  password[CUSTOM_DTCNT_PROF_MAX_PW_LEN+1];         /* Password for a Data Account (ASCII) (MAX: CUSTOM_DTCNT_PROF_MAX_PW_LEN) */
    lua_socket_info_struct socket_info[LUA_MAX_SOCKET_SUPPORT];
}lua_socket_context_struct;
//socket发送一部分数据后剩余等待发送的数据
/*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
typedef struct 
{
   UINT32 readySendLen;
   UINT32 alreadySendLen;
   CycleQueue sbufQueue;
}socketRemainBuf;
/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/

/*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
#ifndef NUM_SOCKETS
#define NUM_SOCKETS 10
#endif
static socketRemainBuf g_s_remain_socket_buf[NUM_SOCKETS] = {0};
static u8 platform_process_read_event(kal_int8 SocketID);
//static mthl_socket_cert g_s_cert;


socketRemainBuf *openatGetRemainBuf(UINT8 soc_id)
{
    return &g_s_remain_socket_buf[soc_id];
}
void openatResetRemainBuf(UINT8 soc_id)
{
    socketRemainBuf *sbuf = openatGetRemainBuf(soc_id);
    sbuf->alreadySendLen = 0;
    sbuf->readySendLen = 0;
    QueueClean(&sbuf->sbufQueue);
}
void openatInitRemainBuf(UINT8 soc_id)
{
    socketRemainBuf *remainBuf = openatGetRemainBuf(soc_id);
    QueueClean(&remainBuf->sbufQueue);
    remainBuf->sbufQueue.buf = OPENAT_malloc(5600);
    remainBuf->sbufQueue.size = 5600;
}
void openatFreeRemainBuf(UINT8 soc_id)
{
    socketRemainBuf *remainBuf = openatGetRemainBuf(soc_id);
    OPENAT_free(remainBuf->sbufQueue.buf);
    remainBuf->sbufQueue.buf = NULL;
}
/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/

extern void setfieldInt(lua_State *L, const char *key, int value);

lua_socket_context_struct lua_socket_context;


static UINT8 platformGetFreeSocket(void)
{
  lua_socket_info_struct* sockPtr;
  UINT8 i;

  for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
  {
    if(lua_socket_context.socket_info[i].sock_id == LUA_INVALID_SOKCET_ID)
    {
      break;
    }
  }
  return i;
}
static void platform_pdp_active_cb(BOOL result)
{
  PlatformMsgData msg;
  msg.pdpActiveCnf.result = result;
  msg.pdpActiveCnf.errorCause = 0;
  platform_rtos_send(MSG_ID_APP_MTHL_ACTIVATE_PDP_CNF, &msg);
}





static UINT8 platform_socket_index(lua_socket_info_struct* sock)
{ 
  UINT8 i;
  for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
  {
    if(&lua_socket_context.socket_info[i] == sock)
    {
      break;
    }
  }
  return i;
}

static lua_socket_info_struct* platform_socket_ctx(int s)
{ 
  UINT8 i;
  for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
  {
    if(lua_socket_context.socket_info[i].sock_id == s)
    {
      return &lua_socket_context.socket_info[i];
    }
  }
  return NULL;
}

static void platform_socket_connectCnf(lua_socket_info_struct* sock, BOOL success)
{
  PlatformMsgData msg;
  UINT8 socketId = platform_socket_index(sock);
  socketRemainBuf *sbuf = openatGetRemainBuf(socketId);
  if(sock->sock_type == SOC_SOCK_STREAM)
  {
    /*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
    if(success)
    {
        if(sbuf->sbufQueue.buf != NULL)
        {
            openatFreeRemainBuf(socketId);
        }
        openatResetRemainBuf(socketId);
        openatInitRemainBuf(socketId); 
        OPENAT_print("%s , %d ",__FUNCTION__,socketId);
    }
    /*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
  }
  msg.socketConnectCnf.socket_index = socketId;
  msg.socketConnectCnf.result = success;
  platform_rtos_send(MSG_ID_APP_MTHL_CREATE_CONN_CNF, &msg);
}

static void platform_socket_recvInd(lua_socket_info_struct* sock, INT32 len)
{
  PlatformMsgData msg;
  msg.socketRecvInd.socket_index = platform_socket_index(sock);
  msg.socketRecvInd.recv_len = len;
  platform_rtos_send(MSG_ID_APP_MTHL_SOCK_RECV_IND, &msg);
}

static void platform_socket_closeCnf(lua_socket_info_struct* sock)
{
  PlatformMsgData msg;
  msg.socketCloseCnf.socket_index = platform_socket_index(sock);
  msg.socketCloseCnf.result = TRUE;
  socketRemainBuf *sbuf = openatGetRemainBuf(platform_socket_index(sock));
  if(sock->sock_type == SOC_SOCK_STREAM)
  {
    /*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
    if (sbuf->sbufQueue.buf != NULL)
    {
        openatFreeRemainBuf(msg.socketCloseCnf.socket_index);
    }
    /*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
  }
  OPENAT_print(" %s , %d ",__FUNCTION__, msg.socketConnectCnf.socket_index);
  platform_rtos_send(MSG_ID_APP_MTHL_SOCK_CLOSE_CNF, &msg);
}

static void platform_socket_closeInd(lua_socket_info_struct* sock)
{
  PlatformMsgData msg;
  msg.socketCloseInd.socket_index = platform_socket_index(sock);
  msg.socketCloseInd.result = TRUE;
  platform_rtos_send(MSG_ID_APP_MTHL_SOCK_CLOSE_IND, &msg);
}

static void platform_socket_sendCnf(lua_socket_info_struct* sock, BOOL result, UINT32 len)
{
  PlatformMsgData msg;
  UINT8 socketId = platform_socket_index(sock);
  socketRemainBuf *sbuf = openatGetRemainBuf(socketId);
  int ret;
  char *readBuf = NULL;
  INT32 readLen;
  /*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
  OPENAT_print("%s,%d,%d,%d,%d,%d,%d",__FUNCTION__,result,len,sock->sentLen,sock->totalSendLen,sbuf->alreadySendLen,
                    sbuf->readySendLen);
  if(sock->sock_type == SOC_SOCK_STREAM)
  {
      if(!result)
      {
        msg.socketSendCnf.socket_index  = platform_socket_index(sock);
        msg.socketSendCnf.length = sbuf->alreadySendLen;
        msg.socketSendCnf.result = result;
        platform_rtos_send(MSG_ID_APP_MTHL_SOCK_SEND_CNF, &msg);
      }
      else
      {
        if(sbuf->sbufQueue.empty != 1)
        {
           if(QueueLen(&sbuf->sbufQueue) > len)
           {   
                readBuf = OPENAT_malloc(len);
                memset(readBuf,0,len);
                QueueDelete(&sbuf->sbufQueue,readBuf,len);
                ret = OPENAT_socket_send(socketId,readBuf,len,0);
           }
           else
           {
                readBuf = OPENAT_malloc(QueueLen(&sbuf->sbufQueue));
                memset(readBuf,0,QueueLen(&sbuf->sbufQueue));
                readLen = QueueLen(&sbuf->sbufQueue);
                QueueDelete(&sbuf->sbufQueue,readBuf,readLen);
                ret = OPENAT_socket_send(socketId,readBuf,readLen,0);
           }
           if(ret<0)
           {
           		/*+\bug\wj\2020.3.11\修改一处内存泄漏情况*/
           		OPENAT_free(readBuf);
				/*-\bug\wj\2020.3.11\修改一处内存泄漏情况*/
                platform_socket_sendCnf(sock, FALSE, 0);
           }
           else
           {
                OPENAT_free(readBuf);
           }
        }   
      }
  }
  /*+\Bug 271\zhutianhua\2019.1.25 14:8\tcp ssl发送一次数据，会收到两次send cnf*/
  else if(sock->sock_type == SOC_SOCK_STREAM_SSL)
  {
      if(result)
      {
	  	/*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
          //sock->sentLen += len;
		  /*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
          if(sock->sentLen >= sock->totalSendLen)
          {
              msg.socketSendCnf.socket_index  = platform_socket_index(sock);
              msg.socketSendCnf.length = sbuf->alreadySendLen;
              msg.socketSendCnf.result = result;
              platform_rtos_send(MSG_ID_APP_MTHL_SOCK_SEND_CNF, &msg);
          }
      }
      else
      {
          msg.socketSendCnf.socket_index  = platform_socket_index(sock);
          msg.socketSendCnf.length = sbuf->alreadySendLen;
          msg.socketSendCnf.result = result;
          platform_rtos_send(MSG_ID_APP_MTHL_SOCK_SEND_CNF, &msg);
      }
  }
  /*-\Bug 271\zhutianhua\2019.1.25 14:8\tcp ssl发送一次数据，会收到两次send cnf*/
  else
  {
    msg.socketSendCnf.socket_index  = platform_socket_index(sock);
    msg.socketSendCnf.length = sbuf->alreadySendLen;
    msg.socketSendCnf.result = result;
    platform_rtos_send(MSG_ID_APP_MTHL_SOCK_SEND_CNF, &msg);
  }
  /*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
}
/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
static platform_socket_sendAckCnf(lua_socket_info_struct* sock, UINT32 len)
{ 
    UINT8 socketId = platform_socket_index(sock);
    if(sock->sock_type == SOC_SOCK_STREAM)
    {
        PlatformMsgData msg;
        socketRemainBuf *sbuf = openatGetRemainBuf(socketId);
        sbuf->alreadySendLen = sbuf->alreadySendLen + len;
        OPENAT_print("%s readySendLen = %d;alreadySendLen = %d",__FUNCTION__,
            sbuf->readySendLen,sbuf->alreadySendLen); 
        if(sbuf->alreadySendLen == sbuf->readySendLen)
        {
            msg.socketSendCnf.socket_index  = platform_socket_index(sock);
            msg.socketSendCnf.result = TRUE;
            msg.socketSendCnf.length = sbuf->readySendLen;
            openatResetRemainBuf(socketId);
            platform_rtos_send(MSG_ID_APP_MTHL_SOCK_SEND_CNF, &msg);
        }
    }
}
/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/

/*+\Bug 183\zhutianhua\2018.12.19 11:11\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
static void platform_socket_stop_remote_close_delay_timer(lua_socket_info_struct* sock)
{
    OPENAT_print("%s sock=%x, remoteCloseDelayed=%d", __FUNCTION__, sock, sock ? sock->remoteCloseDelayed : 0);
    if(sock && sock->remoteCloseDelayed)
    {
        sock->remoteCloseDelayed = FALSE;
        OPENAT_stop_timer(sock->remoteCloseDelayTimerId);
        OPENAT_delete_timer(sock->remoteCloseDelayTimerId);
    }
}

void platform_socket_remote_close_delay_timer_cb(UINT32 param)
{
    lua_socket_info_struct* sock = platform_socket_ctx(param);

    OPENAT_print("%s sock=%x, remoteCloseDelayed=%d", __FUNCTION__, sock, sock ? sock->remoteCloseDelayed : 0);

    if(NULL!=sock && sock->remoteCloseDelayed)
    {        
        platform_socket_stop_remote_close_delay_timer(sock);
        platform_socket_closeInd(sock);
    }   
}
/*-\Bug 183\zhutianhua\2018.12.19 11:11\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/

static void platform_socket_cb(int s, openSocketEvent evt, int err, char* data, int len)
{
  	OPENAT_print("%s got socket %d evt %d err %d dataLen = %d", __FUNCTION__, s, evt, err, len);
  	lua_socket_info_struct* sock = platform_socket_ctx(s);
	HANDLE timerref;
  
  	ASSERT(sock != NULL);
  	switch(evt)
  	{
	  	case SOC_EVT_CONNECT:
	  	{
	  		if(err != 0)//connect fail
			{
				OPENAT_socket_close(s);
			}else// connect success
			{
				if(sock->sock_type == SOC_SOCK_STREAM_SSL)
		  		{
		  			if(platform_ssl_create(sock) !=0)
					{
						sock->connected = FALSE;
						OPENAT_socket_close(sock->sock_id);
						platform_ssl_close(sock->sock_id,sock->ssl_ctx_id);
			  			platform_socket_connectCnf(sock, FALSE);
					}
					break;
		  		}else
		  		{
		  			sock->connected = TRUE;
					
		  		}
			}
			platform_socket_connectCnf(sock, (err == 0) ? TRUE:FALSE);
	  		break;
	  	}

		case SOC_EVT_READ:
      		platform_socket_recvInd(sock, len);
      	break;
    	case SOC_EVT_CLOSE:
      		platform_socket_closeCnf(sock);
      	break;
    	case SOC_EVT_CLOSE_IND:
      /*+\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
		sock->remoteCloseDelayTimerId = OPENAT_create_timerTask(platform_socket_remote_close_delay_timer_cb,s);
		if(sock->remoteCloseDelayTimerId)
      	{
          	if(OPENAT_start_timer(sock->remoteCloseDelayTimerId, 500))
          	{
              	sock->remoteCloseDelayed = TRUE;
          	}
          	else
          	{
              	OPENAT_print("%s SOC_EVT_CLOSE_IND OSATimerStart fail", __FUNCTION__);
              	OPENAT_delete_timer(sock->remoteCloseDelayTimerId);
              	platform_socket_closeInd(sock);
          	}
          
      	}
      	else
      	{
          	OPENAT_print("%s SOC_EVT_CLOSE_IND OSATimerCreate fail", __FUNCTION__);
          	platform_socket_closeInd(sock);
      	}
      /*-\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
      	break;

    	case SOC_EVT_WRITE:
      		platform_socket_sendCnf(sock, (err == 0) ? TRUE:FALSE, len);
      	break;
		
    	case SOC_EVT_SEND_ACK:
      		platform_socket_sendAckCnf(sock,len);
      	break;
		
    	default:
      		ASSERT(0);
      	break;
  }
}





void platform_lua_socket_init(void)
{
    int i =0;
    CycleQueue* queue;
    
    for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
    {
        lua_socket_context.socket_info[i].sock_id = LUA_INVALID_SOKCET_ID;
		/*+\bug\wj\2020.4.5\SSL初次建立会连接死机问题*/
		lua_socket_context.socket_info[i].sslRxQ.buf = NULL;
		/*-\bug\wj\2020.4.5\SSL初次建立会连接死机问题*/
    }
    OPENAT_socket_init();
	openat_tls_init();
}



kal_bool platform_is_domain_name(kal_char * pData)
{    
    if ((*pData < '0') || (*pData > '9'))
    {
        return KAL_TRUE;
    }
    
    while(*pData)
    {
        if ((*pData != '.') && ((*pData < '0') || (*pData > '9')))
        {
            return KAL_TRUE;
        } 

        pData++;
    }
    
    return KAL_FALSE;
}



kal_uint32 platform_lua_socket_id_to_index(kal_uint32 sock_id)
{
    int i =0;
    
    for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
    {
        if(lua_socket_context.socket_info[i].sock_id == sock_id)
        {
            return i;
        }
    }

    return LUA_INVALID_SOCKET_INDEX;
}


char* platform_get_ip_by_name(kal_char* url)
{
  struct openSocketHostent* ipAddr = OPENAT_socket_gethostbyname(url);

  OPENAT_print("%s ipAddr %p length = %d", __func__, ipAddr, (ipAddr ? ipAddr->h_length : 0));

  if(ipAddr && ipAddr->h_length == 4)
  {
    return ipAddr->h_addr_list[0];
  }
  else
  {
    return NULL;
  }
}



kal_bool platform_activate_pdp(kal_char* apn, 
                                      kal_char* user_name, 
                                      kal_char* password)
{
  return OPENAT_active_pdp(apn, user_name, password, platform_pdp_active_cb);
}


kal_bool platform_deactivate_pdp(void)
{  
  return OPENAT_deactivate_pdp();
}

kal_bool platform_socket_send(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
  lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
  socketRemainBuf *sbuf = openatGetRemainBuf(socket_index);
  INT32 ret;
  if(sock->sock_id != LUA_INVALID_SOKCET_ID && sock->connected)
  {
    do{
      if(sock->sock_type == SOC_SOCK_STREAM)
      {
        /*+\NEW\WJ\2018.11.30\加快TCP发送速度*/
        sbuf->readySendLen = length;
        /*-\NEW\WJ\2018.11.30\加快TCP发送速度*/
        //TODO 发送长度不足的时候如何处理？？？
        /*+\bug\wangjian\2019.3.20\修改ssl和socket同时连接发送出现问题*/
        ret = OPENAT_socket_send(sock->sock_id, data, length, 0);
        /*-\bug\wangjian\2019.3.20\修改ssl和socket同时连接发送出现问题*/
        /*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
        OPENAT_print("%s size = %d, written = %d",__FUNCTION__,sbuf->readySendLen,ret);
        if(ret < 0)
        {
          break;
        }else
        {
        	platform_socket_sendCnf(sock,TRUE,ret);
        }
        if(sbuf->readySendLen > ret)
        {
            QueueInsert(&sbuf->sbufQueue,data+ret,sbuf->readySendLen-ret);
        }
        /*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
      }
      else if(sock->sock_type == SOC_SOCK_STREAM_SSL)
      {
        //ret = OPENAT_ssl_send(sock->sock_id, data, length);
		ret = openat_tls_write(sock->sock_id, data, length);
        if(ret < 0)
        {
          break;
        }
		/*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
		#if 0
		else
        {
        	platform_socket_sendCnf(sock,TRUE,ret);
        }
		#endif
        /*+\Bug 271\zhutianhua\2019.1.25 14:9\tcp ssl发送一次数据，会收到两次send cnf*/
        sock->totalSendLen = openat_tls_get_encrypt_len(sock->sock_id);
		/*-\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
        sock->sentLen = 0;
        /*-\Bug 271\zhutianhua\2019.1.25 14:9\tcp ssl发送一次数据，会收到两次send cnf*/
      }
      else
      {
        struct openSocketAddr toAddr;
        toAddr.sin_port = sock->port;
        toAddr.sin_addr.s_addr = sock->sock_addr.s_addr;
        ret = OPENAT_socket_sendto(sock->sock_id, data, length, 0, &toAddr, sizeof(toAddr));
        if(ret < 0)
        {
          break;
        }
        else
        { 
          platform_socket_sendCnf(sock,TRUE,ret);
        }
      }
      return TRUE;
    }while(0);
    
    platform_socket_sendCnf(sock, FALSE, 0);
  }
  return FALSE;
}

kal_int32 platform_ssl_recv(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
	lua_socket_info_struct* sock_info=NULL;
	int ret = 0;

	sock_info = &lua_socket_context.socket_info[socket_index];

	
	if(LUA_INVALID_SOKCET_ID !=sock_info->sock_id)
	{
	  	ret = QueueDelete(&sock_info->sslRxQ, data, length);//openatSslPopRxQ(sock, data, size);

	  	OPENAT_print("%s lua get datalen %d QueueLen %d  quenefreelen %d",__func__,ret,QueueLen(&sock_info->sslRxQ),QueueGetFreeSpace(&sock_info->sslRxQ));
	  	if (QueueLen(&sock_info->sslRxQ)==0)
	  	{
	  		platform_process_read_event(sock_info->sock_id);		
	  	}
	}
	OPENAT_print("%s recv %d %s", __func__, ret,data);
	return ret;
}
kal_int32 platform_socket_recv(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
  ASSERT(data != NULL && length > 0);

  lua_socket_info_struct* sock_info;
  struct openSocketAddr from_addr;
  int recv_result;
    
  sock_info = &lua_socket_context.socket_info[socket_index];

  int addrlen = sizeof(from_addr);

  from_addr.sin_addr.s_addr = sock_info->sock_addr.s_addr;
  
  from_addr.sin_port = sock_info->port;
  
  if(sock_info->sock_type == SOC_SOCK_STREAM)
  {           
      recv_result = OPENAT_socket_recv(sock_info->sock_id,
                              data, 
                              length, 
                              0);                                
  }
  else if(sock_info->sock_type == SOC_SOCK_STREAM_SSL)
  {
      recv_result = platform_ssl_recv(socket_index,data, length );

  }
  else if(sock_info->sock_type == SOC_SOCK_DGRAM)
  {
      recv_result = OPENAT_socket_recvfrom(sock_info->sock_id,
                                 data, 
                                 length, 
                                 0,
                                 &from_addr,
                                 &addrlen);               
  }

 
  return recv_result;
}
 
void platform_socket_on_recv_done(kal_uint8 socket_index, kal_uint32 recv_len)
{
  lua_socket_info_struct* sock_info;
  sock_info = &lua_socket_context.socket_info[socket_index];

	#if 0
  if(sock_info->sock_type == SOC_SOCK_STREAM_SSL)
  {
    OPENAT_ssl_recved(sock_info->sock_id, recv_len);
  }
  else
  {
    OPENAT_socket_recved(sock_info->sock_id, recv_len);
  }
  #endif
}

kal_bool platform_socket_close(kal_uint8 socket_index)
{
  	//关闭socket
  	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];

  	OPENAT_print("%s close socket id = %d", __FUNCTION__, sock->sock_id);

  	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
  	{
	    /*+\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
	    platform_socket_stop_remote_close_delay_timer(sock);
	    /*-\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
		OPENAT_socket_close(sock->sock_id);
		if(sock->sock_type == SOC_SOCK_STREAM_SSL)
	    {
	      platform_ssl_close(sock->sock_id,sock->ssl_ctx_id);
	    }


	    //sock->sock_id = LUA_INVALID_SOKCET_ID;
  	}
  	sock->connected = FALSE;
  	//TODO 异步通知
  	platform_socket_closeCnf(sock);
  	return KAL_TRUE;
}


kal_bool platform_socket_destroy(kal_uint8 socket_index)
{
  	//关闭socket
  	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];

  	OPENAT_print("%s destroy socket id = %d", __FUNCTION__, sock->sock_id);

 	sock->sock_id = LUA_INVALID_SOKCET_ID;
  	return KAL_TRUE;
}

static u8 platform_process_read_event(kal_int8 SocketID)
{
	#define RECVLEN  1460
	lua_socket_info_struct* sock = platform_socket_ctx(SocketID);
	int recvLen;
	int recved = 0;
	char buf[RECVLEN+1];
	int remainlen=0;
	
	if(LUA_INVALID_SOKCET_ID !=SocketID && sock)
	{
		OPENAT_print("%s SocketID %d enter",__func__, SocketID);
		while(1)
		{
			recvLen = RECVLEN;
			if(QueueGetFreeSpace(&sock->sslRxQ) < RECVLEN)
			{
			  	OPENAT_print("%s no free space for recv",__func__);
			  	break;
			}
			//recvLen = ssl_read(sock->ssl, buf, recvLen);
			memset(buf,0,sizeof(buf));
			recvLen = openat_tls_read(SocketID,
	                              buf, 
	                              recvLen 
	                              );

			
			if(recvLen > 0)
			{
			  	QueueInsert(&sock->sslRxQ, buf, recvLen);
			  	recved += recvLen;
				memset(buf,0,sizeof(buf));

				OPENAT_socket_ioctl(SocketID, OPEANT_SOCKET_FIONREAD_CMD, &remainlen);
				OPENAT_print("%s ssl_recv  remainlen %d recved %d recvLen %d",__func__, remainlen,recved,recvLen);

				//if(remainlen == 0)
				// 	break;
			}
			else
			{
				OPENAT_print("%s recv WOULDBLOCK %d",__func__, recvLen);
			  	break;
			}
		}

		OPENAT_print("%s recved %d QueueLen %d leave",__func__, recved,QueueLen(&sock->sslRxQ));

		if(recved > 0)
      	{
        	platform_socket_recvInd(sock, QueueLen(&sock->sslRxQ));
      	}
	}
}
/*+\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
void platform_free_cert(lua_socket_info_struct* sock)
{
	if(sock->soc_cert.serverCacert)
	{
		OPENAT_free(sock->soc_cert.serverCacert);
		sock->soc_cert.serverCacert = NULL;
		//g_s_cert.serverCacert = NULL;//malloc in lua script,should not free here
	}

	if(sock->soc_cert.clientCacert)
	{
		OPENAT_free(sock->soc_cert.clientCacert);
		sock->soc_cert.clientCacert = NULL;
		//g_s_cert.clientCacert = NULL;//malloc in lua script,should not free here
	}

	if(sock->soc_cert.clientKey)
	{
		OPENAT_free(sock->soc_cert.clientKey);
		sock->soc_cert.clientKey = NULL;
		//g_s_cert.clientKey = NULL;//malloc in lua script,should not free here
	}

	if(sock->soc_cert.hostName)
	{
		OPENAT_free(sock->soc_cert.hostName);
		sock->soc_cert.hostName = NULL;
		//g_s_cert.hostName = NULL;//malloc in lua script,should not free here
	}
}

void platform_load_cert(lua_socket_info_struct* lua_socket_info,mthl_socket_cert* cert_t)
{
	int serverCacertLen=0;
	int clientCacertLen=0;
	int clientKeyLen=0;
	int hostNameLen=0;
	
	if(lua_socket_info->soc_cert.serverCacert || lua_socket_info->soc_cert.clientCacert
		|| lua_socket_info->soc_cert.clientKey || lua_socket_info->soc_cert.hostName)// it will never happen
	{
		platform_assert(__FUNCTION__,__LINE__);
	}else
	{
		if(cert_t->serverCacert != NULL)
		{
			serverCacertLen= strlen(cert_t->serverCacert);
			lua_socket_info->soc_cert.serverCacert = OPENAT_malloc(serverCacertLen+1);
			if(!lua_socket_info->soc_cert.serverCacert)
				platform_assert(__FUNCTION__,__LINE__);
			memset(lua_socket_info->soc_cert.serverCacert,0,serverCacertLen+1);
			memcpy(lua_socket_info->soc_cert.serverCacert,cert_t->serverCacert,serverCacertLen);
		}
		if(cert_t->clientCacert != NULL)
		{
			clientCacertLen = strlen(cert_t->clientCacert);
			lua_socket_info->soc_cert.clientCacert = OPENAT_malloc(clientCacertLen+1);
			if(!lua_socket_info->soc_cert.clientCacert)
				platform_assert(__FUNCTION__,__LINE__);
			memset(lua_socket_info->soc_cert.clientCacert,0,clientCacertLen+1);
			memcpy(lua_socket_info->soc_cert.clientCacert,cert_t->clientCacert,clientCacertLen);
		}
		if(cert_t->clientKey != NULL)
		{
			clientKeyLen = strlen(cert_t->clientKey);
			lua_socket_info->soc_cert.clientKey = OPENAT_malloc(clientKeyLen+1);
			if(!lua_socket_info->soc_cert.clientKey)
				platform_assert(__FUNCTION__,__LINE__);
			memset(lua_socket_info->soc_cert.clientKey,0,clientKeyLen+1);
			memcpy(lua_socket_info->soc_cert.clientKey,cert_t->clientKey,clientKeyLen);
		}
		if(cert_t->hostName != NULL)
		{
			hostNameLen = strlen(cert_t->hostName);
			lua_socket_info->soc_cert.hostName = OPENAT_malloc(hostNameLen+1);
			if(!lua_socket_info->soc_cert.hostName)
				platform_assert(__FUNCTION__,__LINE__);
			memset(lua_socket_info->soc_cert.hostName,0,hostNameLen+1);
			memcpy(lua_socket_info->soc_cert.hostName,cert_t->hostName,hostNameLen);
		}
	}
}

/*-\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
static void platform_ssl_cb(int s, openSocketEvent evt, int err, char* data, int len)
{
	/*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
	lua_socket_info_struct* sock = platform_socket_ctx(s);
	if(evt == SOC_EVT_SEND_ACK)
	{
		sock->sentLen += len;
	}
	/*-\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
	openat_tls_soc_notify_ind_hdlr(s,evt, (err == 0) ? KAL_TRUE : KAL_FALSE);
}


void platform_ssl_notify_ind(kal_int16 mod,
                                 kal_int8 s,
                                 openat_tls_event_enum event,
                                 kal_bool result,
                                 kal_int32 error,
                                 kal_int32 detail_cause)
{
	lua_socket_info_struct* sock = platform_socket_ctx(s);
	int shakeRet = 0;
	OPENAT_print("%s s %d got event %d error %d result %d", __FUNCTION__, s,event,error,result);
    switch(event)
    {
    	#if 0
        case OPENAT_TLS_HANDSHAKE_READY:
            if (result) {                                   /* error */
                
            }
            else{
                
                /* handshake */
            }
            break;
			#endif
        case OPENAT_TLS_HANDSHAKING:
            if (result) 
			{                
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{
					OPENAT_print("%s sock->sock_id %d", __func__,sock->sock_id);
                    shakeRet = openat_tls_handshake(sock->sock_id);
                    OPENAT_print("%s openat_tls_handshake ret %d", __func__, shakeRet);
                }
            }else
            {
            	OPENAT_print("%s OPENAT_TLS_HANDSHAKING result false", __func__);
            }
            break;
        case OPENAT_TLS_HANDSHAKE_DONE:
            if (result)
			{
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{ 
					sock->connected = TRUE;
			      	platform_socket_connectCnf(sock, TRUE);
             	}
            }
            else 
			{
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{ 
					OPENAT_socket_close(sock->sock_id);
					platform_ssl_close(sock->sock_id,sock->ssl_ctx_id);
                    platform_socket_connectCnf(sock, FALSE);
                }
            }
            break;

        case OPENAT_TLS_READ:
            platform_process_read_event(sock->sock_id);
            break;

        case OPENAT_TLS_WRITE:
			/*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
            platform_socket_sendCnf(sock, result, detail_cause);//这里需要特别注意
			/*-\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
            break;

        case OPENAT_TLS_CLOSE:
			OPENAT_socket_close(sock->sock_id);
			platform_ssl_close(sock->sock_id,sock->ssl_ctx_id);
			platform_socket_closeInd(sock);
            break;
    }

    return;
}


void platform_ssl_close(u8 sockID,s32 ctxID){
    u32 ret = 0;
	lua_socket_info_struct* sock = platform_socket_ctx(sockID);
	
	if (sock->sslRxQ.buf)
  	{
  		OPENAT_free(sock->sslRxQ.buf);
		sock->sslRxQ.buf = NULL;
  	}
  	sock->sslRxQ.size = 0;
  	QueueClean(&sock->sslRxQ);
  
    ret = openat_tls_delete_conn(sockID);
    if (OPENAT_TLS_ERR_NONE != ret){
        OPENAT_print("%s delete_conn fail ret=%d", __FUNCTION__,ret);
    }
    ret = openat_tls_delete_ctx(ctxID);
    if (OPENAT_TLS_ERR_NONE != ret){
        OPENAT_print("%s delete_ctx fail ret=%d", __FUNCTION__,ret);
    }

	platform_free_cert(sock);
}


int platform_ssl_create(lua_socket_info_struct* lua_socket_info)
{
	openat_tls_socaddr_struct faddr;
	s8   ssl_ctx_id; 
	int ret;
	
	
	if(lua_socket_info->sslRxQ.buf)
	{
		platform_assert(__FUNCTION__,__LINE__);
	}else
	{
		lua_socket_info->sslRxQ.buf = OPENAT_malloc(SSL_SOCKET_RX_BUF_SIZE);//buf;//这里的buf什么时候释放，这里是否考虑放到platform_ssl_create
		lua_socket_info->sslRxQ.size = SSL_SOCKET_RX_BUF_SIZE;
		QueueClean(&lua_socket_info->sslRxQ);
	}
	faddr.port = lua_socket_info->port;
	memcpy(faddr.addr, &lua_socket_info->sock_addr.s_addr, 4);

	OPENAT_Socket_set_cb(lua_socket_info->sock_id, platform_ssl_cb); 
	ssl_ctx_id = openat_tls_new_ctx(OPENAT_TLS_ALL_VERSIONS,OPENAT_TLS_CLIENT_SIDE,0,0);
	lua_socket_info->ssl_ctx_id = ssl_ctx_id;

	openat_tls_new_conn(ssl_ctx_id ,lua_socket_info->sock_id,&faddr,platform_ssl_notify_ind);//CB ???
	if(lua_socket_info->soc_cert.serverCacert != NULL)
	{
		OPENAT_print("%s cert cert->serverCacert=0x%x %s", __FUNCTION__, lua_socket_info->soc_cert.serverCacert,lua_socket_info->soc_cert.serverCacert);
		ret = openat_tls_check_invalid_cert(lua_socket_info->sock_id,KAL_TRUE,lua_socket_info->soc_cert.serverCacert,KAL_FALSE);
		if(ret != OPENAT_TLS_ERR_NONE)
		{
			return ret;
		}
	}

	if(lua_socket_info->soc_cert.clientCacert)
	{
		ret = openat_tls_set_client_auth(ssl_ctx_id,lua_socket_info->soc_cert.clientCacert,lua_socket_info->soc_cert.clientKey,NULL,FALSE);
		if(ret != OPENAT_TLS_ERR_NONE)
		{
			return ret;
		}
	}

	//openat_tls_set_ssl_cfg(sockRec->SocketID,&ssl_cfg);

	openat_tls_handshake(lua_socket_info->sock_id);
	return 0;
	

}


void platform_dns_cb(const char *name, struct openSocketip_addr *ipaddr, void *callback_arg)
{
	lua_socket_info_struct* lua_socket_info =(lua_socket_info_struct*)callback_arg;

	if(lua_socket_info->sock_id == LUA_INVALID_SOKCET_ID)
	{	
		return;// when OPENAT_socket_create is fail
	}
	
	/*+\bug\zhuwangbin\2020.4.20\解决域名解析失败会死机的问题*/
	if (!ipaddr)
	{
		lua_socket_info->sock_addr.s_addr = 0;
	}
	else
	{
		lua_socket_info->sock_addr.s_addr = ipaddr->addr;
	}
	/*-\bug\zhuwangbin\2020.4.20\解决域名解析失败会死机的问题*/
	platform_socket_conn(lua_socket_info);
}

int platform_socket_open(
                                         openSocketType sock_type,
                                         kal_uint16    port,
                                         kal_char*    addr,
                                         mthl_socket_cert* cert)
{
	lua_socket_info_struct* lua_socket_info;
	BOOL result;
	INT32 ret;
	UINT8 socket_index;
	struct openSocketip_addr ipAddr;
	char* buf;
	
	socket_index = platformGetFreeSocket();
	if(socket_index == LUA_MAX_SOCKET_SUPPORT)
	{
		OPENAT_print("%s no enough socket for connect", __FUNCTION__);
		return -1;
	}

	if(strlen(addr) > MAX_SOCK_ADDR_LEN)
	{
		OPENAT_print("%s addr too long", __FUNCTION__, addr);
		return -1;
	}

	OPENAT_print("%s type=%d addr = %s:%d", __FUNCTION__, sock_type, addr, port);

	lua_socket_info = &lua_socket_context.socket_info[socket_index];
	lua_socket_info->port = port;
	lua_socket_info->sock_type = sock_type;
	strcpy(lua_socket_info->addr, addr);
	lua_socket_info->connected = FALSE;
	/*+\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
	#if 0	
	if(cert)
	{
		g_s_cert.serverCacert = cert->serverCacert;
		g_s_cert.clientCacert = cert->clientCacert;
		g_s_cert.clientKey = cert->clientKey;
		g_s_cert.hostName = cert->hostName;
	}
	#endif
	/*-\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
	ret = OPENAT_socket_gethostbyname_ex(addr,&ipAddr,platform_dns_cb,lua_socket_info);	
	
	OPENAT_print("%s gethostbyname ret=%d", __FUNCTION__, ret);
	if(ret !=0 && ret != -5)// 0 sucess   -5 INPROGRESS  
	{
	  	OPENAT_print("%s gethostbyname error", __FUNCTION__);
      	return -2;
    }else
	{
		lua_socket_info->sock_id = OPENAT_socket_create(sock_type, platform_socket_cb);
		if(lua_socket_info->sock_id < 0)
		{
			lua_socket_info->sock_id = LUA_INVALID_SOKCET_ID;
			//platform_socket_connectCnf(lua_socket_info, FALSE);
			OPENAT_print("%s create socket error", __FUNCTION__);
			return -1;
		}
		/*+\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
		if(sock_type == SOC_SOCK_STREAM_SSL && cert != NULL){
			platform_load_cert(lua_socket_info,cert);
		}
		/*-\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
	}

	if(ret == 0)
    {
		lua_socket_info->sock_addr.s_addr = ipAddr.addr;
		/*+\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
		platform_socket_conn(lua_socket_info,cert);
		/*-\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
		return socket_index;
    }
    else if(-5 == ret)//阻塞
    {
      	return socket_index;
    }
    
}

int platform_socket_conn(lua_socket_info_struct* lua_socket_info)
{
	BOOL result;
	INT32 ret, error;
	struct openSocketAddr sockaddr;
	INT32  sslret=0;

	sockaddr.sin_port = lua_socket_info->port;
	sockaddr.sin_addr.s_addr = lua_socket_info->sock_addr.s_addr;

	//lua_socket_info->sock_addr.s_addr=0: maybe dns fail  
	if(!lua_socket_info->sock_addr.s_addr || lua_socket_info->sock_id == LUA_INVALID_SOKCET_ID)
	{
		platform_socket_connectCnf(lua_socket_info, FALSE);
		OPENAT_socket_close(lua_socket_info->sock_id);
		return;//return socket_index;
	}

	if(lua_socket_info->sock_type == SOC_SOCK_STREAM || lua_socket_info->sock_type == SOC_SOCK_STREAM_SSL)
	{
		ret = OPENAT_socket_connect(lua_socket_info->sock_id, &sockaddr, sizeof(sockaddr));
		error = OPENAT_socket_error(lua_socket_info->sock_id);

		OPENAT_print("%s socket connect ret %d, err %d", __FUNCTION__, ret, error);

		if(ret < 0 && error != OPENAT_SOCKET_EINPROGRESS && error != OPENAT_SOCKET_EWOULDBLOCK)
		{
		  	platform_socket_connectCnf(lua_socket_info, FALSE);
		  	OPENAT_socket_close(lua_socket_info->sock_id);
		  	//lua_socket_info->sock_id = LUA_INVALID_SOKCET_ID;
		  	return;//return socket_index;
		}
		else if(ret == 0)
		{
			if(lua_socket_info->sock_type == SOC_SOCK_STREAM_SSL)
			{
				if(platform_ssl_create(lua_socket_info) !=0)
				{
					lua_socket_info->connected = FALSE;
					OPENAT_socket_close(lua_socket_info->sock_id);
					platform_ssl_close(lua_socket_info->sock_id,lua_socket_info->ssl_ctx_id);
		  			platform_socket_connectCnf(lua_socket_info, FALSE);
				}
				return;
			}
		  lua_socket_info->connected = TRUE;
		  platform_socket_connectCnf(lua_socket_info, TRUE);
		}
	}
	else
	{
		lua_socket_info->connected = TRUE;
		platform_socket_connectCnf(lua_socket_info, TRUE);
	}
  	//return socket_index;
}


kal_int32 platform_on_create_conn_cnf(lua_State *L,
                                             PlatformSocketConnectCnf* create_conn_cnf)
{
    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_CONNECT_CNF);
    setfieldInt(L, "socket_index", create_conn_cnf->socket_index);
    setfieldInt(L, "result", !create_conn_cnf->result);
    return 1;
}


kal_int32 platform_on_create_sock_ind(lua_State *L,
                                             mthl_create_sock_ind_struct* create_sock_ind)
{
    kal_uint32 socket_index = create_sock_ind->user_data;
    lua_socket_context.socket_info[socket_index].sock_id = create_sock_ind->sock_id;
    return 0;
}



kal_int32 platform_on_create_pdp_param_cnf(lua_State *L,
                                                     mthl_create_pdp_param_cnf_struct* create_pdp_param_cnf)
{

    OPENAT_print("lua platform_on_create_pdp_param_cnf :%d", create_pdp_param_cnf->result);
}



kal_int32 platform_on_active_pdp_cnf(lua_State *L,
                                           PlatformPdpActiveCnf* active_pdp_cnf)
{
    OPENAT_print("lua platform_on_active_pdp_cnf :%d %d", active_pdp_cnf->result,active_pdp_cnf->errorCause);

    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_PDP_ACTIVATE_CNF);
    setfieldInt(L, "result", active_pdp_cnf->result ? 0 : active_pdp_cnf->errorCause);
    return 1;
}



kal_int32 platform_on_send_data_cnf(lua_State *L,
                                          PlatformSocketSendCnf* send_data_cnf)
{
  lua_newtable(L);
  setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_SEND_CNF);
  setfieldInt(L, "socket_index", send_data_cnf->socket_index);
  setfieldInt(L, "result", !send_data_cnf->result);
  setfieldInt(L, "send_len", send_data_cnf->length);
  return 1;
}



kal_int32 platform_on_send_data_ind(lua_State *L,
                                          mthl_send_data_ind_struct* send_data_ind)
{
    lua_pushinteger(L, MSG_ID_TCPIP_SOCKET_SEND_IND);
    lua_pushinteger(L, platform_lua_socket_id_to_index(send_data_ind->sock_id));
    lua_pushinteger(L, send_data_ind->result);
    
    if(send_data_ind->result != KAL_TRUE)
    {
        /*错误处理 */
        lua_pushinteger(L, send_data_ind->ret_val); 
    }
    else
    {
        lua_pushinteger(L, 0); 
    }

    return 4;
}


kal_int32 platform_on_recv_data_ind(lua_State *L,
                                          PlatformSocketRecvInd* recv_data_ind)
{    

  lua_newtable(L);
  setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_RECV_IND);
  setfieldInt(L, "socket_index", recv_data_ind->socket_index);
  setfieldInt(L, "result", 0);
  setfieldInt(L, "recv_len", recv_data_ind->recv_len);

  return 1;
}



kal_int32 platform_on_sock_close_ind(lua_State *L,
                                            PlatformSocketCloseInd* sock_close_ind)
{
    lua_socket_info_struct* sock = &lua_socket_context.socket_info[sock_close_ind->socket_index];

    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_CLOSE_IND);
    setfieldInt(L, "socket_index", sock_close_ind->socket_index);
    setfieldInt(L, "result", !sock_close_ind->result);

    /*+\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
    platform_socket_stop_remote_close_delay_timer(sock);
    /*-\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
	OPENAT_socket_close(sock->sock_id);
	if(sock->sock_type == SOC_SOCK_STREAM_SSL)
    {
      platform_ssl_close(sock->sock_id,sock->ssl_ctx_id);
    }
    
    //sock->sock_id = LUA_INVALID_SOKCET_ID;    
    return 1;
}



kal_int32 platform_on_sock_close_cnf(lua_State *L,
                                           PlatformSocketCloseCnf* sock_close_cnf)
{
    kal_uint32 socket_index = sock_close_cnf->socket_index;

    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_CLOSE_CNF);
    setfieldInt(L, "socket_index", socket_index);
    setfieldInt(L, "result", !sock_close_cnf->result);

    return 1;
}



kal_int32 platform_on_deactivate_pdp_cnf(lua_State *L,
                                                 mthl_deactivate_pdp_cnf_struct* deactivate_pdp_cnf)
{
    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_PDP_DEACTIVATE_CNF);
    setfieldInt(L, "result", deactivate_pdp_cnf->result ? 0 : deactivate_pdp_cnf->error_cause);
    return 1;
}


kal_int32 platform_on_deactivate_pdp_ind(lua_State *L,
                                                 mthl_deactivate_pdp_ind_struct* deactivate_pdp_ind)
{

    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_PDP_DEACTIVATE_IND);
    if(deactivate_pdp_ind->error == 0)
    {
        setfieldInt(L, "result", 0);
    }
    else
    {
        setfieldInt(L, "result", deactivate_pdp_ind->error);
    }

    return 1;
}



