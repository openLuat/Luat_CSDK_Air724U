/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_pmd.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          platform power manage 接口
 **************************************************************************/

#ifndef _PLATFORM_PMD_H_
#define _PLATFORM_PMD_H_

typedef enum PlatformLdoIdTag
{
    PLATFORM_LDO_VLCD,
    PLATFORM_LDO_VMMC,
	/*+\new\wj\2020.4.14\添加电压域VSIM1控制gpio29，30，31*/
    PLATFORM_LDO_VSIM1,
	/*-\new\wj\2020.4.14\添加电压域VSIM1控制gpio29，30，31*/
	/*+\new\shenyuanyuan\2020.5.21\模块无VCAM输出*/
	PLATFORM_LDO_VCAMA,
	PLATFORM_LDO_VCAMD,
	/*-\new\shenyuanyuan\2020.5.21\模块无VCAM输出*/
	/*+\BUG\wangyuan\2020.08.22\BUG_2883:lua开发820GPS供电引脚设置*/
	PLATFORM_LDO_VIBR,
	/*-\BUG\wangyuan\2020.08.22\BUG_2883:lua开发820GPS供电引脚设置*/
	/*+\BUG3154\zhuwangbin\2020.10.10\添加backlight设置*/
	PLATFORM_LDO_VBACKLIGHT_R,
	PLATFORM_LDO_VBACKLIGHT_G,
	PLATFORM_LDO_VBACKLIGHT_B,
	PLATFORM_LDO_VBACKLIGHT_W,
	/*-\BUG3154\zhuwangbin\2020.10.10\添加backlight设置*/
	
	/*+\BUG3753\zhuwangbin\2020.12.4\添加audio hmic bias ldo设置*/
	PLATFORM_LDO_POWER_HMICBIAS,
	/*-\BUG3753\zhuwangbin\2020.12.4\添加audio hmic bias ldo设置*/
    PLATFORM_LDO_QTY
}PlatformLdoId;

/*+\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */
/*+\NEW\liweiqiang\2014.2.8\完善电源管理配置接口 */
#define PMD_CFG_INVALID_VALUE           (0xffff)

typedef struct PlatformPmdCfgTag
{
/*+\NEW\RUFEI\2015.5.8\完善充电控制*/
    u16             ccLevel;/*恒流阶段:4.1*/
    u16             cvLevel;/*恒压阶段:4.2*/
    u16             ovLevel;/*充电限制：4.3*/
    u16             pvLevel;/*回充4.1*/
    u16             poweroffLevel;/*关机电压：3.4，仅用于计算电量百分比，实际由上层控制关机*/
    u16             ccCurrent;/*恒流阶段电流*/
    u16             fullCurrent;/*恒压充满电流：30*/
/*-\NEW\RUFEI\2015.5.8\完善充电控制*/
    /*+\NEW\zhuth\2014.11.6\电源管理配置参数中添加是否检测电池的配置*/
    u16             batdetectEnable;
    /*-\NEW\zhuth\2014.11.6\电源管理配置参数中添加是否检测电池的配置*/
}PlatformPmdCfg;
/*-\NEW\liweiqiang\2014.2.8\完善电源管理配置接口 */

int platform_pmd_init(PlatformPmdCfg *pmdCfg);
/*-\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */

int platform_ldo_set(PlatformLdoId id, int level);

//sleep_wake: 1 sleep 0 wakeup
int platform_pmd_powersave(int sleep_wake);

/*+\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */
int platform_pmd_get_charger(void);
/*-\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */

#endif//_PLATFORM_PMD_H_
