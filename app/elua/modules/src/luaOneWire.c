

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "auxmods.h"
#include "lrotable.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "OneWire.h"
#include "ds18b20.h"
#include "dht11.h"

// #pragma GCC push_options
// #pragma GCC optimize("O0")
// static int Delay1us(lua_State *L)
// {
//     uint32 num = luaL_checkinteger(L, 1);
//     if (num >= 5 && num < 20)
//     {
//         for (volatile int i = 60 * num - 70 * 4; i > 0; i--)
//             ;
//     }
//     else if (num >= 20 && num < 100)
//     {
//         for (volatile int i = 60 * num - 90 * 4; i > 0; i--)
//             ;
//     }
//     else if (num >= 100)
//     {
//         for (volatile int i = 63 * num - 90 * 4; i > 0; i--)
//             ;
//     }
//     lua_pushinteger(L, 1);
//     return 1;
// }
// #pragma GCC pop_options
static int read_ds18b20(lua_State *L)
{
    uint8 pin = luaL_checkinteger(L, 1);
    int TempNum = 0;
    int data1 = 0, data2 = 0;
    char str1[10] = {0};
    char str2[10] = {0};
    char TempStr[20] = {0};
    uint8 var = 0;
    var = DS18B20_GetTemp_Num(pin, &TempNum);
    switch (var)
    {
    case 0:
        data1 = TempNum / 10000;
        data2 = TempNum % 10000;
        itoa(data1, str1, 10);
        itoa(data2, str2, 10);
        sprintf(TempStr, "%s.%s C", str1, str2);
        lua_pushinteger(L, TempNum);
        lua_pushlstring(L, TempStr, strlen(TempStr) + 1);
        break;
    case 1:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]TempStr=NULL", sizeof("[OneWire]TempStr=NULL"));
        break;
    case 2:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]The pin passed in is not in the allowed range", sizeof("[OneWire]The pin passed in is not in the allowed range"));
        break;
    case 3:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]DS18B20 was not detected", sizeof("[OneWire]DS18B20 was not detected"));
        break;
    }

    return 2;
}

static int read_dht11(lua_State *L)
{
    uint8 pin = luaL_checkinteger(L, 1);

    uint8 HumNum = 0, TemNum = 0;
    char str1[10] = {0};
    char str2[10] = {0};
    char HumStr[20] = {0};
    char TemStr[20] = {0};
    uint8 var = 0;
    var = DHT11_GetData_Num(pin, &HumNum, &TemNum);
    switch (var)
    {
    case 0:
        itoa(HumNum, str1, 10);
        itoa(TemNum, str2, 10);
        char len = sprintf(HumStr, "%s%% RH", str1);
        HumStr[(int)len] = '\0';
        len = sprintf(TemStr, "%s C", str2);
        TemStr[(int)len] = '\0';
        lua_pushinteger(L, HumNum);
        lua_pushlstring(L, HumStr, strlen(HumStr) + 1);
        lua_pushinteger(L, TemNum);
        lua_pushlstring(L, TemStr, strlen(TemStr) + 1);
        break;
    case 1:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]HumStr == NULL || TemStr == NULL", sizeof("[OneWire]HumStr == NULL || TemStr == NULL"));
        return 2;
    case 2:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]The pin passed in is not in the allowed range", sizeof("[OneWire]The pin passed in is not in the allowed range"));
        return 2;
    case 3:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]DHT11 was not detected", sizeof("[OneWire]DHT11 was not detected"));
        return 2;
    case 4:
        lua_pushinteger(L, 0xffff);
        lua_pushlstring(L, "[OneWire]Data verification error", sizeof("[OneWire]Data verification error"));
        return 2;
    }
    return 4;
}

// static int IOIN(lua_State *L)
// {
//     uint8 pin = luaL_checkinteger(L, 1);
//     lua_pushinteger(L, OneWire_IO_IN(pin));
//     return 1;
// }

// static int IOOUT(lua_State *L)
// {
//     uint8 pin = luaL_checkinteger(L, 1);
//     OneWire_IO_OUT(pin);
//     lua_pushinteger(L, 1);
//     return 1;
// }

// static int DQOUT(lua_State *L)
// {
//     uint8 pin = luaL_checkinteger(L, 1);
//     uint8 level = luaL_checkinteger(L, 2);
//     OneWire_DQ_OUT(pin, level);
//     lua_pushinteger(L, 1);
//     return 1;
// }

// static int DQIN(lua_State *L)
// {
//     uint8 pin = luaL_checkinteger(L, 1);
//     lua_pushinteger(L, OneWire_DQ_IN(pin));
//     return 1;
// }
#include "lrodefs.h"
const LUA_REG_TYPE onewire_map[] = {
    // {LSTRKEY("Delay1us"), LFUNCVAL(Delay1us)},
    // {LSTRKEY("IOIN"), LFUNCVAL(IOIN)},
    // {LSTRKEY("IOOUT"), LFUNCVAL(IOOUT)},
    // {LSTRKEY("DQIN"), LFUNCVAL(DQIN)},
    // {LSTRKEY("DQOUT"), LFUNCVAL(DQOUT)},
    {LSTRKEY("read_ds18b20"), LFUNCVAL(read_ds18b20)},
    {LSTRKEY("read_dht11"), LFUNCVAL(read_dht11)},
    {LNILKEY, LNILVAL}};

LUALIB_API int luaopen_onewire(lua_State *L)
{
    luaL_register(L, AUXLIB_OneWire, onewire_map);
    return 1;
}
