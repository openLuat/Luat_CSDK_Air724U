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

#ifndef _AUDIO_CONFIG_H_
#define _AUDIO_CONFIG_H_

// Auto generated. Don't edit it manually!

/**
 * whether audio feature is enabled
 */
#define CONFIG_AUDIO_ENABLE

/**
 * audio work queue stack size
 */
#define CONFIG_AUDIO_WQ_STACK_SIZE 16384

/**
 * whether MP3 decoder enabled
 */
#define CONFIG_AUDIO_MP3_DEC_ENABLE

/**
 * whether AMR-NB decoder enabled
 */
#define CONFIG_AUDIO_AMRNB_DEC_ENABLE

/**
 * whether AMR-WB decoder enabled
 */
#define CONFIG_AUDIO_AMRWB_DEC_ENABLE

/**
 * whether SBC decoder enabled
 */
#define CONFIG_AUDIO_SBC_DEC_ENABLE

/**
 * whether AMR-NB encoder enabled
 */
#define CONFIG_AUDIO_AMRNB_ENC_ENABLE

/**
 * whether AMR-WB encoder enabled
 */
#define CONFIG_AUDIO_AMRWB_ENC_ENABLE
#ifdef CONFIG_AUDIO_ENABLE
/**
 * whether ext i2s enable
 */
/* #undef CONFIG_AUDIO_EXT_I2S_ENABLE */
#endif
#ifdef CONFIG_AUDIO_ENABLE
/**
 * whether customer config audio ipc trigger mode enable
 */
/* #undef CONFIG_AUDIO_IPC_SET_TRIGGER_MODE_ENABLE */
#endif
#endif
