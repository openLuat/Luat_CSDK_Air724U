
#include "lv_gui_main.h"
#include "lv_test_theme_1.h"
#include "iot_debug.h"
#include "iot_lcd.h"
#include "iot_os.h"

static UINT32 lcdRegTable[] =
{
    0x00020011,
    0x00010078,
    0x000200B1,
    0x00030002,
    0x00030035,
    0x00030036,
    0x000200B2,
    0x00030002,
    0x00030035,
    0x00030036,
    0x000200B3,
    0x00030002,
    0x00030035,
    0x00030036,
    0x00030002,
    0x00030035,
    0x00030036,
    0x000200B4,
    0x00030007,
    0x000200C0,
    0x000300A2,
    0x00030002,
    0x00030084,
    0x000200C1,
    0x000300C5,
    0x000200C2,
    0x0003000A,
    0x00030000,
    0x000200C3,
    0x0003008A,
    0x0003002A,
    0x000200C4,
    0x0003008A,
    0x000300EE,
    0x000200C5,
    0x0003000E,
    0x00020036,
    0x000300C0,
    0x000200E0,
    0x00030012,
    0x0003001C,
    0x00030010,
    0x00030018,
    0x00030033,
    0x0003002C,
    0x00030025,
    0x00030028,
    0x00030028,
    0x00030027,
    0x0003002F,
    0x0003003C,
    0x00030000,
    0x00030003,
    0x00030003,
    0x00030010,
    0x000200E1,
    0x00030012,
    0x0003001C,
    0x00030010,
    0x00030018,
    0x0003002D,
    0x00030028,
    0x00030023,
    0x00030028,
    0x00030028,
    0x00030026,
    0x0003002F,
    0x0003003B,
    0x00030000,
    0x00030003,
    0x00030003,
    0x00030010,
    0x0002003A,
    0x00030005,
    0x00020029,
};

extern void lvglAnimCreate();
extern void lvglKeyboardCreate();

static void lvglTestTheme()
{
    lv_theme_t *th = lv_theme_zen_init(210, NULL);
    lv_test_theme_1(th, lvGuiGetKeyPad());
}

int appimg_enter(void *param)
{
    iot_debug_print("[ui] appimg_enter");
    static lvGuiCfg_t cfg;
    cfg.init_cmds = lcdRegTable;
    cfg.cmds_size = sizeof(lcdRegTable) / sizeof(lcdRegTable[0]);
    cfg.lvGuiCreate = lvglTestTheme;

    cfg.lcd_param.width = 132;
    cfg.lcd_param.height = 162;
    cfg.lcd_param.msgCallback = NULL;
    cfg.lcd_param.bus = OPENAT_LCD_SPI4LINE;

    lvGuiInit(&cfg);

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[ui] appimg_exit");
}

