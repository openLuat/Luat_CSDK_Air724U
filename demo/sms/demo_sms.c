#include <stdio.h>
#include <string.h>
#include "iot_debug.h"
#include "iot_os.h"
#include "iot_network.h"
#include "am_openat_sms.h"

#define sms_print 	iot_debug_print
#define SOCKET_MSG_NETWORK_READY (0)
#define SOCKET_MSG_NETWORK_LINKED (1)

typedef struct {
    UINT8 type;
    UINT8 data;
}DEMO_NETWORK_MESSAGE;

HANDLE demo_sms_task;

void demo_unsolSMSHandler(int status, char* data, char* num, char * datetime)
{
	OPENAT_lua_print("[sms]read sms status: %d, data: %s, num: %s, datetime: %s", status, data, num, datetime);
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

    OPENAT_send_at_command("AT+SETVOLTE=1\r", strlen("AT+SETVOLTE=1\r"));
    iot_os_sleep(10000);

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
				err = sms_init();
				if(err)
					err = sms_send("10086","10086");
				sms_print("[sms]send sms err: %d", err);
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
	sms_setsmscb(demo_unsolSMSHandler);

	demo_sms_task = iot_os_create_task(demo_smstask, NULL, 1024*8, 32, OPENAT_OS_CREATE_DEFAULT, "sms");
    return 0;
}

void appimg_exit(void)
{
    sms_print("[sms]appimg_exit");
}
