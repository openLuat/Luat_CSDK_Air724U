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

#ifdef LUA_BLUETOOTH_LIB

#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_bluetooth.h"

#include "am_openat_bluetooth.h"

static int l_ble_open(lua_State *L)
{

    const u8 mode = luaL_optint(L, 1, 0);
    bool ret = platform_ble_open(mode);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_close(lua_State *L)
{

    bool ret = platform_ble_close();
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_send(lua_State *L)
{
    size_t len = 0;
    bool ret = 0;
    char *data = (char *)luaL_checklstring(L, 1, &len);
    const u16 handle = luaL_optint(L, 3, 0);
    if (lua_type(L, 2) == LUA_TNUMBER)
    {
        const u16 uuid_c = luaL_checkinteger(L, 2);
        ret = platform_ble_send(data, len, uuid_c, handle);
    }
    else if (lua_type(L, 2) == LUA_TSTRING)
    {
        const char *uuid_c = luaL_checkstring(L, 2);
        ret = platform_ble_send_string(data, len, uuid_c, handle);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

/*+\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/
static int l_ble_recv(lua_State *L)
{

    int maxsize = lua_tointeger(L, 1);
    if (maxsize < 0)
        return luaL_error(L, "invalid max size");

    OPENAT_print("[ble]:%s line:%d  maxsize %d", __func__, __LINE__, maxsize);

    luaL_Buffer b;
    luaL_buffinit(L, &b);

	/*+\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报*/
    plat_ble_recv_buff data = {0};
    data.len = maxsize;
    data.dataPtr = malloc(maxsize);
    if (data.dataPtr == NULL)
        OPENAT_print("[ble]:%s line:%d malloc false ,not enough memory ", __func__, __LINE__);

    int recvlen = platform_ble_recv(&data);

    OPENAT_print("[ble]:%s line:%d  platform_ble_recv recvlen:%d", __func__, __LINE__,recvlen);

    luaL_Buffer uuid;
    luaL_buffinit(L, &uuid);

    if (data.uuid_flag == 0)
        luaL_addlstring(&uuid, &data.uuid, sizeof(data.uuid));
    else
        luaL_addlstring(&uuid, &data.long_uuid, sizeof(data.long_uuid));

    luaL_addlstring(&b, data.dataPtr, recvlen);
    free(data.dataPtr);

    luaL_pushresult(&uuid);
	/*-\NEW\czm\2020.11.25\BUG 3702: 保留当前数据包的uuid属性，在读取时上报*/
    luaL_pushresult(&b);
    lua_pushinteger(L, recvlen);
    return 3;
}

/*-\NEW\czm\2020.11.25\BUG 3702: 1.3 蓝牙lua 收到通知带有数据，改为收到通知后，读取缓冲区数据*/

static int l_ble_set_name(lua_State *L)
{
    const char *name = luaL_checkstring(L, 1);
    bool ret = platform_ble_set_name(name);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_adv_param(lua_State *L)
{

    plat_advparam_t param;
    param.AdvMin = (u16)luaL_checkinteger(L, 1);
    param.AdvMax = (u16)luaL_checkinteger(L, 2);
    param.AdvType = (u8)luaL_checkinteger(L, 3);
    param.OwnAddrType = (u8)luaL_checkinteger(L, 4);
    param.AdvChannMap = (u8)luaL_checkinteger(L, 5);
    param.AdvFilter = (u8)luaL_checkinteger(L, 6);
    param.DirectAddrType = (u8)luaL_optint(L, 7, 0);
    param.DirectAddr = (char *)luaL_optstring(L, 8, NULL);
    bool ret = platform_ble_set_adv_param(&param);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_scan_param(lua_State *L)
{

    plat_scanparam_t param;
    param.scanType = (u8)luaL_checkinteger(L, 1);
    param.scanInterval = (u16)luaL_checkinteger(L, 2);
    param.scanWindow = (u16)luaL_checkinteger(L, 3);
    param.own_addr_type = (u8)luaL_checkinteger(L, 4);
    param.filterPolicy = (u8)luaL_checkinteger(L, 5);
    bool ret = platform_ble_set_scan_param(&param);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_adv_data(lua_State *L)
{
    size_t len = 0;
    char *data = (char *)luaL_checklstring(L, 1, &len);
    bool ret = platform_ble_set_adv_data(data, len);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_scanrsp_data(lua_State *L)
{
    size_t len = 0;
    char *data = (char *)luaL_checklstring(L, 1, &len);
    bool ret = platform_ble_set_scanrsp_data(data, len);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_adv_enable(lua_State *L)
{
    const u8 enable = (u8)luaL_checkinteger(L, 1);
    bool ret = platform_ble_set_adv_enable(enable);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_read_state(lua_State *L)
{
    bool ret = platform_ble_read_state();
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_set_scan_enable(lua_State *L)
{
    const u8 enable = (u8)luaL_checkinteger(L, 1);
    bool ret = platform_ble_set_scan_enable(enable);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_disconnect(lua_State *L)
{
    const u16 handle = luaL_optint(L, 1, 0);
    bool ret = platform_ble_disconnect(handle);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}
static int l_ble_connect(lua_State *L)
{
    int Index, Size;
    const char *addr = luaL_checkstring(L, 1);
    const u8 addr_type = luaL_optint(L, 2, 0);
    bool ret = platform_ble_connect(addr_type, addr);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_add_service(lua_State *L)
{
    bool ret = 0;
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_s = luaL_checkinteger(L, 1);
        ret = platform_ble_add_service(uuid_s);
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_s = luaL_checkstring(L, 1);
        ret = platform_ble_add_service_string(uuid_s);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_add_characteristic(lua_State *L)
{
    bool ret = 0;
    const u8 type = (u8)luaL_checkinteger(L, 2);
    const u16 permission = luaL_optint(L, 3, 0);
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_c = luaL_checkinteger(L, 1);
        ret = platform_ble_add_characteristic(uuid_c, type,permission);
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_c = luaL_checkstring(L, 1);
        ret = platform_ble_add_characteristic_string(uuid_c, type,permission);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_add_descriptor(lua_State *L)
{
    bool ret = 0;
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_d = luaL_checkinteger(L, 1);
        if((uuid_d == 0x2901)||(uuid_d == 0x2904))
        {
            const char *value = luaL_checkstring(L, 2);
            ret = platform_ble_add_descriptor(uuid_d,value,0);
        }
        else
        {
            const u16 configurationBits = luaL_optint(L, 2, 0);
            ret = platform_ble_add_descriptor(uuid_d,NULL,configurationBits);
        }
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_d = luaL_checkstring(L, 1);
        const u16 uuid = uuid_d[0] << 8 | uuid_d[1];
        if((uuid_d == 0x2901)||(uuid_d == 0x2904))
        {
            const char *value = luaL_checkstring(L, 2);
            ret = platform_ble_add_descriptor_string(uuid_d,value,0);
        }
        else
        {
            const u16 configurationBits = luaL_optint(L, 2, 0);
            ret = platform_ble_add_descriptor(uuid_d,NULL,configurationBits);
        }
        
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_find_characteristic(lua_State *L)
{
    bool ret = 0;
    const u16 handle = luaL_optint(L, 2, 0);
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_s = luaL_checkinteger(L, 1);
        ret = platform_ble_find_characteristic(uuid_s, handle);
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_s = luaL_checkstring(L, 1);
        ret = platform_ble_find_characteristic_string(uuid_s, handle);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_find_service(lua_State *L)
{
    const u16 handle = luaL_optint(L, 1, 0);
    bool ret = platform_ble_find_service(handle);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_open_notification(lua_State *L)
{
    bool ret = 0;
    const u16 handle = luaL_optint(L, 2, 0);
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_c = luaL_checkinteger(L, 1);
        ret = platform_ble_open_notification(uuid_c, handle);
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_c = luaL_checkstring(L, 1);
        ret = platform_ble_open_notification_string(uuid_c, handle);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_close_notification(lua_State *L)
{
    bool ret = 0;
    const u16 handle = luaL_optint(L, 2, 0);
    if (lua_type(L, 1) == LUA_TNUMBER)
    {
        const u16 uuid_c = luaL_checkinteger(L, 1);
        ret = platform_ble_close_notification(uuid_c, handle);
    }
    else if (lua_type(L, 1) == LUA_TSTRING)
    {
        const char *uuid_c = luaL_checkstring(L, 1);
        ret = platform_ble_close_notification_string(uuid_c, handle);
    }
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

static int l_ble_get_addr(lua_State *L) {

    plat_bt_addr addr;
    char Rspaddr[20] = {'\0'};
    platform_ble_get_addr(&addr);
    sprintf(Rspaddr, "%02x:%02x:%02x:%02x:%02x:%02x", addr.addr[0],addr.addr[1],addr.addr[2],addr.addr[3],addr.addr[4],addr.addr[5]);

    luaL_Buffer b;
    luaL_buffinit(L, &b);
    luaL_addlstring(&b,Rspaddr,strlen(Rspaddr));
    luaL_pushresult(&b);
    return 1;
}

static int l_ble_set_beacon_data(lua_State *L)
{
    bool ret = 0;
    const char *uuid = luaL_checkstring(L, 1);
    const u16 major = luaL_checkinteger(L, 2);
    const u16 minor = luaL_checkinteger(L, 3);
    ret = platform_ble_set_beacon_data(uuid,major,minor);
    lua_pushinteger(L, ret ? 0 : -1);
    return 1;
}

#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE bluetooth_map[] =
    {
        {LSTRKEY("open"), LFUNCVAL(l_ble_open)},
        {LSTRKEY("close"), LFUNCVAL(l_ble_close)},
        {LSTRKEY("send"), LFUNCVAL(l_ble_send)},
        {LSTRKEY("recv"), LFUNCVAL(l_ble_recv)},
        {LSTRKEY("setname"), LFUNCVAL(l_ble_set_name)},
        {LSTRKEY("setadvparam"), LFUNCVAL(l_ble_set_adv_param)},
        {LSTRKEY("setadvdata"), LFUNCVAL(l_ble_set_adv_data)},
        {LSTRKEY("setscanrspdata"), LFUNCVAL(l_ble_set_scanrsp_data)},
        {LSTRKEY("advertising"), LFUNCVAL(l_ble_set_adv_enable)},
        {LSTRKEY("setscanparam"), LFUNCVAL(l_ble_set_scan_param)},
        {LSTRKEY("scan"), LFUNCVAL(l_ble_set_scan_enable)},
        {LSTRKEY("state"), LFUNCVAL(l_ble_read_state)},
        {LSTRKEY("disconnect"), LFUNCVAL(l_ble_disconnect)},
        {LSTRKEY("connect"), LFUNCVAL(l_ble_connect)},
        {LSTRKEY("addservice"), LFUNCVAL(l_ble_add_service)},
        {LSTRKEY("addcharacteristic"), LFUNCVAL(l_ble_add_characteristic)},
        {LSTRKEY("adddescriptor"), LFUNCVAL(l_ble_add_descriptor)},
        {LSTRKEY("findcharacteristic"), LFUNCVAL(l_ble_find_characteristic)},
        {LSTRKEY("findservice"), LFUNCVAL(l_ble_find_service)},
        {LSTRKEY("opennotification"), LFUNCVAL(l_ble_open_notification)},
        {LSTRKEY("closenotification"), LFUNCVAL(l_ble_close_notification)},
        {LSTRKEY( "getaddr" ),  LFUNCVAL( l_ble_get_addr ) },
        {LSTRKEY( "setbeacondata" ),  LFUNCVAL( l_ble_set_beacon_data ) },
        {LNILKEY, LNILVAL}};

LUALIB_API int luaopen_bluetooth(lua_State *L)
{
    luaL_register(L, AUXLIB_BLUETOOTH, bluetooth_map);

    MOD_REG_NUMBER(L, "MSG_OPEN_CNF", OPENAT_BT_ME_ON_CNF);

    MOD_REG_NUMBER(L, "MSG_BLE_CONNECT_CNF", OPENAT_BLE_CONNECT);
    MOD_REG_NUMBER(L, "MSG_BLE_CONNECT_IND", OPENAT_BLE_CONNECT_IND);
    MOD_REG_NUMBER(L, "MSG_BLE_DISCONNECT_CNF", OPENAT_BLE_DISCONNECT);
    MOD_REG_NUMBER(L, "MSG_BLE_DISCONNECT_IND", OPENAT_BLE_DISCONNECT_IND);
    MOD_REG_NUMBER(L, "MSG_BLE_DATA_IND", OPENAT_BLE_RECV_DATA);
    MOD_REG_NUMBER(L, "MSG_BLE_SCAN_CNF", OPENAT_BLE_SET_SCAN_ENABLE);
    MOD_REG_NUMBER(L, "MSG_BLE_SCAN_IND", OPENAT_BLE_SET_SCAN_REPORT);
    MOD_REG_NUMBER(L, "MSG_BLE_FIND_CHARACTERISTIC_IND", OPENAT_BLE_FIND_CHARACTERISTIC_IND);
    MOD_REG_NUMBER(L, "MSG_BLE_FIND_CHARACTERISTIC_UUID_IND", OPENAT_BLE_FIND_CHARACTERISTIC_UUID_IND);
    MOD_REG_NUMBER(L, "MSG_BLE_FIND_SERVICE_IND", OPENAT_BLE_FIND_SERVICE_IND);
    return 1;
}
#endif
