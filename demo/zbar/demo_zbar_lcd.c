
#include "iot_os.h"
#include "iot_debug.h"

#include "demo_zbar.h"

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

    for(index = 0; index < size && table[index] != (UINT32)-1; index++)
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

void lcdLedSet(BOOL isOn)
{
	static BOOL init = FALSE;

	if (!init)
	{
		T_AMOPENAT_GPIO_CFG cfg = {0};

		cfg.mode = OPENAT_GPIO_OUTPUT;
		iot_gpio_config(LCD_LED, &cfg);
		init = TRUE;	
	}

	iot_gpio_set(LCD_LED, isOn);
}

BOOL lcdInit(void)
{
	T_AMOPENAT_COLOR_LCD_PARAM param;

	iot_pmd_poweron_ldo(OPENAT_LDO_POWER_VLCD,15);

	memset(&param, 0, sizeof(param));
    param.width = LCD_WIDTH;
    param.height = LCD_HEIGHT;
    param.msgCallback = NULL;
    param.bus = OPENAT_LCD_SPI4LINE; 
    iot_lcd_color_init(&param);
	
	write_command_table(lcdRegTable, sizeof(lcdRegTable)/sizeof(int));
	
	return TRUE;
}

