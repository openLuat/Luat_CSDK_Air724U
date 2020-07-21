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

// #define OSI_LOCAL_LOG_LEVEL OSI_LOG_LEVEL_DEBUG

#include "lv_gui_main.h"
#include "lvgl.h"
#include "hal_keypad_def.h"
#include "iot_debug.h"
#include "iot_pmd.h"
#include "iot_keypad.h"
#include "iot_os.h"
#include <string.h>
#include <stdlib.h>

#define LV_GUI_ASSERT(con) iot_debug_assert((con), (char *)__FUNCTION__, __LINE__)

typedef enum
{
    LV_GUI_MSG_TYPE_QUIT,
    LV_GUI_MSG_TYPE_CB,
} LV_GUI_MSG_TYPE_E;

typedef struct
{
    LV_GUI_MSG_TYPE_E type;
    CHAR value[1];
} lvGuiMsg_t;

typedef struct
{
    lvGuiCbFunc_t func;
    VOID *ctx;
} lvGuiCb_t;

typedef enum
{
    LV_GUI_KEY_STATE_RELEASE,
    LV_GUI_KEY_STATE_PRESS,
} lvGuiKeyState_t;

typedef struct
{
    bool screen_on;            	    // state of screen on
    bool keypad_pending;       	    // keypad pending, set in ISR, clear in thread
    bool anim_inactive;        	    // property of whether animation is regarded as inactive
    HANDLE thread;             	    // gui thread
    HANDLE task_timer;         	    // timer to trigger task handler
    lv_disp_buf_t disp_buf;    	    // display buffer
    lv_disp_t *disp;           	    // display device
    lv_indev_t *keypad;        	    // keypad device
    keyMap_t last_key;              // last key from ISR
    lvGuiKeyState_t last_key_state; // last key state from ISR
    lvGuiCfg_t *cfg;           	    // display config
    uint32_t screen_on_users;  	    // screen on user bitmap
    uint32_t inactive_timeout; 	    // property of inactive timeout
} lvGuiContext_t;

typedef struct
{
    uint8_t keypad;
    uint8_t lv_key;
} lvGuiKeypadMap_t;

/*
static const lvGuiKeypadMap_t gLvKeyMapx[] =
{
    {KEY_MAP_POWER, 0xf0},
    {KEY_MAP_SIM1, 0xf1},
    {KEY_MAP_SIM2, 0xf2},
    {KEY_MAP_0, '0'},
    {KEY_MAP_1, '1'},
    {KEY_MAP_2, '2'},
    {KEY_MAP_3, '3'},
    {KEY_MAP_4, '4'},
    {KEY_MAP_5, '5'},
    {KEY_MAP_6, '6'},
    {KEY_MAP_7, '7'},
    {KEY_MAP_8, '8'},
    {KEY_MAP_9, '9'},
    {KEY_MAP_STAR, '*'},
    {KEY_MAP_WELL, '#'},
    {KEY_MAP_OK, LV_KEY_ENTER},
    {KEY_MAP_LEFT, LV_KEY_LEFT},
    {KEY_MAP_RIGHT, LV_KEY_RIGHT},
    {KEY_MAP_UP, LV_KEY_UP},
    {KEY_MAP_DOWN, LV_KEY_DOWN},
    {KEY_MAP_SOFT_L, LV_KEY_PREV},
    {KEY_MAP_SOFT_R, LV_KEY_NEXT},
};
*/

static const lvGuiKeypadMap_t gLvKeyMap[] =
{
    {29, LV_KEY_LEFT},
    {5, LV_KEY_RIGHT},
    {31, LV_KEY_UP},
    {35, LV_KEY_DOWN},
    {25, LV_KEY_ENTER},
    {26, LV_KEY_PREV},
    {32, LV_KEY_NEXT},
};

static lvGuiContext_t gLvGuiCtx;

/**
 * flush display forcedly
 */
static void prvDispForceFlush(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    T_AMOPENAT_LCD_RECT_T rect;
    rect.ltX = 0;
    rect.ltY = 0;
    rect.rbX = d->disp->driver.hor_res;
    rect.rbY = d->disp->driver.ver_res;

    iot_lcd_update_color_screen(&rect, d->disp_buf.buf1);
}

/**
 * display device flush_cb
 */
static void prvDispFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    T_AMOPENAT_LCD_RECT_T rect;
    rect.ltX = area->x1;
    rect.ltY = area->y1;
    rect.rbX = area->x2;
    rect.rbY = area->y2;

    iot_lcd_update_color_screen(&rect, (UINT16 *)color_p);

    lv_disp_flush_ready(disp);
}

static void prvDispInit(const uint32_t *table, uint16_t size)
{
    uint16_t flag;
    uint16_t value;
    uint16_t index;

    for(index = 0; index < size && table[index] != (uint32_t)-1; index++)
    {
        flag = table[index]>>16;
        value = table[index]&0xffff;

        switch(flag)
        {
        case 1:
            iot_os_sleep(value);
            break;

        case 0:
        case 2:
            iot_lcd_write_cmd(value&0x00ff);
            break;

        case 3:
            iot_lcd_write_data(value&0x00ff);
            break;

        default:
            break;
        }
    }
}


/**
 * initialize LCD display device
 */
static bool prvLvInitLcd(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VLCD,15);

    T_AMOPENAT_COLOR_LCD_PARAM *param = &d->cfg->lcd_param;
    if (!iot_lcd_color_init(param))
        return false;

    prvDispInit(d->cfg->init_cmds, d->cfg->cmds_size);

    unsigned pixel_cnt = param->width * param->height;
    lv_color_t *buf = (lv_color_t *)malloc(pixel_cnt * sizeof(lv_color_t));
    if (buf == NULL)
        return false;

    lv_disp_buf_init(&d->disp_buf, buf, buf, pixel_cnt);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = prvDispFlush;
    disp_drv.buffer = &d->disp_buf;
    disp_drv.hor_res = d->cfg->lcd_param.width;
    disp_drv.ver_res = d->cfg->lcd_param.height;
    d->disp = lv_disp_drv_register(&disp_drv); // pointer copy
    return true;
}

/**
 * callback of keypad driver, called in ISR
 */
static void prvKeypadCallback(T_AMOPENAT_KEYPAD_MESSAGE *event)
{
    lvGuiContext_t *d = &gLvGuiCtx;

#define MATRIX_TO_ID(r, c) (8 * (c) + (r) + 1)
    d->last_key = MATRIX_TO_ID(event->data.matrix.r, event->data.matrix.c);
    iot_debug_print("keypad lastkey %d", d->last_key);
    d->last_key_state = event->bPressed;
    d->keypad_pending = true;
}

/**
 * keypad device read_cb
 */
static bool prvLvKeypadRead(lv_indev_drv_t *kp, lv_indev_data_t *data)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    HANDLE critical = iot_os_enter_critical_section();
    keyMap_t last_key = d->last_key;
    lvGuiKeyState_t last_key_state = d->last_key_state;
    bool keypad_pending = d->keypad_pending;
    d->keypad_pending = false;
    iot_os_exit_critical_section(critical);

    if (keypad_pending)
    {
        data->state = (last_key_state & LV_GUI_KEY_STATE_PRESS) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
        data->key = 0xff;
        for (unsigned n = 0; n < OSI_ARRAY_SIZE(gLvKeyMap); n++)
        {
            if (gLvKeyMap[n].keypad == last_key)
            {
                data->key = gLvKeyMap[n].lv_key;
                break;
            }
        }
    }

    // no more to be read
    return false;
}

/**
 * initialize keypad device
 */
static bool prvLvInitKeypad(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    lv_indev_drv_t kp_drv;
    lv_indev_drv_init(&kp_drv);
    kp_drv.type = LV_INDEV_TYPE_KEYPAD;
    kp_drv.read_cb = prvLvKeypadRead;
    d->keypad = lv_indev_drv_register(&kp_drv); // pointer copy

    static T_AMOPENAT_KEYPAD_CONFIG cfg;
    cfg.type = OPENAT_KEYPAD_TYPE_MATRIX;
    cfg.pKeypadMessageCallback = prvKeypadCallback;
    cfg.config.matrix.keyInMask = 0x1f;
    cfg.config.matrix.keyOutMask = 0x1f;
    BOOL ret = iot_keypad_init(&cfg);
    LV_GUI_ASSERT(ret);

    return true;
}

static void prvLvRefr(lvGuiContext_t *d)
{
    lv_task_handler();
    uint32_t next = lv_task_get_tick_next_run();
    iot_os_start_timer(d->task_timer, next ? next : 10);
    lv_refr_now(d->disp);
}

/**
 * run littlevgl task handler
 */
static void prvLvTaskHandler(void *d)
{
    lvGuiThreadCallback((lvGuiCbFunc_t)prvLvRefr, d);
}

/**
 * whether inactive timeout
 */
static bool prvIsInactiveTimeout(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    if (d->screen_on_users != 0)
        return false;
    if (d->inactive_timeout == 0)
        return false;
    if (!d->anim_inactive && lv_anim_count_running())
        return false;
    return lv_disp_get_inactive_time(d->disp) > d->inactive_timeout;
}

/**
 * gui thread entry
 */
static void prvLvThreadEntry(void *param)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    d->cfg = (lvGuiCfg_t *)param;
    d->thread = iot_os_current_task();
    d->task_timer = iot_os_create_timer(prvLvTaskHandler, d);
    // d->task_timer = osiTimerCreate(d->thread, (osiCallback_t)prvLvTaskHandler, NULL);

    lv_init();
    if (!prvLvInitLcd())
        LV_GUI_ASSERT(0);

    prvLvInitKeypad();

    if (d->cfg->lvGuiCreate != NULL)
        d->cfg->lvGuiCreate();

    lv_disp_trig_activity(d->disp);
    iot_os_start_timer(d->task_timer, 500);
    for (;;)
    {
        lvGuiMsg_t *msg = NULL;
        lvGuiCb_t *cb = NULL;

        iot_os_wait_message(d->thread, (void **)&msg);

        switch (msg->type)
        {
        case LV_GUI_MSG_TYPE_CB:
            cb = (lvGuiCb_t *)msg->value;
            cb->func(cb->ctx);
            break;
        case LV_GUI_MSG_TYPE_QUIT:
            iot_os_free(msg);
            return;
        }

        iot_os_free(msg);

        if (d->screen_on && prvIsInactiveTimeout())
        {
            iot_debug_print("inactive timeout, screen off");
            lvGuiScreenOff();
        }
    }
}

/**
 * start gui based on littlevgl
 */
void lvGuiInit(lvGuiCfg_t *create)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    d->screen_on = true;
    d->keypad_pending = false;
    d->anim_inactive = false;

    d->screen_on_users = 0;
    d->inactive_timeout = CONFIG_LV_GUI_SCREEN_OFF_TIMEOUT;

    iot_os_create_task(prvLvThreadEntry, (PVOID) create,
                       CONFIG_LV_GUI_THREAD_STACK_SIZE, 1,
                       OPENAT_OS_CREATE_DEFAULT, "lvgl");
}

/**
 * get littlevgl gui thread
 */
HANDLE lvGuiGetThread(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;
    return d->thread;
}

/**
 * get keypad input device
 */
lv_indev_t *lvGuiGetKeyPad(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;
    return d->keypad;
}

/**
 * schedule a callback to be executed in gui thread
 */
void lvGuiThreadCallback(lvGuiCbFunc_t cb, void *param)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    lvGuiMsg_t *msg = iot_os_malloc(sizeof(lvGuiMsg_t) + sizeof(lvGuiCb_t) - 1);
    msg->type = LV_GUI_MSG_TYPE_CB;
    lvGuiCb_t *value = (lvGuiCb_t *)msg->value;
    value->func = cb;
    value->ctx = param;

    iot_os_send_message(d->thread, msg);
}

/**
 * request screen on
 */
bool lvGuiRequestSceenOn(uint8_t id)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    if (id > 31)
        return false;

    unsigned mask = (1 << id);
    d->screen_on_users |= mask;
    return true;
}

/**
 * release screen on request
 */
bool lvGuiReleaseScreenOn(uint8_t id)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    if (id > 31)
        return false;

    unsigned mask = (1 << id);
    d->screen_on_users &= ~mask;
    return true;
}

/**
 * turn off screen
 */
void lvGuiScreenOff(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    if (!d->screen_on)
        return;

    iot_debug_print("screen off");
    d->screen_on = false;
}

/**
 * turn on screen
 */
void lvGuiScreenOn(void)
{
    lvGuiContext_t *d = &gLvGuiCtx;

    if (d->screen_on)
        return;

    iot_debug_print("screen on");
    prvDispForceFlush();
    d->screen_on = true;
}

/**
 * set screen off timeout at inactive
 */
void lvGuiSetInactiveTimeout(unsigned timeout)
{
    lvGuiContext_t *d = &gLvGuiCtx;
    d->inactive_timeout = timeout;
}

/**
 * set whether animation is regarded as inactive
 */
void lvGuiSetAnimationInactive(bool inactive)
{
    lvGuiContext_t *d = &gLvGuiCtx;
    d->anim_inactive = inactive;
}
