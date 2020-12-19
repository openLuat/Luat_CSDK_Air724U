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

#ifndef _BOOT_CONFIG_H_
#define _BOOT_CONFIG_H_

#include "hal_config.h"

// Auto generated. Don't edit it manually!

/**
 * Whether to enable debug event in bootloader and fdl
 */
/* #undef CONFIG_BOOT_EVENT_ENABLED */

/**
 * Whether to enable trace in bootloader and fdl
 */
/* #undef CONFIG_BOOT_TRACE_ENABLED */

/**
 * bootloader image start address (8910, 8811)
 *
 * This is the bootloader loaded address in SRAM, rather than the address
 * on flash. Also, it is start address of image header. It should match
 * system ROM.
 */
#define CONFIG_BOOT_IMAGE_START 0x8000c0

/**
 * bootloader maximum image size
 */
#define CONFIG_BOOT_IMAGE_SIZE 0xbf40

/**
 * BOOT2 (decompressed) image address
 */
#define CONFIG_BOOT2_IMAGE_START 0x810000

/**
 * BOOT2 (decompressed) image size
 */
#define CONFIG_BOOT2_IMAGE_SIZE 0x28000

/**
 * FDL1 image address in SRAM (8910)
 *
 * It should match system ROM.
 */
#define CONFIG_FDL1_IMAGE_START 0x838000

/**
 * FDL1 maximum image size (8910)
 */
#define CONFIG_FDL1_IMAGE_SIZE 0x8000

/**
 * FDL2 image address in SRAM (8910)
 */
#define CONFIG_FDL2_IMAGE_START 0x810000

/**
 * FDL2 maximum image size (8910)
 */
#define CONFIG_FDL2_IMAGE_SIZE 0x28000

/**
 * NORFDL image address in SRAM (8811)
 *
 * It should match system ROM.
 */
/* #undef CONFIG_NORFDL_IMAGE_START */

/**
 * NORFDL maximum image size (8811)
 */
/* #undef CONFIG_NORFDL_IMAGE_SIZE */

/**
 * bootloader SVC stack size (8910)
 */
/* #undef CONFIG_BOOT_SVC_STACK_SIZE */

/**
 * bootloader IRQ stack size (8910)
 */
#define CONFIG_BOOT_IRQ_STACK_SIZE 0x800

/**
 * bootloader blue screen stack size (8910)
 */
#define CONFIG_BOOT_BLUE_SCREEN_STACK_SIZE 0x800

/**
 * bootloader SYS stack top address (8910)
 */
#define CONFIG_BOOT_SYS_STACK_TOP 0x810000

/**
 * bootloader IRQ stack top address (8910)
 */
#define CONFIG_BOOT_IRQ_STACK_TOP 0x80f000

/**
 * bootloader blue screen stack top address (8910)
 */
#define CONFIG_BOOT_BLUE_SCREEN_STACK_TOP 0x80e800

/**
 * NORFDL stack size (8811)
 */
/* #undef CONFIG_NORFDL_STACK_SIZE */

/**
 * whether to enable timer interrupt in bootloader (8910)
 */
/* #undef CONFIG_BOOT_TIMER_IRQ_ENABLE */

/**
 * bootloader timer interval in milliseconds (8910)
 */
/* #undef CONFIG_BOOT_TIMER_PERIOD */

/**
 * maximum FDL packet length
 */
#define CONFIG_FDL1_PACKET_MAX_LEN 0x840

/**
 * maximum FDL packet length
 */
#define CONFIG_FDL_PACKET_MAX_LEN 0x3000

/**
 * fixed nv bin maximum size in bytes (8910, 8811)
 */
#define CONFIG_NVBIN_FIXED_SIZE 0x20000

/**
 * FDL1, FDL2 default uart device (8910)
 */
#define CONFIG_FDL_DEFAULT_UART DRV_NAME_UART2

/**
 * FDL1, FDL2 default uart baud rate (8910)
 */
#define CONFIG_FDL_UART_BAUD 921600

/**
 * Enable SMPL(Sudden Momentary Power Loss)(8910)
 */
/* #undef CONFIG_BOOT_SMPL_ENABLE */

/**
 * Set SMPL thimer threshold (8910)
 */
#define CONFIG_BOOT_SMPL_THRESHOLD 7

/**
 * Enable pbint 7s reset (8910)
 */
/* #undef CONFIG_BOOT_PB_7S_RESET_ENABLE */

/**
 * Enable UART download in bootloader through PDL protocol
 */
/* #undef CONFIG_BOOT_UART1_PDL_ENABLE */

#endif
