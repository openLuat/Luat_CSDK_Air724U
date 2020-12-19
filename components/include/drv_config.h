/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _DRV_CONFIG_H_
#define _DRV_CONFIG_H_

// Auto generated. Don't edit it manually!

/**
 * debughost rx buffer size in bytes
 *
 * This is the buffer before parsing host packet format.
 */
#define CONFIG_DEBUGHOST_RX_BUF_SIZE 0x200

/**
 * whether to support charger
 */
#define CONFIG_SUPPORT_BATTERY_CHARGER

/**
 * whether to support camera single buffer
 */
/* #undef CONFIG_CAMERA_SINGLE_BUFFER */

/**
 * host command engine packet size
 */
#define CONFIG_HOST_CMD_ENGINE_MAX_PACKET_SIZE 0x2020

/**
 * uart TX baud rate at adaptive mode, before baud rate is detected
 */
#define CONFIG_UART_AUTOMODE_DEFAULT_BAUD 115200

/**
 * enable wcn wifi scan or not
 */
#define CONFIG_WCN_WIFI_SCAN_SUPPORT

/**
 * whether USB is supported
 */
#define CONFIG_USB_SUPPORT

#ifdef CONFIG_USB_SUPPORT

/**
 * usb connect timeout
 */
#define CONFIG_USB_CONNECT_TIMEOUT 10000

/**
 * usb debounce time in milliseconds before enumarating
 */
#define CONFIG_USB_DETECT_DEBOUNCE_TIME 800

/**
 * udc features, lower 8 bits for config->bmAttributes, higher are software defined
 */
#define CONFIG_USB_DEVICE_CONTROLLER_FEATURE 0xE0

/**
 * usb ether host mac address
 */
#define CONFIG_USB_ETHER_HOST_MAC "\xac\x4b\xb3\xb9\xeb\xe5"

/**
 * usb ether dev mac address
 */
#define CONFIG_USB_ETHER_DEV_MAC "\xfa\x32\x47\x15\xe1\x88"

/**
 * usb ether support dynamic change host mac address
 */
#define CONFIG_USB_ETHER_DYNAMIC_HOST_MAC_SUPPORT on

#endif

/**
 * support LCD driver
 */
#define CONFIG_LCD_SUPPORT

/**
 * support GC9305 panel
 */
#define CONFIG_SUPPORT_LCD_GC9305

/**
 * support ST7735S panel
 */
#define CONFIG_SUPPORT_LCD_ST7735S

/**
 * whether headset detect is supported
 */
#define CONFIG_HEADSET_DETECT_SUPPORT
#ifdef CONFIG_HEADSET_DETECT_SUPPORT
/**
 * headset thread stack size in bytes
 */
#define CONFIG_HEADSET_STACK_SIZE 16384
#endif
#endif
