#include <stdio.h>
#include "string.h"
#include "iot_debug.h"
#include "iot_pmd.h"

static void cust_pm_message(T_AMOPENAT_PM_MSG* pmMessage)
{
    switch(pmMessage->evtId)
    {
        case OPENAT_DRV_EVT_PM_POWERON_ON_IND:
			iot_debug_print("[pmd] powerOnReason: %d",pmMessage->param.poweronind.powerOnReason);
            break;
        case OPENAT_DRV_EVT_CHR_PRESENT_IND:
			iot_debug_print("[pmd]CHR TurnOff");
            break;
        case OPENAT_DRV_EVT_BAT_CHARGING:
			iot_debug_print("[pmd]BAT_CHARGING");
            break;
		case OPENAT_DRV_EVT_BAT_CHR_FULL:
			iot_debug_print("[pmd]BAT_CHR_FULL");
            break;
        default:
            break;
    }
}

int appimg_enter(void *param)
{    
    iot_debug_print("[pmd] app_main");
	T_AMOPENAT_PMD_CFG pmdcfg;
	E_AMOPENAT_PM_CHR_MODE pmdmode;
              
    memset(&pmdcfg, 0, sizeof(T_AMOPENAT_PMD_CFG));
    /*模块内置充电方案*/
    pmdmode = OPENAT_PM_CHR_BY_DEFAULT;
    /*和硬件设计有关*/
    pmdcfg.deFault.batdetectEnable = TRUE; 
    pmdcfg.deFault.tempdetectEnable = FALSE;
    pmdcfg.deFault.templowLevel = 0;
    pmdcfg.deFault.temphighLevel = 0;
    pmdcfg.deFault.batLevelEnable = FALSE;

    iot_pmd_init(pmdmode, &pmdcfg, cust_pm_message);
	
    return 0;
}

void appimg_exit(void)
{
	iot_debug_print("[pmd] appimg_exit");
}