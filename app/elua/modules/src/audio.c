/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    audio.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/10/21
 *
 * Description:
 *          audio.core
 *
 * History:
 *     panjun 2015.04.30 Add audio's API according to MTK.
 **************************************************************************/

#ifdef LUA_AUDIO_LIB

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_audio.h"


static PlatformAudioFormat l_getFileFormat(const char *filename)
{
	if(strstr(filename,".amr") || strstr(filename,".AMR"))
    {
       return PLATFORM_AUD_AMRNB;
    } 
    else if(strstr(filename,".mp3") || strstr(filename,".MP3"))
    {
        return PLATFORM_AUD_MP3;
    }
	else if(strstr(filename,".pcm") || strstr(filename,".PCM"))
    {
        return PLATFORM_AUD_PCM;
    }else
    {
    	return NumOfPlatformAudFormats;
    }
}
static int l_audio_play_file(lua_State *L) {
    const char *name = luaL_checkstring(L,1);
	/*+\NEW\lijiaodi\2020.4.09\通过播buffer的方式播放*/
    AudioPlayParam param;
	PlatformAudioFormat format;
	/*+\bug\wj\2020.5.27\播放不了大文件问题*/
	u8 * playBuffer;
	FILE *file;
	int len = 0;
    format = l_getFileFormat(name);
	if(format == NumOfPlatformAudFormats)
	{
		printf("l_audio_play_file:not supported format!");
		return PLATFORM_ERR;
	}
	file = fopen_ext(name, "rb");
	if(file!= NULL)
	{
	
	    if(!file)
	    {
	        printf("l_audio_play_file:open file failed!");
	        return PLATFORM_ERR;
	    }

	    fseek(file, 0, SEEK_END);
	    len = ftell(file);

		fseek(file, 0, SEEK_SET);

		playBuffer = platform_malloc(len);

		 if(playBuffer == NULL)
	    {
	        printf("l_audio_play_file:not enough memory");
	        fclose(file);
	        return PLATFORM_ERR;
	    }
	    
	    if(fread(playBuffer, 1, len, file) != len)
	    {
	        printf("[l_audio_play_file]: read file error!\n");
	        platform_free(playBuffer);
	        fclose(file);
	        return PLATFORM_ERR;
	    }
	    
	    fclose(file);
		param.isBuffer = TRUE;
	    param.u.buffer.data = playBuffer;
	    param.u.buffer.len = len;
	    param.u.buffer.format = format;
	    param.u.buffer.loop = FALSE;
	}
	else
	{
	    param.isBuffer = FALSE;
	    param.u.filename = name;
	    //param.u.buffer.len = len;
	    param.u.buffer.format = format;
	    param.u.buffer.loop = FALSE;
	}
    lua_pushboolean(L, platform_audio_play(&param) == PLATFORM_OK);
	/*-\bug\wj\2020.5.27\播放不了大文件问题*/
	if(param.isBuffer)
		platform_free(playBuffer);
	/*-\NEW\lijiaodi\2020.4.09\通过播buffer的方式播放*/
    return 1;
}

static int l_audio_play_data(lua_State *L) {
    const char *data;
    int l;
    AudioPlayParam param;
    
    data = luaL_checklstring(L, 1, &l);
    param.isBuffer = TRUE;
    param.u.buffer.format = luaL_checkinteger(L, 2);
    param.u.buffer.loop = luaL_optinteger(L, 3, 0);
    param.u.buffer.data = data;
    param.u.buffer.len = l;
    
    lua_pushboolean(L, platform_audio_play(&param) == PLATFORM_OK);
    return 1;
}

static int l_audio_stop(lua_State *L) {
    platform_audio_stop();
    return 0;
}

/*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
static int l_audio_set_channel(lua_State *L) {
    u32 channel = luaL_checkinteger(L,1);
    u32 res;
    
    res = platform_audio_set_channel(channel);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_vol(lua_State *L) {
    u32 vol = luaL_checkinteger(L,1);
    u32 res;

/*+\NEW\xiongjunqun\2015.05.28\修改无法切换到听筒模式问题*/	
    //platform_audio_set_channel(PLATFORM_AUD_CHANNEL_LOUDSPEAKER);
/*-\NEW\xiongjunqun\2015.05.28\修改无法切换到听筒模式问题*/
    res = platform_audio_set_vol(vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*+\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/
static int l_audio_set_sph_vol(lua_State *L) {
    u32 vol = luaL_checkinteger(L,1);
    u32 res;

    res = platform_audio_set_sph_vol(vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*-\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/

static int l_audio_speaker_set_vol(lua_State *L) {
    u32 vol = luaL_checkinteger(L,1);
    u32 res;

    platform_audio_set_channel(PLATFORM_AUD_CHANNEL_LOUDSPEAKER);
    res = platform_audio_set_vol(vol);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_mic_vol(lua_State *L) {
    u32 vol = luaL_checkinteger(L,1);
    u32 res;
    
    res = platform_audio_set_mic_vol(vol);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_set_loopback(lua_State *L) {
    u32 flag = luaL_checkinteger(L,1);
    u32 typ = luaL_checkinteger(L,2);
    u32 setvol = luaL_checkinteger(L,3);
    u32 vol = luaL_checkinteger(L,4);
    u32 res;
    
    res = platform_audio_set_loopback(flag,typ,setvol,vol);
    lua_pushinteger(L, res);
    
    return 1;
}
/*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/

static int l_audio_record(lua_State *L) {
    size_t  len      = 0;
    char* file_name      = (char*)luaL_checklstring(L, 1, &len);
    int     time_sec        = luaL_checkinteger(L, 2);
    int    quality        =  luaL_optint(L, 3, 0);
	int 	type 		= luaL_optint(L, 4, 0);
	int 	format 		= luaL_optint(L, 5, 0);
    int res = platform_audio_record(file_name,time_sec,quality,type,format);
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_stream_record(lua_State *L) {

    int     time_sec        = luaL_checkinteger(L, 1);
    int    quality        =  luaL_optint(L, 2, 0);
	int 	type 		= luaL_optint(L, 3, 0);
	int 	format 		= luaL_optint(L, 4, 0);
	/*+\bug2241\zhuwangbin\2020.6.20\流录音可配置回调长度阀值*/
	int 	length 		= luaL_optint(L, 5, 0);
    int res = platform_audio_stream_record(time_sec,quality,type,format, length);
	/*-\bug2241\zhuwangbin\2020.6.20\流录音可配置回调长度阀值*/
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_stop_record(lua_State *L) {
    int res = platform_audio_stop_record();
    lua_pushinteger(L, res);
    
    return 1;
}

static int l_audio_delete_record(lua_State *L){
	int res = platform_audio_delete_record();
	lua_pushinteger(L, res);
    
    return 1;

}

static int l_audio_stream_record_recv(lua_State *L) {

    int total_len = 0; 
    luaL_Buffer b;
    int count ;
    int ret;
    char* buf;
    int recved_len = 0;
    
    total_len      = luaL_checkinteger(L, 1);
    count          = total_len / LUAL_BUFFERSIZE;
    
    luaL_buffinit( L, &b );

    while(count-- > 0)
    {
      buf = luaL_prepbuffer(&b);
      ret = platform_audio_read_record_buf(buf, LUAL_BUFFERSIZE);
      if(ret > 0)
      {
        recved_len += ret;
        luaL_addsize(&b, ret);
      }
      else
      { 
        break;
      }
    }

    if(total_len % LUAL_BUFFERSIZE)
    {
        buf = luaL_prepbuffer(&b);
        ret = platform_audio_read_record_buf(buf, total_len % LUAL_BUFFERSIZE);
        if(ret > 0)
        {
          recved_len += ret;
          luaL_addsize(&b, ret);
        }
    }
    
    luaL_pushresult( &b );
    
    return 1;
} 


static int l_audio_streamplay(lua_State *L) {
    PlatformAudioFormat format;
    char *data;
	int len;

	format = luaL_checkinteger(L, 1);
    data = luaL_checklstring(L, 2, &len);
    
    lua_pushinteger(L, platform_audio_streamplay(format,data,len));
    return 1;
}
/*+\bug\wj\2020.5.14\流播放问题PCM无上报，流式播放是同步阻塞接口不合适*/
static int l_audio_get_stream_remain_dataLen(lua_State *L)
{
	lua_pushinteger(L, platform_audio_getStreamRemainDataLen());
    return 1;
}
/*-\bug\wj\2020.5.14\流播放问题PCM无上报，流式播放是同步阻塞接口不合适*/
/*+\new\wj\2020.5.29\通话前可以播放音频，接通后还可以正常通话*/
static int l_audio_start_voice(lua_State *L)
{
	lua_pushboolean(L, platform_audio_start_voice());
	return 1;
}
/*-\new\wj\2020.5.29\通话前可以播放音频，接通后还可以正常通话*/

/*+\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
static int l_audio_setpa(lua_State *L) {
    PlatformSpkPaType type;

	type = luaL_checkinteger(L, 1);
    
    lua_pushinteger(L, platform_setpa(type));
    return 1;
}

static int l_audio_getpa(lua_State *L) {
   
    lua_pushinteger(L, platform_getpa());
    return 1;
}
/*-\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE audiocore_map[] =
{ 
  { LSTRKEY( "play" ),  LFUNCVAL( l_audio_play_file ) },
  /*+\bug\wj\2020.5.14\流播放问题PCM无上报，流式播放是同步阻塞接口不合适*/
  { LSTRKEY( "streamremain" ),  LFUNCVAL( l_audio_get_stream_remain_dataLen ) },
  /*-\bug\wj\2020.5.14\流播放问题PCM无上报，流式播放是同步阻塞接口不合适*/
  { LSTRKEY( "playdata" ),  LFUNCVAL( l_audio_play_data ) },
  { LSTRKEY( "streamplay" ),  LFUNCVAL( l_audio_streamplay ) },
  { LSTRKEY( "stop" ),  LFUNCVAL( l_audio_stop ) },
  /*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
  { LSTRKEY( "setchannel" ),  LFUNCVAL( l_audio_set_channel ) },
  { LSTRKEY( "setvol" ),  LFUNCVAL( l_audio_set_vol ) },
/*+\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/  
  { LSTRKEY( "setsphvol" ),  LFUNCVAL( l_audio_set_sph_vol ) },
/*-\NEW\xiongjunqun\2015.05.28\增加通话中调节音量接口*/  
  { LSTRKEY( "setspeakervol" ),  LFUNCVAL( l_audio_speaker_set_vol ) },
  { LSTRKEY( "setmicvol" ),  LFUNCVAL( l_audio_set_mic_vol ) },
  { LSTRKEY( "setloopback" ),  LFUNCVAL( l_audio_set_loopback ) },
  /*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
  { LSTRKEY( "record" ),  LFUNCVAL( l_audio_record) },
  {	LSTRKEY( "streamrecord" ),  LFUNCVAL(l_audio_stream_record)},
  { LSTRKEY( "stoprecord" ),  LFUNCVAL( l_audio_stop_record)},
  { LSTRKEY( "deleterecord" ),  LFUNCVAL( l_audio_delete_record) },
  { LSTRKEY( "streamrecordread" ),  LFUNCVAL( l_audio_stream_record_recv)},
  /*+\new\wj\2020.5.29\通话前可以播放音频，接通后还可以正常通话*/
  { LSTRKEY( "startvoice" ),  LFUNCVAL(l_audio_start_voice)},
  /*+\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
  { LSTRKEY( "setpa" ),  LFUNCVAL(l_audio_setpa)},
   { LSTRKEY( "getpa" ),  LFUNCVAL(l_audio_getpa)},
   /*-\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
  /*-\new\wj\2020.5.29\通话前可以播放音频，接通后还可以正常通话*/
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_audiocore( lua_State *L )
{
    luaL_register( L, AUXLIB_AUDIOCORE, audiocore_map );

    MOD_REG_NUMBER(L, "AMR", PLATFORM_AUD_AMRNB);
    MOD_REG_NUMBER(L, "MP3", PLATFORM_AUD_MP3);
    MOD_REG_NUMBER(L, "PCM", PLATFORM_AUD_PCM);
	/*+\NEW\zhuwangbin\2020.05.15\增加speex格式的录音和播放*/
	MOD_REG_NUMBER(L, "SPX", PLATFORM_AUD_SPEEX);
	/*-\NEW\zhuwangbin\2020.05.15\增加speex格式的录音和播放*/
    MOD_REG_NUMBER(L, "WAV", PLATFORM_AUD_WAV);
    //MOD_REG_NUMBER(L, "MIDI", PLATFORM_AUD_MIDI);
	/*+\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
	MOD_REG_NUMBER(L, "CLASS_AB", PLATFORM_SPKPA_TYPE_CLASSAB);
	MOD_REG_NUMBER(L, "CLASS_D", PLATFORM_INPUT_TYPE_CLASSD);
	/*-\new\zhuwangbin\2020.6.2\添加音频功放类型设置接口*/
    /*+\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
    #define REG_AUD_CHANNEL(CHANNEL) MOD_REG_NUMBER(L, #CHANNEL, PLATFORM_AUD_CHANNEL_##CHANNEL)
    REG_AUD_CHANNEL(HANDSET);
    REG_AUD_CHANNEL(EARPIECE);
    REG_AUD_CHANNEL(LOUDSPEAKER);
	/*+\new\wj\2020.4.22\支持音频通道切换接口*/
	#if 0
    REG_AUD_CHANNEL(BLUETOOTH);
    REG_AUD_CHANNEL(FM);
    REG_AUD_CHANNEL(FM_LP);
    REG_AUD_CHANNEL(TV);
    REG_AUD_CHANNEL(AUX_HANDSET);
    REG_AUD_CHANNEL(AUX_LOUDSPEAKER);
    REG_AUD_CHANNEL(AUX_EARPIECE);
    REG_AUD_CHANNEL(DUMMY_HANDSET);
    REG_AUD_CHANNEL(DUMMY_AUX_HANDSET);
    REG_AUD_CHANNEL(DUMMY_LOUDSPEAKER);    
    REG_AUD_CHANNEL(DUMMY_AUX_LOUDSPEAKER);
	#endif
	/*-\new\wj\2020.4.22\支持音频通道切换接口*/
    #define REG_AUD_VOL(VOL) MOD_REG_NUMBER(L, #VOL, PLATFORM_AUD_##VOL)
    REG_AUD_VOL(VOL0);
    REG_AUD_VOL(VOL1);
    REG_AUD_VOL(VOL2);
    REG_AUD_VOL(VOL3);
    REG_AUD_VOL(VOL4);
    REG_AUD_VOL(VOL5);
    REG_AUD_VOL(VOL6);
    REG_AUD_VOL(VOL7);   

    #define REG_MIC_VOL(VOL) MOD_REG_NUMBER(L, #VOL, PLATFORM_##VOL)
    REG_MIC_VOL(MIC_VOL0);
    REG_MIC_VOL(MIC_VOL1);
    REG_MIC_VOL(MIC_VOL2);
    REG_MIC_VOL(MIC_VOL3);
    REG_MIC_VOL(MIC_VOL4);
    REG_MIC_VOL(MIC_VOL5);
    REG_MIC_VOL(MIC_VOL6);
    REG_MIC_VOL(MIC_VOL7); 
    REG_MIC_VOL(MIC_VOL8);
    REG_MIC_VOL(MIC_VOL9);
    REG_MIC_VOL(MIC_VOL10);
    REG_MIC_VOL(MIC_VOL11);
    REG_MIC_VOL(MIC_VOL12);
    REG_MIC_VOL(MIC_VOL13);
    REG_MIC_VOL(MIC_VOL14);
    REG_MIC_VOL(MIC_VOL15); 

    #define REG_AUD_LOOPBACK(TYPE) MOD_REG_NUMBER(L, #TYPE, PLATFORM_AUD_##TYPE)
    REG_AUD_LOOPBACK(LOOPBACK_HANDSET);
    REG_AUD_LOOPBACK(LOOPBACK_EARPIECE);
    REG_AUD_LOOPBACK(LOOPBACK_LOUDSPEAKER);
	/*+\new\wj\2020.4.22\支持音频通道切换接口*/
	#if 0
    REG_AUD_LOOPBACK(LOOPBACK_AUX_HANDSET);
    REG_AUD_LOOPBACK(LOOPBACK_AUX_LOUDSPEAKER);
	#endif
	/*-\new\wj\2020.4.22\支持音频通道切换接口*/
    /*-\NEW\zhuth\2014.7.25\新增设置音频通道和音量的同步接口*/
	
    return 1;
}  
#endif
