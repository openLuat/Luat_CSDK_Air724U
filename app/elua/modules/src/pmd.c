/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    pmd.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/28
 *
 * Description:
 *          lua.pmd库 电源管理库
 **************************************************************************/

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_pmd.h"

/*+\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */
static int getFiledInt(lua_State *L, int index, const char *key, int defval)
{
    lua_getfield(L, index, key);
    return luaL_optint(L, -1, defval);
}

// pmd.init
static int pmd_init(lua_State *L) {
    #define GET_FIELD_VAL(fIELD, dEFault) pmdcfg.fIELD = getFiledInt( L, 1, #fIELD, dEFault)

    PlatformPmdCfg pmdcfg;

    luaL_checktype(L, 1, LUA_TTABLE);
    
/*+\NEW\RUFEI\2015.5.8\完善充电控制*/
    GET_FIELD_VAL(ccLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(cvLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(ovLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(pvLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(poweroffLevel, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(ccCurrent, PMD_CFG_INVALID_VALUE);
    GET_FIELD_VAL(fullCurrent, PMD_CFG_INVALID_VALUE);
    /*+\NEW\zhuth\2014.11.6\电源管理配置参数中添加是否检测电池的配置*/
    GET_FIELD_VAL(batdetectEnable, PMD_CFG_INVALID_VALUE);
    /*-\NEW\zhuth\2014.11.6\电源管理配置参数中添加是否检测电池的配置*/
/*-\NEW\RUFEI\2015.5.8\完善充电控制*/

    lua_pushinteger(L, platform_pmd_init(&pmdcfg));

    return 1;
}
/*-\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */

// pmd.ldoset
static int pmd_ldo_set(lua_State *L) {
    int total = lua_gettop(L);
    int level = luaL_checkinteger(L, 1);
    int i;
    int ldo;

    for(i = 2; i <= total; i++)
    {
        ldo = luaL_checkinteger(L, i);
        platform_ldo_set(ldo, level);
    }

    return 0; 
}

// pmd.sleep(sleepornot)
static int pmd_deepsleep(lua_State *L) {    
    int sleep = luaL_checkinteger(L,1);

    platform_pmd_powersave(sleep);
    return 0; 
}

/*+\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */
//pmd.charger()
static int pmd_charger(lua_State *L) {
    lua_pushboolean(L, platform_pmd_get_charger());
    return 1;
}
/*-\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */


static int pmd_chargingcurrent(lua_State *L) {
    lua_pushinteger(L, platform_pmd_getChargingCurrent());
    return 1;
}
static int pmd_chg_param_get(lua_State *L)
{
	BOOL    battStatus = TRUE;
    BOOL    chargerStatus = 0;
    u32      chargeState = 0;
    u8      battLevel = 100;
    u16     battVolt = 4200;

	platform_pmd_get_chg_param(&battStatus, &battVolt, &battLevel, &chargerStatus, &chargeState);
	lua_pushboolean(L, battStatus);
	lua_pushinteger(L, battVolt);
	lua_pushinteger(L, battLevel);
	lua_pushboolean(L, chargerStatus);
	lua_pushinteger(L, chargeState);
	
	return 5;
}

static int pmd_speakerMode_set(lua_State *L) {
    int mode = luaL_checkinteger(L,1);

    //platform_pmd_speakerMode_set(mode);
    return 0;
}


#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE pmd_map[] =
{ 
/*+\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */
  { LSTRKEY( "init" ),  LFUNCVAL( pmd_init ) },
/*-\NEW\liweiqiang\2013.9.8\增加pmd.init设置充电电流接口 */
  { LSTRKEY( "ldoset" ),  LFUNCVAL( pmd_ldo_set ) },
  { LSTRKEY( "sleep" ),  LFUNCVAL( pmd_deepsleep ) },
  /*+\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */
  { LSTRKEY( "charger" ),  LFUNCVAL( pmd_charger ) },
  /*-\NEW\liweiqiang\2014.2.13\增加pmd.charger查询充电器状态接口 */

  { LSTRKEY( "chrcurrent" ),  LFUNCVAL( pmd_chargingcurrent ) },
/*+NEW\zhuwangbin\2017.2.10\添加充电参数查询接口*/
  { LSTRKEY( "param_get" ),  LFUNCVAL( pmd_chg_param_get ) },
/*+NEW\zhuwangbin\2017.2.10\添加充电参数查询接口*/
  { LSTRKEY( "speakerMode_set" ),  LFUNCVAL( pmd_speakerMode_set ) },
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_pmd( lua_State *L )
{
    luaL_register( L, AUXLIB_PMD, pmd_map );

/*+\NEW\liweiqiang\2013.10.10\增加LDO_VLCD控制POWER_VLCD*/
    MOD_REG_NUMBER(L, "LDO_VLCD", PLATFORM_LDO_VLCD);
/*-\NEW\liweiqiang\2013.10.10\增加LDO_VLCD控制POWER_VLCD*/

/*+\NEW\liweiqiang\2013.11.8\增加LDO_VASW,VMMC控制*/
    MOD_REG_NUMBER(L, "LDO_VMMC", PLATFORM_LDO_VMMC);
/*+\new\wj\2020.4.14\添加电压域VSIM1控制gpio29，30，31*/
	MOD_REG_NUMBER(L, "LDO_VSIM1", PLATFORM_LDO_VSIM1);
/*-\new\wj\2020.4.14\添加电压域VSIM1控制gpio29，30，31*/
	/*+\new\shenyuanyuan\2020.5.21\模块无VCAM输出*/
	MOD_REG_NUMBER(L, "LDO_VCAMA", PLATFORM_LDO_VCAMA);
	MOD_REG_NUMBER(L, "LDO_VCAMD", PLATFORM_LDO_VCAMD);
	/*-\new\shenyuanyuan\2020.5.21\模块无VCAM输出*/
    return 1;
}  
