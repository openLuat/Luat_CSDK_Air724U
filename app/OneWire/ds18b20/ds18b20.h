#ifndef __ONEWIRE_DS18B20_H
#define __ONEWIRE_DS18B20_H

#include "OneWire.h"


//pin取值0、1、2、3、7对应GPIO0、GPIO1、GPIO2、GPIO3、GPIO7，这几个都可以正常读取单总线，其他的没有测试


/**
 * @description: 从ds18b20得到温度值数字，精确到0.0625。结果被扩大10000倍
 * @param : pin{uint8}:要操作的引脚，可选范围0、1、2、3、7
 *          TempNum{int *}:输出温度值
 * @return  0:正常获取
 *          2:传入的pin不在允许范围
 *          3:未检测到ds18b20
 */
uint8 DS18B20_GetTemp_Num(uint8 pin, int *TempNum);


/**
 * @description: 从ds18b20得到温度值字符串
 * @param : pin{uint8}:要操作的引脚，可选范围0、1、2、3、7
 *          TempNum{char *}:输出温度字符串
 * @return  0:正常获取
 *          1:TempStr=NULL
 *          2:传入的pin不在允许范围
 *          3:未检测到ds18b20
 */
uint8 DS18B20_GetTemp_String(uint8 pin, char *TempStr);
#endif
