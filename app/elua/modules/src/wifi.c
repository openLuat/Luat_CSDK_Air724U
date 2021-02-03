/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    wifi.c
 * Author:  shenyuanyuan
 * Version: V0.1
 * Date:    2020/5/11
 *
 * Description:
 *          lua.wifi访问库
 **************************************************************************/
#if defined(LUA_WIFISCAN_SUPPORT)
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_malloc.h"

HANDLE wifiscantimer=0;
UINT32 wifiscanmax = 0;
UINT32 wifiscanmaxtimeout = 0;

/*+\bug\wj\2020.11.23\将wifi扫描改为异步，否则会阻塞luaTask*/
void wifi_scan_cb(OPENAT_wifiScanRequest* req)
{
	char* wifiinfo = NULL;
	u32 j = 0;
	u32 num = 0;	
	PlatformMsgData rtosmsg;
	if(req->found != 0)
	{
		for (u32 i = 0; i < req->found; i++)
		{
			OPENAT_wifiApInfo *w = &req->aps[i];
			OPENAT_print("wifi_getinfo amWifilocCellinfoCb found ap - {mac address: %x%lx, rssival: %d dBm, channel: %u}",
			w->bssid_high, w->bssid_low, w->rssival, w->channel);
		}

		wifiinfo = platform_malloc(req->max * 50);
		if(wifiinfo != NULL)
		{
			while(j < req->found)
			{
				OPENAT_wifiApInfo *w = &req->aps[j];
				/*+\BUG\wangyuan\2020.07.10\BUG_2539:V0018 wifi扫描得到的ap mac地址有的丢了0*/
				num += sprintf(wifiinfo + num, "%04x%08lx,%d,%u;",w->bssid_high, w->bssid_low, w->rssival, w->channel);
				/*-\BUG\wangyuan\2020.07.10\BUG_2539:V0018 wifi扫描得到的ap mac地址有的丢了0*/
				j++;
			}
		}
	}
	
	OPENAT_free(req->aps);
	OPENAT_free(req);

	rtosmsg.wifiData.num = req->found;
	rtosmsg.wifiData.pData = wifiinfo;
	platform_rtos_send(MSG_ID_RTOS_MSG_WIFI, &rtosmsg);
}



BOOL wifi_scan(int scan_max,int scan_timeout)
{
	scan_max = scan_max > 0 ? scan_max : 10;
	scan_timeout = scan_timeout > 0 ? scan_timeout : 300;
	if(!OPENAT_GetWifiScanState())
	{ 
		OPENAT_wifiScanRequest *scan_req = OPENAT_malloc(sizeof(OPENAT_wifiScanRequest));
		memset(scan_req,0,sizeof(OPENAT_wifiScanRequest));
		if(!scan_req)
			return FALSE;
			
		OPENAT_wifiApInfo* aps = (OPENAT_wifiApInfo*)OPENAT_malloc(scan_max * sizeof(OPENAT_wifiApInfo));
		if(!aps)
		{
			OPENAT_free(scan_req);
			return FALSE;
		}

		scan_req->max = scan_max;
		scan_req->maxtimeout = scan_timeout;
		scan_req->aps = aps;
		if(OPENAT_WifiAsyncScanAll(scan_req,wifi_scan_cb))
			return TRUE;
		else 
		{
			OPENAT_free(aps);
			OPENAT_free(scan_req);	
		}
	}
	return FALSE;
	
}


static int wifi_getinfo(lua_State *L) {

	OPENAT_wifiScanRequest wreq = {0};
	CHAR* wifiinfo;
	UINT16 num = 0;
	u32 j = 0;
	
	int wifiscanmax = luaL_optint(L, 1, 0);
    int wifiscanmaxtimeout = luaL_optint(L, 2, 0);

	lua_pushinteger(L, wifi_scan(wifiscanmax,wifiscanmaxtimeout) ? 0 : 1);

    return 1;

}
/*-\bug\wj\2020.11.23\将wifi扫描改为异步，否则会阻塞luaTask*/
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

