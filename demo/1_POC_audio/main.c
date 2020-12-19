/***************
	demo_hello
****************/
#include <string.h>
#include "iot_debug.h"
#include "iot_os.h"

#include "am_openat_drv.h"
#include "iot_audio.h"

#include "iot_vat.h"

char audiobuf[1024 * 300] = {0};
uint32 recvbufsize = 0;

void audio_rec_handle(E_AMOPENAT_RECORD_ERROR result)
{
    iot_debug_print("[poc-audio] audio_rec_handle result:%d", result);
}

void audio_streamplay_handle(E_AMOPENAT_PLAY_ERROR result)
{
    iot_debug_print("[poc-audio] audio_streamplay_handle result:%d", result);
}

void audio_stream_record_cb(int ret, char *data, int len)
{
    //iot_debug_print("[poc-audio] audio_stream_record_cb ret:%d len:%d", ret, len);
    memcpy(audiobuf + recvbufsize, data, len); //流数据转存
    recvbufsize += len;
}

void poc_audio_stream_record()
{
    recvbufsize = 0;
    iot_debug_print("[poc-audio] poc_audio_stream_record start ");
    E_AMOPENAT_RECORD_PARAM param;
    param.record_mode = OPENAT_RECORD_STREAM; //使用流录音方式
    param.quality = OPENAT_RECORD_QUALITY_MEDIUM;
    param.type = OPENAT_RECORD_TYPE_POC;  //只有选择POC类型才具有消噪功能
    param.format = OPENAT_AUD_FORMAT_PCM; //poc消噪只能使用PCM格式
    param.time_sec = 0;
    param.stream_record_cb = audio_stream_record_cb; //流数据回调函数
    iot_audio_rec_start(&param, audio_rec_handle);
}

int appimg_enter(void *param)
{
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);

    //打开调试信息，默认关闭
    iot_vat_send_cmd((UINT8 *)"AT^TRACECTRL=0,1,1\r\n", sizeof("AT^TRACECTRL=0,1,1\r\n"));

    //GPIO15设置为高，启用外部功放
    T_AMOPENAT_GPIO_CFG output_cfg = {0};
    output_cfg.mode = OPENAT_GPIO_OUTPUT; //配置输出
    output_cfg.param.defaultState = 1;    // 默认高电平
    // gpio0初始化
    iot_gpio_open(15, &output_cfg);
    iot_audio_set_speaker_vol(90);                       //设置音量
    iot_audio_set_channel(OPENAT_AUDIOHAL_ITF_RECEIVER); //设置播放通道
    HANDLE g_demo_timer1 = iot_os_create_timer((PTIMER_EXPFUNC)poc_audio_stream_record, NULL);
    //将GPIO0设置为高电平
    while (1)
    {
        iot_gpio_set(15, 0); //录音时关闭功放，避免噪音
        // 1、1s后启动流录音
        iot_os_start_timer(g_demo_timer1, 1000);
        iot_os_sleep(5 * 1000); //5s后关闭录音
        //2、关闭录音
        iot_audio_rec_stop();
        //3、流播放录音
        iot_debug_print("[poc-audio] recvbufsize: %d", recvbufsize);
        iot_gpio_set(15, 1); //设置为高电平,打开外部功放
        int plen = 0;
        while (plen <= recvbufsize)
        {
            //消噪专用，流播放接口，PCM格式一次最多播放4096字节
            int len = iot_audio_streamplayV2(OPENAT_AUD_PLAY_TYPE_POC, OPENAT_AUD_FORMAT_PCM, audio_streamplay_handle, audiobuf + plen, recvbufsize / 15);
            plen += len;
            iot_debug_print("[poc-audio] iot_audio_streamplay len: %d plen:%d", len, plen);
            iot_os_sleep(280); //自己控制时间，流播放接口不是阻塞接口。
        }
        iot_debug_print("[poc-audio] iot_audio_streamplay over");
        //4、停止播放
        iot_audio_stop_music();
    }
    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[poc-audio] appimg_exit");
}
