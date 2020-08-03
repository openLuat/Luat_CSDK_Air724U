#include <stdio.h>
#include "string.h"
#include "iot_debug.h"
#include "iot_audio.h"
#include "iot_fs.h"

#define recordstream_print iot_debug_print
HANDLE g_demo_timer1;
HANDLE g_demo_timer2;

#define FILE_NAME "record.amr"
#define DEMO_RECORD_CH OPENAT_AUDIOHAL_ITF_LOUDSPEAKER
#define DEMO_RECSTREAM_TIMER_TIMEOUT (5000) // 5000ms

BOOL demo_recordstream_fs_init(char* file)
{
    INT32 fd;

    fd = iot_fs_open_file(file, FS_O_RDONLY);

    if (fd >= 0) //FILE_NAME文件存在，就删除重新创建
    {
        iot_fs_delete_file(file);
    }
    
    // 创建文件FILE_NAME
    iot_fs_create_file(file);

    recordstream_print("[recordstream] create FILE_NAME");
    iot_fs_close_file(fd);

    return TRUE;
}

VOID demo_recordstream_fs_write(char* file, char *write_buff, INT32 len)
{
    INT32 fd;
    INT32 write_len = 0;
	
    fd = iot_fs_open_file(file, FS_O_RDWR);

    if (fd < 0)
        return;
	
    iot_fs_seek_file(fd, 0, FS_SEEK_END);
	
    write_len = iot_fs_write_file(fd, (UINT8 *)write_buff, len);

    if (write_len < 0)
        return;
    
    recordstream_print("[recordstream] write_len %d", write_len);

    iot_fs_close_file(fd);
}

static VOID demo_paly_handle(E_AMOPENAT_PLAY_ERROR result)
{
    recordstream_print("[recordstream] demo_paly_handle play end %d", result);
}

static VOID demo_time_handle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    T_AMOPENAT_PLAY_PARAM playParam;
    BOOL err;

    //6. 关闭录音
    err = iot_audio_rec_stop();
    recordstream_print("[recordstream] AUDREC stop BOOL %d", err);

    //7. 播放录音  
    playParam.playBuffer = FALSE;
    playParam.playFileParam.callback = demo_paly_handle;
    playParam.playFileParam.fileFormat = OPENAT_AUD_FORMAT_AMRNB;
    playParam.playFileParam.fileName = FILE_NAME;
    err = iot_audio_play_music(&playParam);

    recordstream_print("[recordstream] AUDRECStream play BOOL %d", err);

	
}

static VOID demo_audio_set_channel(VOID)
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

    recordstream_print("[recordstream] demo_audio_set_channel channel %d", DEMO_RECORD_CH);
}

static void demo_audio_rec_handle(E_AMOPENAT_RECORD_ERROR result)
{
    recordstream_print("[recordstream] demo_audio_rec_handle result: %d",result);
}

static void audio_stream_record_cb(int event,char* data,int len)
{
	recordstream_print("[recordstream]audio_stream_record_cb event: %d, len: %d", event, len);
	//将流录音数据写到文件FILE_NAME
	demo_recordstream_fs_write(FILE_NAME, data, len);	
}

static VOID demo_audRecStreamStart(VOID)
{
    BOOL err = FALSE;
	E_AMOPENAT_RECORD_PARAM param;
	
	param.record_mode = OPENAT_RECORD_STREAM;
	param.quality = (E_AMOPENAT_RECORD_QUALITY)1;
	param.type = (E_AMOPENAT_RECORD_TYPE)1;
	param.format = OPENAT_AUD_FORMAT_AMRNB;
	param.time_sec = 0;
	param.stream_record_cb = audio_stream_record_cb;
    err = iot_audio_rec_start(&param,demo_audio_rec_handle);

    recordstream_print("[recordstream] AUDRECStream start %d", err);
}

VOID demo_audRecStopTimer(VOID)
{
    // 定时5秒钟停止录音
    g_demo_timer2 = iot_os_create_timer((PTIMER_EXPFUNC)demo_time_handle, NULL);
    iot_os_start_timer(g_demo_timer2, DEMO_RECSTREAM_TIMER_TIMEOUT);
}


VOID demo_audio_init(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    // 1.设置通道和声音
    demo_audio_set_channel();

    //2.  开始录音
    demo_audRecStreamStart();

    //3. 设置定时关闭录音, 并播放录音
    demo_audRecStopTimer();
}

int appimg_enter(void *param)
{    
    recordstream_print("[recordstream] appimg_enter");
	
	iot_debug_set_fault_mode(OPENAT_FAULT_HANG);//设置debug模式

	//创建录音文件FILE_NAME
	demo_recordstream_fs_init(FILE_NAME);

	g_demo_timer1 = iot_os_create_timer((PTIMER_EXPFUNC)demo_audio_init, NULL);
    iot_os_start_timer(g_demo_timer1, DEMO_RECSTREAM_TIMER_TIMEOUT);
	
    return 0;
}

void appimg_exit(void)
{
    recordstream_print("[audio] appimg_exit");
}

