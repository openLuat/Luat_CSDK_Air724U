#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "httpclient.h"
#include "iot_vat.h"
#include "am_openat_httpclient.h"


#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)
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
static u8 GSMLOC_IMEI[16] = {0};
extern gsmloc_cellinfo GSMLOC_CELL;

static AtCmdRsp AtCmdCb_wimei(u8* pRspStr)
{
	iot_debug_print("[gsmloc]AtCmdCb_wimei");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"+CME ERROR","+WIMEI:","OK"};
    s16  rspType = -1;
    u8* imei = GSMLOC_IMEI;
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				strncpy(imei,p+strlen(rspStrTable[i]),15);
				iot_debug_print("[gsmloc]imei: %s",imei);
            }
            break;
        }
    }
    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* +wimei */
		rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
        rspValue  = AT_RSP_CONTINUE;
        break;

        default:
        break;
    }
    return rspValue;
}
static u8 state = 0xf;
static AtCmdRsp AtCmdCb_EEMGINFO(u8* pRspStr)
{
	iot_debug_print("[gsmloc]AtCmdCb_EEMGINFO");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    u8 *rspStrTable[ ] = {"+CME ERROR","+EEMGINFO: ", "OK"};
    s16  rspType = -1;
	u8 type = 0xf;
    u8  i = 0;
    u8  *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				
				state = atoi(&p[strlen(rspStrTable[i])]);
				iot_debug_print("[gsmloc]state: %d",state);
            }
            break;
        }
    }
    switch (rspType)
    {
		case 0:  /* ERROR */
		rspValue = AT_RSP_ERROR;
		break;

		
		case 1:  /* +EEMLTESVC: */
			rspValue  = AT_RSP_WAIT;
        break;

		case 2:  /* OK */
		if( state == 3)
		{
			rspValue  = AT_RSP_CONTINUE;
		}
		else if(state == 0 || state == 1 || state == 2){
			rspValue  = AT_RSP_STEP;
		}
		break;

		default:
		break;
    }
    return rspValue;
}


static VOID gsmloc_SendATCmd(VOID)
{
	BOOL result = FALSE;
	AtCmdEntity atCmdInit[]={
		{"AT+WIMEI?"AT_CMD_END,11,AtCmdCb_wimei},
		{"AT+EEMOPT=1"AT_CMD_END,13,NULL},
		{AT_CMD_DELAY"1000",10,NULL},
		{"AT+EEMGINFO?"AT_CMD_END,14,AtCmdCb_EEMGINFO},
	};
	result = iot_vat_queue_fun_append(atCmdInit,sizeof(atCmdInit) / sizeof(atCmdInit[0]));
	iot_vat_SendCMD();
    return result;
}

static void get_gsmlocinfo(CHAR* http_url)
{
	CHAR* p = NULL;
	p = http_url;
	while(GSMLOC_CELL.Cellinfo[0].Mcc == 0)
	{
		iot_debug_print("[gsmloc]get GSMLOC_CELL");
		iot_os_sleep(2000);
	}
	p += sprintf(p,"http://bs.openluat.com/cps?cell=%03x,%d,%d,%d,%d",
		GSMLOC_CELL.Cellinfo[0].Mcc,
		GSMLOC_CELL.Cellinfo[0].Mnc,
		GSMLOC_CELL.Cellinfo[0].Lac,
		GSMLOC_CELL.Cellinfo[0].CellId, 
		GSMLOC_CELL.Cellinfo[0].rssi);

	iot_debug_print("[gsmloc] data = %s", http_url);
}

static void gsmloc_print(char* pData)
{
	int status = -1;
	char* p = NULL;
	p = pData;
	if(!strncmp(p, "status=", strlen("status=")))
	{
		p +=  strlen("status=");
		status = atoi(&p[0]);
	}
	p += 2;
	if(0 == status )
	{
		iot_debug_print("[gsmloc]info: %s", p);
	}
	else
	{
		iot_debug_print("[gsmloc]error: %d", status);
	}

	
}

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
    iot_debug_print("[gsmloc] network ind state %d", state);
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



void get_gsmloc(void)
{
  HTTP_SESSION_HANDLE pHTTP;
  CHAR readBuff[1460];
  UINT32 readSize = 0;
  UINT32 readTotalLen = 0;
  CHAR token[32];
  UINT32 tokenSize=32;
  UINT32 nRetCode;
  CHAR http_url[256] = {0};


  pHTTP = HTTPClientOpenRequest(0);
  
  HTTPClientSetDebugHook(pHTTP, http_debug);

  if (HTTPClientSetVerb(pHTTP,VerbGet) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[gsmloc] HTTPClientSetVerb error");
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

  get_gsmlocinfo(http_url);
  
  iot_debug_print("[gsmloc] HTTPClientSendRequest enter");
  if (HTTPClientSendRequest(pHTTP,http_url, NULL, 0,TRUE,0,0) != HTTP_CLIENT_SUCCESS ) 
  {
    iot_debug_print("[gsmloc] HTTPClientSendRequest error");
    return;
  }
  iot_debug_print("[gsmloc] HTTPClientRecvResponse enter");

  if(HTTPClientRecvResponse(pHTTP,20000) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[gsmloc] HTTPClientRecvResponse error");
    return;
  }
  
  if((nRetCode = HTTPClientFindFirstHeader(pHTTP, "content-length", token, &tokenSize)) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[gsmloc] HTTPClientFindFirstHeader error");
    return;
  }
  else
  {
    iot_debug_print("[gsmloc] HTTPClientFindFirstHeader %d,%s", tokenSize, token);
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
        iot_debug_print("[gsmloc] HTTPClientReadData end nRetCode %d", nRetCode);
		gsmloc_print(readBuff);
        break;
      }

      iot_debug_print("[gsmloc] HTTPClientReadData readTotalLen %d, %d, nRetCode %d", readTotalLen, readSize, nRetCode);
  }

  if(HTTPClientCloseRequest(&pHTTP) != HTTP_CLIENT_SUCCESS)
  {
    iot_debug_print("[gsmloc] HTTPIntrnConnectionClose error");
    return;
  }
}

static void demo_http_task(PVOID pParameter)
{
    DEMO_NETWORK_MESSAGE*    msg;
    iot_debug_print("[gsmloc] wait network ready....");

    while(1)
    {
        iot_os_wait_message(g_s_http_task, (PVOID)&msg);

        switch(msg->type)
        {
            case SOCKET_MSG_NETWORK_READY:
                iot_debug_print("[gsmloc] network connecting....");
                demo_network_connetck();
                break;
            case SOCKET_MSG_NETWORK_LINKED:
                iot_debug_print("[gsmloc] network connected");
				gsmloc_SendATCmd();
                get_gsmloc();
                break;
        }

        iot_os_free(msg);
    }
}

void demo_http_init(void)
{ 
  iot_debug_print("[gsmloc] demo_http_init");

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
    iot_os_sleep(400);
	iot_debug_print("[gsmloc] app_main");
	demo_http_init();
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[gsmloc] application image exit");
}


