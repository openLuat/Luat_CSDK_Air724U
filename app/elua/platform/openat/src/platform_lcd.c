/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_lcd.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *           lcd接口实现
 * History:
 *     panjun 2015.05.20 Modify some APIs for color-LCD.
 *     panjun 2015.05.29 Add an 'ASSERT' for 'lcd_bus_xxx_update'.
 *     panjun 2015.06.06 Fix a display's flaw for mono-LCD.
 **************************************************************************/
#ifdef LUA_DISP_LIB
#include "string.h"
#include "malloc.h"
//#include "lcd_if_hw.h"
//#include "lcd_if.h"

#include "assert.h"
#include "lplatform.h"
#include "platform_lcd.h"
#include "am_openat_drv.h"

typedef void (*lcd_init_op)();
typedef void (*lcd_cmd_data_op)(uint16 val);
typedef void (*lcd_update_op)(PlatformRect *pRect, u8 *buffer);
typedef void (*lcd_write_buf_op)(uint8 *buf, uint32 len);

// +panjun, 2015.04.21, Commit SSD1306's driver code.
#define delayMilliseconds(ms)  IVTBL(sleep)(ms);

static u8 lcd_bpp;
static u16 lcd_x_offset;
static u16 lcd_y_offset;
/*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
static u32 *lcd_sleep_cmd;
static u32 *lcd_wake_cmd;
/*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */

static lcd_update_op    lcd_bus_update;
static lcd_cmd_data_op  lcd_bus_write_cmd = NULL;
static lcd_cmd_data_op  lcd_bus_write_data = NULL;
static lcd_write_buf_op lcd_bus_write_data_buf = NULL;

static PlatformLcdBus lcd_bus;
static lcd_itf_t lcd_bus_itf;
static uint8 lcd_bus_assert_fail=KAL_FALSE;

/*+\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */
uint8 lcd_first_update = 0;
/*-\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */

//extern E_AMOPENAT_GPIO_PORT platform_pio_get_gpio_port(int port_pin);

extern void spi_interface_init(void);

static void lcd_bus_spi4line_init(const PlatformDispInitParam *pParam);
static void lcd_bus_parallel_init(const PlatformDispInitParam *pParam);
static void lcd_bus_i2c_init(const PlatformDispInitParam *pParam);
static void lcd_bus_spi_init(const PlatformDispInitParam *pParam);
static void lcd_common_reset(E_AMOPENAT_GPIO_PORT pin_rst);

static const lcd_init_op lcd_init_table[PLATFORM_LCD_BUS_QTY] = {
    lcd_bus_spi4line_init,
    lcd_bus_parallel_init,
    lcd_bus_i2c_init,
    lcd_bus_spi_init,
};

void lcd_bus_color_blockwrite_set(u16 startX, u16 startY, u16 endX, u16 endY) {
    u16 ltx, lty, rbx, rby;

    if(lcd_bus_write_cmd == NULL || lcd_bus_write_data == NULL) return;

    ltx = startX + lcd_x_offset;
    lty = startY + lcd_y_offset;
    rbx = endX + lcd_x_offset;
    rby = endY + lcd_y_offset;

    lcd_bus_write_cmd(0x2A);
    lcd_bus_write_data(ltx>>8);
    lcd_bus_write_data(ltx&0x00ff);
    lcd_bus_write_data(rbx>>8);
    lcd_bus_write_data(rbx&0x00ff);
    
    lcd_bus_write_cmd(0x2B);
    lcd_bus_write_data(lty>>8);
    lcd_bus_write_data(lty&0x00ff);
    lcd_bus_write_data(rby>>8);
    lcd_bus_write_data(rby&0x00ff);
    
    lcd_bus_write_cmd(0x2C);
}

static void lcd_bus_color_update(PlatformRect *pRect, u8 *buffer){
    T_AMOPENAT_LCD_RECT_T rect;
    u16 i, j, data;
    ASSERT(lcd_bus_assert_fail==KAL_FALSE);
    lcd_bus_assert_fail = KAL_TRUE;

 	/*+\NEW\shenyuanyuan\2020.4.18\同一块蓝色的st7735的屏，使用相同的ui demo中的color_lcd_spi_st7735.lua驱动文件，在2G模块上显示正常，在cat.1模块上会有两列一行花屏*/   
    rect.ltX = pRect->ltx + lcd_x_offset;
    rect.ltY = pRect->lty + lcd_y_offset;
    rect.rbX = pRect->rbx + lcd_x_offset;
    rect.rbY = pRect->rby + lcd_y_offset;
    
    //lcd_bus_color_blockwrite_set(rect.ltX, rect.ltY, rect.rbX, rect.rbY);
	/*-\NEW\shenyuanyuan\2020.4.18\同一块蓝色的st7735的屏，使用相同的ui demo中的color_lcd_spi_st7735.lua驱动文件，在2G模块上显示正常，在cat.1模块上会有两列一行花屏*/
        
    IVTBL(update_color_lcd_screen)(&rect, (UINT16 *)buffer);

    lcd_bus_assert_fail = KAL_FALSE;
	
}
/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
static void lcd_spi_color_update(PlatformRect *pRect, u8 *buffer){
    // T_AMOPENAT_LCD_RECT_T rect;
    
    // rect.ltX = pRect->ltx;
    // rect.ltY = pRect->lty;
    // rect.rbX = pRect->rbx;
    // rect.rbY = pRect->rby;
    
    // lcd_write_update_area(pRect);
	
    // IVTBL(send_event)(0x1cd1cd1);
    // lcd_bus_write_data_buf(buffer, (rect.rbY - rect.ltY + 1)* (rect.rbX - rect.ltX + 1)  * (lcd_bpp / 8));
    // IVTBL(send_event)(0x1cd1cde);
}
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
static void lcd_bus_mono_update(PlatformRect *pRect, u8 *buffer)
{
    T_AMOPENAT_LCD_RECT_T rect;

    ASSERT(lcd_bus_assert_fail==KAL_FALSE);
    lcd_bus_assert_fail = KAL_TRUE;
    
    rect.ltX = pRect->ltx;
    rect.ltY = pRect->lty;
    rect.rbX = pRect->rbx;
    rect.rbY = pRect->rby;
    
    IVTBL(update_mono_lcd_screen)(&rect);

    lcd_bus_assert_fail = KAL_FALSE;
}

static void lcd_common_mono_update(PlatformRect *pRect, u8 *buffer){
    int page;
    int width = pRect->rbx + 1;
    
	for(page = 0; page < pRect->rby/8+1; page++)
	{   
        // set page
        lcd_bus_write_cmd(0xB0|page);
/*+\NEW\RUFEI\2015.9.22\Updat lcd lcd_common_mono_update interface*/
        // set start column
        /*+\bug\czm\2015.9.22\bug:2520  偏移方向不对*/
        lcd_bus_write_cmd(0x00+lcd_x_offset%16);
        lcd_bus_write_cmd(0x10+lcd_x_offset/16);

        // set start line
        lcd_bus_write_cmd(0x40+lcd_y_offset);
        /*+\bug\czm\2015.9.22\bug:2520  偏移方向不对*/
/*-\NEW\RUFEI\2015.9.22\Updat lcd lcd_common_mono_update interface*/
        //lcd_bus_write_data_buf((buffer+page*width), width);
        /*+\bug\czm\2015.9.22\bug:2520  ssd1306屏无法显示*/
		for (int n = 0; n < width; n++)
        {
            
            lcd_bus_write_data(*(buffer + page * width + n));
        }
        /*-\bug\czm\2015.9.22\bug:2520  ssd1306屏无法显示*/
	}
}

static void lcd_bus_i2c_write_cmd(uint16 cmd){
    /*
    platform_i2c_send_data( lcd_bus_itf.bus_i2c.bus_id, 
                            lcd_bus_itf.bus_i2c.slave_addr, 
                            (const u8 *)&lcd_bus_itf.bus_i2c.cmd_addr, 
                            (const u8 *)&cmd, 
                            1);
    */
}

static void lcd_bus_i2c_write_data(uint16 data){
    /*
    platform_i2c_send_data( lcd_bus_itf.bus_i2c.bus_id, 
                            lcd_bus_itf.bus_i2c.slave_addr, 
                            (const u8 *)&lcd_bus_itf.bus_i2c.data_addr, 
                            (const u8 *)&data, 
                            1);
	*/
}

static void lcd_bus_i2c_write_data_buf(uint8 *buf, uint32 len){
    /*
    platform_i2c_send_data( lcd_bus_itf.bus_i2c.bus_id, 
                            lcd_bus_itf.bus_i2c.slave_addr, 
                            (const u8 *)&lcd_bus_itf.bus_i2c.data_addr, 
                            buf, 
                            len);
    */
}

static void lcd_bus_spi_write_cmd(uint16 cmd){
    //IVTBL(set_gpio)(lcd_bus_itf.bus_spi.pin_rs, 0);
    IVTBL(WriteSPI)(OPENAT_SPI_1, (const UINT8 *)&cmd, 1, 0);
}

static void lcd_bus_spi_write_data(uint16 data){
    //IVTBL(set_gpio)(lcd_bus_itf.bus_spi.pin_rs, 1);
    IVTBL(WriteSPI)(OPENAT_SPI_1, (const UINT8 *)&data, 1, 1);
}

static void lcd_bus_spi_write_data_buf(uint8 *buf, uint32 len){
    //IVTBL(set_gpio)(lcd_bus_itf.bus_spi.pin_rs, 1);
    IVTBL(WriteSPI)(OPENAT_SPI_1, buf, len, 1);
}

static void lcd_bus_spi4line_init(const PlatformDispInitParam *pParam){
    if(lcd_bpp == 16 || lcd_bpp == 24){
        T_AMOPENAT_COLOR_LCD_PARAM param;
        
        param.width = pParam->width;
        param.height = pParam->height;
        param.msgCallback = NULL;
        param.bus = OPENAT_LCD_SPI4LINE;
		/**+\BUG:3316\czm\2020.10.16\Luat 的LCD_SPI 驱动能力弱，LCD_SPI 频率不可调希望支持* */
        //param.lcdItf.spi.frequence = (6000000);
        param.lcdItf.spi.frequence = pParam->lcd_itf.bus_spi.freq;
		/**-\BUG:3316\czm\2020.10.16\Luat 的LCD_SPI 驱动能力弱，LCD_SPI 频率不可调希望支持* */
        //param.lcdItf.spi.csPort = platform_pio_get_gpio_port(pParam->pin_cs);
        //param.lcdItf.spi.rstPort = platform_pio_get_gpio_port(pParam->pin_rst);
        param.lcdItf.spi.rstPort  = OPENAT_LCD_RST_GPIO;
        param.lcd_bpp = pParam->bpp;
        /*+\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
        param.lcdItf.spi.Driving = pParam->Driving;
        /*-\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
#ifndef SHOW_LOGO_SUPPORT
        lcd_common_reset(param.lcdItf.spi.rstPort); //lcdParam.rstPort
#endif
        IVTBL(init_color_lcd)(&param);

        lcd_bus_update = lcd_bus_color_update;
        lcd_bus_write_cmd = (lcd_cmd_data_op)IVTBL(send_color_lcd_command);
        lcd_bus_write_data = (lcd_cmd_data_op)IVTBL(send_color_lcd_data);
    } else {
        T_AMOPENAT_MONO_LCD_PARAM lcdParam;
        
        lcdParam.height = pParam->height;
        lcdParam.width = pParam->width;
        lcdParam.xoffset = pParam->x_offset;
        lcdParam.yoffset = pParam->y_offset;
		/**+\BUG:3316\czm\2020.10.16\Luat 的LCD_SPI 驱动能力弱，LCD_SPI 频率不可调希望支持* */
        //lcdParam.frequence = 6000000;//6M
        lcdParam.frequence = pParam->lcd_itf.bus_spi.freq;
		/**-\BUG:3316\czm\2020.10.16\Luat 的LCD_SPI 驱动能力弱，LCD_SPI 频率不可调希望支持* */
        lcdParam.fameBuffer = pParam->framebuffer;
        lcdParam.pixelBits = 1;
        //lcdParam.csPort = platform_pio_get_gpio_port(pParam->pin_cs);
        //lcdParam.rstPort = platform_pio_get_gpio_port(pParam->pin_rst);;
        lcdParam.rstPort = OPENAT_LCD_RST_GPIO; // pParam->pin_rst
        /*+\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
        lcdParam.Driving = pParam->Driving;  
        /*-\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/        
        lcd_common_reset(lcdParam.rstPort);
        IVTBL(init_mono_lcd)(&lcdParam);

        //lcd_bus_update = lcd_bus_mono_update;
        lcd_bus_update = lcd_common_mono_update;
        lcd_bus_write_cmd = (lcd_cmd_data_op)IVTBL(send_mono_lcd_command);
        lcd_bus_write_data = (lcd_cmd_data_op)IVTBL(send_mono_lcd_data);
        lcd_bus_write_data_buf = lcd_bus_spi_write_data_buf;
    }
}

static void lcd_bus_parallel_init(const PlatformDispInitParam *pParam){
    /*
    T_AMOPENAT_COLOR_LCD_PARAM param;
    
    ASSERT(lcd_bpp == 16);
    
    param.width = pParam->width;
    param.height = pParam->height;
    param.msgCallback = NULL;
    param.bus = OPENAT_LCD_PARALLEL_8800;
    param.lcdItf.parallel.csPort = platform_pio_get_gpio_port(pParam->pin_cs);
    param.lcdItf.parallel.rstPort = platform_pio_get_gpio_port(pParam->pin_rst);
    
    IVTBL(init_color_lcd)(&param);
    
    lcd_bus_update = lcd_bus_color_update;
    lcd_bus_write_cmd = (lcd_cmd_data_op)IVTBL(send_color_lcd_command);
    lcd_bus_write_data = (lcd_cmd_data_op)IVTBL(send_color_lcd_data);
	*/
}

static void lcd_common_reset(E_AMOPENAT_GPIO_PORT pin_rst){
/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    T_AMOPENAT_GPIO_CFG cfg={0};
    cfg.mode=OPENAT_GPIO_OUTPUT;
    cfg.param.defaultState = 1;
    OPENAT_config_gpio(pin_rst,&cfg);
    delayMilliseconds(100);
    OPENAT_set_gpio(pin_rst,0);
    delayMilliseconds(5);
    OPENAT_set_gpio(pin_rst,1);
    delayMilliseconds(100);
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
}

static void lcd_bus_i2c_init(const PlatformDispInitParam *pParam){
    /*
    PlatformI2CParam i2c_param;
    
    ASSERT(lcd_bpp == 1);

    memcpy(&lcd_bus_itf, &pParam->lcd_itf, sizeof(lcd_itf_t));
    
    i2c_param.speed = pParam->lcd_itf.bus_i2c.freq;
    i2c_param.slaveAddr = pParam->lcd_itf.bus_i2c.slave_addr;
    
    platform_i2c_setup(pParam->lcd_itf.bus_i2c.bus_id, &i2c_param);
    
    lcd_common_reset(platform_pio_get_gpio_port(pParam->pin_rst));
    
    lcd_bus_update = lcd_common_mono_update;
    lcd_bus_write_cmd = lcd_bus_i2c_write_cmd;
    lcd_bus_write_data = lcd_bus_i2c_write_data;
    lcd_bus_write_data_buf = lcd_bus_i2c_write_data_buf;
	*/
}

/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
static void lcd_bus_spi_init(const PlatformDispInitParam *pParam){
    T_AMOPENAT_SPI_PARAM spiParam;
    
    ASSERT(lcd_bpp == 1 || lcd_bpp == 16);
    
    memcpy(&lcd_bus_itf, &pParam->lcd_itf, sizeof(lcd_itf_t));
    lcd_bus_itf.bus_spi.pin_rs = platform_pio_get_gpio_port(pParam->pin_rs);
    
    spiParam.fullDuplex = FALSE;
    spiParam.cpol = FALSE;
    spiParam.cpha = 0;
    spiParam.dataBits = 8;
    spiParam.clock = pParam->lcd_itf.bus_spi.freq;
    spiParam.withCS = TRUE;

    IVTBL(OpenSPI)(OPENAT_SPI_1, &spiParam);

    {
        T_AMOPENAT_GPIO_CFG cfg;
        
        cfg.mode = OPENAT_GPIO_OUTPUT;
        cfg.param.defaultState = 0;
        IVTBL(config_gpio)(platform_pio_get_gpio_port(pParam->pin_rs), &cfg);
    }
    
    lcd_common_reset(platform_pio_get_gpio_port(pParam->pin_rst));
    
    if(lcd_bpp == 1)
    {
        lcd_bus_update = lcd_common_mono_update;
    }
    else
    {
        lcd_bus_update = lcd_spi_color_update;
    }
        
    lcd_bus_write_cmd = lcd_bus_spi_write_cmd;
    lcd_bus_write_data = lcd_bus_spi_write_data;
    lcd_bus_write_data_buf = lcd_bus_spi_write_data_buf;
}
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
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
                delayMilliseconds(value);
                break;

            case 0:
            case 2:
                lcd_bus_write_cmd(value&0x00ff);
                break;

            case 3:
                lcd_bus_write_data(value&0x00ff);
                break;

            default:
                ASSERT(0);
                break;
        }
    }
}

/*+\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
void platform_lcd_close(void)
{
    if(lcd_bpp == 16 || lcd_bpp == 24)
        IVTBL(close_color_lcd)();
    else
        IVTBL(close_mono_lcd)();

    if(lcd_sleep_cmd != NULL)
    {
        L_FREE(lcd_sleep_cmd);
        lcd_sleep_cmd=NULL;
    }

    if(lcd_wake_cmd != NULL)
    {
        L_FREE(lcd_wake_cmd);
        lcd_wake_cmd=NULL;
    }
}
/*-\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
void platform_lcd_init(const PlatformDispInitParam *pParam)
{
    lcd_bpp = pParam->bpp;
    lcd_x_offset = pParam->x_offset;
    lcd_y_offset = pParam->y_offset;

/*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
/*+\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
    if(lcd_sleep_cmd == NULL)
/*-\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
    {
        lcd_sleep_cmd = L_MALLOC(sizeof(UINT32)*(pParam->sleepCmdSize+1));
        memcpy(lcd_sleep_cmd, pParam->pLcdSleepCmd, sizeof(UINT32)*pParam->sleepCmdSize);
        lcd_sleep_cmd[pParam->sleepCmdSize] = -1;
    }
/*+\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
    if(lcd_wake_cmd == NULL)
/*-\bug2958\czm\2020.9.1\disp.close() 之后再执行disp.init 无提示直接重启*/
    {
        lcd_wake_cmd = L_MALLOC(sizeof(UINT32)*(pParam->wakeCmdSize+1));
        memcpy(lcd_wake_cmd, pParam->pLcdWakeCmd, sizeof(UINT32)*pParam->wakeCmdSize);
        lcd_wake_cmd[pParam->wakeCmdSize] = -1;
    }
/*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */

    lcd_bus = pParam->bus;
#ifndef SHOW_LOGO_SUPPORT
    //init_lcd_interface();

#endif
    lcd_init_table[lcd_bus](pParam);


/*+\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */
    if(lcd_bpp == 1){
        write_command_table(pParam->pLcdCmdTable, pParam->tableSize);
    } else {
#ifndef SHOW_LOGO_SUPPORT
        delayMilliseconds(50);
        
        write_command_table(pParam->pLcdCmdTable, pParam->tableSize);
        
        delayMilliseconds(100);
#endif
    }
/*-\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */
	
	/*+\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
	IVTBL(CameraLcdUpdateEnable)(!pParam->camera_preview_no_update_screen);	
	/*-\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
}

void platform_lcd_update(PlatformRect *pRect, u8 *buffer)
{
	
    lcd_bus_update(pRect, buffer);

/*+\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */
    if(lcd_first_update) {
        if(lcd_bpp == 1){
            lcd_bus_write_cmd(0xAF);
        }

#if 0
       spi_interface_init();
       delayMilliseconds(10);
       init_lcd_interface();
#endif
       lcd_first_update = 0;
    }
/*-\new\liweiqiang\2014.10.27\修正黑白屏开机花屏问题 */
}

/*+\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/
void platform_lcd_wrire(int cmd)
{

	UINT16 flag;
	UINT16 value;
	UINT16 index;

	if(lcd_bus_write_cmd == NULL || lcd_bus_write_data == NULL) return;

    flag = cmd>>16;
    value = cmd&0xffff;

    switch(flag)
    {
        case 1:
            delayMilliseconds(value);
            break;

        case 0:
        case 2:
            lcd_bus_write_cmd(value&0x00ff);
            break;

        case 3:
            lcd_bus_write_data(value&0x00ff);
            break;

        default:
            ASSERT(0);
            break;
    }
}

void platform_lcd_wrire_data(int data)
{
    lcd_bus_write_data(data);
}
/*-\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/

/*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
void platform_lcd_powersave(int sleep_wake)
{
    if(sleep_wake)
    {       
        if(lcd_sleep_cmd)
        {
            write_command_table(lcd_sleep_cmd, (UINT16)-1);
        }
    }
    else
    {
        if(lcd_wake_cmd)
        {
            write_command_table(lcd_wake_cmd, (UINT16)-1);
        }
    }
}

/*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
// -panjun, 2015.04.21, Commit SSD1306's driver code.

#endif

