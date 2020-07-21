/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:   platform_ttsply.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/03/26
 *
 * Description:
 *          TTSPLY API
 **************************************************************************/

#ifndef __PLATFORM_TTSPLY_H__
#define __PLATFORM_TTSPLY_H__

#include "type.h"

typedef struct tagTtsPlyParam
{
    s16    volume;
    s16    speed;
    s16    pitch;
    u16    codepage;
    u8      digit_mode;
    u8      punc_mode;
    u8      tag_mode;
    u8      wav_format;
    u8      eng_mode;
}ttsPlyParam;
/*+\new\wj\2019.12.27\添加TTS功能*/
typedef enum 
{
	PLATFORM_TTS_MSG_ERROR,	//TTS错误消息
	PLATFORM_TTS_MSG_STATUS	//TTS状态消息
}PLATFORM_TTS_MSG;

typedef struct tagTtsPly
{
    char *text;
    u32 text_size;
    u8 spk_vol;
}ttsPly;

int platform_ttsply_initEngine();
int platform_ttsply_setParam(u16 plyParam, u16 value);
//int platform_ttsply_getParam(u16 plyParam);
int platform_ttsply_play(ttsPly *param);
//int platform_ttsply_pause(void);
int platform_ttsply_stop(void);
/*-\new\wj\2019.12.27\添加TTS功能*/
#endif  //__PLATFORM_TTSPLY_H__

