/***************
	demo_hello
****************/
#include <string.h>
#include "iot_debug.h"
#include "iot_os.h"

#include "am_openat_drv.h"

#include "iot_vat.h"

#include "iot_keypad.h"

typedef struct
{
    BOOL bPressed;
    UINT8 matrix_r;
    UINT8 matrix_c;
} POC_KEYPAD_MESSAGE;

HANDLE poc_lcd_test_hand = NULL;

#define LCD_WIDTH 128
#define LCD_HEIGH 160
#define LCD_PIXEL_BYTES 2 //两字节16色

char lcd_buf[LCD_WIDTH * LCD_HEIGH * LCD_PIXEL_BYTES] = {0};

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 //棕色
#define BRRED 0XFC07 //棕红色
#define GRAY 0X8430  //灰色

u16 disp_bkcolor[] = {WHITE, BLACK, BLUE, BRED, GRED, GBLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY};

const UINT32 lcdRegTable[] =
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

static void write_command_table(const UINT32 *table, UINT16 size)
{
    UINT16 flag;
    UINT16 value;
    UINT16 index;

    for (index = 0; index < size && table[index] != (UINT32)-1; index++)
    {
        flag = table[index] >> 16;
        value = table[index] & 0xffff;

        switch (flag)
        {
        case 1:
            iot_os_sleep(value);
            break;

        case 0:
        case 2:
            iot_lcd_write_cmd(value & 0x00ff);
            break;

        case 3:
            iot_lcd_write_data(value & 0x00ff);
            break;

        default:
            break;
        }
    }
}

BOOL lcdInit(void)
{
    T_AMOPENAT_COLOR_LCD_PARAM param;
    iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VLCD, 15);
    memset(&param, 0, sizeof(param));
    param.width = LCD_WIDTH;
    param.height = LCD_HEIGH;
    param.msgCallback = NULL;
    param.bus = OPENAT_LCD_SPI4LINE;
    iot_lcd_color_init(&param);
    write_command_table(lcdRegTable, sizeof(lcdRegTable) / sizeof(int));
    return TRUE;
}

void poc_lcd_test()
{
    lcdInit();
    T_AMOPENAT_LCD_RECT_T rect = {0};
    rect.ltX = 0;
    rect.ltY = 0;
    rect.rbX = LCD_WIDTH;
    rect.rbY = LCD_HEIGH;

    uint8 color = 0;
    POC_KEYPAD_MESSAGE *mymsg = NULL;
    while (1)
    {
        iot_os_wait_message(poc_lcd_test_hand, &mymsg);
        iot_debug_print("[poc-key] mymsg->bPressed :%d", mymsg->bPressed);
        iot_debug_print("[poc-key] mymsg->matrix_r :%d", mymsg->matrix_r);
        iot_debug_print("[poc-key] mymsg->matrix_c :%d", mymsg->matrix_c);

        if (mymsg->matrix_r == 1 && mymsg->matrix_c == 0)
        {
            iot_debug_print("[poc-key] Key MENU is pressed");
            color = 3;
        }
        else if (mymsg->matrix_r == 1 && mymsg->matrix_c == 1)
        {
            iot_debug_print("[poc-key] Key OK is pressed");
            color = 4;
        }
        else if (mymsg->matrix_r == 1 && mymsg->matrix_c == 2)
        {
            iot_debug_print("[poc-key] Key EXIT is pressed");
            color = 5;
        }
        else if (mymsg->matrix_r == 2 && mymsg->matrix_c == 0)
        {
            iot_debug_print("[poc-key] Key + is pressed");
            color = 6;
        }
        else if (mymsg->matrix_r == 2 && mymsg->matrix_c == 1)
        {
            iot_debug_print("[poc-key] Key O is pressed");
            color = 7;
        }        
        else if (mymsg->matrix_r == 2 && mymsg->matrix_c == 2)
        {
            iot_debug_print("[poc-key] Key - is pressed");
            color = 8;
        }
        u16 *pPixel16 = (u16 *)lcd_buf;
        for (u16 col = 0; col < LCD_WIDTH; col++)
        {
            for (u16 row = 0; row < LCD_HEIGH; row++)
            {
                pPixel16[row * LCD_WIDTH + col] = disp_bkcolor[color];
            }
        }
        iot_lcd_update_color_screen(&rect, lcd_buf);
    }
}

VOID keypad_cb(T_AMOPENAT_KEYPAD_MESSAGE *pKeypadMessage)
{
    static POC_KEYPAD_MESSAGE msg = {0};

    msg.bPressed = pKeypadMessage->bPressed;
    msg.matrix_c = pKeypadMessage->data.matrix.c;
    msg.matrix_r = pKeypadMessage->data.matrix.r;

    iot_os_send_message(poc_lcd_test_hand, &msg);
}

int appimg_enter(void *param)
{
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);

    //打开调试信息，默认关闭
    iot_vat_send_cmd((UINT8 *)"AT^TRACECTRL=0,1,1\r\n", sizeof("AT^TRACECTRL=0,1,1\r\n"));

    poc_lcd_test_hand = iot_os_create_task(poc_lcd_test, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "hello");

    T_AMOPENAT_KEYPAD_CONFIG keypadConfig = {0};
    //pConfig.config=
    keypadConfig.type = OPENAT_KEYPAD_TYPE_MATRIX;
    keypadConfig.pKeypadMessageCallback = keypad_cb;
    keypadConfig.config.matrix.keyInMask = 0x0f;
    keypadConfig.config.matrix.keyOutMask = 0x0f;
    BOOL err = iot_keypad_init(&keypadConfig);
    if (!err)
        iot_debug_print("[poc-key] iot_keypad_init false!");

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[poc-key] appimg_exit");
}
