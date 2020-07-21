/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    wifi.c
 * Author:  shenyuanyuan
 * Version: V0.1
 * Date:    2020/5/11
 *
 * Description:
 *          lua.wifi∑√Œ ø‚
 **************************************************************************/
#if !defined(LUAT_TTSFLOAT_SUPPORT)
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"

HANDLE wifiscantimer=0;
UINT32 wifiscanmax = 0;
UINT32 wifiscanmaxtimeout = 0;

BOOL wifi_scan()
{
	OPENAT_wifiScanRequest wreq = {0};
	CHAR* wifiinfo;
	UINT16 num = 0;
	u32 j = 0;
	PlatformMsgData rtosmsg;
		
	wreq.max = wifiscanmax > 0 ? wifiscanmax : 10;
	wreq.maxtimeout = wifiscanmaxtimeout > 0 ? wifiscanmaxtimeout : 300;

	OPENAT_wifiApInfo* aps = (OPENAT_wifiApInfo*)OPENAT_malloc(wreq.max * sizeof(OPENAT_wifiApInfo));
	wreq.aps = aps;
	
	OPENAT_get_wifiinfo(&wreq);
	
	for (u32 i = 0; i < wreq.found; i++)
	{
		OPENAT_wifiApInfo *w = &wreq.aps[i];
		OPENAT_print("wifi_getinfo amWifilocCellinfoCb found ap - {mac address: %x%lx, rssival: %d dBm, channel: %u}",
		w->bssid_high, w->bssid_low, w->rssival, w->channel);
	}

	wifiinfo = OPENAT_malloc(wreq.max * 30);
	memset(wifiinfo, 0, wreq.max * 30);

	while(j < wreq.found)
	{
		OPENAT_wifiApInfo *w = &wreq.aps[j];
		num += sprintf(wifiinfo + num, "%x%lx,%d,%u;",w->bssid_high, w->bssid_low, w->rssival, w->channel);
		j++;
	}
	
	OPENAT_free(aps);

	rtosmsg.wifiData.num = wreq.found;
	rtosmsg.wifiData.pData = wifiinfo;
	platform_rtos_send(MSG_ID_RTOS_MSG_WIFI, &rtosmsg);

	OPENAT_free(wifiinfo);
   
	return 0; 	
}


static int wifi_scan_timer()
{
   if (wifiscantimer==0)
    {  
      wifiscantimer= OPENAT_create_timerTask(wifi_scan,NULL);
    }
    OPENAT_start_timer(wifiscantimer, 10);
    return 0;
}

static int wifi_getinfo(lua_State *L) {

	OPENAT_wifiScanRequest wreq = {0};
	CHAR* wifiinfo;
	UINT16 num = 0;
	u32 j = 0;
	
	wifiscanmax = luaL_optint(L, 1, 0);
    wifiscanmaxtimeout = luaL_optint(L, 2, 0);

	wifi_scan_timer();

	lua_pushinteger(L, 0);

    return 1;

}

#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE wifi_map[] =
{ 
  { LSTRKEY( "getinfo" ),  LFUNCVAL( wifi_getinfo ) },
  
  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_wificore( lua_State *L )
{
    luaL_register( L, AUXLIB_WIFI, wifi_map );

    return 1;
}
#endif

