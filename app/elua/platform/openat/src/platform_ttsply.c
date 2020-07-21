/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_ttsply.c
 * Author:  panjun 
 * Version: V0.1
 * Date:    2016/03/17
 *
 * Description:
 *          TTS Play API
 * History:
 *     panjun 2016.03.17 Create some APIs of TTS play.
 **************************************************************************/

/*+\new\wj\2019.12.27\添加TTS功能*/
#include "string.h"
//#include "FileMgrSrvGProt.h"

#include "lplatform.h"
#include "platform_malloc.h"
#include "am_openat.h"
#include "platform_ttsply.h"
#if 0
int platform_ttsply_init(ttsPlyParam *param)
{
   return IVTBL(ttsply_init)((void*)param);
}
#endif
void platform_ttsply_push_error(int ttsError);
void platform_ttsply_push_status(int ttsStatus);


void platform_ttsplay_callback(u8 msg_id,u32 event)
{
	OPENAT_print("platform_ttsplay_callback msg_id = %d,event = %d",event);
	if(msg_id == 0)
	{
		platform_ttsply_push_error(event);	
	}
	else if(msg_id == 1)
	{
		platform_ttsply_push_status(event);
	}
}

int platform_ttsply_initEngine()
{
	
   return IVTBL(tts_init)(platform_ttsplay_callback);
}

void platform_ttsply_push_status(int ttsStatus)
{
   PlatformMsgData rtosmsg;
   rtosmsg.ttsPlyData.ttsPlyStatusInd = ttsStatus;
   platform_rtos_send(MSG_ID_RTOS_TTSPLY_STATUS, &rtosmsg);
}

void platform_ttsply_push_error(int ttsError)
{
   PlatformMsgData rtosmsg;

   rtosmsg.ttsPlyData.ttsPlyErrorInd = ttsError;
   platform_rtos_send(MSG_ID_RTOS_TTSPLY_ERROR, &rtosmsg);
}


int platform_ttsply_setParam(u16 playParam, u16 value)
{
   return IVTBL(tts_set_param)(playParam, value);
}

#if 0
int platform_ttsply_getParam(u16 playParam)
{
   return IVTBL(ttsply_getParam)(playParam);
}
#endif

int platform_ttsply_play(ttsPly *param)
{
	u32 len;
	len = strlen(param->text);
    return IVTBL(tts_play)(param->text,len);
}
#if 0
int platform_ttsply_pause(void)
{
    IVTBL(ttsply_pause)();

    return PLATFORM_OK;
}
#endif

int platform_ttsply_stop(void)
{
    return IVTBL(tts_stop)();
}
/*+\new\wj\2019.12.27\添加TTS功能*/

