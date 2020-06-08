#ifndef __IOT_AUDIO_H__
#define __IOT_AUDIO_H__

#include "iot_os.h"
#include "am_openat.h"



/**
 * @defgroup iot_sdk_audio 音频接口
 * @{
 */
/**@example audio/demo_audio.c
* audio接口示例
*/

/**打开语音
*@note  在通话开始时调用
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_open_tch(                                        
                        VOID
                );

/**关闭语音
*@note  通话结束时调用
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_close_tch(                                      
                        VOID
                 );

/**播放TONE音
*@param  toneType:      TONE音类型
*@param  duration:      播放时长
*@param  volume:        播放音量
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_tone(                                        
                        E_AMOPENAT_TONE_TYPE toneType,     
                        UINT16 duration,                   
                        E_AMOPENAT_SPEAKER_GAIN volume     
                 );

/**停止播放TONE音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_tone(                                        
                        VOID
                 );


/**播放DTMF音
*@param  dtmfType:      DTMF类型
*@param  duration:      播放时长
*@param  volume:        播放音量
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_dtmf(                                        
                        E_AMOPENAT_DTMF_TYPE dtmfType,     
                        UINT16 duration,                   
                        E_AMOPENAT_SPEAKER_GAIN volume     
                 );

/**停止播放DTMF音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_dtmf(                            
                        VOID
                 );

/**播放音频
*@param  playParam:     播放参数
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_play_music(T_AMOPENAT_PLAY_PARAM*  playParam);

/**停止音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_stop_music(                                        
                        VOID
                  );

/**暂停音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_pause_music(                                     
                        VOID
                   );

/**恢复音频播放
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_resume_music(                                       
                        VOID
                    );

/**设置扬声器静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_mute_speaker(                                     
                        VOID
                    );

/**解除扬声器静音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_unmute_speaker(                                   
                        VOID
                      );

/**设置扬声器的音量值
*@param     vol:   设置扬声器音量值
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_speaker_vol(                                   
                        UINT32 vol 
                        );

/**获取扬声器的音量
*@return	UINT32: 	 返回扬声器的音量值
**/
UINT32 iot_audio_get_speaker_vol(                
                        VOID
                                           );

/**设置音频通道
*@param     channel:    通道
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_set_channel(                                       
                        E_AMOPENAT_AUDIO_CHANNEL channel    
                   );

/**获取当前通道
*@return	E_AMOPENAT_AUDIO_CHANNEL: 	  返回通道值
**/
E_AMOPENAT_AUDIO_CHANNEL iot_audio_get_current_channel(            
                        VOID
                                               );

/**开始录音
*@param     E_AMOPENAT_RECORD_PARAM:   录音参数
*@param     AUD_RECORD_CALLBACK_T:     获取录音数据回调
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_rec_start(
                    			E_AMOPENAT_RECORD_PARAM* param,
								AUD_RECORD_CALLBACK_T cb);

/**停止录音
*@return	TRUE: 	    成功
*           FALSE:      失败
**/
BOOL iot_audio_rec_stop();



/** @}*/


#endif
