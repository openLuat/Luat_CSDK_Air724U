/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    platform_disp.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          platform display 接口
 **************************************************************************/

#ifndef _PLATFORM_DISP_H_
#define _PLATFORM_DISP_H_

#include <sys/queue.h>

typedef struct PlatformRectTag
{
    u16 ltx;        //left top x y
    u16 lty;
    u16 rbx;        //right bottom x y
    u16 rby;
}PlatformRect;

// 颜色定义 RGB(5,6,5)
#define COLOR_WHITE_16 0xffff
#define COLOR_BLACK_16 0x0000
#define COLOR_WHITE_24 0xffffff
#define COLOR_BLACK_24 0x000000

#define COLOR_WHITE_1 0xff
#define COLOR_BLACK_1  0x00

#define BASIC_LAYER_ID  0
#define USER_LAYER_1_ID  1
#define USER_LAYER_2_ID  2
#define INVALID_LAYER_ID  -1


typedef enum PlatformLcdBusTag
{
    PLATFORM_LCD_BUS_SPI4LINE,
    PLATFORM_LCD_BUS_PARALLEL,

/*+\new\liweiqiang\2014.10.11\添加lcd i2c spi接口 */
    PLATFORM_LCD_BUS_I2C,
    PLATFORM_LCD_BUS_SPI,
/*-\new\liweiqiang\2014.10.11\添加lcd i2c spi接口 */
    
    PLATFORM_LCD_BUS_QTY,
}PlatformLcdBus;

/*+\new\liweiqiang\2014.10.11\添加lcd i2c spi接口 */
typedef union {
    struct {
        int bus_id;
        int pin_rs;
        int pin_cs;
        int freq;
    } bus_spi;
    
    struct {
        int bus_id;
        int freq;
        int slave_addr;
        int cmd_addr;
        int data_addr;
    } bus_i2c;
} lcd_itf_t;
/*-\new\liweiqiang\2014.10.11\添加lcd i2c spi接口 */

typedef struct PlatformDispInitParamTag
{
    u16 width;  // lcd设备宽度
    u16 height; // lcd设备高度
    u8  bpp; // bits per pixel lcd设备色深 1:黑白 16:16位色彩屏
    u16 x_offset;
    u16 y_offset;
    u32 *pLcdCmdTable;    //lcd初始化指令表
    u16 tableSize;         //lcd初始化指定表大小
/*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
    u32 *pLcdSleepCmd;  // lcd sleep指令表
    u16 sleepCmdSize;
    u32 *pLcdWakeCmd;   // lcd wake指令表
    u16 wakeCmdSize;
/*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
    PlatformLcdBus bus;
/*+\new\liweiqiang\2014.10.11\添加lcd i2c接口 */
    lcd_itf_t lcd_itf;
/*-\new\liweiqiang\2014.10.11\添加lcd i2c接口 */
    int pin_rst; //reset pin
    /*+\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */
    int hwfillcolor; // lcd物理填充色
    /*-\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */
/*+\NEW\2013.4.10\增加黑白屏显示支持 */
    int pin_cs; // cs pin
/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    int pin_rs; // rs pin
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
	/*+\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
    u8 camera_preview_no_update_screen;
	/*-\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
    u8 *framebuffer;
/*-\NEW\2013.4.10\增加黑白屏显示支持 */

    /*+\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
    u8 Driving;//lcd_spi的驱动能力最大值为15
    /*-\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
}PlatformDispInitParam;

typedef SLIST_ENTRY(platform_layer) platform_layer_iter_t;
typedef SLIST_HEAD(platform_layer_head, platform_layer) platform_layer_head_t;

typedef struct platform_layer
{
    platform_layer_iter_t iter;
	int id;
	int height;
	int width;
	int bpp;
	void *buf;
	void (*update) (PlatformRect *area, struct platform_layer *layer);
    void (*destroy) (struct platform_layer *layer);
} platform_layer_t;

int platform_disp_create_layer(platform_layer_t *layer);

int platform_disp_set_act_layer(int id);

void platform_disp_destroy_layer(int id);

void platform_disp_init(PlatformDispInitParam *pParam);

void platform_disp_close(void);

void platform_disp_clear(void);

void platform_disp_update(void);

u16* platform_disp_puttext(const char *string, u16 x, u16 y);

/*+\NEW\liweiqiang\2013.12.6\增加图片透明色设置 */
/*+\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */
/*+\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
int platform_disp_putimage(const char *filename, u16 x, u16 y, int transcolor, u16 left, u16 top, u16 right, u16 bottom,int transtype);
/*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
/*-\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */
/*-\NEW\liweiqiang\2013.12.6\增加图片透明色设置 */

void platform_layer_start_move(int layer_id1, int layer_id2,int layer_id3,int delay_ms,int x_inc,int y_inc);

void platform_disp_playgif(const char* gif_file_name, int x, int y,  int times);


void platform_disp_stopgif(void);


int platform_disp_preload_png_foreground(const char *filename, int index);

int platform_disp_preload_png_background(const char *filename);

int platform_disp_move_foreimg(int x1, int y1, int x2, int y2);


/*+\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */
int platform_disp_drawrect(int x1, int y1, int x2, int y2, int color);
/*-\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */

/*+\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */
int platform_disp_setcolor(int color);
int platform_disp_setbkcolor(int color);
/*-\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */

/*+\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
int platform_disp_loadfont(const char *name);
int platform_disp_setfont(int id);
/*-\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
/*+NEW\brezen\2016.05.13\字体缩放*/  
int platform_disp_setfontHeight(int height);
int platform_disp_getfontHeight(void);
/*-NEW\brezen\2016.05.13\字体缩放*/  
/******************************************************************************
***func name---- platform_disp_preload_png_to_layer
***param    ---- filename: 要装载的PNG图片名称
                 layer_id: 图片装载的目的图层ID
***desc     ---- 预装载PNG图片到图层
***return   ---- 是否成功
***note     
******************************************************************************/
int platform_disp_preload_png_to_layer(const char *filename, int layer_id);





/******************************************************************************
***func name---- platform_create_user_layer
***param    ---- layer_id:要创建的用户图层
            -----layer_width:图层宽度
            -----layer_height:图层高度
***desc     ---- 创建用户图层
***return   ---- 是否成功
***note     
-----1.在创建之前，请确保此图层从未被创建或者已经销毁
-----2.目前只支持两个用户图层的创建
-----3.基础层不用创建，系统自动创建
-----4.用户图层的默认大小为屏幕大小，默认格式为24位
******************************************************************************/
int platform_create_user_layer(int layer_id, int start_x, int start_y, int layer_width, int layer_height);




/******************************************************************************
***func name---- platform_destroy_user_layer
***param    ---- layer_id:要销毁的用户图层
***desc     ---- 销毁用户图层
***return   ---- 是否成功
***note     
-----1. layer_id只能是USER_LAYER_1_ID或者USER_LAYER_2_ID
-----2. 图层使用完成之后应该尽早释放，否则会占用系统内存
******************************************************************************/
int platform_destroy_user_layer(int layer_id);



/******************************************************************************
***func name---- platform_set_active_layer
***param    ---- layer_id:要激活的图层ID
***desc     ---- 激活图层，对LCD的所有绘图动作都会在激活图层上进行
***return   ---- NULL
***note     
******************************************************************************/
void platform_set_active_layer(int layer_id);




/******************************************************************************
***func name---- platform_swap_user_layer
***param    ---- NULL
***desc     ---- 对换两个用户图层的内容。此函数不涉及图层的数据拷贝，因此速度很快。
                 适用于对速度要求高的情况下快速交换两个用户图层的内容.
***return   ---- NULL
***note     
******************************************************************************/
void platform_swap_user_layer(void);





/******************************************************************************
***func name---- platform_copy_layer
***param    ---- dst_layer_id: 要COPY的目的图层的ID
            -----display_x:  源图层在目的图层上的位置X
            -----display_y:  源图层在目的图层上的位置Y
            ---- src_layer_id: 要COPY的源图层的ID
            -----src_rect:  需要COPY的源图层的区域
***desc     ---- 图层内容拷贝。
***return   ---- NULL
***note
******************************************************************************/
void platform_copy_layer(int dst_layer_id,
                                 int display_x,
                                 int display_y,
                                 int src_layer_id,
                                 T_AMOPENAT_LCD_RECT_T* src_rect);




/******************************************************************************
***func name---- platform_copy_to_basic_layer
***param    ---- layer_id1: 要显示的图层1的ID
            -----x1:  源图层1在LCD的显示座标X.
            -----y1:  源图层1在LCD的显示座标y.
            ---- layer_id2: 要显示的图层2的ID
            -----x2:  源图层2在LCD的显示座标X.
            -----y2:  源图层2在LCD的显示座标y.            
            ---- layer_id3: 要显示的图层3的ID
            -----x2:  源图层3在LCD的显示座标X.
            -----y2:  源图层3在LCD的显示座标y.            
***desc     -----多图层的显示。
***return   ----- NULL
***note     ----- 如果不需要显示某个图层，将layer_id置为INVALID_LAYER_ID即可
******************************************************************************/
void platform_layer_flatten(int layer_id1, int x1, int y1, 
                                  int layer_id2, int x2, int y2,
                                  int layer_id3, int x3, int y3);



int platform_get_png_file_resolution(const char *filename, u32* width, u32* height);

/*+\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
void platform_disp_new(PlatformDispInitParam *pParam);

int platform_disp_get();
/*-\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#define MAX_LCD_WIDTH_SUPPORT 240
#define MAX_LCD_HEIGH_SUPPORT 240

#define MAX_LCD_PIXEL_BYTES   3
#define MAX_LCD_BUFF_SIZE (MAX_LCD_WIDTH_SUPPORT*MAX_LCD_HEIGH_SUPPORT* MAX_LCD_PIXEL_BYTES)
/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/

#endif//_PLATFORM_DISP_H_
