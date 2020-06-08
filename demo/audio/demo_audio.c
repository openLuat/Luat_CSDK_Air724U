#include "string.h"
#include "iot_debug.h"
#include "iot_audio.h"

#define audio_print iot_debug_print
HANDLE g_demo_timer1;

#define FILE_NAME "record.amr"
#define DEMO_RECORD_CH OPENAT_AUDIOHAL_ITF_LOUDSPEAKER
#define DEMO_TIMER_TIMEOUT (5000) // 5000ms


VOID demo_paly_handle(E_AMOPENAT_PLAY_ERROR result)
{
    audio_print("[audio] demo_paly_handle play end %d", result);
}

VOID demo_time_handle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    T_AMOPENAT_PLAY_PARAM playParam;
    BOOL err;

    //6. 关闭录音
    err = iot_audio_rec_stop();
    audio_print("[audio] AUDREC stop BOOL %d", err);

    //7. 播放录音  
    playParam.playBuffer = FALSE;
    playParam.playFileParam.callback = demo_paly_handle;
    playParam.playFileParam.fileFormat = OPENAT_AUD_FORMAT_AMRNB;
    playParam.playFileParam.fileName = FILE_NAME;
    err = iot_audio_play_music(&playParam);
    
    audio_print("[audio] AUDREC play BOOL %d", err);
}

static void demo_audio_rec_handle(E_AMOPENAT_RECORD_ERROR result)
{
    audio_print("[audio] result: %d",result);
	iot_audio_rec_stop();
}

VOID demo_audio_set_channel(VOID)
{
    // 设置通道
    switch(DEMO_RECORD_CH)
    {
        case OPENAT_AUDIOHAL_ITF_EARPIECE:
            
            iot_audio_set_channel(OPENAT_AUDIOHAL_ITF_EARPIECE);
            break;

         case OPENAT_AUDIOHAL_ITF_LOUDSPEAKER:
         default:   
            iot_audio_set_channel(OPENAT_AUDIOHAL_ITF_LOUDSPEAKER);
            iot_audio_set_speaker_vol(60);
            break;   
    }

    audio_print("[audio] AUDREC channel %d", DEMO_RECORD_CH);
}


VOID demo_audRecStart(VOID)
{
    BOOL err = FALSE;
	E_AMOPENAT_RECORD_PARAM param;
	
	param.fileName = FILE_NAME;
	param.record_mode = OPENAT_RECORD_FILE;
	param.quality = OPENAT_RECORD_QUALITY_MEDIUM;
	param.type = OPENAT_RECORD_TYPE_MIC;
	param.format = OPENAT_AUD_FORMAT_AMRNB;
	param.time_sec = 10;
    err = iot_audio_rec_start(&param, demo_audio_rec_handle);

    audio_print("[audio] AUDREC start BOOL %d", err);
}

VOID demo_audRecStopTimer(VOID)
{
    // 定时5秒钟停止录音
    g_demo_timer1 = iot_os_create_timer(demo_time_handle, NULL);
    iot_os_start_timer(g_demo_timer1, DEMO_TIMER_TIMEOUT);
}


VOID demo_audio_init(VOID)
{
   
    // 1.设置通道和声音
    demo_audio_set_channel();

    //2.  开始录音
    demo_audRecStart();

    //3. 设置定时关闭录音, 并播放录音
    demo_audRecStopTimer();
}

int appimg_enter(void *param)
{    
    audio_print("[audio] appimg_enter");

    demo_audio_init();
    return 0;
}

void appimg_exit(void)
{
    audio_print("[audio] appimg_exit");
}

