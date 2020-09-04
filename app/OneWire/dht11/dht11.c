
#include "dht11.h"
#include "stdio.h"
#include "stdlib.h"

static uint8 dht11_data = -1;

static void DHT11_Rst(void)
{
    OneWire_IO_OUT(dht11_data);
    OneWire_DQ_OUT(dht11_data, 0);
    OneWire_Delay_1us(1000 * 25);
    OneWire_DQ_OUT(dht11_data, 1);
}

static uint8 DHT11_Check(void)
{
    volatile uint8 retry = 0;
    OneWire_IO_IN(dht11_data);
    while (OneWire_DQ_IN(dht11_data) && retry < 30)
    {
        retry++;
        OneWire_Delay_1us(10);
    }
    if (retry >= 30)
    {
        OneWire_printf("[dht11]DHT11_Check: retry1 >= 300us!");
        return 3;
    }

    retry = 0;
    while (!OneWire_DQ_IN(dht11_data) && retry < 30)
    {
        retry++;
        OneWire_Delay_1us(10);
    }
    if (retry >= 30)
    {
        OneWire_printf("[dht11]DHT11_Check: retry2 >= 300us!");
        return 3;
    }
    return 0;
}

static int DHT11_Read_Bit(void)
{
    volatile uint8 recvretry = 0;
    while (OneWire_DQ_IN(dht11_data) && recvretry < 20)
    {
        recvretry++;
        OneWire_Delay_1us(10);
    }
    if (recvretry >= 20)
    {
        OneWire_printf("[dht11]DHT11_Read_Bit: recvretry1 >= 200!");
        return 6;
    }
    recvretry = 0;

    while (!OneWire_DQ_IN(dht11_data) && recvretry < 20)
    {
        recvretry++;
        OneWire_Delay_1us(10);
    }
    if (recvretry >= 20)
    {
        OneWire_printf("[dht11]DHT11_Read_Bit: recvretry2 >= 200!");
        return 6;
    }
    OneWire_Delay_1us(40);
    return OneWire_DQ_IN(dht11_data);
}

static char DHT11_Read_Byte(void)
{
    char i = 0, dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        char bit = DHT11_Read_Bit();
        if (bit == 6)
        {
            OneWire_printf("[dht11]DHT11_Read_Byte: DHT11_Read_Bit:%d", bit);
            return 0;
        }
        dat |= bit;
    }
    return dat;
}

//采集数据
static uint8 DHT11_Colle(char *data)
{

    DHT11_Rst();
    uint8 var = DHT11_Check();
    if (var != 0)
    {
        OneWire_printf("[dht11]DHT11_Colle: DHT11_Check:%d", var);
        return var;
    }

    data[0] = DHT11_Read_Byte(); // HumInt
    data[1] = DHT11_Read_Byte(); // HumFlo
    data[2] = DHT11_Read_Byte(); // TemInt
    data[3] = DHT11_Read_Byte(); // TemFlo
    data[4] = DHT11_Read_Byte(); // Check

    // 数据校验
    if (data[4] != data[0] + data[1] + data[2] + data[3])
    {
        OneWire_printf("[dht11]DHT11_Colle: data[4] != data[0] + data[1] + data[2] + data[3]");
        return 5;
    }
    return 0;
}

/**
 * @description: 从dht11得到温/湿度值数字
 * @param : pin{uint8}:要操作的引脚，可选范围0、1、2、3、7
 *          HumNum{uint8 *}:输出温度值
 *          TemNum{uint8 *}:输出湿度值
 * @return  0:正常获取
 *          2:传入的pin不在允许范围
 *          3:未检测到dht11
 *          4:数据校验错误
 */

uint8 DHT11_GetData_Num(uint8 pin, uint8 *HumNum, uint8 *TemNum)
{
    HANDLE crihand=NULL;
    char data[5] = {0};
    uint8 var = 0;
    if (pin == 0 || pin == 1 || pin == 2 || pin == 3 || pin == 7)
    {
        dht11_data = pin;
        goto start;
    }
    return 2;

start:
    crihand = OPENAT_enter_critical_section();
    var = DHT11_Colle(&data[0]);
    OPENAT_exit_critical_section(crihand);
    if (var != 0)
    {
        OneWire_printf("[dht11]DHT11_GetData_Num: DHT11_Colle:%d", var);
        return var;
    }
    // OneWire_printf("[dht11]DHT11_GetTemp_Num: data[0]:%d", data[0]);
    // OneWire_printf("[dht11]DHT11_GetTemp_Num: data[1]:%d", data[1]);
    // OneWire_printf("[dht11]DHT11_GetTemp_Num: data[2]:%d", data[2]);
    // OneWire_printf("[dht11]DHT11_GetTemp_Num: data[3]:%d", data[3]);
    // OneWire_printf("[dht11]DHT11_GetTemp_Num: data[4]:%d", data[4]);
    *HumNum = data[0];
    *TemNum = data[2];
    dht11_data = -1;
    return 0;
    
}

/**
 * @description: 从dht11得到温/湿度值字符串
 * @param : pin{uint8}:要操作的引脚，可选范围0、1、2、3、7
 *          HumStr{char *}:输出温度值字符串
 *          TemStr{char *}:输出湿度值字符串
 * @return  0:正常获取
 *          1:HumStr == NULL || TemStr == NULL
 *          2:传入的pin不在允许范围
 *          3:未检测到dht11
 *          4:数据校验错误
 */
uint8 DHT11_GetData_String(uint8 pin, char *HumStr, char *TemStr)
{
    if (HumStr == NULL || TemStr == NULL)
    {
        return 1;
    }

    uint8 HumNum = 0, TemNum = 0;
    uint8 var = DHT11_GetData_Num(pin, &HumNum, &TemNum);
    if (var != 0)
    {
        OneWire_printf("[dht11]DHT11_GetData_String: DHT11_GetData_Num:%d", var);
        return var;
    }
    char str1[10] = {0};
    itoa(HumNum, str1, 10);
    char str2[10] = {0};
    itoa(TemNum, str2, 10);
    char len = sprintf(HumStr, "%s%% RH", str1);
    HumStr[(int)len] = '\0';
    len = sprintf(TemStr, "%s C", str2);
    TemStr[(int)len] = '\0';
    return 0;
}