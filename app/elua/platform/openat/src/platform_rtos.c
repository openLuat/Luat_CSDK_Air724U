/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_rtos.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/7
 *
 * Description:
 *          lua rtos API
  * History:
 *     panjun 2015.06.16 ME continually restart when to power on by 'PWR' key during
 *                                 power-off charging procedure.
 *     panjun 2015.06.22 Optimize mechanism of LUA's timer.
 *     panjun 2015.07.04 Define a macro 'OPENAT_MAX_TIMER_ID', Increase quantity of OPENAT's timer.
 **************************************************************************/
#include "stdio.h"
#include "assert.h"
#include "time.h"
#include "cs_types.h"
#include "lplatform.h"
#include "platform_rtos.h"
#include "am_openat.h"
#include "platform_malloc.h"
#include "malloc.h"
//#include "teldef.h"


#define DEBUG_RTOS

#if defined(DEBUG_RTOS)
#define DEBUG_RTOS_TRACE(fmt, ...)      PUB_TRACE(fmt, ##__VA_ARGS__)
#else
#define DEBUG_RTOS_TRACE(fmt, ...)
#endif

typedef struct LuaTimerParamTag
{
    HANDLE              hOsTimer;
    int                 luaTimerId;
}LuaTimerParam;

static LuaTimerParam luaTimerParam[OPENAT_MAX_TIMERS];
static HANDLE hLuaTimerSem = OPENAT_INVALID_HANDLE;

static HANDLE           hRTOSWaitMsgTimer;

extern HANDLE g_LuaShellTaskHandle;

int platform_rtos_send(platform_msg_type msg_id, PlatformMsgData* pMsg)
{
    OPENAT_print("platform_rtos_send msg %d", msg_id);
    OPENAT_send_message(g_LuaShellTaskHandle, msg_id, pMsg, sizeof(PlatformMsgData));
    //OPENAT_print("platform_rtos_send msg end", msg_id);
    return PLATFORM_OK;
}
/*+\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
kal_bool platform_rtos_send_high_priority(platform_msg_type msg_id, PlatformMsgData* pMsg)
{
    OPENAT_print("platform_rtos_send_high_priority msg %d", msg_id);
    return OPENAT_SendHighPriorityMessage(g_LuaShellTaskHandle, msg_id, pMsg, sizeof(PlatformMsgData));
}
/*-\new\wj\2020.11.13\兼容2G版本 uart.config功能*/
int platform_rtos_receive(platform_msg_type* msg_id, void **ppMessage, u32 timeout)
{
    BOOL ret;
    
    ret = OPENAT_wait_message(g_LuaShellTaskHandle, msg_id, ppMessage, timeout);

    //OPENAT_print("platform_rtos_recv got msg %d", *msg_id);

    
    return ret == TRUE ? PLATFORM_OK : PLATFORM_ERR;
}



void platform_rtos_free_msg(void *msg_body)
{
    OPENAT_free_message(msg_body);
    
}

void platform_rtos_timer_callback(void *pParameter)
{
    #if defined(NUCLEUS_TIMER_MECHANISM_ENABLE)
    int timer_id = ( int )(pParameter);
    #else
    int timer_id = (int)(pParameter);
    #endif //NUCLEUS_TIMER_MECHANISM_ENABLE
    u8 index;    
    PlatformMsgData msgData;

    msgData.timer_id = timer_id;

    OPENAT_print("platform_rtos_timer_callback %d",timer_id);

    
    //OPENAT_wait_semaphore(hLuaTimerSem, 0);

    for(index = 0; index < OPENAT_MAX_TIMERS; index++)
    {
        if(luaTimerParam[index].luaTimerId == timer_id)
        {
            break;
        }
    }
	
    //OPENAT_release_semaphore(hLuaTimerSem);


    if(index != OPENAT_MAX_TIMERS)
    {
      HANDLE cr;
      HANDLE hTimer;
      
      cr = OPENAT_enter_critical_section();
      if(luaTimerParam[index].hOsTimer != OPENAT_INVALID_HANDLE)
      {
          hTimer = luaTimerParam[index].hOsTimer;
          luaTimerParam[index].hOsTimer = OPENAT_INVALID_HANDLE;
          OPENAT_exit_critical_section(cr);
          OPENAT_delete_timer(hTimer);
          platform_rtos_send(MSG_ID_RTOS_TIMER, &msgData);
      }
      else
      {
          OPENAT_exit_critical_section(cr);
      }
      
      
    }
}

void rtos_stop_timer(int timer_id)
{
    u8 index;
    HANDLE hTimer = NULL;
    for(index = 0; index < OPENAT_MAX_TIMERS; index++)
    {
        if(luaTimerParam[index].hOsTimer != OPENAT_INVALID_HANDLE &&
            luaTimerParam[index].luaTimerId == timer_id)
        {
            break;
        }
    }
    if(index != OPENAT_MAX_TIMERS)
    {
      HANDLE cr;
      HANDLE hTimer;
      
      cr = OPENAT_enter_critical_section();
      hTimer = luaTimerParam[index].hOsTimer;
      luaTimerParam[index].hOsTimer = OPENAT_INVALID_HANDLE;
      OPENAT_exit_critical_section(cr);

      OPENAT_stop_timer(hTimer);
      OPENAT_delete_timer(hTimer);
    }

}


	
int platform_rtos_start_timer(int timer_id, int milliSecond)
{
    u8 index;
    HANDLE hTimer = OPENAT_INVALID_HANDLE;
    int real_timer_id = timer_id;
    HANDLE cr;
    
    //OPENAT_print("platform_rtos_start_timer");

    
    /*if(!hLuaTimerSem)
    {
        hLuaTimerSem = OPENAT_create_semaphore(1);
    }*/
    
    //OPENAT_wait_semaphore(hLuaTimerSem, 0);

    for(index = 0; index < OPENAT_MAX_TIMERS; index++)
    {
        if(luaTimerParam[index].hOsTimer != OPENAT_INVALID_HANDLE &&
            luaTimerParam[index].luaTimerId == real_timer_id)
        {
            rtos_stop_timer(real_timer_id);
            break;
        }
    }

    for(index = 0; index < OPENAT_MAX_TIMERS; index++)
    {
        if(luaTimerParam[index].hOsTimer == NULL)
        {
            break;
        }
    }

    if(index == OPENAT_MAX_TIMERS)
    {
        DEBUG_RTOS_TRACE("[platform_rtos_start_timer]: no timer resource.");
        goto start_timer_error;
    }
	
    hTimer = OPENAT_create_timerTask(platform_rtos_timer_callback, (PVOID)timer_id);

    if(OPENAT_INVALID_HANDLE == hTimer)
    {
        DEBUG_RTOS_TRACE("[platform_rtos_start_timer]: create timer failed.");
        goto start_timer_error;
    }
    cr = OPENAT_enter_critical_section();
    luaTimerParam[index].hOsTimer = hTimer;
    luaTimerParam[index].luaTimerId = timer_id;
    OPENAT_exit_critical_section(cr);

    if(!OPENAT_start_timer(hTimer, milliSecond))
    {
        DEBUG_RTOS_TRACE("[platform_rtos_start_timer]: start timer failed.");
        cr = OPENAT_enter_critical_section();
        luaTimerParam[index].hOsTimer = OPENAT_INVALID_HANDLE;
        luaTimerParam[index].luaTimerId = 0;
        OPENAT_exit_critical_section(cr);
        goto start_timer_error;
    }
    OPENAT_print("[platform_rtos_start_timer] start_timer_sucess index = %d timer_id = %d.", index, timer_id);

    //OPENAT_release_semaphore(hLuaTimerSem);
    
    return PLATFORM_OK;

start_timer_error:
    //OPENAT_release_semaphore(hLuaTimerSem);

/*+\NEW\liweiqiang\2014.7.22\修正启动定时器失败把定时器资源耗尽的问题 */
    if(OPENAT_INVALID_HANDLE != hTimer)
/*-\NEW\liweiqiang\2014.7.22\修正启动定时器失败把定时器资源耗尽的问题 */
    {
        OPENAT_print("[platform_rtos_start_timer] start_timer_error.");
        OPENAT_stop_timer(hTimer);
        OPENAT_delete_timer(hTimer);
    }
    
    return PLATFORM_ERR;
}

int platform_rtos_stop_timer(int timer_id)
{
    u8 index;
    int real_timer_id = timer_id;
    
    /*if(OPENAT_INVALID_HANDLE == hLuaTimerSem)
    {
        hLuaTimerSem = OPENAT_create_semaphore(1);
    }*/
    

    //OPENAT_wait_semaphore(hLuaTimerSem, 0);

    for(index = 0; index < OPENAT_MAX_TIMERS; index++)
    {
        if(luaTimerParam[index].hOsTimer != OPENAT_INVALID_HANDLE &&
            luaTimerParam[index].luaTimerId == real_timer_id)
        {
            break;
        }
    }

    if(index != OPENAT_MAX_TIMERS)
    {
      HANDLE cr;
      HANDLE hTimer;

      OPENAT_print("[platform_rtos_stop_timer] index = %d timer_id = %d.", index, timer_id);
      cr = OPENAT_enter_critical_section();
      if(luaTimerParam[index].hOsTimer != OPENAT_INVALID_HANDLE)
      {
          hTimer = luaTimerParam[index].hOsTimer;
          luaTimerParam[index].hOsTimer = OPENAT_INVALID_HANDLE;
          OPENAT_exit_critical_section(cr);
          OPENAT_stop_timer(hTimer);
          OPENAT_delete_timer(hTimer);
      }
      else
      {
          OPENAT_exit_critical_section(cr);
      }

      
    }
    
    //OPENAT_release_semaphore(hLuaTimerSem);
    
    return PLATFORM_OK;
}


/*+\NEW\zhuwangbin\2016.5.24\lua初始化结束前， 是否有按键*/
static kal_bool keypad_is_press_bool = KAL_FALSE;
static kal_bool g_s_init_end_bool = KAL_FALSE;

kal_bool platform_lua_get_keypad_is_press(void)
{
  return keypad_is_press_bool;
}

kal_bool paltform_is_lua_init_end(void)
{
  OPENAT_print("paltform_is_lua_init_end ");
  return g_s_init_end_bool = KAL_TRUE;
}

void paltform_keypad_record_is_press(void)
{
#ifdef SHOW_LOGO_SUPPORT
  kal_bool lua_init_end_bool = KAL_FALSE;
  kal_bool show_logo_init_end_bool = KAL_FALSE;
  static kal_uint8 comeInCount = 0;



  /*判断showlogo是否初始化结束*/
  show_logo_init_end_bool = show_logo_init_is_end();
  
  if (!g_s_init_end_bool)
  {
    comeInCount++;

    OPENAT_print("paltform_keypad_record_is_press comeInCount %d, %d", comeInCount, kal_get_systicks());
    
    /*lua 未 初始化结束记录按键状态*/
    keypad_is_press_bool = KAL_TRUE;

    /*SHOW logo 初始化结束电量背光*/
    if (show_logo_init_end_bool)
      platform_ldo_set(10, 2);
  }
#endif
}

void platform_keypad_message(T_AMOPENAT_KEYPAD_MESSAGE *pKeypadMessage)
{
    PlatformMsgData msgData;
    
    /*
    DEBUG_RTOS_TRACE("[platform_keypad_message]: p(%d) r(%d) c(%d)", 
                    pKeypadMessage->bPressed, 
                    pKeypadMessage->data.matrix.r, 
                    pKeypadMessage->data.matrix.c);
    */
    
    msgData.keypadMsgData.bPressed = pKeypadMessage->bPressed;
    msgData.keypadMsgData.data.matrix.row = pKeypadMessage->data.matrix.r;
    msgData.keypadMsgData.data.matrix.col = pKeypadMessage->data.matrix.c;

    platform_rtos_send(MSG_ID_RTOS_KEYPAD, &msgData);
}
/*+\NEW\rufei\2015.3.13\增加闹钟消息 */
void platform_alarm_message(T_AMOPENAT_ALARM_MESSAGE *pAlarmMessage)
{
    PlatformMsgData msgData;
    
    platform_rtos_send(MSG_ID_RTOS_ALARM, &msgData);
}
/*-\NEW\rufei\2015.3.13\增加闹钟消息 */

void platform_touch_message(T_AMOPENAT_TOUCHSCREEN_MESSAGE *pTouchMessage)
{
    PlatformMsgData msgData;

    #if 0
    DEBUG_RTOS_TRACE("ZHY [platform_touch_message]: t(%d) x(%d) y(%d)", 
                    pTouchMessage->penState, 
                    pTouchMessage->x, 
                    pTouchMessage->y);
    #endif
    msgData.touchMsgData.type = pTouchMessage->penState;
    msgData.touchMsgData.x = pTouchMessage->x;
    msgData.touchMsgData.y = pTouchMessage->y;

    platform_rtos_send(MSG_ID_RTOS_TOUCH, &msgData);
}

int platform_rtos_init_module(int module, void *pParam)
{
    int ret = PLATFORM_OK;
    
    switch(module)
    {
    case RTOS_MODULE_ID_KEYPAD:
        {
            T_AMOPENAT_KEYPAD_CONFIG keypadConfig;
            PlatformKeypadInitParam *pKeypadParam = (PlatformKeypadInitParam *)pParam;

            keypadConfig.type = OPENAT_KEYPAD_TYPE_MATRIX;

            keypadConfig.pKeypadMessageCallback = platform_keypad_message;
            
            keypadConfig.config.matrix.keyInMask = pKeypadParam->matrix.inMask;
            keypadConfig.config.matrix.keyOutMask = pKeypadParam->matrix.outMask;

            OPENAT_init_keypad(&keypadConfig);
        }
        break;
		/*+\NEW\rufei\2015.3.13\增加闹钟消息 */
    case RTOS_MODULE_ID_ALARM:
        {
             T_AMOPENAT_ALARM_CONFIG  alarmConfig;

		alarmConfig.pAlarmMessageCallback = platform_alarm_message;
             OPENAT_InitAlarm(&alarmConfig);
        }
        break;
		/*-\NEW\rufei\2015.3.13\增加闹钟消息 */

#ifdef TOUCH_PANEL_SUPPORT
    case RTOS_MODULE_ID_TOUCH:
        {
            OPENAT_print("zhy OPENAT_init_touchScreen");
            OPENAT_init_touchScreen(platform_touch_message);
        }
        break;
#endif

    default:
        DEBUG_RTOS_TRACE("[platform_rtos_init_module]: unknown module(%d)", module);
        ret = PLATFORM_ERR;
        break;
    }
    
    return ret;
}

static void rtos_wait_message_timeout(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    PlatformMsgData msgData;

    platform_rtos_send(MSG_ID_RTOS_WAIT_MSG_TIMEOUT, &msgData);
}

int platform_rtos_init(void)
{
    hLuaTimerSem = OPENAT_create_semaphore(1);
    hRTOSWaitMsgTimer = OPENAT_create_timerTask(rtos_wait_message_timeout, NULL);

    return PLATFORM_OK;
}

/*+\NEW\liweiqiang\2013.12.12\增加充电开机时由用户自行决定是否启动系统 */
extern int cust_get_poweron_reason(void);
extern void cust_poweron_system(void);
/*+\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/
extern void cust_repoweron_system(void);
/*-\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/

int platform_get_poweron_reason(void)
{
    /*+\NEW\zhuth\2014.7.25\修改开机原因值实现*/
    return (int)OPENAT_get_poweronCause();
    /*-\NEW\zhuth\2014.7.25\修改开机原因值实现*/
}


void platform_lua_dead_loop(void)
{
    platform_msg_type msg_id;
    void* pMesage;
    
    while(1)
    {
        platform_rtos_receive(&msg_id, &pMesage, 0);
        platform_rtos_free_msg(pMesage);
        OPENAT_sleep(1);
    }
}

static int poweron_flag = -1;/*-1: 未设置 0: 不启动 1:启动*/
/*+\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
static BOOL cust_sys_flag = FALSE; 
/*-\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/

int platform_rtos_poweron(int flag)
{
    OPENAT_print("platform_rtos_poweron");
    OPENAT_sleep(10);

    poweron_flag = flag;

    if(1 == poweron_flag)
    {
        cust_poweron_system();
        /*+\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
        cust_sys_flag = TRUE;
        /*-\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
    }
    
    return PLATFORM_OK;
}

/*+\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/
int platform_rtos_repoweron(void)
{
    OPENAT_print("platform_rtos_repoweron");
    OPENAT_sleep(10);

   cust_repoweron_system();
   
    return PLATFORM_OK;
}
/*-\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/

void platform_poweron_try(void)
{
    // 为兼容旧脚本,在未设置开机标志时自动启动系统
    if(-1 == poweron_flag)
    {
        cust_poweron_system();    
        /*+\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
        cust_sys_flag = TRUE;
        /*-\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
    }
}
/*-\NEW\liweiqiang\2013.12.12\增加充电开机时由用户自行决定是否启动系统 */

/*+\BUG3096\zhuwangbin\2020.9.17\添加关机充电功能*/
#ifdef AM_LUA_SUPPORT
int platform_rtos_poweroff(int type)
{
    OPENAT_print("platform_rtos_poweroff type %d", type);
    OPENAT_sleep(10);

	if (type)
	{
		//OPENAT_shutdown_charger();
	}
	else
    {
	    /*+\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
	    if((platform_get_poweron_reason() == OPENAT_PM_POWERON_BY_CHARGER)
	        && !cust_sys_flag)
	    {
	        OPENAT_shut_down();
	    }
	    else
	    {
	        OPENAT_poweroff_system();
	    }
    	/*-\NEW\zhuth\2014.2.14\充电开机并且用户没有启动协议栈的情况下使用shutdown关机，其余情况使用poweroff_system关机*/
	}
	
    platform_lua_dead_loop();
    
    return PLATFORM_OK;
}
/*+\BUG3096\zhuwangbin\2020.9.17\添加关机充电功能*/
#endif

/*+\NEW\liweiqiang\2013.9.7\增加rtos.restart接口*/
int platform_rtos_restart(void)
{
    OPENAT_print("platform_rtos_restart");
    OPENAT_restart();
    OPENAT_sleep(10);    
    platform_lua_dead_loop();
    return PLATFORM_OK;
}
/*-\NEW\liweiqiang\2013.9.7\增加rtos.restart接口*/

/*+\NEW\liweiqiang\2013.4.5\增加rtos.tick接口*/
int platform_rtos_tick(void)
{
    return OPENAT_get_system_tick();
}
int platform_rtos_sms_is_ready(void)
{
    return TRUE;//srv_sms_is_sms_ready();
}
/*-\NEW\liweiqiang\2013.4.5\增加rtos.tick接口*/
/*+\NEW\rufei\2015.3.13\增加闹钟消息 */
int platform_rtos_setalarm(void *pParam)
{
    PlatformSetAlarmParam *platformalarm = (PlatformSetAlarmParam *)pParam;
    T_AMOPENAT_ALARM_PARAM pAlarmSet;

    pAlarmSet.alarmOn = platformalarm->alarmon;
    pAlarmSet.alarmTime.nYear = platformalarm->year;
    pAlarmSet.alarmTime.nMonth = platformalarm->month;
    pAlarmSet.alarmTime.nDay = platformalarm->day;
    pAlarmSet.alarmTime.nHour = platformalarm->hour;
    pAlarmSet.alarmTime.nMin = platformalarm->min;
    pAlarmSet.alarmTime.nSec = platformalarm->sec;
    
    return ((TRUE == OPENAT_SetAlarm(&pAlarmSet))?PLATFORM_OK:PLATFORM_ERR);
}
/*-+\NEW\rufei\2015.3.13\增加闹钟消息 */

static   char   base64_table[]   =       
{
    'A',   'B',   'C',   'D',   'E',   'F',   'G',   'H',   'I',   'J',   'K',   'L',   'M',       
    'N',   'O',   'P',   'Q',   'R',   'S',   'T',   'U',   'V',   'W',   'X',   'Y',   'Z',       
    'a',   'b',   'c',   'd',   'e',   'f',   'g',   'h',   'i',   'j',   'k',   'l',   'm',       
    'n',   'o',   'p',   'q',   'r',   's',   't',   'u',   'v',   'w',   'x',   'y',   'z',       
    '0',   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '+',   '/',   '\0'       
};

char*  platform_base64_encode(const char *str, int length)
{

  

    static char  base64_pad = '=';       
    const char *current = (const   char*)str;     
    int i = 0;       
    char *result = (char *)L_MALLOC(((length + 3 - length % 3) * 4 / 3 + 1) * sizeof(char)); 
    
    while (length > 2) 
    { 
        /*   keep   going   until   we   have   less   than   24   bits   */     
        result[i++] = base64_table[current[0] >> 2];   
        result[i++] = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];       
        result[i++] = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];       
        result[i++] = base64_table[current[2] & 0x3f];       
        current += 3;       
        length -= 3;   /*   we   just   handle   3   octets   of   data   */       
    } 
    
    /*   now   deal   with   the   tail   end   of   things   */       
    if   (length != 0)  
    {       
        result[i++] = base64_table[current[0] >> 2]; 
        
        if   (length > 1) 
        {       
            result[i++] = base64_table[((current[0] & 0x03 ) << 4) + (current[1] >> 4)];       
            result[i++] = base64_table[(current[1] & 0x0f) << 2];   
            result[i++] = base64_pad;     
        }     
        else   
        {     
            result[i++] = base64_table[(current[0] & 0x03) << 4];     
            result[i++] = base64_pad;     
            result[i++] = base64_pad;     
        }     
    }    
    
    result[i] = '\0';   //   printf("%s/n",result);     
    return result;     
}    
/*+\NEW\brezen\2016.4.25\增加base64接口*/
char* platform_base64_decode (const char *src, int length, int* decodedLen) 
{
  int i = 0;
  int j = 0;
  int l = 0;
  size_t size = 0;
  char *dec = NULL;
  char buf[3];
  char tmp[4];

  if(!src)
  {
   return NULL;
  }
  dec = (unsigned char *)L_MALLOC(((length+3)/4)*3 + 1);
  if (NULL == dec) 
  {
    return NULL;
  }

  // parse until end of source
  while (length--) {
   // break if char is `=' or not base64 char
   if ('=' == src[j]) { break; }
   if (!(isalnum(src[j]) || '+' == src[j] || '/' == src[j])) { break; }

   // read up to 4 bytes at a time into `tmp'
   tmp[i++] = src[j++];

   // if 4 bytes read then decode into `buf'
   if (4 == i) {
     // translate values in `tmp' from table
     for (i = 0; i < 4; ++i) {
       // find translation char in `b64_table'
       for (l = 0; l < 64; ++l) {
         if (tmp[i] == base64_table[l]) {
           tmp[i] = l;
           break;
         }
       }
     }

     // decode
     buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
     buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
     buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

     // write decoded buffer to `dec'
     //dec = (unsigned char *) tls_mem_realloc(dec, size + 3);
     for (i = 0; i < 3; ++i) {
       dec[size++] = buf[i];
     }

     // reset
     i = 0;
   }
  }

  // remainder
  if (i > 0) {
   // fill `tmp' with `\0' at most 4 times
   for (j = i; j < 4; ++j) {
     tmp[j] = '\0';
   }

   // translate remainder
   for (j = 0; j < 4; ++j) {
       // find translation char in `b64_table'
       for (l = 0; l < 64; ++l) {
         if (tmp[j] == base64_table[l]) {
           tmp[j] = l;
           break;
         }
       }
   }

   // decode remainder
   buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
   buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
   buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

   // write remainer decoded buffer to `dec'
   //dec = (unsigned char *) tls_mem_realloc(dec, size + (i - 1));
   for (j = 0; (j < i - 1); ++j) {
     dec[size++] = buf[j];
   }
  }

  // Make sure we have enough space to add '\0' character at end.
  //dec = (unsigned char *) tls_mem_realloc(dec, size + 1);
  dec[size] = '\0';

  // Return back the size of decoded string if demanded.
  if (decodedLen != NULL) *decodedLen = size;

  return dec;
}
/*-\NEW\brezen\2016.4.25\增加base64接口*/

//+panjun,160503,Add an API "rtos.disk_free". 
long platform_rtos_disk_free(int drvtype)
{
   return 0;
}

int platform_rtos_disk_volume(int drvtype)
{
   return 0;
}
//-panjun,160503,Add an API "rtos.disk_free".
/*+:\NEW\brezen\2016.10.13\支持SIM卡切换*/
void platform_sim_status_ind(int insert)
{
  
}
/*+:\NEW\brezen\2016.10.13\支持SIM卡切换*/
/*+\add\liangjian\2020.06.22\增加SD 卡显示范围*/
UINT32 platform_fs_get_free_size(int isSD,unsigned int type)
{
    T_AMOPENAT_FILE_INFO info = {0};
	
	/*+:\NEW\zhuwangbin\2020.8.8\修改文件系统的信息的获取*/
	char *path;
	
	if (isSD)
	{
		path = "/sdcard0";
	}
	else
	{
		path = "/";
	}
	
    if(IVTBL(get_fs_info)(E_AMOPENAT_FS_INTERNAL,&info,path,type)==0)
    {
        UINT32 freeSize = info.totalSize-info.usedSize;
        //DEBUG_RTOS_TRACE("platform_fs_get_free_size %l %l %l\n", info.totalSize, info.usedSize, freeSize);
        return freeSize;
    }
    else
    {
        return 0;
    }
	/*-:\NEW\zhuwangbin\2020.8.8\修改文件系统的信息的获取*/
}
/*end\NEW\zhutianhua\2017.9.5 31:2\增加get_fs_free_size接口*/

/*+:\NEW\liangjian\2020.6.16\增加获取文件空间大小接口*/

UINT32 platform_fs_get_total_size(int isSD,unsigned int type)
{
	T_AMOPENAT_FILE_INFO info = {0};
	
	/*+:\NEW\zhuwangbin\2020.8.8\修改文件系统的信息的获取*/
	char *path;
	
	if (isSD)
	{
		path = "/sdcard0";
	}
	else
	{
		path = "/";
	}
    if(IVTBL(get_fs_info)(E_AMOPENAT_FS_INTERNAL,&info ,path,type)==0)
    {
        return info.totalSize;
    }
    else
    {
        return 0;
    }
	/*-:\NEW\zhuwangbin\2020.8.8\修改文件系统的信息的获取*/
}
/*end\NEW\liangjian\2020.6.16\增加获取文件空间大小接口*/
/*-\bug\add\2020.06.22\增加SD 卡显示范围*/

int platform_make_dir(char *pDir, int len)
{
    char* sysDir[] = {"/", "/lua", "/ldata", "/luazip", "/RecDir"};
    u8 i = 0;
    int length = len;

    if(len == 0)
    {
        IVTBL(print)("platform_make_dir dir invalid");
        return 0;
    }

    if(len>1 && *(pDir+len-1) == '/')
    {
        length = len - 1;
    }

    *(pDir + length) = 0;

    for(i=0; i<sizeof(sysDir)/sizeof(u8*); i++)
    {
        if(memcmp(pDir, sysDir[i], strlen(sysDir[i]))==0 && strlen(sysDir[i])==length)
        {
            IVTBL(print)("platform_make_dir no authority to make %s", sysDir[i]);
            return 0;
        }
    }

    if(IVTBL(make_dir)(pDir, 0) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int platform_remove_dir(char *pDir, int len)
{
    char* sysDir[] = {"/", "/lua", "/ldata", "/luazip", "/RecDir"};
    u8 i = 0;
    int length = len;

    if(len == 0)
    {
        IVTBL(print)("platform_make_dir dir invalid");
        return 0;
    }

    if(len>1 && *(pDir+len-1)=='/')
    {
        length = len-1;
    }
    
    *(pDir + length) = 0;

    for(i=0; i<sizeof(sysDir)/sizeof(u8*); i++)
    {
        if(memcmp(pDir, sysDir[i], strlen(sysDir[i]))==0 && strlen(sysDir[i])==length)
        {
            IVTBL(print)("platform_make_dir no authority to make %s", sysDir[i]);
            return 0;
        }
    }
    
    return (IVTBL(remove_dir_rec)(pDir) == 0);
}

void platform_rtos_set_time(u32 timestamp){
    T_AMOPENAT_SYSTEM_DATETIME datetime;
    struct tm *stm = localtime(&timestamp);

    datetime.nYear = stm->tm_year + 1900;
    datetime.nMonth = stm->tm_mon + 1;
    datetime.nDay = stm->tm_mday;
    datetime.nHour = stm->tm_hour;
    datetime.nMin = stm->tm_min;
    datetime.nSec = stm->tm_sec;
        
    IVTBL(set_system_datetime)(&datetime);
}


int platform_rtos_fota_start(void){

   return (int)openat_otaInit(); 
}

int platform_rtos_fota_process(const char* data, int len, int total){

	return (int)openat_otaProcess(data, len, total); 
}

int platform_rtos_fota_end(void){

	return (int)openat_otaDone(); 
}

BOOL platform_rtos_set_trace_port(u8 port, u8 usb_port_diag_output)
{
    return OPENAT_set_trace_port(port, usb_port_diag_output);
}

u8 platform_rtos_get_trace_port(void)
{
    return OPENAT_get_trace_port();
}

/*+\NEW\shenyuanyuan\2020.4.14\添加rtos.set_trace设置trace开关和端口*/
BOOL platform_rtos_set_trace(u8 print, u8 port)
{
	return OPENAT_set_trace(print,port);
}
/*-\NEW\shenyuanyuan\2020.4.14\添加rtos.set_trace设置trace开关和端口*/

/*+\NEW\shenyuanyuan\2019.4.19\开发AT+TRANSDATA命令*/
int platform_rtos_sendok(char *src)
{
#ifdef LUA_TODO
   OPENAT_rtos_sendok(src);   
#endif
   return 0; 
}
/*-\NEW\shenyuanyuan\2019.4.19\开发AT+TRANSDATA命令*/

int platform_rtos_get_fatal_info(char *info, int maxLen)
{
#ifdef LUA_TODO
    return OPENAT_get_fatal_info(info,maxLen); 
#endif
	return 0;
}

int platform_rtos_remove_fatal_info()
{
#ifdef LUA_TODO
    return OPENAT_remove_fatal_info(); 
#endif
	return 0;
}

/*+\NEW\hedonghao\2020.4.10\添加LUA软狗接口*/
BOOL platform_rtos_open_SoftDog(u32 timeout)
{
   OPENAT_rtos_open_SoftDog(timeout);   
   return 0; 		
}

BOOL platform_rtos_eat_SoftDog(void)
{
	OPENAT_rtos_eat_SoftDog(); 
	return 0;
}

BOOL platform_rtos_close_SoftDog(void)
{
	OPENAT_rtos_close_SoftDog(); 
	return 0;
}

/*-\NEW\hedonghao\2020.4.10\添加LUA软狗接口*/
/*+\BUG\wangyuan\2020.07.28\BUG_2640:8910平台LUA版本增加读取客户版本号的AT指令，兼容之前1802平台的“AT+LUAINFO?”*/
int platform_rtos_set_luainfo(char *src)
{
   OPENAT_rtos_set_luainfo(src);   
   return 0; 
}
/*-\BUG\wangyuan\2020.07.28\BUG_2640:8910平台LUA版本增加读取客户版本号的AT指令，兼容之前1802平台的“AT+LUAINFO?”*/
/*+\new\wj\lua添加热插拔接口*/
void platform_rtos_notify_sim_detect(int simNum,BOOL connect) 
{
	/*目前sim卡num固定为1卡，后面再优化*/
	OPENAT_notify_sim_detect(simNum,connect);
}

/*-\new\wj\lua添加热插拔接口*/


/*+\BUG3109\zhuwangbin\2020.9.21\对讲机需要在开机状态下通过串口或者USB串口进行POC小包下载和写号*/
#ifdef AM_LUA_POC_SUPPORT
BOOL platform_rtos_poc_ota(void)
{
	extern BOOL openat_pocImageIsVaild(void);
	
	return openat_pocImageIsVaild(); 
}

int platform_rtos_poc_flash_erase(void)
{
	return OPENAT_flash_erase(CONFIG_PLATFORM_FLASH_OFFSET, CONFIG_PLATFORM_FLASH_OFFSET+CONFIG_PLATFORM_FLASH_SIZE);
}

int platform_rtos_poc_flash_read(UINT32 offset, UINT32 size, UINT8 * buf)
{
	UINT32 readSize;
	return OPENAT_flash_read(CONFIG_PLATFORM_FLASH_OFFSET+offset, size, &readSize, (CONST UINT8* )buf);
}

int platform_rtos_poc_flash_write(UINT32 offset, UINT32 size, UINT8 * buf)
{
	UINT32 writenSize;
	return OPENAT_flash_write(CONFIG_PLATFORM_FLASH_OFFSET+offset, size, &writenSize, (CONST UINT8* )buf);
}

#endif
/*-\BUG3109\zhuwangbin\2020.9.21\对讲机需要在开机状态下通过串口或者USB串口进行POC小包下载和写号*/

/*+\bug\rww\2020.9.22\添加rtos.setTransData*/
// typedef struct
// {
//     int len;
//     char *data;
// } trans_data_t;

// static void _set_trans_data(void *ctx)
// {
//     extern int trans_data_len;
//     extern char *trans_data;
//     trans_data_t *td = (trans_data_t *)ctx; 
//     if (trans_data != NULL)
//     {
//         free(trans_data);
//     }
//     trans_data_len = td->len;
//     trans_data = td->data;
//     free(td);
// }

// platform_rtos_set_trans_data(int len, char *buf)
// {
//     trans_data_t *td = (trans_data_t *)malloc(sizeof(trans_data_t));
//     td->len = len;
//     td->data = buf;
//     if (!osiThreadCallback(atEngineGetThreadId(), _set_trans_data, td))
//     {
//         free(buf);
//         free(td);
//     }
// }
/*-\bug\rww\2020.9.22\添加rtos.setTransData*/