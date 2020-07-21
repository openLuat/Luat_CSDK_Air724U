
#include "string.h"
#include "stdio.h"
#include "am_openat.h"

#include "assert.h"
#include "lplatform.h"
#include "platform_malloc.h"
#include "platform_rtos.h"
#include "platform_pmd.h"

#include "osi_api.h"

#define CUST_MAIN_TASK_PRIO     OPENAT_CUST_TASKS_PRIORITY_BASE
#define LUA_SHELL_TASK_PRIO     (OPENAT_CUST_TASKS_PRIORITY_BASE+10)

#define CUST_MAIN_TASK_STACK_SIZE       (2*1024)
#define LUA_SHELL_TASK_STACK_SIZE       (16*1024)/*堆栈加大至16K避免载入大脚本堆栈溢出*/

typedef enum CustMessageIdTag
{
    OPENAT_VAT_MSG_ID,
    
    NumOfCustMsgIds
}CustMessageId;

typedef struct {
    UINT16 len;
    PVOID data;
}CustMessageContext;

/*+\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
extern void LuaDeleteMainFile(void);
/*-\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/

extern int LuaAppTask(void);
extern void RILAPI_ReceiveData(void *data, int len);
extern void platform_setup_vat_queue(void);

/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
extern u32 Get_usbdata_mode(void);
extern void USBAPI_ReceiveData(void *data, int len);
extern void platform_setup_usb_queue(void);
/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */

static VOID cust_poweron_reason_init(VOID);
static void cust_pm_message(T_AMOPENAT_PM_MSG* pmMessage);
static VOID cust_at_message(UINT8 *pData, UINT16 length);

kal_bool lua_task_init ( void);

/*******************************************************
** ATTENTION: For our cust, this part must used. If you 
** have your own main entry file(instead of cust_main.c),
** pls copy this part!
**
** START .. START .. START .. START .. START .. START ..
********************************************************/
/* CUSTOM APP CODE INFOMATION */

/*******************************************************
** ATTENTION:
** END .. END .. END .. END .. END .. END .. END ..
********************************************************/

/* Function table from OpenAT platform */
HANDLE g_LuaShellTaskHandle;

struct
{
    HANDLE poweronSyncSemaphore;
    E_AMOPENAT_POWERON_REASON reason;
}g_PowronInfo = {0};


#if 0

/* Main function call by OpenAT platform */
VOID cust_main(VOID)
{
    /* 密钥已经准备好 需要发出通知 */
    IVTBL(set_enc_data_ok)();

    cust_poweron_reason_init();

    /*+\NEW\liweiqiang\2013.9.22\增加vat缓冲区信号量保护*/
    platform_setup_vat_queue();
    /*-\NEW\liweiqiang\2013.9.22\增加vat缓冲区信号量保护*/
    
    IVTBL(init_at)(cust_at_message);
    
    /* FOR power on reason and charging messages */
    T_AMOPENAT_PMD_CFG pmdcfg;
    E_AMOPENAT_PM_CHR_MODE pmdmode;

    memset(&pmdcfg, 0, sizeof(T_AMOPENAT_PMD_CFG));
#if 1  /*模块内置充电方案*/
    pmdmode = OPENAT_PM_CHR_BY_DEFAULT;
    /*和硬件设计有关*/
    pmdcfg.deFault.batdetectEnable = TRUE;
    
    pmdcfg.deFault.tempdetectEnable = FALSE;
    pmdcfg.deFault.templowLevel = 0;
    pmdcfg.deFault.temphighLevel = 0;

/*+\NEW\2013.8.5\A9321调整充电配置,充满电压4.25V 快充电流150mA 实测217mA */
    pmdcfg.deFault.batLevelEnable = TRUE;
    pmdcfg.deFault.batfullLevel = 4200;
    pmdcfg.deFault.batPreChargLevel = 4050;
    pmdcfg.deFault.poweronLevel = 3450;
    pmdcfg.deFault.poweroffLevel = 3400;
    pmdcfg.deFault.batAdc = OPENAT_ADC_7;/*adc_sense  or adc_vbat:OPENAT_ADC_0*/
    pmdcfg.deFault.tempAdc = OPENAT_ADC_1;
    
    /*level:  poweron-----levelFirst-----levelSecond-----levelFull*/
    /*current:----currentFirst----currentSecond---currentThird----*/
    pmdcfg.deFault.currentControlEnable = TRUE; 
    pmdcfg.deFault.currentFirst = OPENAT_PM_CHARGER_500MA;
    pmdcfg.deFault.intervalTimeFirst = 9*60; /*9分钟*/
    pmdcfg.deFault.batLevelFirst = 4150;
    pmdcfg.deFault.currentSecond = OPENAT_PM_CHARGER_300MA;
    pmdcfg.deFault.intervalTimeSecond = 6*60;/*6分钟*/
    pmdcfg.deFault.batLevelSecond = 4190;
    pmdcfg.deFault.currentThird = OPENAT_PM_CHARGER_100MA;
    pmdcfg.deFault.intervalTimeThird = 3*60; /*3分钟*/  
/*-\NEW\2013.8.5\A9321调整充电配置,充满电压4.25V 快充电流150mA 实测217mA */

    pmdcfg.deFault.chargTimeOutEnable = FALSE;
    pmdcfg.deFault.TimeOutMinutes = 240;
#endif
#if 0
    pmdmode = OPENAT_PM_CHR_BY_IC;
    /*和硬件设计有关*/
    pmdcfg.ic.batdetectEnable = TRUE;
    pmdcfg.ic.tempdetectEnable = FALSE;
    pmdcfg.ic.templowLevel = 0;
    pmdcfg.ic.temphighLevel = 0;
    
    pmdcfg.ic.chrswitchport = OPENAT_GPIO_8;
    pmdcfg.ic.batstatusport = OPENAT_GPIO_1;
    
    pmdcfg.ic.batLevelEnable = FALSE;
    pmdcfg.ic.batfullLevel = 4200;
    pmdcfg.ic.poweronLevel = 3450;
    pmdcfg.ic.poweroffLevel = 3400;
    pmdcfg.ic.batAdc = OPENAT_ADC_7;/*adc_sense  or adc_vbat:OPENAT_ADC_0*/
    pmdcfg.ic.tempAdc = OPENAT_ADC_1;/*adc_battemp*/

    pmdcfg.ic.chargTimeOutEnable = FALSE;
    pmdcfg.ic.TimeOutMinutes = 240;
#endif
    ASSERT(IVTBL(init_pmd)(pmdmode, &pmdcfg, cust_pm_message));
    
    /* 创建custom app线程 */
    g_CustTaskHandle = IVTBL(create_task)((PTASK_MAIN)cust_task_main, 
                                            NULL, 
                                            NULL, 
                                            CUST_MAIN_TASK_STACK_SIZE, 
                                            CUST_MAIN_TASK_PRIO, 
                                            OPENAT_OS_CREATE_DEFAULT, 
                                            0, 
                                            "cust task");

    if(OPENAT_INVALID_HANDLE == g_CustTaskHandle)
    {
        ASSERT(0);
    }
    
    g_LuaShellTaskHandle = IVTBL(create_task)((PTASK_MAIN)lua_shell_main, 
                                                NULL, 
                                                NULL, 
                                                LUA_SHELL_TASK_STACK_SIZE, 
                                                LUA_SHELL_TASK_PRIO, 
                                                OPENAT_OS_CREATE_DEFAULT, 
                                                0, 
                                                "lua shell task");

    if(OPENAT_INVALID_HANDLE == g_LuaShellTaskHandle)
    {
        ASSERT(0);
    }
}
#endif

static void cust_pm_message(T_AMOPENAT_PM_MSG* pmMessage)
{
/*+\NEW\liweiqiang\2013.7.8\增加rtos.pmd消息*/
    static PlatformPmdData pmdData = 
    {
        TRUE,
         FALSE,
        PLATFORM_BATT_NOT_CHARGING,
        0xff,
         0,
    };
    T_AMOPENAT_BAT_STATUS battStatus;
    T_AMOPENAT_CHARGER_STATUS chargerStatus;

    //PUB_TRACE("[cust_pm_message]: event %d",pmMessage->evtId);
    
    switch(pmMessage->evtId)
    {
#ifdef LUA_TODO
        case OPENAT_DRV_EVT_PM_POWERON_ON_IND:
            if(OPENAT_PM_POWERON_BY_INVALID == g_PowronInfo.reason)
            {
                IVTBL(get_batteryStatus)(&battStatus);
                IVTBL(get_chargerStatus)(&chargerStatus);

                pmdData.battVolt = battStatus.batteryVolt;
                pmdData.chargerStatus = chargerStatus.chargerStatus == OPENAT_PM_CHARGER_PLUG_IN;
                
                g_PowronInfo.reason = pmMessage->param.poweronind.powerOnReason;
                
                if(0 != g_PowronInfo.poweronSyncSemaphore)
                {
                    OPENAT_release_semaphore(g_PowronInfo.poweronSyncSemaphore);
                }
            }
            else
            {
                /* received poweron reason message again */
            }
#endif
/*+\NEW\RUFEI\2015.6.11\Modify the charging message sequence*/
            return;
/*-\NEW\RUFEI\2015.6.11\Modify the charging message sequence*/
            break;
        case OPENAT_DRV_EVT_CHR_PRESENT_IND:
            pmdData.chargerStatus = pmMessage->param.chrpresentind.present;
            if(pmdData.chargerStatus == FALSE)
                pmdData.chargeState = PLATFORM_BATT_NOT_CHARGING; // 充电器拔出,设为不在充电状态
            break;
        case OPENAT_DRV_EVT_BAT_PRESENT_IND:
            pmdData.battStatus = pmMessage->param.batpresentind.present;
            break;
        case OPENAT_DRV_EVT_BAT_LEVEL_IND:
            if(pmdData.battLevel == pmMessage->param.batlevelind.batteryLevel)
            {
                //电池容量没有变化不作消息提示
                return;
            }
            pmdData.battLevel = pmMessage->param.batlevelind.batteryLevel;
            IVTBL(get_batteryStatus)(&battStatus);
            pmdData.battVolt = battStatus.batteryVolt;
            break;
        case OPENAT_DRV_EVT_BAT_CHARGING:
        /*+\NEW\liweiqiang\2013.7.19\修正充电时电池容量不更新的问题*/
/*+\NEW\RUFEI\2015.6.5\解决充电开机，不上报充电器在位问题*/
            if(pmdData.chargerStatus == FALSE)
            {
                pmdData.chargerStatus = TRUE;
            }
/*-\NEW\RUFEI\2015.6.5\解决充电开机，不上报充电器在位问题*/
            if(PLATFORM_BATT_CHARING == pmdData.chargeState &&
                pmdData.battLevel == pmMessage->param.chargingind.batteryLevel)
            {
                // 充电时一直会上报该消息 若状态未改变且电池容量未变化不作频繁提示
                return;
            }
            pmdData.chargeState = PLATFORM_BATT_CHARING;
            pmdData.battLevel = pmMessage->param.chargingind.batteryLevel;
            IVTBL(get_batteryStatus)(&battStatus);
            pmdData.battVolt = battStatus.batteryVolt;
        /*-\NEW\liweiqiang\2013.7.19\修正充电时电池容量不更新的问题*/
            break;
        case OPENAT_DRV_EVT_BAT_CHR_FULL:
/*+\NEW\RUFEI\2015.6.5\解决充电开机，不上报充电器在位问题*/
            if(pmdData.chargerStatus == FALSE)
            {
                pmdData.chargerStatus = TRUE;
            }
/*-\NEW\RUFEI\2015.6.5\解决充电开机，不上报充电器在位问题*/
        /*+\NEW\liweiqiang\2013.7.19\修正充电时电池容量不更新的问题*/
            pmdData.chargeState = PLATFORM_BATT_CHARGE_STOP;
            pmdData.battLevel = pmMessage->param.chrfullind.batteryLevel;
            IVTBL(get_batteryStatus)(&battStatus);
            pmdData.battVolt = battStatus.batteryVolt;
        /*-\NEW\liweiqiang\2013.7.19\修正充电时电池容量不更新的问题*/
            break;
        case OPENAT_DRV_EVT_BAT_CHR_STOP:
            if(pmMessage->param.chrstopind.chrStopReason == OPENAT_PM_CHR_STOP_NO_CHARGER)
                pmdData.chargeState = PLATFORM_BATT_NOT_CHARGING;
            else
                pmdData.chargeState = PLATFORM_BATT_CHARGE_STOP;
            break;
        case OPENAT_DRV_EVT_BAT_CHR_ERR:
            pmdData.chargeState = PLATFORM_BATT_CHARGE_STOP;
            break;
        default:
            break;
    }
    
    //if(pmdData.battLevel != 0xff) //去掉level未初始化的判断,避免直接插充电器开机未上报电池容量导致一直无法上报pmd消息
    {
        PlatformMsgData msgData;

        msgData.pmdData = pmdData;
        
        platform_rtos_send(MSG_ID_RTOS_PMD, &msgData);
    }
/*-\NEW\liweiqiang\2013.7.8\增加rtos.pmd消息*/
}

/* AT message from OpenAT platform */
static VOID cust_at_message(UINT8 *pData, UINT16 length)
{
#if 0
    CustMessageContext *pMessage;
    T_AMOPENAT_MSG openatMsg;
    
    if(NULL != pMessage)
    {        //copy data
        pMessage = IVTBL(malloc)(length + sizeof(CustMessageContext));
        
        if(NULL != pMessage)
        {
            pMessage->data = (PVOID)(pMessage + 1);
            memcpy(pMessage->data, pData, length);
            pMessage->len = length;
        }

        openatMsg.openat_msg_id = MSG_ID_OPENAT_VAT_MSG_ID;
        openatMsg.openat_msg_context = pMessage;
        
        IVTBL(send_message)(&openatMsg);
    }
#endif

    RILAPI_ReceiveData(pData,length);
}

static VOID cust_poweron_reason_init(VOID)
{
    if(0 == g_PowronInfo.poweronSyncSemaphore)
    {
        g_PowronInfo.poweronSyncSemaphore = OPENAT_create_semaphore(0);
        ASSERT(0 != g_PowronInfo.poweronSyncSemaphore);
    }
}

static VOID cust_wait_for_poweron_reason(BOOL bDeleteSema)
{
    ASSERT(0 != g_PowronInfo.poweronSyncSemaphore);

    OPENAT_wait_semaphore(g_PowronInfo.poweronSyncSemaphore, 0);

    if(TRUE == bDeleteSema)
    {
        OPENAT_delete_semaphore(g_PowronInfo.poweronSyncSemaphore);
        g_PowronInfo.poweronSyncSemaphore = 0;
    }
}

/*+\NEW\liweiqiang\2013.12.12\增加充电开机时由用户自行决定是否启动系统 */
int cust_get_poweron_reason(void)
{
    return (int)g_PowronInfo.reason;
}

void cust_poweron_system(void)
{
    static BOOL hasPoweron = FALSE;

    if(!hasPoweron)
    {
        hasPoweron = TRUE;
        IVTBL(poweron_system)(OPENAT_PM_STARTUP_MODE_DEFAULT, OPENAT_PM_STARTUP_MODE_DEFAULT);
    }
}
/*-\NEW\liweiqiang\2013.12.12\增加充电开机时由用户自行决定是否启动系统 */

/*+\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/
void cust_repoweron_system(void)
{
    static BOOL hasRePoweron = FALSE;

    if(!hasRePoweron)
    {
        hasRePoweron = TRUE;
        IVTBL(poweron_system)(OPENAT_PM_STARTUP_MODE_ON, OPENAT_PM_STARTUP_MODE_ON);
    }
}
/*-\NEW\rufei\2015.4.17\实现闹钟开机后重新启动系统L4层功能*/

/*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
#define LUA_DIR "/lua"
#define LUA_DATA_DIR "/ldata"
#define LUA_DIR_UNI     L"/lua"
#define LUA_DATA_UNI    L"/ldata"

char *getLuaPath(void)
{
    return LUA_DIR "/?.lua;" LUA_DIR "/?.luac";
}

char *getLuaDir(void)
{
    return LUA_DIR;
}

char *getLuaDataDir(void)
{
    return LUA_DATA_DIR;
}
/*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
void removeLuaDir(void)
{
    /*+\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
    LuaDeleteMainFile();
/*-\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
/*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
    IVTBL(remove_dir)(LUA_DIR); //异常退出删除所有脚本
/*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
/*+\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */
    IVTBL(remove_dir)(LUA_DATA_DIR); //异常退出删除所有数据
    IVTBL(remove_dir)("/luazip"); //异常退出删除升级包
/*-\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */
}

/*lua app main*/
static HANDLE luaShellSem = 0;
/*+\NEW\zhuwangbin\2020.2.26\添加lua 远程升级 */
#include "hal_config.h"
#define OTA_APP_MAGIN ("APPUPDATE")
#define OTA_APP_FILE_NAME "app"
#define OTA_APP_FILE_PATH CONFIG_FS_FOTA_DATA_DIR "/" OTA_APP_FILE_NAME


static void lua_update(void)
{
	INT32 size, readLen;
	INT32 appMaginLen = strlen(OTA_APP_MAGIN);
	INT32 fd = OPENAT_open_file(OTA_APP_FILE_PATH, FS_O_RDONLY, 0);
	UINT32 flash_size = LUA_SCRIPT_SIZE;
	UINT8 appMagin[20] = {0};

	IVTBL(print)("lua_update fd %d\r\n", fd);	
	if (fd < 0)
	{
		goto updateEnd;
	}
	
	size = OPENAT_get_file_size_h(fd);

	IVTBL(print)("lua_update size %d\r\n", size);
	if (size <= appMaginLen || size >= flash_size)
	{
		goto updateEnd;
	}
	
	readLen = OPENAT_seek_file(fd, size-appMaginLen, 0);

	IVTBL(print)("lua_update OPENAT_seek_file %d\r\n", readLen);
	
	readLen = OPENAT_read_file(fd, appMagin, appMaginLen);

	IVTBL(print)("lua_update readLen %d, appMagin %s\r\n", readLen, appMagin);
	if (readLen != appMaginLen)
	{
		IVTBL(print)("lua_update appMagin error %s\r\n", appMagin);
		goto updateEnd;
	}

	if (memcmp(appMagin, OTA_APP_MAGIN, appMaginLen) != 0)
	{
		IVTBL(print)("lua_update appMagin error %s\r\n", appMagin);
		goto updateEnd;
	}

	readLen = OPENAT_seek_file(fd, 0, 0);
	IVTBL(print)("lua_update OPENAT_seek_file %d\r\n", readLen);
	{
		#define blockSize (0X10000)
		UINT8 *data = OPENAT_malloc(blockSize);
		UINT32 addr = LUA_SCRIPT_ADDR;;

		if (!data)
		{
			IVTBL(print)("lua_update data %x\r\n", data);
			goto updateEnd;
		}
		
		IVTBL(print)("lua_update LUA_SCRIPT_ADDR [%x,%x]\r\n", LUA_SCRIPT_ADDR,LUA_SCRIPT_ADDR+flash_size);
		OPENAT_flash_erase(LUA_SCRIPT_ADDR, LUA_SCRIPT_ADDR+flash_size);

		while(1)
		{
			readLen = OPENAT_read_file(fd, data, blockSize);
			
			IVTBL(print)("lua_update write readLen %x, addr %x\r\n", readLen, addr);
			if (readLen > 0)
			{
				UINT32 writenSize;
				
				OPENAT_flash_write(addr, readLen, &writenSize, data);
				if (writenSize == 0)
				{
					ASSERT(0);
				}
				addr += readLen;
			}
			else
			{
				OPENAT_close_file(fd);
				OPENAT_delete_file(OTA_APP_FILE_PATH);
				OPENAT_free(data);
	
				return;
			}
		}
	}
	
updateEnd:
	if (fd > 0)
	{
		OPENAT_close_file(fd);
	}
	return;
}
/*-\NEW\zhuwangbin\2020.2.26\添加lua 远程升级 */

VOID __lua_exit()
{
    OPENAT_sleep(2000);
	ASSERT(FALSE);
}

static VOID lua_shell_main(void *task_entry_ptr)
{
/*+\NEW\liweiqiang\2013.4.25\增加lua退出assert处理 */
    int luaExitStatus;
	/*+\BUG\wangyuan\2020.06.22\BUG_2360:coretest测试，一直打印SET_PDP_4G_WAITAPN*/
    lua_task_init();
	
	/*+\NEW\zhuwangbin\2020.2.26\添加lua 远程升级 */
	lua_update();
	/*-\NEW\zhuwangbin\2020.2.26\添加lua 远程升级 */
	/*-\BUG\wangyuan\2020.06.22\BUG_2360:coretest测试，一直打印SET_PDP_4G_WAITAPN*/
    IVTBL(print)("lua_shell_main - LUA_SYNC_MMI(OK).");

    #if 0
    OPENAT_print("lua_shell_main enter 111");
    if(0 == luaShellSem)
    {
        luaShellSem = OPENAT_create_semaphore(0);
        ASSERT(0 != luaShellSem);
    }

    platform_setup_vat_queue();
    OPENAT_RegisterVirtualATCallBack(cust_at_message);
    
    OPENAT_print("lua_shell_main enter 22222");

/*+\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
    IVTBL(make_dir)(L"/luazip", 0);
/*-\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
/*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
    IVTBL(make_dir)(LUA_DIR_UNI, 0);
    IVTBL(make_dir)(LUA_DATA_UNI, 0);
/*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
/*+\NEW\liweiqiang\2013.10.24\创建录音目录,以播放录音文件 */
    IVTBL(make_dir)(L"/RecDir", 0);
/*-\NEW\liweiqiang\2013.10.24\创建录音目录,以播放录音文件 */

    OPENAT_print("lua_shell_main enter 33333");
    //OPENAT_sleep(1000);
    #endif

#if 0
    OPENAT_send_at_command("ati\r", strlen("ati\r"));

    while(1)
    {
        OPENAT_wait_message(&msg, 0);
        
        switch(msg->openat_msg_id)
        {
            case MSG_ID_OPENAT_VAT_MSG_ID:
            {
                CustMessageContext* openatVatMsg = msg->openat_msg_context;
                OPENAT_print("OPENAT %d DATA11:%s", (UINT32)openatVatMsg->len, openatVatMsg->data);
                OPENAT_free(openatVatMsg);
                break;
            }
        }
    }
#endif

   
    luaExitStatus = LuaAppTask();

	osiThreadCallback(osiThreadCurrent(), __lua_exit, NULL);
    
/*+\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
    LuaDeleteMainFile();
/*-\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
    /*+\NEW\WJ\2018.10.10\去掉USES_NOR_FLASH宏*/
    if(!OPENAT_is_nor_flash())
    {
        IVTBL(remove_file_rec)(LUA_DIR_UNI); //异常退出删除所有脚本
        IVTBL(remove_file_rec)(LUA_DATA_UNI); //异常退出删除所有数据
        IVTBL(remove_file_rec)(L"/luazip"); //异常退出删除升级包
    }
    else
    {
    /*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
        IVTBL(remove_file_rec)(LUA_DIR); //异常退出删除所有脚本
    /*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
    /*+\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */
        IVTBL(remove_file_rec)(LUA_DATA_DIR); //异常退出删除所有数据
        IVTBL(remove_file_rec)("/luazip"); //异常退出删除升级包
    /*-\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */
    }
    /*-\NEW\WJ\2018.10.10\去掉USES_NOR_FLASH宏*/
    OPENAT_print("[lua_shell_main]: lua exit status %d", luaExitStatus);
	/*+\BUG\wangyuan\2020.04.07\BUG_1328：720U，Lua脚本如果开机就报错，没有lua错误信息输出*/	
    IVTBL(wait_semaphore)(luaShellSem, 10000); // 10秒后重启系统
	/*-\BUG\wangyuan\2020.04.07\BUG_1328：720U，Lua脚本如果开机就报错，没有lua错误信息输出*/	

/*+\NEW\liweiqiang\2013.9.20\lua异常退出后重启系统,避免文件未写入到flash中*/
    // ASSERT(FALSE);
    //IVTBL(restart)();
/*-\NEW\liweiqiang\2013.9.20\lua异常退出后重启系统,避免文件未写入到flash中*/
/*-\NEW\liweiqiang\2013.4.25\增加lua退出assert处理 */
    osiEvent_t event;

    for (;;)
    {
        if (osiEventWait(osiThreadCurrent(), &event) == true)
        {
        }
    }
}

/*+\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */
BOOL cust_pmd_init(PlatformPmdCfg *cfg)
{
/*+\NEW\liweiqiang\2014.2.8\完善电源管理配置接口 */
#define GET_FILED_VAL(fIELD, dEFault) (cfg->fIELD == PMD_CFG_INVALID_VALUE ? (dEFault) : (cfg->fIELD))
    T_AMOPENAT_PMD_CFG pmdcfg;
    E_AMOPENAT_PM_CHR_MODE pmdmode;

    pmdmode = OPENAT_PM_CHR_BY_DEFAULT;
    /*和硬件设计有关*/
/*+\NEW\RUFEI\2015.5.8\完善充电控制*/
    pmdcfg.deFault.batdetectEnable = (GET_FILED_VAL(batdetectEnable, 0) == 1);
    
    pmdcfg.deFault.tempdetectEnable = FALSE;
    pmdcfg.deFault.templowLevel = 0;
    pmdcfg.deFault.temphighLevel = 0;

    pmdcfg.deFault.batLevelEnable = TRUE;
    pmdcfg.deFault.ccLevel = GET_FILED_VAL(ccLevel, OPENAT_PM_VOLT_04_050V);
    pmdcfg.deFault.cvLevel = GET_FILED_VAL(cvLevel, OPENAT_PM_VOLT_04_200V);
    pmdcfg.deFault.ovLevel = GET_FILED_VAL(ovLevel, OPENAT_PM_VOLT_04_250V);
    pmdcfg.deFault.pvLevel = GET_FILED_VAL(pvLevel, OPENAT_PM_VOLT_04_050V);
    pmdcfg.deFault.poweroffLevel = GET_FILED_VAL(poweroffLevel, OPENAT_PM_VOLT_03_400V);
/*+\NEW\RUFEI\2015.8.27\Add adc fuction*/
    pmdcfg.deFault.batAdc = OPENAT_ADC_0;/*adc_sense  or adc_vbat:OPENAT_ADC_0*/
    pmdcfg.deFault.tempAdc = OPENAT_ADC_1;
/*-\NEW\RUFEI\2015.8.27\Add adc fuction*/    
    pmdcfg.deFault.currentControlEnable = TRUE; 
    pmdcfg.deFault.ccCurrent = GET_FILED_VAL(ccCurrent, OPENAT_PM_CHARGER_200MA); 
    pmdcfg.deFault.fullCurrent = GET_FILED_VAL(fullCurrent, OPENAT_PM_CHARGER_30MA);
    pmdcfg.deFault.ccOnTime = 0; 
    pmdcfg.deFault.ccOnTime = 0; 
/*-\NEW\RUFEI\2015.5.8\完善充电控制*/
    pmdcfg.deFault.chargTimeOutEnable = FALSE;
    pmdcfg.deFault.TimeOutMinutes = 240;
/*-\NEW\liweiqiang\2014.2.8\完善电源管理配置接口 */

    return OPENAT_init_pmd(pmdmode, &pmdcfg, cust_pm_message);
}
/*-\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */



kal_bool lua_task_init ( void)
{
    OPENAT_print("lua_task_init enter 111");
    if(0 == luaShellSem)
    {
        luaShellSem = OPENAT_create_semaphore(0);
        ASSERT(0 != luaShellSem);
    }

    platform_setup_vat_queue();

	/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
	platform_setup_usb_queue();
	/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
	
    OPENAT_init_at(cust_at_message);
    //OPENAT_RegisterVirtualATCallBack(cust_at_message);
    
    OPENAT_print("lua_task_init enter 22222");
    /*+\NEW\WJ\2018.10.10\去掉USES_NOR_FLASH宏*/
    if(!OPENAT_is_nor_flash())
    {
        IVTBL(make_dir)(L"/luazip", 0);
        IVTBL(make_dir)(LUA_DIR_UNI, 0);
        IVTBL(make_dir)(LUA_DATA_UNI, 0);
        IVTBL(make_dir)(L"/RecDir", 0);
    }
    else
    {
    /*+\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
        IVTBL(make_dir)("/luazip", 0);
    /*-\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
    /*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
        IVTBL(make_dir)(LUA_DIR, 0);
        IVTBL(make_dir)(LUA_DATA_DIR, 0);
    /*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
    /*+\NEW\liweiqiang\2013.10.24\创建录音目录,以播放录音文件 */
        IVTBL(make_dir)("/RecDir", 0);
    /*-\NEW\liweiqiang\2013.10.24\创建录音目录,以播放录音文件 */
    }
    /*-\NEW\WJ\2018.10.10\去掉USES_NOR_FLASH宏*/

    OPENAT_print("lua_task_init enter 33333");

    return KAL_TRUE;
}




kal_bool lua_task_create(void)
{
    //TODO 创建线程
    OPENAT_create_task(&g_LuaShellTaskHandle, lua_shell_main, NULL, NULL, 
                  32*1024, 
                  24,
                  OPENAT_OS_CREATE_DEFAULT,
                  300,
                  "lua_task");
    OPENAT_print("lua_task create end handle %x", g_LuaShellTaskHandle);
    return KAL_TRUE;
}


void luatEluaInit()
{
  //openat_init();
  lua_task_create();
}


