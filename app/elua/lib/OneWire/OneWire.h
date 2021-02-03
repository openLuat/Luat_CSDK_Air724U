#ifndef __ONEWIRE_H
#define __ONEWIRE_H


#include "am_openat.h"

/**
 * @description: 输出调试信息
 */
#define OneWire_printf OPENAT_print

/**
 * @description: 设置单总线信号线为输入。默认上拉
 * @param :pin{uint8}：要操作的引脚，可选范围0、1、2、3、7
 * @return: TRUE：正确
 *          FALSE：引脚不在允许范围
 */
BOOL OneWire_IO_IN(uint8 pin);

/**
 * @description: 设置单总线信号线为输出。默认输出高定平
 * @param :pin{uint8}：要操作的引脚，可选范围0、1、2、3、7
 * @return: 无
 */
void OneWire_IO_OUT(uint8 pin);

/**
 * @description: 单总线输出高低电平
 * @param : pin{uint8}：要操作的引脚，可选范围0、1、2、3、7
 *          level{uint8}}：要输出的电平。0或者1
 * @return: 无
 */
void OneWire_DQ_OUT(uint8 pin, uint8 level);

/**
 * @description: 读取单总线的高低电平信号
 * @param : pin{uint8}：要操作的引脚，可选范围0、1、2、3、7
 * @return: 0或者1
 */
bool OneWire_DQ_IN(uint8 pin);

/**
 * @description: 单总线1us延时函数
 * @param : us{uint32}：调用的次数。
 * @return: 无
 */
void OneWire_Delay_1us(volatile uint32 us);
#endif
