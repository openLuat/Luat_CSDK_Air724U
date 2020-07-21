/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_audio.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/10/21
 *
 * Description:
 *          panjun 2015.08.26 Audio API.
 **************************************************************************/

#ifndef __PLATFORM_AUDIO_H__
#define __PLATFORM_AUDIO_H__

#ifdef LUA_AUDIO_LIB

#include "type.h"

typedef enum PlatformAudioFormatTag
{
    PLATFORM_AUD_AMRNB,
	PLATFORM_AUD_AMRWB,
    PLATFORM_AUD_MP3,
    PLATFORM_AUD_PCM,
    PLATFORM_AUD_WAV,
	/*+\NEW\zhuwangbin\2020.05.15\增加speex格式的录音和播放*/
    PLATFORM_AUD_SPEEX,
	/*-\NEW\zhuwangbin\2020.05.15\增加speex格式的录音和播放*/
    //PLATFORM_AUD_MIDI,
    NumOfPlatformAudFormats
}PlatformAudioFormat;

/*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
typedef enum PlatformAudioChannelTag
{
	/*+\new\wj\2020.4.22\支持音频通道切换接口*/
    PLATFORM_AUD_CHANNEL_HANDSET,
    PLATFORM_AUD_CHANNEL_EARPIECE,
    PLATFORM_AUD_CHANNEL_LOUDSPEAKER,
    NumOfPlatformAudChannels
    #if 0
    PLATFORM_AUD_CHANNEL_BLUETOOTH,
    PLATFORM_AUD_CHANNEL_FM,
    PLATFORM_AUD_CHANNEL_FM_LP,
    PLATFORM_AUD_CHANNEL_TV,
    PLATFORM_AUD_CHANNEL_AUX_HANDSET,
    PLATFORM_AUD_CHANNEL_AUX_LOUDSPEAKER,
    PLATFORM_AUD_CHANNEL_AUX_EARPIECE,
    PLATFORM_AUD_CHANNEL_DUMMY_HANDSET,    
    PLATFORM_AUD_CHANNEL_DUMMY_AUX_HANDSET,
    PLATFORM_AUD_CHANNEL_DUMMY_LOUDSPEAKER,
    PLATFORM_AUD_CHANNEL_DUMMY_AUX_LOUDSPEAKER,
	#endif
	/*-\new\wj\2020.4.22\支持音频通道切换接口*/
}PlatformAudioChannel;

typedef enum PlatformAudioVolTag
{
    PLATFORM_AUD_VOL0,
    PLATFORM_AUD_VOL1,
    PLATFORM_AUD_VOL2,
    PLATFORM_AUD_VOL3,
    PLATFORM_AUD_VOL4,
    PLATFORM_AUD_VOL5,
    PLATFORM_AUD_VOL6,
    PLATFORM_AUD_VOL7,
    NumOfPlatformAudVols
}PlatformAudioVol;

typedef enum PlatformMicVolTag
{
    PLATFORM_MIC_VOL0,
    PLATFORM_MIC_VOL1,
    PLATFORM_MIC_VOL2,
    PLATFORM_MIC_VOL3,
    PLATFORM_MIC_VOL4,
    PLATFORM_MIC_VOL5,
    PLATFORM_MIC_VOL6,
    PLATFORM_MIC_VOL7,
    PLATFORM_MIC_VOL8,
    PLATFORM_MIC_VOL9,
    PLATFORM_MIC_VOL10,
    PLATFORM_MIC_VOL11,
    PLATFORM_MIC_VOL12,
    PLATFORM_MIC_VOL13,
    PLATFORM_MIC_VOL14,
    PLATFORM_MIC_VOL15,
    NumOfPlatformMicVols
}PlatformMicVol;

typedef enum PlatformAudioLoopbackTag
{
    PLATFORM_AUD_LOOPBACK_HANDSET,
    PLATFORM_AUD_LOOPBACK_EARPIECE,
    PLATFORM_AUD_LOOPBACK_LOUDSPEAKER,
    PLATFORM_AUD_LOOPBACK_AUX_HANDSET,
    PLATFORM_AUD_LOOPBACK_AUX_LOUDSPEAKER,
    NumOfPlatformAudLoopbacks
}PlatformAudioLoopback;
/*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/

/*+\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
typedef enum
{
    PLATFORM_SPKPA_TYPE_CLASSAB,
    PLATFORM_INPUT_TYPE_CLASSD,
    PLATFORM_INPUT_TYPE_CLASSK,
    PLATFORM_SPKPA_INPUT_TYPE_QTY = 0xFF000000
} PlatformSpkPaType;
/*-\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/

typedef struct AudioPlayParamTag
{
    BOOL isBuffer;
    union u_tag
    {
        struct
        {
            const u8 *data;
            u32 len;
            PlatformAudioFormat format;
            BOOL loop;
        }buffer;
        const char *filename;
    }u;
}AudioPlayParam;

int platform_audio_play(AudioPlayParam *param);

int platform_audio_stop(void);

/*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
int platform_audio_set_channel(PlatformAudioChannel channel);

int platform_audiod_set_vol(int vol);

int platform_audio_set_mic_vol(PlatformMicVol vol);

int platform_audio_set_loopback(BOOL flag, PlatformAudioLoopback typ, BOOL setvol, u32 vol);
/*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
/*+\new\wj\2020.4.26\实现录音接口*/
int platform_audio_record(char* file_name, int time_sec, int quality, int type, int format);
/*-\new\wj\2020.4.26\实现录音接口*/
int platform_audio_stop_record(void);

/*+\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
int platform_setpa(PlatformSpkPaType type);
int platform_getpa(void);
/*-\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/

#endif

#endif //__PLATFORM_AUDIO_H__

