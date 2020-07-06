#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iot_sys.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_socket.h"
#include "iot_network.h"
#include "iot_fs.h"
#include "iot_vat.h"
#include "httpclient.h"
#include "am_openat_httpclient.h"



#define DEMO_OTA_ASSERT(c) iot_debug_assert(c, (CHAR*)__func__, __LINE__)

#define DEMO_OTA_MSG_NETWORK_READY (0)
#define DEMO_OTA_MSG_NETWORK_LINKED (1)

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
}DEMO_OTA_MESSAGE;

static HANDLE g_s_ota_task;
static char g_imei[16] = {0};
static char g_version[6] = {0};

#define AM_FOTA_URL_LEN 	420
#define PRODUCT_KEY  		"T8epEmhBsH63HmmUwoEEE9n8aJoSwU7K"

static VOID demo_ota_getinfo(VOID);

static VOID demo_ota_url(char* url, int len)
{
	snprintf(url, len, "http://iot.openluat.com/api/site/firmware_upgrade?project_key=%s&imei=%s&firmware_name=http_CSDK_RDA8910&core_version=%s&dfota=1&version=1.0.0",
			PRODUCT_KEY,
			g_imei,
			g_version);
	iot_debug_print("[ota] demo_ota_url url: %s", url);
}


void http_debug(const char * fun,const char* data,UINT32 len ,char * fmt, ...)
{
  va_list ap;
  char fmtString[128] = {0};
  UINT16 fmtStrlen;
  strcat(fmtString, "[ota]--");
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



int demo_ota_httpdownload(void)
{
	HTTP_SESSION_HANDLE pHTTP;
	CHAR readBuff[512];
	UINT32 readSize = 0;
	UINT32 readTotalLen = 0;
	CHAR token[32] = {0};
	UINT32 tokenSize=32;
	UINT32 nRetCode;
	int rv = 1;
	int fsz = 0;
	char url[AM_FOTA_URL_LEN+1] = {0};
	demo_ota_url(url, AM_FOTA_URL_LEN);

	pHTTP = HTTPClientOpenRequest(0);

	HTTPClientSetDebugHook(pHTTP, http_debug);

	if (HTTPClientSetVerb(pHTTP,VerbGet) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[ota] HTTPClientSetVerb error");
		return 0;
	}

	if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_KEY, HEAD_ACCEPT_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return 0;
	}
	if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_ACCEPT_L_KEY, HEAD_ACCEPT_L_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return 0;
	}
	if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_USER_KEY, HEAD_USER_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return 0;
	}
	if((nRetCode = HTTPClientAddRequestHeaders(pHTTP, HEAD_CONNECTION_KEY, HEAD_CONNECTION_VALUE, TRUE)) != HTTP_CLIENT_SUCCESS)
	{
		return 0;
	}

	if (HTTPClientSendRequest(pHTTP, url, NULL, 0,TRUE,0,0) != HTTP_CLIENT_SUCCESS ) 
	{
		iot_debug_print("[ota] HTTPClientSendRequest error");
		return 0;
	}

	if(HTTPClientRecvResponse(pHTTP,20000) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[ota] HTTPClientRecvResponse error");
		return 0;
	}

	if((nRetCode = HTTPClientFindFirstHeader(pHTTP, "content-length", token, &tokenSize)) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[ota] HTTPClientFindFirstHeader error");
		return 0;
	}
	else
	{
		iot_debug_print("[ota] HTTPClientFindFirstHeader %d,%s", tokenSize, token);
	}
	
	if(strlen(token) > 0)
	{
		sscanf(token, "%*s %d", &fsz);
		iot_debug_print("[ota]GetSize fsz: %d",fsz);
	}
	else
	{
		iot_debug_print("[ota]GetSize faild");
		return 0; 
	}

	HTTPClientFindCloseHeader(pHTTP);
	while(nRetCode == HTTP_CLIENT_SUCCESS || nRetCode != HTTP_CLIENT_EOS)
	{
		// Set the size of our buffer

		// Get the data
		nRetCode = HTTPClientReadData(pHTTP,readBuff,sizeof(readBuff),300,&readSize);
		//升级错误码
		if(!strncmp("{\"code\":", readBuff, strlen("{\"code\":")))
		{
			int status = 0, cout = 0;
			char* buf = readBuff + strlen("{\"code\":");
			char str_p[10] = {0};
			while(buf++)
			{
				if(*buf == ',')
					break;
				str_p[cout] = *buf;
				cout++;
			}
			extern int atoi_p(char * str_p);
			status = atoi(str_p);
			iot_debug_print(">>>>>>>>>>>>Error code<<<<<<<<<<<<<<<<<");
			iot_debug_print("[ota]Error code: %d",status);
			iot_debug_print(">>>>>>>>>>>>Error code<<<<<<<<<<<<<<<<<");
			rv = 0;
			break;
		}
		readTotalLen += readSize;
		iot_debug_print("[ota] HTTPClientReadData readSize %d", readSize);
		iot_debug_print("[ota] HTTPClientReadData readTotalLen %d, %d", readTotalLen, fsz);
		//远程升级
		int ret = iot_fota_download(readBuff, readSize, fsz);
		if(ret != 0)
		{
			iot_debug_print("[http] iot_fota_download error");
			rv = 0;
			break;
		}		
		if(nRetCode != HTTP_CLIENT_SUCCESS || nRetCode == HTTP_CLIENT_EOS)
		{
			iot_debug_print("[ota] HTTPClientReadData end nRetCode %d", nRetCode);
			rv = 0;
			break;
		}
	}

	if(HTTPClientCloseRequest(&pHTTP) != HTTP_CLIENT_SUCCESS)
	{
		iot_debug_print("[ota] HTTPIntrnConnectionClose error");
		return 0;
	}
	return rv;
}



static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}


static void demo_ota_task(PVOID pParameter)
{
    DEMO_OTA_MESSAGE*    msg;
    iot_debug_print("[ota] wait network ready....");
    BOOL sock = FALSE;

    while(1)
    {
        iot_os_wait_message(g_s_ota_task, (PVOID)&msg);

        switch(msg->type)
        {
            case DEMO_OTA_MSG_NETWORK_READY:
                iot_debug_print("[ota] network connecting....");
			    demo_ota_getinfo();
                demo_network_connetck();
                break;
            case DEMO_OTA_MSG_NETWORK_LINKED:
                iot_debug_print("[ota] network connected");
                if(!sock)
                {
                    sock = TRUE;
					if(iot_fota_init() != 0)//fail
                    {
                        iot_debug_print("[ota] fota_init fail");
                        break;
                    }
                    iot_debug_print("[ota] fota_init suc,start demo_ota_download");
				  	if(demo_ota_httpdownload())
                    {
                        iot_debug_print("[ota] demo_ota_download fail");
                    }
					int r = iot_fota_done();
					if(r < 0)
						iot_debug_print("[ota]fota error %d",r);
					sock = FALSE;
					iot_debug_print("[ota]fota end %d",r);
                }
                break;
        }

        iot_os_free(msg);
    }
}


static void demo_otaworkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_OTA_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_OTA_MESSAGE));
    iot_debug_print("[ota] network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = DEMO_OTA_MSG_NETWORK_LINKED;
        iot_os_send_message(g_s_ota_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = DEMO_OTA_MSG_NETWORK_READY;
        iot_os_send_message(g_s_ota_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

static AtCmdRsp demo_ota_getimei(char *pRspStr)
{
	iot_debug_print("[ota]demo_ota_getimei");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    char *rspStrTable[ ] = {"+CME ERROR","+WIMEI: ", "OK"};
    s16  rspType = -1;
    char imei[16] = {0};
    u8  i = 0;
    char *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				strncpy(imei,p+strlen(rspStrTable[i]),15);
				strncpy(g_imei, imei, 15);
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
static AtCmdRsp demo_ota_getversion(char *pRspStr)
{
	iot_debug_print("[ota]demo_ota_getversion");
    AtCmdRsp  rspValue = AT_RSP_WAIT;
    char *rspStrTable[ ] = {"ERROR","CSDK", "OK"};
    s16  rspType = -1;
    char version[6] = {0};
    u8  i = 0, cout = 0, num = 0;
    char *p = pRspStr + 2;
    for (i = 0; i < sizeof(rspStrTable) / sizeof(rspStrTable[0]); i++)
    {
        if (!strncmp(rspStrTable[i], p, strlen(rspStrTable[i])))
        {
            rspType = i;
            if (rspType == 1){
				while(p++)
				{
					if(*p == '_')
					{
						cout++;
						p++;
					}
					if(*p == 'V')
					{
						p++;
					}
					if(cout == 2)
						break;
					if(cout == 1)
					{
						version[num++] = *p;
					}
				}
				strncpy(g_version, (version), (strlen(version)));
				iot_debug_print("[ota]demo_ota_getversion g_version: %s", g_version);
            }
            break;
        }
    }
    switch (rspType)
    {
        case 0:  /* ERROR */
        rspValue = AT_RSP_ERROR;
        break;

        case 1:  /* version */
        	rspValue  = AT_RSP_WAIT;
        break;
		
		case 2: /* OK */
			rspValue  = AT_RSP_CONTINUE;
		break;

        default:
        break;
    }
    return rspValue;
}


static VOID demo_ota_getinfo(VOID)
{
	AtCmdEntity atCmdInit[]={
		{AT_CMD_DELAY"2000",10,NULL},
		{"AT+VER"AT_CMD_END, 8, demo_ota_getversion},
		{"AT+WIMEI?"AT_CMD_END,11,demo_ota_getimei},
	};
	iot_vat_push_cmd(atCmdInit,sizeof(atCmdInit) / sizeof(atCmdInit[0]));
}

int appimg_enter(void *param)
{    
    //开机立刻使用文件系统，会看不到打印信息
    iot_debug_print("[ota] appimg_enter");
	iot_os_sleep(2000);
    //注册网络状态回调函数
    iot_network_set_cb(demo_otaworkIndCallBack);
    g_s_ota_task = iot_os_create_task(demo_ota_task,
                        NULL,
                        10*1024,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_ota");


    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[ota] appimg_exit");
}


