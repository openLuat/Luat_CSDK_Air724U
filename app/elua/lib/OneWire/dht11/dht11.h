#ifndef __ONEWIRE_DHT11_H
#define __ONEWIRE_DHT11_H

#include "OneWire.h"

//pin取值0、1、2、3、7对应GPIO0、GPIO1、GPIO2、GPIO3、GPIO7，这几个都可以正常读取单总线，其他的没有测试

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

uint8 DHT11_GetData_Num(uint8 pin, uint8 *HumNum, uint8 *TemNum);
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
uint8 DHT11_GetData_String(uint8 pin, char *HumStr, char *TemStr);
#endif
