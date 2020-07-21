/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_conf.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2012/10/8
 *
 * Description:
 * 
 **************************************************************************/

#ifndef __PLATFORM_CONF_H__
#define __PLATFORM_CONF_H__

#include "auxmods.h"

// *****************************************************************************
// 定义平台要开启的功能
#define BUILD_LUA_INT_HANDLERS
#define BUILD_C_INT_HANDLERS

/*+\NEW\liweiqiang\2013.12.6\对于超过500K的dl内存池,那么伪libc的malloc从dlmalloc分配 */
#if DLMALLOC_DEFAULT_GRANULARITY > (200*1024)
#define USE_DLMALLOC_ALLOCATOR
#else
#define USE_PLATFORM_ALLOCATOR
#endif
/*-\NEW\liweiqiang\2013.12.6\对于超过500K的dl内存池,那么伪libc的malloc从dlmalloc分配 */

// *****************************************************************************
// Configuration data

// Virtual timers (0 if not used)
#define VTMR_NUM_TIMERS       0

// Number of resources (0 if not available/not implemented)
#define NUM_PIO               3 // port 0:gpio0-31; port 1:gpio32-gpio55; port 2: gpio ex;
#define NUM_SPI               0
#define NUM_UART              4 //实际只有2个物理串口 id0-兼容旧版本为uart2 id1-uart1 id2-uart2 id3-hostuart
#define NUM_TIMER             2
#define NUM_PWM               0
#define NUM_ADC               8
#define NUM_CAN               0
#define NUM_I2C               3

#define PIO_PIN_EX            9 /*gpio ex 0~6,7,8*/
#define PIO_PIN_ARRAY         {32 /* gpio_num 32 */, 32/* gpio_num 56 */, 32}

//虚拟at命令通道
#define PLATFORM_UART_ID_ATC              0x7f

//host uart debug通道
#define PLATFORM_PORT_ID_DEBUG            0x80

/*+\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */
#define PLATFORM_PORT_ID_USB              0x81
/*-\NEW\shenyuanyuan\2019.5.8\将lua版本的usb AT口改为lua脚本可控制的普通数据传输口 */

//命令行通道
#define CON_UART_ID           (platform_get_console_port())
#define CON_UART_SPEED        115200
#define CON_TIMER_ID          0

// PIO prefix ('0' for P0, P1, ... or 'A' for PA, PB, ...)
#define PIO_PREFIX            '0'

/*+\NEW\liweiqiang\2013.7.16\增加iconv字符编码转换库 */
#ifdef LUA_ICONV_LIB
#define ICONV_LINE   _ROM( AUXLIB_ICONV, luaopen_iconv, iconv_map )
#else
#define ICONV_LINE   
#endif
/*-\NEW\liweiqiang\2013.7.16\增加iconv字符编码转换库 */

/*+\NEW\liweiqiang\2014.2.9\增加zlib库 */
#ifdef LUA_ZLIB_LIB
#define ZLIB_LINE   _ROM( AUXLIB_ZLIB, luaopen_zlib, zlib_map )
#else
#define ZLIB_LINE
#endif
/*-\NEW\liweiqiang\2014.2.9\增加zlib库 */

/*+\NEW\liweiqiang, panjun\2015.04.21\AM002_LUA不支持显示接口 */
#ifdef LUA_DISP_LIB
#define DISP_LIB_LINE   _ROM( AUXLIB_DISP, luaopen_disp, disp_map )
#else
#define DISP_LIB_LINE
#endif
/*-\NEW\liweiqiang, panjun\2015.04.21\AM002_LUA不支持显示接口 */

#define JSON_LIB_LINE   _ROM( AUXLIB_JSON, luaopen_cjson, json_map )
#define CRYPTO_LIB_LINE _ROM( AUXLIB_CRYPTO,luaopen_crypto, crypto_map)
#ifdef HRD_SENSOR_SUPPORT
#define HRD_SENSOR_LINE   _ROM( AUXLIB_HRSENSOR, luaopen_hrsensorcore, hrsensor_map )
#else
#define HRD_SENSOR_LINE
#endif

#if defined(AM_PBC_SUPPORT)
#define PBC_LIB_LINE   _ROM( AUXLIB_PBC, luaopen_protobuf_c, pbc_map )
#else
#define PBC_LIB_LINE
#endif

/*+\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */
#ifdef LUA_QRENCODE_SUPPORT
#define QRENCODE_LIB_LINE	_ROM( AUXLIB_QRENCODE, luaopen_qr_encode, qr_encode_map )	 
#else
#define QRENCODE_LIB_LINE
#endif
/*-\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */

#if !defined(LUAT_TTSFLOAT_SUPPORT)
#define WIFI_LIB_LINE	_ROM( AUXLIB_WIFI, luaopen_wificore, wifi_map)	 
#else
#define WIFI_LIB_LINE
#endif

#ifdef LUA_GPIO_I2C
#define GPIO_I2C_LINE _ROM( AUXLIB_GPIO_I2C, luaopen_gpio_i2c, gpio_i2c_map ) 
#else
#define GPIO_I2C_LINE
#endif

#ifdef LUA_GPS_LIB
#define GPS_LIB_LINE _ROM( AUXLIB_GPSCORE, luaopen_gpscore, gpscore_map ) 
#else
#define GPS_LIB_LINE
#endif

#define LUA_PLATFORM_LIBS_ROM \
    _ROM( AUXLIB_BIT, luaopen_bit, bit_map ) \
    _ROM( AUXLIB_SPI, luaopen_spi, spi_map) \
    _ROM( AUXLIB_BITARRAY, luaopen_bitarray, bitarray_map ) \
    _ROM( AUXLIB_PACK, luaopen_pack, pack_map ) \
    _ROM( AUXLIB_PIO, luaopen_pio, pio_map ) \
    _ROM( AUXLIB_UART, luaopen_uart, uart_map ) \
    _ROM( AUXLIB_I2C, luaopen_i2c, i2c_map ) \
    _ROM( AUXLIB_RTOS, luaopen_rtos, rtos_map ) \
    _ROM( AUXLIB_PMD, luaopen_pmd, pmd_map ) \
    _ROM( AUXLIB_ADC, luaopen_adc, adc_map ) \
    _ROM( AUXLIB_PWM, luaopen_pwm, pwm_map ) \
    _ROM( AUXLIB_TTSPLYCORE, luaopen_ttsplycore, ttsplycore_map) \
    _ROM( AUXLIB_AUDIOCORE, luaopen_audiocore, audiocore_map ) \
    DISP_LIB_LINE \
    ICONV_LINE \
    ZLIB_LINE \
    HRD_SENSOR_LINE \
    JSON_LIB_LINE \
    CRYPTO_LIB_LINE \
    PBC_LIB_LINE \
    QRENCODE_LIB_LINE \
    WIFI_LIB_LINE \
    GPIO_I2C_LINE \
    GPS_LIB_LINE \
    _ROM( AUXLIB_CPU, luaopen_cpu, cpu_map) \
    _ROM( AUXLIB_TCPIPSOCK, luaopen_tcpipsock, tcpipsock_map) \
    _ROM( AUXLIB_WATCHDOG, luaopen_watchdog, watchdog_map ) \
    _ROM( AUXLIB_FACTORY, luaopen_factorycore, factorycore_map)
 
    // Interrupt queue size
#define PLATFORM_INT_QUEUE_LOG_SIZE 5

#define CPU_FREQUENCY         ( 26 * 1000 * 1000 )

// Interrupt list
#define INT_GPIO_POSEDGE      ELUA_INT_FIRST_ID
#define INT_GPIO_NEGEDGE      ( ELUA_INT_FIRST_ID + 1 )
#define INT_ELUA_LAST         INT_GPIO_NEGEDGE
    
#define PLATFORM_CPU_CONSTANTS \
     _C( INT_GPIO_POSEDGE ),\
     _C( INT_GPIO_NEGEDGE )

#endif //__PLATFORM_CONF_H__
