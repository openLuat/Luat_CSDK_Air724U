#include <stdio.h>
#include <string.h>
#include "at_process.h"
#include "at_tok.h"
#include "iot_debug.h"
#include "iot_os.h"
#include "iot_vat.h"
#include "iot_network.h"

#define sms_print 	iot_debug_print
HANDLE demo_sms_task;
extern bool g_IsSMSRdy;

#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)

typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_NETWORK_MESSAGE;

typedef struct T_SMS_DATETIME_TAG
{
    UINT16 nYear;
    UINT8  nMonth;
    UINT8  nDay;
    UINT8  nHour;
    UINT8  nMin;
    UINT8  nSec;
    INT8  nTimeZone;
}T_SMS_DATETIME;

typedef struct T_SMS_INFO_TAG
{
	char smsStatus[10];
	char smsDa[18];
	char smsTime[32];
	char smsData[160];
	int smsDatalen;
}T_SMS_INFO;

void check_smsready(void)
{
	while(!g_IsSMSRdy)
	{
		iot_os_sleep(1000);
	}
}

static bool sms_init(void)
{
	/*¼ì²âµÈ´ýSMS READYÉÏ±¨*/
	check_smsready();

	int err;
  	ATResponse *p_response = NULL;
	
	err = at_send_command("AT+CMGF=1", &p_response);
	sms_print("[sms]CMGF error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CSMP=17,167,0,0", &p_response);
	sms_print("[sms]CSMP error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}

	err = at_send_command("AT+CNMI=2,1,0,0", &p_response);
	sms_print("[sms]CNMI error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}
	return TRUE;
error:
	at_response_free(p_response);
	return FALSE;
}

static bool send_sms(char *da, char *ud, int udlen)
{
	ATResponse *p_response = NULL;
    bool result = FALSE;
	char* out;
	int err;
	char cz = 0x1A;
	char cmd[64];

	sms_print("[sms] send_sms start");

	memset(cmd, 0, 64);
	sprintf(cmd, "AT+CMGS=%s",da);
	
    err = at_send_command_sms(cmd, ud, "+CMGS:", &p_response);
    if (err < 0 || p_response->success == 0)
    {
        sms_print("[sms]at_send_command_singleline error %d",__LINE__);
        goto error;
    }
	
	return TRUE;
error:
	sms_print("[sms] send_sms end");
	if(p_response!=NULL)
	{
		at_response_free(p_response);
		p_response=NULL;
	}  
	return result;
}

static bool read_sms(int index, T_SMS_INFO* smsInfo)
{
	int err;
  	ATResponse *p_response = NULL;
	char* out;
	char cmd[64];

	sms_print("[sms] send_sms start");

	memset(cmd, 0, 64);
	sprintf(cmd, "AT+CMGR=%d", index);

	//+CMGR: "REC READ","+86152****3962",,"2020/08/19,14:52:23+32"
	//1234567890abc
	err = at_send_command_singleline(cmd, "+CMGR:", &p_response);
	if (err < 0 || p_response->success == 0){
		goto error;
	}
	char* line = p_response->p_intermediates->line;  
	sms_print("[sms] read_sms 0 line: %s", line);
    err = at_tok_start(&line);
    if (err < 0)
        goto error;
	err = at_tok_nextstr(&line, &out);
    if (err < 0)
        goto error;
	strcpy(smsInfo->smsStatus, out);
	err = at_tok_nextstr(&line, &out);
    if (err < 0)
        goto error;
	strcpy(smsInfo->smsDa, out);
	sms_print("[sms] read_sms 1 line: %s", line);
	at_tok_nextstr(&line, &out);
	sms_print("[sms] read_sms 2 line: %s", line);
	err = at_tok_nextstr(&line, &out);
	if (err < 0)
		goto error;
	sms_print("[sms] read_sms 3 out: %s", out);
	strcpy(smsInfo->smsTime, out);

	line = p_response->p_intermediates->p_next->line; 
	strcpy(smsInfo->smsData, line);
	smsInfo->smsDatalen = strlen(line);
		
	return TRUE;
error:
	at_response_free(p_response);
	return FALSE;

}

static bool delete_sms(int index)
{
	int err;
  	ATResponse *p_response = NULL;
	char cmd[64];

	sms_print("[sms] send_sms start");

	memset(cmd, 0, 64);
	sprintf(cmd, "AT+CMGD=%d", index);
	err = at_send_command(cmd, &p_response);
	sms_print("[sms]CMGD error %d, success %d", err,(p_response?p_response->success:-1));
	if (err < 0 || p_response->success == 0){
		goto error;
	}
	
	return TRUE;
error:
	at_response_free(p_response);
	return FALSE;

}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    DEMO_NETWORK_MESSAGE* msgptr = iot_os_malloc(sizeof(DEMO_NETWORK_MESSAGE));
    sms_print("[sms]network ind state %d", state);
    if(state == OPENAT_NETWORK_LINKED)
    {
        msgptr->type = SOCKET_MSG_NETWORK_LINKED;
        iot_os_send_message(demo_sms_task, (PVOID)msgptr);
        return;
    }
    else if(state == OPENAT_NETWORK_READY)
    {
        msgptr->type = SOCKET_MSG_NETWORK_READY;
        iot_os_send_message(demo_sms_task,(PVOID)msgptr);
        return;
    }
    iot_os_free(msgptr);
}

static void demo_network_connetck(void)
{
    T_OPENAT_NETWORK_CONNECT networkparam;
    
    memset(&networkparam, 0, sizeof(T_OPENAT_NETWORK_CONNECT));
    memcpy(networkparam.apn, "CMNET", strlen("CMNET"));

    iot_network_connect(&networkparam);

}

static void demo_smstask(PVOID pParameter)
{
	DEMO_NETWORK_MESSAGE*    msg;
    sms_print("[sms]wait network ready....");
	bool err;
	T_SMS_INFO smsInfo;
	if(!(err = sms_init()))
	{
		sms_print("[sms]sms init faild");
		return;
	}
	
    while(1)
    {
        iot_os_wait_message(demo_sms_task, (PVOID)&msg);

        switch(msg->type)
        {
            case SOCKET_MSG_NETWORK_READY:
                sms_print("[sms]network connecting....");
                demo_network_connetck();
                break;
            case SOCKET_MSG_NETWORK_LINKED:
                sms_print("[sms]network connected");
				err = send_sms("10086", "10086", 13);
				sms_print("[sms]send sms err: %d", err);
				err = read_sms(1, &smsInfo);
				sms_print("[sms]read sms err: %d", err);
				sms_print("[sms]read sms info: %s, %s, %s", smsInfo.smsStatus, smsInfo.smsDa, smsInfo.smsTime);
				sms_print("[sms]read sms info: %s, %d", smsInfo.smsData, smsInfo.smsDatalen);
				err = delete_sms(1);
				sms_print("[sms]delete sms err: %d", err);
                break;
        }

        iot_os_free(msg);
    }
}

int appimg_enter(void *param)
{
    sms_print("[sms]appimg_enter");

	 //×¢²áÍøÂç×´Ì¬»Øµ÷º¯Êý
  	iot_network_set_cb(demo_networkIndCallBack);

	demo_sms_task = iot_os_create_task(demo_smstask, NULL, 2048, 5, OPENAT_OS_CREATE_DEFAULT, "sms");
    return 0;
}

void appimg_exit(void)
{
    sms_print("[sms]appimg_exit");
}
