#include <stdio.h>
#include "rtmp_sys.h"
#include "osi_pipe.h"
#include "am_openat.h"
#include "osi_api.h"
#include "log.h"

#define RTMP_PLAY_BUFF_MAX_LEN (4*1024)
#define RTMP_OK 1
#define RTMP_ERR 0
#define RTMP_AUDIO_TYPE_MP3 8

typedef enum{
	RTMP_PLAY_STOP,
	RTMP_PLAY_CLOSING,
	RTMP_PLAY_PLAYING,
}RTMP_PLAYER_STATE;

typedef enum{
	RTMP_COMMAND_PLAY,
}RTMP_COMMAND;

typedef enum{
	RTMP_PLAY_OK,
	RTMP_PLAY_ERR,
	RTMP_STOP_OK,
	RTMP_STOP_ERR,
	RTMP_RECEIVE_TIMEOUT,
	RTMP_CONNECT_ERR,
}RTMP_RESULT_CODE;
typedef void (*RTMP_PLAY_CALLBACK)(RTMP_RESULT_CODE code);


typedef struct{
	RTMP_PLAYER_STATE player_state;
	RTMP *rtmp;
	RTMP_PLAY_CALLBACK cb;
	osiWorkQueue_t *rp_wq;
	osiWork_t *rp_w;
	char *url;
}RTMP_PLAYER;

RTMP_PLAYER g_rtmp_play = {0};
E_OPENAT_NETWORK_STATE g_network_state = OPENAT_NETWORK_DISCONNECT; 
osiSemaphore_t *g_rtmp_sema = NULL;

static void prv_rtmp_play_delete(RTMP_PLAYER *rp)
{
	if(rp->url != NULL){
		free(rp->url);
		rp->url = NULL;
	}
	
	iot_audio_stop_music();

	if(rp->rp_wq){
		osiWorkQueueDelete(rp->rp_wq);
		rp->rp_wq = NULL;
	}
	if(rp->rp_w){
		osiWorkDelete(rp->rp_w); 
		rp->rp_w = NULL;
	}
	if(rp->rtmp)
	{
		RTMP_Free(rp->rtmp);
		rp->rtmp = NULL;
	}
	rp->player_state = RTMP_PLAY_STOP;
}


void prv_rtmp_audio_play_cb(E_AMOPENAT_PLAY_ERROR result)
{
	iot_debug_print("[rtmp] prv_rtmp_audio_play_cb result = %d",result);
}

void prv_rtmp_recv_work(void *param)
{
	RTMP_PLAYER *rp = (RTMP_PLAYER*)param;
	RTMPPacket pack = {0};
	int written;
	bool first_enter = true;
	long countbufsize=0;
	RTMP_PLAYER_STATE state = RTMP_CONNECT_ERR;
	//is live stream ?
	bool bLiveStream=true;
	int ret = 0;
	int playLen = 0;
	rp->rtmp = RTMP_Alloc();
	RTMP_Init(rp->rtmp);
	RTMP_Log(RTMP_LOGINFO,"RTMP_Init ok");  //rtmp://mp3.vipvoice.link:6001/live/testing
	
	//set connection timeout,default 30s
	rp->rtmp->Link.timeout=30*1000;

	if(!RTMP_SetupURL(rp->rtmp,rp->url))
	{
		RTMP_Log(RTMP_LOGERROR,"SetupURL Err\n");
		RTMP_Free(rp->rtmp);
		rp->rtmp = NULL;
		goto Finish;
	}
	
	if (bLiveStream){
		rp->rtmp->Link.lFlags|=RTMP_LF_LIVE;
	}
	
	//1hour
	RTMP_SetBufferMS(rp->rtmp, 3600*1000);		

	if(!RTMP_Connect(rp->rtmp,NULL)){
		RTMP_Log(RTMP_LOGERROR,"Connect Err\n");
		RTMP_Free(rp->rtmp);
		rp->rtmp = NULL;
	
	goto Finish;
	}
			
	if(!RTMP_ConnectStream(rp->rtmp,0)){
		RTMP_Log(RTMP_LOGERROR,"ConnectStream Err\n");
		RTMP_Close(rp->rtmp);
		RTMP_Free(rp->rtmp);
		rp->rtmp = NULL;
		goto Finish;
	}
	RTMP_Log(RTMP_LOGINFO,"ConnectStream OK\n");

	while(RTMP_ReadPacket(rp->rtmp,&pack))
	{
		if(pack.m_packetType == RTMP_AUDIO_TYPE_MP3)
		{
			//RTMP_LogHex(RTMP_LOGDEBUG,pack.m_body,pack.m_nBodySize);			
			if(first_enter)
			{
				iot_audio_set_speaker_vol(10);
				if(iot_audio_streamplay(OPENAT_AUD_FORMAT_MP3,prv_rtmp_audio_play_cb,pack.m_body+1,pack.m_nBodySize-1) == -1)
				{
					RTMPPacket_Free(&pack);	
					state = RTMP_PLAY_ERR;
					break;
				}
				if(rp->cb)
					rp->cb(RTMP_PLAY_OK);
				first_enter = false;
			}
			else
			{
				ret = 0;
				playLen = 0;
				while(playLen < pack.m_nBodySize-1)
				{
					ret = iot_audio_streamplay(OPENAT_AUD_FORMAT_MP3,prv_rtmp_audio_play_cb,pack.m_body+1+playLen,pack.m_nBodySize-1 - playLen);
					if(ret == -1)
						break;
					playLen += ret;
				}
				if( ret == -1){
					RTMPPacket_Free(&pack);	
					state = RTMP_PLAY_ERR;
					break;
				}
			}	
			countbufsize += (pack.m_nBodySize-1);
			RTMP_Log(RTMP_LOGDEBUG,"rtmp_recv_main Receive: %5dByte, Total: %dkB\n",pack.m_nBodySize,countbufsize/1024);
		}
		RTMPPacket_Free(&pack);
		memset(&pack,0,sizeof(RTMPPacket));

		if(rp->player_state == RTMP_PLAY_CLOSING){
			state = RTMP_STOP_OK;
			break;
		}
	}
	RTMP_Log(RTMP_LOGINFO,"prv_rtmp_recv_work finish");
	Finish:
	if(rp->rtmp != NULL){
		RTMP_Close(rp->rtmp);
	}
	prv_rtmp_play_delete(rp);
	if(rp->cb != NULL){
		rp->cb(state);
	}
}

bool prv_rtmp_play(const char* rtmp_url)
{
	RTMP_PLAYER *rp = &g_rtmp_play;

	if(rp->player_state != RTMP_PLAY_STOP){
		return false;
	}
	if(rtmp_url == NULL || strlen(rtmp_url) == 0){
		return false;
	}
	rp->url = (char *)calloc(1,strlen(rtmp_url)+1);
	if(rp->url == NULL){
		return false;
	}
	strcpy(rp->url,rtmp_url);

	rp->rp_wq = osiWorkQueueCreate("rtmp_play",1,OSI_PRIORITY_NORMAL,10*1024);
	if(rp->rp_wq == NULL){
		goto fail;
	}
	
	rp->rp_w = osiWorkCreate(prv_rtmp_recv_work, NULL, rp);
	if(rp->rp_w == NULL){
		osiWorkQueueDelete(rp->rp_wq);
		rp->rp_wq = NULL;
		goto fail;
	}
	
	if(!osiWorkEnqueue(rp->rp_w, rp->rp_wq)){
		goto fail;
	}
	
	rp->player_state = RTMP_PLAY_PLAYING;

	return true;
	fail:
	
	prv_rtmp_play_delete(rp);
	RTMP_Log(RTMP_LOGINFO,"quit rtmp .......");
	return false;

}

void rtmp_play_callback(RTMP_RESULT_CODE code)
{
	if(RTMP_PLAY_OK != code)
	{
		if(g_rtmp_sema)
			osiSemaphoreRelease(g_rtmp_sema);
	}
	iot_debug_print("[rtmp] rtmp_play_callback code = %d",code);	
}

bool rtmp_open_url(RTMP_PLAY_CALLBACK cb,const char* url)
{
	RTMP_PLAYER *rp = &g_rtmp_play;
	
	RTMP_LogSetCallback(iot_debug_print); //设置debug函数
	RTMP_LogSetLevel(RTMP_LOGDEBUG);	
	RTMP_Log(RTMP_LOGINFO,"rtmp_open_url state = %d,url = %s",rp->player_state,url);	
	
	if(rp->player_state != RTMP_PLAY_STOP || url == NULL) 
		return false;
	rp->cb = cb;
	
	return prv_rtmp_play(url); 
}

bool rtmp_close()
{
	RTMP_PLAYER *rp = &g_rtmp_play;
	RTMP_Log(RTMP_LOGINFO,"rtmp_close");
		
	if(rp->player_state == RTMP_PLAY_PLAYING){
		rp->player_state = RTMP_PLAY_CLOSING;
	}
	else
		return false;
	
	return true;
}

static void demo_networkIndCallBack(E_OPENAT_NETWORK_STATE state)
{
    iot_debug_print("[rtmp] network ind state %d", state);
    g_network_state = state;
}	

void demo_rtmp_task(void *param)
{
	while(1)
	{
		while(g_network_state != OPENAT_NETWORK_LINKED)	
		{
			iot_os_sleep(500); //等待PDP激活
		}
		g_rtmp_sema = osiSemaphoreCreate(1, 0);
		
		rtmp_open_url(rtmp_play_callback,"rtmp://cdsoft-jinshan.tpddns.cn/live_test/b");

		iot_os_sleep(20000);
		
		rtmp_close();

		if(g_rtmp_sema)
			osiSemaphoreAcquire(g_rtmp_sema); //等待关闭完成
		
		iot_os_sleep(5000);
	}
}

void demo_rtmp_init(void)
{ 
    iot_debug_print("[rtmp] demo_rtmp_init");
	
    //注册网络状态回调函数
    iot_network_set_cb(demo_networkIndCallBack);

    iot_os_create_task(demo_rtmp_task,
                        NULL,
                        20*1024,
                        5,
                        OPENAT_OS_CREATE_DEFAULT,
                        "demo_rtmp");
}

int appimg_enter(void *param)
{    
    iot_debug_print("[rtmp] appimg_enter");
	demo_rtmp_init();

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[rtmp] appimg_exit");
}


