#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "httpclient.h"
#include "am_openat_httpclient.h"


#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)
#define HTTP_URL "http://download.openluat.com:80/9501-xingli/brdcGPD.dat_rda"
#define HEAD_ACCEPT_KEY "Accept"
#define HEAD_ACCEPT_VALUE "*/*"
#define HEAD_ACCEPT_L_KEY "Accept-Language"
#define HEAD_ACCEPT_L_VALUE "cn"
#define HEAD_USER_KEY "User-Agent"
#define HEAD_USER_VALUE "*Mozilla/4.0"
#define HEAD_CONNECTION_KEY "Connection"
#define HEAD_CONNECTION_VALUE "Keep-Alive"

typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_NETWORK_MESSAGE;

static HANDLE g_s_http_task;

static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_NETWORK_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_NETWORK_MESSAGE));
    iot_debug_print("[http] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = SOCKET_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_http_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = SOCKET_MSG_NETWORK_READY;
        iot_os_send_message(g_s_http_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

void http_debug(const char * fun,const char* data,UINT32 len ,char * fmt, ...)
{
  va_list ap;
  char fmtString[128] = {0};
  UINT16 fmtStrlen;
  strcat(fmtString, "[http]--");
  strcat(fmtString, fun);
  strcat(fmtString, "--");

  fmtStrlen = strlen(fmtString);
  va_start (ap, fmt);
  fmtStrlen += vsnprintf(fmtString+fmtStrlen, sizeof(fmtString)-fmtStrlen, fmt, ap);
  va_end (ap);

  if(fmtStrlen != 0)
  {
      iot_debug_print("%s", fmtString);
  }
}



void http_test(void)
{
  HTTP_SESSION_HANDLE pHTTP;
  CHAR readBuff[1460];
  UINT32 readSize = 0;
  UINT32 readTotalLen = 0;
  CHAR token[32];
  UINT32 tokenSize=32;
  UINT32 nRetCode;

  pHTTP = HTTPClientOpenRequest(0);
  
  HTTPClientSetDebugHook(pHTTP, http_debug);

  if (HTTPClientSetVerb(pHTTP,VerbGet) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[http] HTTPClientSetVerb error");
    return;
  }

  if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_KEY, HEAD_ACCEPT_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
  {
    return;
  }
  if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_L_KEY, HEAD_ACCEPT_L_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
  {
    return;
  }
  if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_USER_KEY, HEAD_USER_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
  {
    return;
  }
  if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_CONNECTION_KEY, HEAD_CONNECTION_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
  {
    return;
  }
  iot_debug_print("[http] HTTPClientSendRequest enter");
  if (HTTPClientSendRequest(pHTTP,HTTP_URL, NULL, 0,TRUE,0,0) != HTTP_CLIENT_SUCCESS ) 
  {
    iot_debug_print("[http] HTTPClientSendRequest error");
    return;
  }
  iot_debug_print("[http] HTTPClientRecvResponse enter");

  if(HTTPClientRecvResponse(pHTTP,20000) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[http] HTTPClientRecvResponse error");
    return;
  }
  
  if((nRetCode = HTTPClientFindFirstHeader(pHTTP, "content-length", token, &tokenSize)) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[http] HTTPClientFindFirstHeader error");
    return;
  }
  else
  {
    iot_debug_print("[http] HTTPClientFindFirstHeader %d,%s", tokenSize, token);
  }
  HTTPClientFindCloseHeader(pHTTP);

  while(nRetCode == HTTP_CLIENT_SUCCESS || nRetCode != HTTP_CLIENT_EOS)
  {
      // Set the size of our buffer
      
      // Get the data
      nRetCode = HTTPClientReadData(pHTTP,readBuff,sizeof(readBuff),300,&readSize);

      readTotalLen += readSize;
      if(nRetCode != HTTP_CLIENT_SUCCESS || nRetCode == HTTP_CLIENT_EOS)
      {
        iot_debug_print("[http] HTTPClientReadData end nRetCode %d", nRetCode);
        break;
      }

      iot_debug_print("[http] HTTPClientReadData readTotalLen %d, %d, nRetCode %d", readTotalLen, readSize, nRetCode);
  }

  if(HTTPClientCloseRequest(&pHTTP) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[http] HTTPIntrnConnectionClose error");
    return;
  }
}

static void demo_http_task(PVOID pParameter)
{
    DEMO_NETWORK_MESSAGE*    msg;
    iot_debug_print("[http] wait network ready....");

    while(1)
    {
        iot_os_wait_message(g_s_http_task, (PVOID)&msg);

        switch(msg->type)
        {
            case SOCKET_MSG_NETWORK_READY:
                iot_debug_print("[http] network connecting....");
                demo_network_connetck();
                break;
            case SOCKET_MSG_NETWORK_LINKED:
                iot_debug_print("[http] network connected");
                http_test();
                break;
        }

        iot_os_free(msg);
    }
}

void demo_http_init(void)
{ 
  iot_debug_print("[http] demo_http_init");

  //×¢²áÍøÂç×´Ì¬»Øµ÷º¯Êý
  iot_network_set_cb(demo_networkIndCallBack);

  g_s_http_task = iot_os_create_task(demo_http_task,
                      NULL,
                      4096,
                      5,
                      OPENAT_OS_CREATE_DEFAULT,
                      "demo_http");
}

int appimg_enter(void *param)
{    
	iot_debug_print("[http] appimg_enter");
	iot_pmd_exit_deepsleep();
	demo_http_init();

	
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[http] appimg_exit");
}

