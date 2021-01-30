/***************
	demo_hello
****************/
#include <string.h>
#include "iot_debug.h"
#include "iot_os.h"

#include "am_openat_drv.h"

#include "iot_vat.h"

#include "iot_keypad.h"
#include "am_openat.h"

#define SW_BYA 18
#define SW_BYB 19   

typedef struct
{
    BOOL bPressed;
    UINT8 swkey;
} POC_SWKEY_MESSAGE;

HANDLE poc_lcd_test_hand = NULL;

#define LCD_WIDTH 162
#define LCD_HEIGH 132
#define LCD_PIXEL_BYTES 2 //两字节16色

char lcd_buf[LCD_WIDTH * LCD_HEIGH * LCD_PIXEL_BYTES] = {0};

#define WHITE 0xFFFF   //白色
#define BLACK 0x0000   //黑色
#define BLUE 0x001F    //蓝色
#define BRED 0X8802    //黑红
#define GRED 0XFBE4    //绿红
#define GBLUE 0X9EDD   //蓝绿
#define RED 0xF800     //红色
#define MAGENTA 0xF81F //洋红
#define GREEN 0x07E0   //绿色
#define CYAN 0x051D    //青色
#define YELLOW 0xFFE0  //黄色
#define BROWN 0XBC40   //棕色
#define BRRED 0XFC07   //棕红
#define GRAY 0X8430    //灰色

u16 disp_bkcolor[] = {WHITE, BLACK, BLUE, BRED, GRED, GBLUE, RED, MAGENTA, GREEN, CYAN, YELLOW, BROWN, BRRED, GRAY};

const UINT32 lcdRegTable[] =
    {
        0x00020011,
        0x00010078,
        0x000200B1,
        0x00030005,
        0x0003003a,
        0x0003003a,
        0x000200B2,
        0x00030005,
        0x0003003a,
        0x0003003a,
        0x000200B3,
        0x00030005,
        0x0003003a,
        0x0003003a,
        0x00030005,
        0x0003003a,
        0x0003003a,
        0x000200B4,
        0x00030003,
        0x000200C0,
        0x00030062,
        0x00030002,
        0x00030004,
        0x000200C1,
        0x000300C0,
        0x000200C2,
        0x0003000D,
        0x00030000,
        0x000200C3,
        0x0003008D,
        0x0003006A,
        0x000200C4,
        0x0003008D,
        0x000300EE,
        0x000200C5,
        0x00030012,
        0x000200E0,
        0x00030003,
        0x0003001B,
        0x00030012,
        0x00030011,
        0x0003003F,
        0x0003003A,
        0x00030032,
        0x00030034,
        0x0003002F,
        0x0003002B,
        0x00030030,
        0x0003003A,
        0x00030000,
        0x00030001,
        0x00030002,
        0x00030005,
        0x000200E1,
        0x00030003,
        0x0003001B,
        0x00030012,
        0x00030011,
        0x00030032,
        0x0003002F,
        0x0003002A,
        0x0003002F,
        0x0003002E,
        0x0003002C,
        0x00030035,
        0x0003003F,
        0x00030000,
        0x00030000,
        0x00030001,
        0x00030005,
        0x000200FC,
        0x0003008c,
        0x0002003A,
        0x00030005,
        0x00020036,
        0x00030060,
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
    OPENAT_gpioPulse(13, 1, 6000, 1, 0); //BCT3220背光控制芯片复位
    OPENAT_gpioPulse(13, 40, 10, 1, 1);  //BCT3220背光控制一个下降沿，百分百亮度

    lcdInit();
    T_AMOPENAT_LCD_RECT_T rect = {0};
    rect.ltX = 0;
    rect.ltY = 0;
    rect.rbX = LCD_WIDTH;
    rect.rbY = LCD_HEIGH;

    uint8 color = 0;
    int msgId;
    POC_SWKEY_MESSAGE *mymsg = NULL;
    while (1)
    {
        OPENAT_wait_message(poc_lcd_test_hand, &msgId, &mymsg, OPENAT_OS_SUSPENDED);
        if (mymsg->swkey == SW_BYA)
        {
            OPENAT_wait_message(poc_lcd_test_hand, &msgId, &mymsg, 100);
            if (mymsg->swkey == SW_BYB)
            {
                iot_debug_print("[knob_lcd] The knob rotates clockwise");
                color++;
            }
        }
        else if (mymsg->swkey == SW_BYB)
        {
            OPENAT_wait_message(poc_lcd_test_hand, &msgId, &mymsg, 100);
            if (mymsg->swkey == SW_BYA)
            {
                iot_debug_print("[knob_lcd] The knob rotates counterclockwise");
                color--;
            }
        }

        u16 *pPixel16 = (u16 *)lcd_buf;
        for (u16 col = 0; col < LCD_WIDTH; col++)
        {
            for (u16 row = 0; row < LCD_HEIGH; row++)
            {
                pPixel16[row * LCD_WIDTH + col] = disp_bkcolor[color % (sizeof(disp_bkcolor) / sizeof(disp_bkcolor[0]))];
            }
        }
        iot_lcd_update_color_screen(&rect, lcd_buf);
    }
}

void gpio_handle(E_OPENAT_DRV_EVT evt, E_AMOPENAT_GPIO_PORT gpioPort, unsigned char state)
{
    // 判断是gpio中断
    if (evt == OPENAT_DRV_EVT_GPIO_INT_IND)
    {
        iot_debug_print("[knob_lcd] input handle gpio %d, state %d", gpioPort, state);
        static POC_SWKEY_MESSAGE msg = {0};
        msg.bPressed = state;
        msg.swkey = gpioPort;
        OPENAT_send_message(poc_lcd_test_hand, 0, &msg, 0);
    }
}

int appimg_enter(void *param)
{
    iot_debug_print("[knob_lcd]appimg_enter");
    //关闭看门狗，死机不会重启。默认打开
    iot_debug_set_fault_mode(OPENAT_FAULT_HANG);
    iot_os_sleep(100);
    //设置keypad,第3行,第2列为物理强制下载按键（OK按键）。开机时按下该按键会进入下载模式
    //poc项目没引出boot按键，必须调用一次设置keypad进入下载模式，否则变砖后只能拆机了。
    iot_vat_send_cmd("AT*DOWNLOAD=2,3,2\r\n", sizeof("AT*DOWNLOAD=2,3,2\r\n"));
    iot_os_sleep(100);
    //打开调试信息，默认关闭
    iot_vat_send_cmd("AT^TRACECTRL=0,1,2\r\n", sizeof("AT^TRACECTRL=0,1,2\r\n"));
    iot_os_sleep(100);

    poc_lcd_test_hand = iot_os_create_task(poc_lcd_test, NULL, 1024, 1, OPENAT_OS_CREATE_DEFAULT, "hello");

    T_AMOPENAT_GPIO_CFG input_cfg = {0};
    input_cfg.mode = OPENAT_GPIO_INPUT_INT;                     //配置输入中断
    input_cfg.param.intCfg.debounce = 10;                       //防抖10ms
    input_cfg.param.intCfg.intType = OPENAT_GPIO_INT_BOTH_EDGE; //中断触发方式双边沿
    input_cfg.param.intCfg.intCb = gpio_handle;                 //中断处理函数
    input_cfg.param.pullState = OPENAT_GPIO_NO_PULL;            //高阻态
    iot_gpio_open(SW_BYA, &input_cfg);                          //初始化gpio为高阻态中断输入模式
    iot_gpio_open(SW_BYB, &input_cfg);                          //初始化gpio为高阻态中断输入模式

    return 0;
}

void appimg_exit(void)
{
    iot_debug_print("[knob_lcd] appimg_exit");
}
