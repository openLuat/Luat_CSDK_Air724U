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

#define LUA_MAX_RECV_IND_LEN 2*1024

#define DELAY_CLOSE_IND_TIME 500  //延迟主动上报的时间，单位毫秒
//#define LUA_SOCKET_TEST
/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
typedef enum {
    PLATFORM_CONN_INIT = 0 << 0, 
    PLATFORM_CONN_CONNECTING = 1 << 0, 
    PLATFORM_CONN_CONNECTED= 1 << 1,
    PLATFORM_CONN_CLOSE = 1 << 2
} platform_conn_status_enum;
/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/	

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
	/*+\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	int   ssl_ctx_id;
	/*-\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	platform_conn_status_enum conn_status;
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	mthl_socket_cert soc_cert;
	CycleQueue sslRxQ;
	/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
	kal_uint32 reqLen;  /*通知上层准备要接收的数据长度*/
	kal_uint32 buffLen;  /*剩余还没有通知上层处理的待接收的数据长度*/
	/*-\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
}lua_socket_info_struct;
/*+\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
typedef struct
{
    kal_int8        sock_id;       /* Socket id to handle this notification. */
    openat_tls_event_enum  event;         /* Reported event from TLS task.*/
    kal_bool        result;        /* Success or failure of the notification. */
    kal_int32       error;         /* Error code. */
    kal_int32       detail_cause;  /* Detail error cause. */
} platform_tls_evt_ind_struct;
/*-\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/

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
void platform_socket_conn(lua_socket_info_struct* lua_socket_info);
kal_bool platform_socket_close(kal_uint8 socket_index);

//static mthl_socket_cert g_s_cert;
/*+\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
void platform_ssl_notify_process(
                                 kal_int8 s,
                                 openat_tls_event_enum event,
                                 kal_bool result,
                                 kal_int32 error,
                                 kal_int32 detail_cause);

static HANDLE g_s_tlsTask;
#define PLATFORM_MSG_TLS    (1)
/*-\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
/*+\wangjian\2019.12.18\ssl下载大文件存在线程间临界区问题，用信号量做临界区保护*/
static HANDLE ssl_semaphore_Ref = NULL;
VOID platformSslSemaphoreAcquire()
{
	if(ssl_semaphore_Ref == NULL)
	{
		ssl_semaphore_Ref = OPENAT_create_semaphore(1);
	}
	if( !OPENAT_wait_semaphore(ssl_semaphore_Ref, OPENAT_OS_SUSPENDED))
	{
		platform_assert(__FUNCTION__,__LINE__);
	}
}

VOID platformSslSemaphoreRelease()
{
	if(!OPENAT_release_semaphore(ssl_semaphore_Ref))
	{
		platform_assert(__FUNCTION__,__LINE__);
	}
}
/*-\wangjian\2019.12.18\ssl下载大文件存在线程间临界区问题，用信号量做临界区保护*/

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
  UINT8 i;

  for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
  {
/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
    if(lua_socket_context.socket_info[i].sock_id == LUA_INVALID_SOKCET_ID
		&& lua_socket_context.socket_info[i].conn_status == PLATFORM_CONN_INIT)
/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
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
    if(lua_socket_context.socket_info[i].sock_id == s )
    {
      return &lua_socket_context.socket_info[i];
    }
  }
  return NULL;
}

/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
static lua_socket_info_struct* platform_socket_ctx_ext(int s,platform_conn_status_enum status)
{
  UINT8 i;
  for(i = 0; i < LUA_MAX_SOCKET_SUPPORT; i++)
  {
    if((lua_socket_context.socket_info[i].sock_id == s) && (lua_socket_context.socket_info[i].conn_status & status) )
    {
      return &lua_socket_context.socket_info[i];
    }
  }
  return NULL;
}
/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/

static void platform_socket_connectCnf(lua_socket_info_struct* sock, BOOL success)
{
  PlatformMsgData msg;
  msg.socketConnectCnf.socket_index = platform_socket_index(sock);
  msg.socketConnectCnf.result = success;
  platform_rtos_send(MSG_ID_APP_MTHL_CREATE_CONN_CNF, &msg);
}

static void platform_socket_recvInd(lua_socket_info_struct* sock, INT32 len)
{
  PlatformMsgData msg;
  msg.socketRecvInd.socket_index = platform_socket_index(sock);
  	/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
  //OPENAT_print("%s wjwj index = %d,len=%d,reqLen = %d,buffLen=%d",__FUNCTION__,msg.socketRecvInd.socket_index,len,sock->reqLen,sock->buffLen);
  HANDLE cr = OPENAT_enter_critical_section();
  if(sock->reqLen == 0)
  {
		
  	
  		if(sock->sock_type == SOC_SOCK_STREAM_SSL)
  		{
			sock->buffLen += len;
	  			
	  		if(sock->buffLen > LUA_MAX_RECV_IND_LEN)
	  		{
				sock->reqLen = LUA_MAX_RECV_IND_LEN;
				sock->buffLen -= LUA_MAX_RECV_IND_LEN;
			}
			else
			{
	  			sock->reqLen = sock->buffLen;
				sock->buffLen = 0;
			}
  		}
		else
		{
			sock->buffLen = OPENAT_socket_getRecvAvailSize(sock->sock_id);
			sock->reqLen = sock->buffLen > LUA_MAX_RECV_IND_LEN ? LUA_MAX_RECV_IND_LEN : sock->buffLen;
		}
  		msg.socketRecvInd.recv_len = sock->reqLen;
		OPENAT_exit_critical_section(cr);
  		platform_rtos_send(MSG_ID_APP_MTHL_SOCK_RECV_IND, &msg);
  }
  else
  {
  		if(sock->sock_type == SOC_SOCK_STREAM_SSL)
			sock->buffLen += len;
		OPENAT_exit_critical_section(cr);
  }
  	/*-\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
}

static void platform_socket_closeCnf(lua_socket_info_struct* sock)
{	
	PlatformMsgData msg;
	msg.socketCloseCnf.socket_index = platform_socket_index(sock);
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
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
    lua_socket_info_struct* sock = platform_socket_ctx_ext(param,PLATFORM_CONN_CONNECTED);
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/

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
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
  	lua_socket_info_struct* sock = platform_socket_ctx_ext(s,(PLATFORM_CONN_CONNECTING | PLATFORM_CONN_CONNECTED));
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/

  	ASSERT(sock != NULL);
  	switch(evt)
  	{
	  	case SOC_EVT_CONNECT:
	  	{
	  		if(err == 0)
			{
				#ifdef LUA_SOCKET_SSL_SUPPORT
				if(sock->sock_type == SOC_SOCK_STREAM_SSL)
		  		{
		  			if(platform_ssl_create(sock) !=0)
					{
			  			platform_socket_connectCnf(sock, FALSE);
					}
					break;
		  		}
				  #endif
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

/*+\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
#ifdef LUA_SOCKET_SSL_SUPPORT
static void platform_tls_task_entry(void* p)
{
  int msgId=-1;
  platform_tls_evt_ind_struct* evt=NULL;
  
  
  while(1)
  {
    OPENAT_wait_message(g_s_tlsTask, &msgId, (void**)&evt, OPENAT_OS_SUSPENDED);

    OPENAT_print("%s msgId %d Task sock %d got evt %d", __func__, msgId,evt->sock_id, evt->event);
	if(msgId == PLATFORM_MSG_TLS)
	{
		platform_ssl_notify_process(evt->sock_id,evt->event,evt->result,evt->error,evt->detail_cause);

	}
    OPENAT_free(evt);
    evt = NULL;
  }
}
#endif
/*-\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/


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
	/*+\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
	#ifdef LUA_SOCKET_SSL_SUPPORT
	OPENAT_create_task(&g_s_tlsTask, 
          platform_tls_task_entry,
          NULL, 
          NULL, 
          5 * 1024, 
          24,
          OPENAT_OS_CREATE_DEFAULT, 
          20, 
          "TLSTask");
	/*-\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
	
	openat_tls_init();
	#endif
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

/*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
kal_int32 platform_socket_error(kal_uint8 socket_index)
{
	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	int error;
	
	error = OPENAT_socket_error(sock->sock_id);

	return error;
}
/*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 

kal_bool platform_socket_send(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
  lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
  socketRemainBuf *sbuf = openatGetRemainBuf(socket_index);
  INT32 ret;
  /*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
  int error;
  /*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
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
	  #ifdef LUA_SOCKET_SSL_SUPPORT
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
      #endif
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

	/*+\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
	error = OPENAT_socket_error(sock->sock_id);

	if(error == OPENAT_SOCKET_EINPROGRESS || error == OPENAT_SOCKET_EWOULDBLOCK)
	{
		OPENAT_print("%s is block",__FUNCTION__);
		platform_socket_sendCnf(sock, FALSE, 0);
		return FALSE;
	}else
	{
		platform_socket_sendCnf(sock, FALSE, 0);
	}
    /*-\BUG \lijiaodi\2020.10.30send的返回结果添加发送失败的原因上报*/ 
  }
  return FALSE;
}

#ifdef LUA_SOCKET_SSL_SUPPORT
kal_int32 platform_ssl_recv(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
	lua_socket_info_struct* sock_info=NULL;
	int ret = -1;

	sock_info = &lua_socket_context.socket_info[socket_index];


	if(LUA_INVALID_SOKCET_ID !=sock_info->sock_id && sock_info->connected)
	{
	  	ret = QueueDelete(&sock_info->sslRxQ, data, length);//openatSslPopRxQ(sock, data, size);

	  	OPENAT_print("%s lua get datalen %d QueueLen %d  quenefreelen %d",__func__,ret,QueueLen(&sock_info->sslRxQ),QueueGetFreeSpace(&sock_info->sslRxQ));
	  	if (QueueLen(&sock_info->sslRxQ)==0)
	  	{
	  		platform_process_read_event(sock_info->sock_id);
	  	}
	}
	OPENAT_print("%s recv %d ", __func__, ret);
	return ret;
}
#endif
kal_int32 platform_socket_recv(kal_uint8 socket_index,
                                       kal_uint8*    data,
                                       kal_uint16    length)
{
  ASSERT(data != NULL && length > 0);

  lua_socket_info_struct* sock_info;
  struct openSocketAddr from_addr;
  int recv_result = -1;
  PlatformMsgData msg;
  	
  sock_info = &lua_socket_context.socket_info[socket_index];
  //OPENAT_print("%s wjwj start index = %d,length = %d reqLen=%d,buffLen=%d",__FUNCTION__,socket_index,length,sock_info->reqLen,sock_info->buffLen);   

  int addrlen = sizeof(from_addr);

  from_addr.sin_addr.s_addr = sock_info->sock_addr.s_addr;

  from_addr.sin_port = sock_info->port;
  if(LUA_INVALID_SOKCET_ID !=sock_info->sock_id && sock_info->connected)
  {
	/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
  	  if(length > sock_info->reqLen)
  	  {
		 length = sock_info->reqLen;
	  }
	/*-\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
	  if(sock_info->sock_type == SOC_SOCK_STREAM)
	  {
	      recv_result = OPENAT_socket_recv(sock_info->sock_id,
	                              data,
	                              length,
	                              0);
	  }
	  #ifdef LUA_SOCKET_SSL_SUPPORT
	  else if(sock_info->sock_type == SOC_SOCK_STREAM_SSL)
	  {
	      recv_result = platform_ssl_recv(socket_index,data, length );

	  }
	  #endif
	  else if(sock_info->sock_type == SOC_SOCK_DGRAM)
	  {
	      recv_result = OPENAT_socket_recvfrom(sock_info->sock_id,
	                                 data,
	                                 length,
	                                 0,
	                                 &from_addr,
	                                 &addrlen);
	  }
	/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
	  //OPENAT_print("%s wjwj end index %d length = %d reqLen=%d,buffLen=%d",__FUNCTION__,socket_index,length,sock_info->reqLen,sock_info->buffLen);
	  HANDLE cr = OPENAT_enter_critical_section();
	  if(recv_result >= 0)
	  {
			sock_info->reqLen -= recv_result;
			if(sock_info->sock_type == SOC_SOCK_DGRAM)
				sock_info->reqLen = 0;
	  }
	  if(sock_info->reqLen == 0 && sock_info->buffLen > 0)
	  {
	  		if(sock_info->sock_type == SOC_SOCK_STREAM_SSL){
		  		if(sock_info->buffLen > LUA_MAX_RECV_IND_LEN){ /*大于LUA_MAX_RECV_IND_LEN的话就先上报上层取LUA_MAX_RECV_IND_LEN*/
					sock_info->reqLen = LUA_MAX_RECV_IND_LEN;
					sock_info->buffLen -= LUA_MAX_RECV_IND_LEN;
		  		}
				else{
		  			sock_info->reqLen = sock_info->buffLen;
					sock_info->buffLen = 0;

				}
	  		}
			else{
				sock_info->buffLen = OPENAT_socket_getRecvAvailSize(sock_info->sock_id);
				sock_info->reqLen = (sock_info->buffLen > LUA_MAX_RECV_IND_LEN) ? LUA_MAX_RECV_IND_LEN : sock_info->buffLen;
			}
			msg.socketRecvInd.socket_index = platform_socket_index(sock_info);
			msg.socketRecvInd.recv_len = sock_info->reqLen;
			OPENAT_exit_critical_section(cr);
			//OPENAT_print("%s wjwj buffLen = %d,reqLen = %d",__FUNCTION__,sock_info->buffLen,sock_info->reqLen);
			if(msg.socketRecvInd.recv_len > 0)
				platform_rtos_send(MSG_ID_APP_MTHL_SOCK_RECV_IND, &msg);
			if(sock_info->remoteCloseDelayed && sock_info->remoteCloseDelayTimerId)
			{
				if(OPENAT_stop_timer(sock_info->remoteCloseDelayTimerId))
				{
					if(!OPENAT_start_timer(sock_info->remoteCloseDelayTimerId, DELAY_CLOSE_IND_TIME))
					{
						platform_socket_closeInd(sock_info);						
					}
				}
			}
	  }
	  else{
			OPENAT_exit_critical_section(cr);
	  }
	  /*-\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
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

kal_bool platform_socket_close_without_cnf(kal_uint8 socket_index)
{
  	//关闭socket
  	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	socketRemainBuf *sbuf = NULL;
  	OPENAT_print("%s close socket id = %d,socket_index = %d", __FUNCTION__, sock->sock_id,socket_index);
	
	
	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
	{
		/*+\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
		platform_socket_stop_remote_close_delay_timer(sock);
		/*-\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
		/*+\bug\wj\2021.1.15\BUG4223 SSL接收过程中突然关闭，造成死机*/
		#ifdef LUA_SOCKET_SSL_SUPPORT
		if(sock->sock_type == SOC_SOCK_STREAM_SSL)
		{
			platformSslSemaphoreAcquire();
			OPENAT_socket_close(sock->sock_id);
			platform_ssl_close(socket_index);
			platformSslSemaphoreRelease();
		}
		else
		#endif
		{
			OPENAT_socket_close(sock->sock_id);
		}
		
		if(sock->sock_type == SOC_SOCK_STREAM)
		{
			sbuf = openatGetRemainBuf(socket_index);
			if (sbuf->sbufQueue.buf != NULL)
			{
			    openatFreeRemainBuf(socket_index);
			}
		}
		
		sock->conn_status = PLATFORM_CONN_CLOSE;	
		/*-\bug\wj\2021.1.15\BUG4223 SSL接收过程中突然关闭，造成死机*/
		sock->connected = FALSE;
		sock->sock_id = LUA_INVALID_SOKCET_ID;
	}

	//platform_socket_closeCnf(sock);
  	return KAL_TRUE;

}

kal_bool platform_socket_close(kal_uint8 socket_index)
{
	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	
	platform_socket_close_without_cnf(socket_index);

	platform_socket_closeCnf(sock);

	return KAL_TRUE;
}


kal_bool platform_socket_destroy(kal_uint8 socket_index)
{
  	//关闭socket
  	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];

	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
  	OPENAT_print("%s destroy socket id = %d sock->conn_status = %d socket_index = %d", __FUNCTION__, sock->sock_id,sock->conn_status,socket_index);
	if(sock->conn_status != PLATFORM_CONN_CLOSE)
	{
		return KAL_FALSE;
	}

	sock->conn_status = PLATFORM_CONN_INIT;
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
 	sock->sock_id = LUA_INVALID_SOKCET_ID;
  	return KAL_TRUE;
}

#ifdef LUA_SOCKET_SSL_SUPPORT
static u8 platform_process_read_event(kal_int8 SocketID)
{
	#define RECVLEN  1460
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	lua_socket_info_struct* sock = platform_socket_ctx_ext(SocketID,PLATFORM_CONN_CONNECTED);
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	int recvLen;
	int recved = 0;
	char buf[RECVLEN+1];
	int remainlen=0;

	if(LUA_INVALID_SOKCET_ID !=SocketID && sock)
	{
		OPENAT_print("%s SocketID %d enter",__func__, SocketID);
		platformSslSemaphoreAcquire();
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
		platformSslSemaphoreRelease();
		
		OPENAT_print("%s recved %d QueueLen %d leave",__func__, recved,QueueLen(&sock->sslRxQ));
		if(recved > 0)
      	{
        	platform_socket_recvInd(sock, recved);
      	}
	}
}
#endif

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

#ifdef LUA_SOCKET_SSL_SUPPORT

static void platform_ssl_cb(int s, openSocketEvent evt, int err, char* data, int len)
{
	/*+\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	lua_socket_info_struct* sock = platform_socket_ctx_ext(s,PLATFORM_CONN_CONNECTED | PLATFORM_CONN_CONNECTING);
	if((evt == SOC_EVT_SEND_ACK) && sock)
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	{
		sock->sentLen += len;
	}
	/*-\bug\wj\2020.4.5\ssl发送后第二次收不到回复*/
	openat_tls_soc_notify_ind_hdlr(s,evt, (err == 0) ? KAL_TRUE : KAL_FALSE);
}

#endif
/*+\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/
void platform_ssl_notify_ind(kal_int16 mod,
                                 kal_int8 s,
                                 openat_tls_event_enum event,
                                 kal_bool result,
                                 kal_int32 error,
                                 kal_int32 detail_cause)
{
	platform_tls_evt_ind_struct indEvt;

	indEvt.error = error;
	indEvt.event = event;
	indEvt.result = result;
	indEvt.sock_id = s;
	indEvt.detail_cause = detail_cause;

	OPENAT_send_message(g_s_tlsTask, PLATFORM_MSG_TLS, &indEvt, sizeof(indEvt));
}
/*-\BUG\lijiaodi\2020.8.12\解决连续收到2次recvind，第一次read error后close了，第二次read的时候死机*/

#ifdef LUA_SOCKET_SSL_SUPPORT
void platform_ssl_notify_process(
                                 kal_int8 s,
                                 openat_tls_event_enum event,
                                 kal_bool result,
                                 kal_int32 error,
                                 kal_int32 detail_cause)
{
	/*+\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/
	lua_socket_info_struct* sock = platform_socket_ctx_ext(s,PLATFORM_CONN_CONNECTED|PLATFORM_CONN_CONNECTING);
	int shakeRet = 0;
	if (!sock)
		return;
	/*-\Bug \LIJIAODI\2020.10.22 \OPENAT_socket_close后socketid就被回收，
但是platform层platform_socket_close后需等platform_socket_destory后才回收id，如果在close跟destory之间有新的socket，
必然存在socketid相同的多条socket_info记录，因此加了conn_status记录不同socket_info的status，通过platform_socket_ctx_ext传入id+status
获取对应的socket_info记录，destory的时候status必须是PLATFORM_CONN_CLOSE*/	
	OPENAT_print("%s sock= %d got event %d error %d result %d", __FUNCTION__, s,event,error,result);
    switch(event)
    {
        case OPENAT_TLS_HANDSHAKING:
            if (result)
			{
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{
                    shakeRet = openat_tls_handshake(sock->sock_id);
                    OPENAT_print("%s openat_tls_handshake ret %d", __func__, shakeRet);
                }
            }else
            {
            	platform_socket_connectCnf(sock, FALSE);
            	OPENAT_print("%s OPENAT_TLS_HANDSHAKING result false", __func__);
            }
            break;
        case OPENAT_TLS_HANDSHAKE_DONE:
            if (result)
			{
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{
			      	platform_socket_connectCnf(sock, TRUE);
             	}
            }
            else
			{
                if(LUA_INVALID_SOKCET_ID !=sock->sock_id)
				{
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
			sock->remoteCloseDelayTimerId = OPENAT_create_timerTask(platform_socket_remote_close_delay_timer_cb,s);
			if(sock->remoteCloseDelayTimerId)
	      	{
	          	if(OPENAT_start_timer(sock->remoteCloseDelayTimerId, DELAY_CLOSE_IND_TIME))
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
            break;
    }

    return;
}

#endif
/*+\wj\bug\2020.9.8\ssl重复释放导致出现死机，platform_socket_closeInd里面也有释放处理*/
#ifdef LUA_SOCKET_SSL_SUPPORT
void platform_ssl_close(UINT8 socket_index){
    u32 ret = 0;
	
	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	/*+\wj\new\2020.12.9\挂测死机，类型错误，导致下面删除的时候出问题*/
	int ctxID = sock->ssl_ctx_id;
	/*-\wj\new\2020.12.9\挂测死机，类型错误，导致下面删除的时候出问题*/
	OPENAT_print("%s index = %d,sock = %d", __FUNCTION__, socket_index,sock->sock_id);
	
	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
	{
		if (sock->sslRxQ.buf)
	  	{
	  		OPENAT_free(sock->sslRxQ.buf);
			sock->sslRxQ.buf = NULL;
	  	}
	  	sock->sslRxQ.size = 0;
	  	QueueClean(&sock->sslRxQ);

	    ret = openat_tls_delete_conn(sock->sock_id);
	    if (OPENAT_TLS_ERR_NONE != ret){
	        OPENAT_print("%s delete_conn fail ret=%d", __FUNCTION__,ret);
	    }

		/*+\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
		if(ctxID >=0)
		{
		    ret = openat_tls_delete_ctx(ctxID);
		    if (OPENAT_TLS_ERR_NONE != ret){
		        OPENAT_print("%s delete_ctx fail ret=%d", __FUNCTION__,ret);
		    }
		}
		/*-\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
		platform_free_cert(sock);
	}
}

#endif

#ifdef LUA_SOCKET_SSL_SUPPORT
int platform_ssl_create(lua_socket_info_struct* lua_socket_info)
{
	openat_tls_socaddr_struct faddr;
	/*+\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	int   ssl_ctx_id;
	/*-\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	int ret;


	/*+\bug3002\lijiaodi\2020.09.19\规避死机,如果buf不为空，先free再malloc */
	if(lua_socket_info->sslRxQ.buf)
	{
		OPENAT_free(lua_socket_info->sslRxQ.buf);
		lua_socket_info->sslRxQ.buf = NULL;
		//platform_assert(__FUNCTION__,__LINE__);
	}
	/*-\bug3002\lijiaodi\2020.09.19\规避死机,如果buf不为空，先free再malloc */
	{
		lua_socket_info->sslRxQ.buf = OPENAT_malloc(SSL_SOCKET_RX_BUF_SIZE);//buf;//这里的buf什么时候释放，这里是否考虑放到platform_ssl_create
		lua_socket_info->sslRxQ.size = SSL_SOCKET_RX_BUF_SIZE;
		QueueClean(&lua_socket_info->sslRxQ);
	}
	faddr.port = lua_socket_info->port;
	/*+\bug:3807\czm\2020.12.8\V25 FOTA+coretest挂测死机*/
	/*没有对长度赋值，长度是个随机值，在openat_tls_new_conn接口中使用到了该长度进行memcpy导致内存覆盖。*/
	faddr.addr_len = sizeof(lua_socket_info->sock_addr.s_addr);
	memcpy(faddr.addr, &lua_socket_info->sock_addr.s_addr, faddr.addr_len);
	/*-\bug:3807\czm\2020.12.8\V25 FOTA+coretest挂测死机*/
	OPENAT_Socket_set_cb(lua_socket_info->sock_id, platform_ssl_cb);
	ssl_ctx_id = openat_tls_new_ctx(OPENAT_TLS_ALL_VERSIONS,OPENAT_TLS_CLIENT_SIDE,0,0);
	lua_socket_info->ssl_ctx_id = ssl_ctx_id;

	/*+\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	if(ssl_ctx_id < 0)   return ssl_ctx_id;
	/*-\bug2727\lijiaodi\2020.07.31\当有大于3路ssl tcp时死机*/
	

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
#endif

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
	
	ret = OPENAT_socket_gethostbyname_ex(addr,&ipAddr,platform_dns_cb,lua_socket_info);

	if(ret !=0 && ret != -5)// 0 sucess   -5 INPROGRESS
	{
	  	OPENAT_print("%s gethostbyname error", __FUNCTION__);
      	return -2;
    }else
	{
		lua_socket_info->sock_id = OPENAT_socket_create(sock_type, platform_socket_cb);
		OPENAT_print("%s index = %d,sock = %d,type=%d,addr=%s,ret=%d", __FUNCTION__,socket_index,lua_socket_info->sock_id,sock_type,addr,ret);
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
	lua_socket_info->conn_status = PLATFORM_CONN_CONNECTING;

	if(ret == 0)
    {
		lua_socket_info->sock_addr.s_addr = ipAddr.addr;
		/*+\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
		platform_socket_conn(lua_socket_info);
		/*-\BUG\WJ\2020.6.12\解决ssl 域名解析异步时 SSL连接不上的问题*/
		return socket_index;
    }
    else if(-5 == ret)//阻塞
    {
      	return socket_index;
    }

}

void platform_socket_conn(lua_socket_info_struct* lua_socket_info)
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
		return;//return socket_index;
	}

	if(lua_socket_info->sock_type == SOC_SOCK_STREAM || lua_socket_info->sock_type == SOC_SOCK_STREAM_SSL)
	{
		ret = OPENAT_socket_connect(lua_socket_info->sock_id, &sockaddr, sizeof(sockaddr));
		error = OPENAT_socket_error(lua_socket_info->sock_id);

		OPENAT_print("%s socket connect sock_id=%d ret %d, err %d", __FUNCTION__, lua_socket_info->sock_id,ret, error);

		if(ret < 0 && error != OPENAT_SOCKET_EINPROGRESS && error != OPENAT_SOCKET_EWOULDBLOCK)
		{
			platform_socket_connectCnf(lua_socket_info, FALSE);
		  	return;//return socket_index;
		}
		else if(ret == 0)
		{
			#ifdef LUA_SOCKET_SSL_SUPPORT
			if(lua_socket_info->sock_type == SOC_SOCK_STREAM_SSL)
			{
				if(platform_ssl_create(lua_socket_info) != 0)
				{
		  			platform_socket_connectCnf(lua_socket_info, FALSE);
				}
				return;
			}
			#endif
		  	platform_socket_connectCnf(lua_socket_info, TRUE);
		}
	}
	else
	{
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
	
	UINT8 socket_index = create_conn_cnf->socket_index;
	BOOL success = create_conn_cnf->result;
	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	
	OPENAT_print("%s index = %d,sock = %d,result = %d",__FUNCTION__,socket_index,sock->sock_id,create_conn_cnf->result);
	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
	{
		if(sock != NULL && !success) //连接失败的CNF
		{
			platform_socket_close_without_cnf(socket_index);
		}
		if(sock->sock_type == SOC_SOCK_STREAM)
		{
			socketRemainBuf *sbuf = openatGetRemainBuf(socket_index);
			if(sbuf->sbufQueue.buf != NULL)
		    {
		        openatFreeRemainBuf(socket_index);
		    }
			/*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
			if(success)
			{
			    openatResetRemainBuf(socket_index);
			    openatInitRemainBuf(socket_index);
			}
			/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
		}
		if(success){
			sock->conn_status = PLATFORM_CONN_CONNECTED;
			sock->connected = TRUE;
			/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
		 	sock->reqLen = 0;
			sock->buffLen = 0;
			/*+\bug\wj\2020.12.8\接收消息过多导致luaTask消息队列满了*/
		}
	}
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
	OPENAT_print("%s index = %d,sock = %d",__FUNCTION__,sock_close_ind->socket_index,sock->sock_id);
    /*+\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
    platform_socket_stop_remote_close_delay_timer(sock);
    /*-\Bug 183\zhutianhua\2018.12.19 11:13\修正"Lua版本，socket短连接，如果服务器发送数据之后，立即主动关闭，终端会先收到close ind，后收到最后一包数据的recv ind"的问题*/
	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
	{
		platform_socket_close_without_cnf(sock_close_ind->socket_index);
		#if 0
		OPENAT_socket_close(sock->sock_id);
		sock->conn_status = PLATFORM_CONN_CLOSE;
		
		if(sock->sock_type == SOC_SOCK_STREAM)
		{
			socketRemainBuf *sbuf = openatGetRemainBuf(sock_close_ind->socket_index);
			if (sbuf->sbufQueue.buf != NULL)
			{
			    openatFreeRemainBuf(sock_close_ind->socket_index);
			}
		}
		if(sock->sock_type == SOC_SOCK_STREAM_SSL)
	    {
			platform_ssl_close(sock_close_ind->socket_index);
	    }
		sock->sock_id = LUA_INVALID_SOKCET_ID;
		sock->connected = FALSE;
		#endif
	}
    return 1;
}



kal_int32 platform_on_sock_close_cnf(lua_State *L,
                                           PlatformSocketCloseCnf* sock_close_cnf)
{
    kal_uint32 socket_index = sock_close_cnf->socket_index;
	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];
	
    lua_newtable(L);
    setfieldInt(L, "id", MSG_ID_TCPIP_SOCKET_CLOSE_CNF);
    setfieldInt(L, "socket_index", socket_index);
    setfieldInt(L, "result", !sock_close_cnf->result);
	
	OPENAT_print("%s index = %d,sock = %d",__FUNCTION__,socket_index,sock->sock_id);	
	
	if(sock->sock_id != LUA_INVALID_SOKCET_ID)
	{
		platform_socket_close_without_cnf(socket_index);
		#if 0
		OPENAT_socket_close(sock->sock_id);
		sock->conn_status = PLATFORM_CONN_CLOSE;	
		
		if(sock->sock_type == SOC_SOCK_STREAM)
		{
			socketRemainBuf *sbuf = openatGetRemainBuf(socket_index);
			/*+\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
			if (sbuf->sbufQueue.buf != NULL)
			{
		    	openatFreeRemainBuf(socket_index);
			}
			/*-\NEW\WJ\2018.11.30\添加TCP发送数据缓存,加快TCP发送速度*/
		}
		else if(sock->sock_type == SOC_SOCK_STREAM_SSL)
		{
			platform_ssl_close(socket_index);
		}

		sock->connected = FALSE;
		sock->sock_id = LUA_INVALID_SOKCET_ID;
		#endif
	}
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

/*+\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/
 kal_int32 platform_socket_setopt(kal_uint8 socket_index, int level, int optname, void *optval, int optlen)
 {
 	if(socket_index >=LUA_MAX_SOCKET_SUPPORT)
		return -1;
	
 	lua_socket_info_struct* sock = &lua_socket_context.socket_info[socket_index];

	if(sock->sock_id == LUA_INVALID_SOKCET_ID)
		return -1;
	
	 return OPENAT_socket_setsockopt(sock->sock_id, level, optname, optval, optlen);
 }
 /*-\bug3105\lijiaodi\2020.09.22 添加Socket Options参数设置接口,lua通过设置opt实现保活功能\*/


