/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    disp.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          lua.disp库
  * History:
 *     panjun 2015.05.29 Add an 'ASSERT' for 'disp_putimage'.
 **************************************************************************/
#ifdef LUA_DISP_LIB
#include <stdlib.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "lplatform.h"
#include "lrotable.h"
#include "platform_conf.h"
#include "platform_disp.h"
#include "platform_camera.h"

static u8 putimage_assert_fail = FALSE;
/*+\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
extern kal_uint8* workingbuffer;
/*-\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 

int checkFiledInt(lua_State *L, int index, const char *key)
{
    int d;
    lua_getfield(L, index, key);
    d = luaL_checkinteger(L, -1);
    lua_remove(L, -1);
    return d;
}

int getFiledInt(lua_State *L, int index, const char *key)
{
    int d;
    lua_getfield(L, index, key);
    d = lua_tointeger(L, -1);
    lua_remove(L, -1);
    return d;
}

static int optFiledInt(lua_State *L, int index, const char *key, int defval)
{
    int d;
    lua_getfield(L, index, key);
    d = luaL_optint(L, -1, defval);
    lua_remove(L, -1);
    return d;
}

/*+\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
// disp.new
static int disp_new(lua_State *L) {

    PlatformDispInitParam param;
    int cmdTableIndex;

    luaL_checktype(L, 1, LUA_TTABLE);

    memset(&param, 0, sizeof(param));

    param.width = getFiledInt(L, 1, "width");
    param.height = getFiledInt(L, 1, "height");
    param.pin_cs = PLATFORM_IO_UNKNOWN_PIN;

    if(param.width == 0 || param.height == 0)
    {
        return luaL_error(L, "disp.init: error param width(%d) height(%d)",
                                param.width, param.height);
    }

    param.bpp = getFiledInt(L, 1, "bpp");

/*+\NEW\2013.4.10\增加黑白屏显示支持 */
    //16位色彩屏or黑白屏
    if(!(param.bpp == 16 || param.bpp == 1 || param.bpp == 24))
    {
        return luaL_error(L, "disp.init: pixel depth must be 16 or 24 or 1!");
    }

    // 不设偏移则默认0
    param.x_offset = getFiledInt(L, 1, "xoffset");
    param.y_offset = getFiledInt(L, 1, "yoffset");

    param.camera_preview_no_update_screen= optFiledInt(L, 1, "camera_preview_no_update_screen",0);

    /*+\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */
    param.hwfillcolor = optFiledInt(L, 1, "hwfillcolor", -1);
    /*-\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */

    platform_disp_new(&param);

    return 0;
}


// disp.getframe
static int disp_getframe(lua_State *L) {
	int frameBufferSize = 0;

	frameBufferSize = platform_disp_get();

    if(frameBufferSize == 0)
        return luaL_error(L, "disp.getframe: buf is null!%d",frameBufferSize);

	lua_pushlstring(L, workingbuffer, frameBufferSize);
	lua_pushnumber(L, frameBufferSize);

	return 2;
}
/*-\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
int bpp_test ;
// disp.init
static int disp_init(lua_State *L) {

    PlatformDispInitParam param;
    int cmdTableIndex;

    luaL_checktype(L, 1, LUA_TTABLE);

    memset(&param, 0, sizeof(param));

    param.width = getFiledInt(L, 1, "width");
    param.height = getFiledInt(L, 1, "height");
    
    if(param.width == 0 || param.height == 0)
    {
        return luaL_error(L, "disp.init: error param width(%d) height(%d)", 
                                param.width, param.height);
    }
    
    param.bpp = getFiledInt(L, 1, "bpp");
    bpp_test = param.bpp;
    
/*+\NEW\2013.4.10\增加黑白屏显示支持 */
    //16位色彩屏or黑白屏
    if(!(param.bpp == 16 || param.bpp == 1 || param.bpp == 24))
    {
        return luaL_error(L, "disp.init: pixel depth must be 16 or 1!%d", param.bpp); 
    }
    
    // lcd传输接口
    param.bus = (PlatformLcdBus)getFiledInt(L, 1, "bus"); // panjun, 2015.04.21, Commit SSD1306's driver code.

    /*+\new\liweiqiang\2014.10.22\lcd不同接口信息定义 */
    // 不同传输接口定义
/*+\DEL\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    // if(param.bus == PLATFORM_LCD_BUS_I2C || param.bus == PLATFORM_LCD_BUS_SPI){
    //     lua_getfield(L, 1, "interface");
    //     luaL_checktype(L, -1, LUA_TTABLE);

    //     if(param.bus == PLATFORM_LCD_BUS_I2C){
    //         param.lcd_itf.bus_i2c.bus_id = checkFiledInt(L, -1, "bus_id");
    //         param.lcd_itf.bus_i2c.freq = checkFiledInt(L, -1, "freq");
    //         param.lcd_itf.bus_i2c.slave_addr = checkFiledInt(L, -1, "slave_addr");
    //         param.lcd_itf.bus_i2c.cmd_addr = checkFiledInt(L, -1, "cmd_addr");
    //         param.lcd_itf.bus_i2c.data_addr = checkFiledInt(L, -1, "data_addr");
    //     } else if(param.bus == PLATFORM_LCD_BUS_SPI){
    //         param.lcd_itf.bus_spi.bus_id = checkFiledInt(L, -1, "bus_id");
    //         param.lcd_itf.bus_spi.pin_rs = checkFiledInt(L, -1, "pin_rs");
    //         param.lcd_itf.bus_spi.pin_cs = optFiledInt(L, -1, "pin_cs", PLATFORM_IO_UNKNOWN_PIN);
    //         param.lcd_itf.bus_spi.freq = checkFiledInt(L, -1, "freq");
    //     }
    // }
/*-\DEL\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    /*-\new\liweiqiang\2014.10.22\lcd不同接口信息定义 */    

    // lcd rst脚必须定义
    param.pin_rst = checkFiledInt(L, 1, "pinrst");
/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    param.pin_rs = checkFiledInt(L, 1, "pinrs");
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */
    lua_getfield(L, 1, "pincs");

    if(lua_type(L,-1) != LUA_TNUMBER)
        param.pin_cs = PLATFORM_IO_UNKNOWN_PIN;
    else
        param.pin_cs = lua_tonumber(L,-1);

    // 不设偏移则默认0
    param.x_offset = getFiledInt(L, 1, "xoffset");
    param.y_offset = getFiledInt(L, 1, "yoffset");
	
	/*+\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
    param.camera_preview_no_update_screen = optFiledInt(L, 1, "camera_preview_no_update_screen",0);
	/*-\bug2406\zhuwangbin\2020.6.28\摄像头扫描预览时，要支持配置是否刷屏显示功能 */
    /*+\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */
    param.hwfillcolor = optFiledInt(L, 1, "hwfillcolor", -1);
    /*-\new\liweiqiang\2014.10.21\增加不同黑白屏填充色处理 */
/*+\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用,1M */
    param.lcd_itf.bus_spi.freq = optFiledInt(L, 1, "freq", 1000000);
/*-\new\czm\2020.9.6\bug:2964 mono_std_spi_st7571.lua 无法正常使用 */

      /*+\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  
    param.Driving = optFiledInt(L, 1, "driving", 0);
      /*+\BUG:3316\czm\2020.10.16\LCD_SPI 驱动能力弱，希望能增强驱动能力*/  


    // .initcmd 初始化指令表
    lua_getfield(L, 1, "initcmd");
    luaL_checktype(L, -1, LUA_TTABLE);
    param.tableSize = luaL_getn(L, -1);
    param.pLcdCmdTable = L_MALLOC(sizeof(int)*param.tableSize);
    
    for(cmdTableIndex = 0; cmdTableIndex < param.tableSize; cmdTableIndex++)
    {
        lua_rawgeti(L, -1, cmdTableIndex+1);
        param.pLcdCmdTable[cmdTableIndex] = lua_tointeger(L, -1);
        lua_remove(L,-1);
    }
/*-\NEW\2013.4.10\增加黑白屏显示支持 */

/*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
    lua_getfield(L, 1, "sleepcmd");
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        param.sleepCmdSize = luaL_getn(L, -1);
        param.pLcdSleepCmd = L_MALLOC(sizeof(int)*param.sleepCmdSize);

        for(cmdTableIndex = 0; cmdTableIndex < param.sleepCmdSize; cmdTableIndex++)
        {
            lua_rawgeti(L, -1, cmdTableIndex+1);
            param.pLcdSleepCmd[cmdTableIndex] = lua_tointeger(L, -1);
            lua_remove(L,-1);
        }
    }

    lua_getfield(L, 1, "wakecmd");
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        param.wakeCmdSize = luaL_getn(L, -1);
        param.pLcdWakeCmd = L_MALLOC(sizeof(int)*param.wakeCmdSize);
        
        for(cmdTableIndex = 0; cmdTableIndex < param.wakeCmdSize; cmdTableIndex++)
        {
            lua_rawgeti(L, -1, cmdTableIndex+1);
            param.pLcdWakeCmd[cmdTableIndex] = lua_tointeger(L, -1);
            lua_remove(L,-1);
        }
    }
/*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */

    platform_disp_init(&param);

    L_FREE(param.pLcdCmdTable);

    /*+\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */
    if(param.pLcdSleepCmd)
        L_FREE(param.pLcdSleepCmd);

    if(param.pLcdWakeCmd)
        L_FREE(param.pLcdWakeCmd);
    /*-\NEW\liweiqiang\2013.12.18\增加lcd睡眠命令支持 */

    return 0;
}

static int disp_close(lua_State *L) {
    platform_disp_close();
    return 0;
}
// disp.clear
static int disp_clear(lua_State *L) {    
  platform_disp_clear();
  return 0; 
}

static int disp_update(lua_State *L){
    platform_disp_update();
    return 0;
}
   
// disp.puttext
static int disp_puttext(lua_State *L) {
  const char *str;
  u16 x, y, *offset;
  
  str   = luaL_checkstring(L, 1);
  x     = cast(u16,luaL_checkinteger(L, 2));
  y     = cast(u16,luaL_checkinteger(L, 3));

  offset = platform_disp_puttext(str, x, y);
  //lua_pushinteger(L, offset[0]);
  //lua_pushinteger(L, offset[1]);

  return 2; 
}

/*+\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */
//disp.putimage
static int disp_putimage(lua_State *L) {
    const char *filename;
    /*+\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    u16 x, y, left, top, right, bottom;
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    int transcolor,transtype;

    ASSERT(putimage_assert_fail==FALSE);
    putimage_assert_fail = TRUE;

    filename   = luaL_checkstring(L, 1);
    x     = luaL_optint(L, 2, 0);
/*+\NEW\liweiqiang\2013.11.12\修正显示图片y坐标无法设置 */
    y     = luaL_optint(L, 3, 0);
/*-\NEW\liweiqiang\2013.11.12\修正显示图片y坐标无法设置 */

/*+\NEW\liweiqiang\2013.12.6\增加图片透明色设置 */
    /*+\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    transcolor = luaL_optint(L, 4, -1); //默认不透明
    left = luaL_optint(L, 5, 0);
    top = luaL_optint(L, 6, 0);
    right = luaL_optint(L, 7, 0);
    bottom = luaL_optint(L, 8, 0);
    transtype = luaL_optint(L, 9, 1);


    platform_disp_putimage(filename, x, y, transcolor, left, top, right, bottom,transtype);
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
/*-\NEW\liweiqiang\2013.12.6\增加图片透明色设置 */

    putimage_assert_fail = FALSE;
    
    return 0; 
}
/*-\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */



static int disp_preloadpng(lua_State *L) {
    const char *filename;
    /*+\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    u16  index;
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/

    filename   = luaL_checkstring(L, 1);
    index     = luaL_optint(L, 2, 0);
   

    platform_disp_preload_png_to_layer(filename, index);
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
/*-\NEW\liweiqiang\2013.12.6\增加图片透明色设置 */    
    return 0; 
}

#ifdef AM_LAYER_SUPPORT
static int disp_layer_display(lua_State *L) {
    /*+\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    int layer_id1, layer_id2, layer_id3, x1,y1,x2,y2,x3,y3;
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/

     layer_id1 = luaL_optint(L, 1, -1);
     x1     = luaL_optint(L, 2, 0);
     y1     = luaL_optint(L, 3, 0);
     layer_id2 = luaL_optint(L, 4, -1);
     x2     = luaL_optint(L, 5, 0);
     y2     = luaL_optint(L, 6, 0);
     layer_id3 = luaL_optint(L, 7, -1);
     x3     = luaL_optint(L, 8, 0);
     y3     = luaL_optint(L, 9, 0);
     
    platform_layer_flatten(layer_id1,  x1,  y1, 
                           layer_id2,  x2,  y2,
                           layer_id3,  x3,  y3);

    return PLATFORM_OK;
}





static int disp_createuserlayer(lua_State *L) {
    int layer_id;
    int layer_width;
    int layer_height;
    int start_x;
    int start_y;
    
    layer_id     = luaL_optint(L, 1, 0);
    start_x      = luaL_optint(L, 2, 0);
    start_y      = luaL_optint(L, 3, 0);
    layer_width  = luaL_optint(L, 4, 0);
    layer_height = luaL_optint(L, 5, 0);
    
    platform_create_user_layer(layer_id, start_x, start_y, layer_width, layer_height); 
    
    return 0;
}





static int disp_destroyuserlayer(lua_State *L) {  
    int layer_id = luaL_optint(L, 1, 0);

    platform_destroy_user_layer(layer_id); 

    return 0;
}


static int disp_setactivelayer(lua_State *L) {  
    int layer_id = luaL_optint(L, 1, 0);

		
    platform_set_active_layer(layer_id); 
    
    return 0;
}


static int disp_copy_layer(lua_State *L) {  
    int layer_id1, layer_id2, x1,y1;
    /*-\NewReq NEW\zhutianhua\2013.12.24\显示图片的指定区域*/
    T_AMOPENAT_LCD_RECT_T rect;
     layer_id1 = luaL_optint(L, 1, -1);
     x1     = luaL_optint(L, 2, 0);
     y1     = luaL_optint(L, 3, 0);
     layer_id2 = luaL_optint(L, 4, -1);
     rect.ltX  = luaL_optint(L, 5, 0);
     rect.ltY  = luaL_optint(L, 6, 0);
     rect.rbX  = luaL_optint(L, 7, 0); 
     rect.rbY  = luaL_optint(L, 8, 0); 

    platform_copy_layer(layer_id1, x1, y1, layer_id2, &rect); 

    return 0;
}


#ifdef TOUCH_PANEL_SUPPORT

/*-\NEW\zhuwangbin\2015.2.23\LUA 操作图层平移，改成底层操作 */
static int disp_layer_start_move(lua_State *L) {
  
  int layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc;

  layer_id1 = luaL_optint(L, 1, 0);
  layer_id2 = luaL_optint(L, 2, 0);
  layer_id3 = luaL_optint(L, 3, 0);
  delay_ms  = luaL_optint(L, 4, 0);
  x_inc  = luaL_optint(L, 5, 0);
  y_inc = luaL_optint(L, 6, 0); 

  OPENAT_print("disp_layer_start_move %d, %d, %d, %d, %d, %d", layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc);

  platform_layer_start_move(layer_id1, layer_id2, layer_id3, delay_ms, x_inc, y_inc);

  return 0;
}
/*-\NEW\zhuwangbin\2015.2.23\LUA 操作图层平移，改成底层操作 */


/*-\NEW\zhuwangbin\2015.2.26\lua 图层悬停改到底层做*/
static int disp_layer_hang_start(lua_State *L) {
  int layer_id1, layer_id2, layer_id3, y_inc, delay_ms;
  int move_config, lost_dirction;

  layer_id1 = luaL_optint(L, 1, 0);
  layer_id2 = luaL_optint(L, 2, 0);
  layer_id3 = luaL_optint(L, 3, 0);
  y_inc  = luaL_optint(L, 4, 0);
  delay_ms  = luaL_optint(L, 5, 0);
  move_config = luaL_optint(L, 6, 0);
  lost_dirction = luaL_optint(L, 7, 0);

  OPENAT_print("disp_layer_hang_start %d, %d, %d, %d, %d, %d, %d", layer_id1, layer_id2, layer_id3, y_inc, delay_ms, move_config, lost_dirction);
  platform_layer_hang_start(layer_id1, layer_id2, layer_id3, y_inc, delay_ms, move_config, lost_dirction);

  return 0;
}

static int disp_layer_hang_stop(lua_State *L) {

  OPENAT_print("disp_layer_hang_stop");
  platform_layer_hang_stop();
  return 0;
}
#endif

static int disp_layer_set_picture(lua_State *L) {
  int file_id, x, y, transcolor, left, right, top, bottom, transtype;
  const char *fileName;
  
  file_id = luaL_optint(L, 1, 0);
  fileName = luaL_checkstring(L, 2);
  x = luaL_optint(L, 3, 0);
  y  = luaL_optint(L, 4, 0);
  transcolor  = luaL_optint(L, 5, -1);
  left = luaL_optint(L, 6, 0);
  top = luaL_optint(L, 7, 0);
  right = luaL_optint(L, 8, 0);
  bottom = luaL_optint(L, 9, 0);
  transtype = luaL_optint(L, 10, 1);

  openat_layer_hang_picture_inset(file_id, fileName, x, y, transcolor, left, top, right, bottom, transtype);
  
  return 0;
}

static int disp_layer_set_text(lua_State *L)
{
  int file_id, x, y, color;
  const char *textString;
  BOOL colorSetBool = FALSE; 	//lua是否设置color标志
  int total;

  total = lua_gettop(L);
  file_id = luaL_optint(L, 1, 0);
  textString = luaL_checkstring(L, 2);
  x = luaL_optint(L, 3, 0);
  y  = luaL_optint(L, 4, 0);
  color = luaL_optint(L, 5, 0);

	// 只有lua设置color参数时，才设置字体颜色
  if (total == 5)
  	colorSetBool = TRUE;

  openat_layer_hang_text_inset(file_id, textString, x, y, color, colorSetBool);
  return 0;
}

static int disp_layer_set_RQcode(lua_State *L)
{
  int file_id;
  const char *urlString;
  int dispHeight;
  
  file_id = luaL_optint(L, 1, 0);
  urlString = luaL_checkstring(L, 2);
  dispHeight = luaL_optint(L, 3, 0);

  openat_layer_hang_RQcode_inset(file_id, urlString, dispHeight);

  return 0;
}

static int disp_layer_set_drawRect(lua_State *L)
{
  int file_id, x1, y1, x2, y2, color;

  file_id = luaL_optint(L, 1, 0);
  x1 = luaL_optint(L, 2, 0);
  y1 = luaL_optint(L, 3, 0);
  x2 = luaL_optint(L, 4, 0);
  y2 = luaL_optint(L, 5, 0);
  color = luaL_optint(L, 6, 0);
  
  openat_layer_hang_drawRect_inset(file_id, x1, y1, x2, y2, color);

  return 0;
}
/*-\NEW\zhuwangbin\2015.2.26\lua 图层悬停改到底层做*/
#endif

static int disp_playgif(lua_State *L) {
    const char *filename;
    u16 x, y, times;
    
#ifdef GIF_DECODE
    ASSERT(putimage_assert_fail==FALSE);
    putimage_assert_fail = TRUE;

    filename   = luaL_checkstring(L, 1);
    x     = luaL_optint(L, 2, 0);
    y     = luaL_optint(L, 3, 0);
    times = luaL_optint(L, 4, 1);


    platform_disp_playgif(filename, x, y, times);
    putimage_assert_fail = FALSE;
#endif
    
    return 0; 
}



static int  disp_stopgif(lua_State *L) {
#ifdef GIF_DECODE
       platform_disp_stopgif();
#endif
	   
    return 0; 
}




/*+\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */
static int disp_drawrect(lua_State *L)
{
    int left = luaL_checkinteger(L, 1);
    int top = luaL_checkinteger(L, 2);
    int right = luaL_checkinteger(L, 3);
    int bottom = luaL_checkinteger(L, 4);
    int color = luaL_optint(L, 5, -1);

    platform_disp_drawrect(left, top, right, bottom, color);

    return 0;
}
/*-\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */

/*+\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */
static int disp_qrcode(lua_State *L)
{
	size_t len = 0;
	int result, i;
	int w, disp_w,start_x,start_y,margin;
	u8 *buf;
    u8 *data = (u8 *)luaL_checklstring(L, 1, &len);
    w = luaL_checkinteger(L, 2);
    disp_w = luaL_checkinteger(L, 3);
    start_x = luaL_optint(L, 4, 0);
    start_y = luaL_optint(L, 5, 0);
    if ( (w <= 0) || (disp_w <= 0) || (start_x < 0) || (start_y < 0))
    {
    	goto __end;
    }
    if (len != (w * w))
    {
        goto __end;
    }
    result = platform_disp_qrcode(data, w, disp_w, start_x, start_y);
    lua_pushinteger(L, result);
    return 1;
__end:
	lua_pushinteger(L, 0);
	return 1;
}
/*-\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */

/*+\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */
static int disp_setcolor(lua_State *L)
{
    int color = luaL_checkinteger(L, 1);
    int retcolor = platform_disp_setcolor(color);
    lua_pushinteger(L, retcolor);
    return 1;
}

static int disp_setbkcolor(lua_State *L)
{
    int color = luaL_checkinteger(L, 1);
    int retcolor = platform_disp_setbkcolor(color);
    lua_pushinteger(L, retcolor);
    return 1;
}
/*-\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */

/*+\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
static int disp_loadfont(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);
    int fontid = platform_disp_loadfont(filename); 
   
    lua_pushinteger(L, fontid);
    return 1;
}

static int disp_setfont(lua_State *L)
{
    int fontid = luaL_checkinteger(L, 1);
    int oldfontid = platform_disp_setfont(fontid);

    lua_pushinteger(L, oldfontid);
    return 1;
}
/*-\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
/*+NEW\brezen\2016.05.13\字体缩放*/  
static int disp_setfontHeight(lua_State *L)
{
    int height = luaL_checkinteger(L, 1);
    lua_pushinteger(L, platform_disp_setfontHeight(height));
    return 1;
}

static int disp_getfontHeight(lua_State *L)
{
    lua_pushinteger(L, platform_disp_getfontHeight());
    return 1;
}
/*-NEW\brezen\2016.05.13\字体缩放*/  

/*+\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/
static int disp_write(lua_State *L)
{
    int cmd = luaL_checkinteger(L, 1);

	platform_disp_wrire(cmd);
	
    return 1;
}
/*-\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/

/*+\NewReq NEW\zhutianhua\2014.11.14\增加disp.sleep接口*/
extern void platform_lcd_powersave(int sleep_wake);
static int disp_sleep(lua_State *L) {    
    int sleep = luaL_checkinteger(L,1);

    platform_lcd_powersave(sleep);
    return 0; 
}
/*-\NewReq NEW\zhutianhua\2014.11.14\增加disp.sleep接口*/

/*+\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp库没有完全兼容2G的disp库*/
static int disp_get_lcd_info(lua_State *L)
{
    u16 width, height;
    u8 bpp;

    int id = platform_disp_set_act_layer(0);
    platform_get_lcd_info(&width, &height, &bpp);
	platform_disp_set_act_layer(id);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, bpp);

    return 3;
}
/*-\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp库没有完全兼容2G的disp库*/

static int disp_get_layer_info(lua_State *L)
{
    u16 width, height;
    u8 bpp;

    platform_get_lcd_info(&width, &height, &bpp);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, bpp);

    return 3;
}

extern int platform_disp_two(int x1, int y1, int x2, int y2);

static int disp_show_two(lua_State *L) { 
    int x1,y1,x2,y2;

     x1     = luaL_optint(L, 1, 0);
     y1     = luaL_optint(L, 2, 0);
     x2     = luaL_optint(L, 3, 0);
     y2     = luaL_optint(L, 4, 0);

    platform_disp_two(x1,y1,x2,y2);
    return 0;
}


static int disp_get_image_resolution(lua_State *L)
{
    u32 width, height;
    int result;
    
    const char* filename   = luaL_checkstring(L, 1);
    result = platform_get_png_file_resolution(filename,  &width, &height);

    lua_pushinteger(L, result);
    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    return 3;
}
/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#ifdef AM_LUA_CAMERA_SUPPORT
int disp_camera_preview_open(lua_State *L)
{
    int offsetx    = luaL_checkinteger(L, 1);
    int offsety   = luaL_checkinteger(L, 2);
    int startx    = luaL_checkinteger(L, 3);
    int starty   = luaL_checkinteger(L, 4);
    int endx    = luaL_checkinteger(L, 5);
    int endy   = luaL_checkinteger(L, 6);

    lua_pushboolean(L, platform_camera_preview_open(offsetx, offsety, startx, starty, endx, endy));
    return 1;
}

/*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
int disp_camera_preview_zoom(lua_State *L)
{
    int zoom    = luaL_checkinteger(L, 1);

    lua_pushboolean(L, platform_camera_preview_zoom(zoom));
    return 1;
}

int disp_camera_preview_rotation(lua_State *L)
{
    int rotation    = luaL_checkinteger(L, 1);

    lua_pushboolean(L, platform_camera_preview_rotation(rotation));
    return 1;
}
/*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/

/*+\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/
int camera_write_reg(lua_State *L)
{
	int nInitCmdSize = 0;
    int *pInitCmd = NULL;
    int nIndex = 0;
	
	lua_getfield(L, 1, "reglist");
    luaL_checktype(L, -1, LUA_TTABLE);
    nInitCmdSize = luaL_getn(L, -1);
    if(nInitCmdSize%2 != 0)
    {
        return luaL_error(L, "disp_camera_open_ext nInitCmdSize=%d error,must be even", nInitCmdSize);
    }
    pInitCmd = malloc(sizeof(int)*nInitCmdSize);
    if(!pInitCmd)
    {
        return luaL_error(L, "disp_camera_open_ext malloc=%d fail", sizeof(int)*nInitCmdSize);
    }
    for(nIndex = 0; nIndex < nInitCmdSize; nIndex++)
    {
        lua_rawgeti(L, -1, nIndex+1);
        pInitCmd[nIndex] = lua_tointeger(L, -1);
        lua_remove(L,-1);
    }

    lua_pushboolean(L, platform_CameraWriteReg(pInitCmd, nInitCmdSize));

    free(pInitCmd);

	return 1;
}
/*-\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/

int disp_camera_open(lua_State *L)
{
    int nCamType = 0;
    int bZbarScan = FALSE;
    int bMirror = TRUE;
    int bJump = TRUE;
    int n = 0;

    n = lua_gettop(L);
    if (n >= 1)
    {
      nCamType = luaL_checkinteger(L, 1);
    }
    if (n >= 2)
    {
      bZbarScan = luaL_checkinteger(L, 2);
    }
    if (n >= 3)
    {
      bMirror = luaL_checkinteger(L, 3);
    }
    if (n >= 4)
    {
      bJump = luaL_checkinteger(L, 4);
    }
	
    lua_pushboolean(L, platform_camera_poweron(FALSE, nCamType, bZbarScan,bMirror,bJump));
    return 1;
}
/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
int disp_camera_open_ext(lua_State *L)
{
    int zbar_scan,i2c_addr,sensor_width,sensor_height,id_reg,id_value,spi_mode;
	int spi_yuv_out,spi_speed;
	int nInitCmdSize, nIndex;
	int *pInitCmd;
	T_PLATFORM_CAMERA_PARAM param;
	PPLATFORM_CAMERA_REG initRegTable_p = NULL;
	
    luaL_checktype(L, 1, LUA_TTABLE);
	
    zbar_scan = optFiledInt(L, 1, "zbar_scan", 0);
	i2c_addr = optFiledInt(L, 1, "i2c_addr", 0);
	sensor_width = optFiledInt(L, 1, "sensor_width", 0);
	sensor_height = optFiledInt(L, 1, "sensor_height", 0);
	id_reg = optFiledInt(L, 1, "id_reg", 0);
	id_value = optFiledInt(L, 1, "id_value", 0);
	spi_mode = optFiledInt(L, 1, "spi_mode", 0);
	spi_yuv_out = optFiledInt(L, 1, "spi_yuv_out", 0);
	spi_speed = optFiledInt(L, 1, "spi_speed", 0);

    lua_getfield(L, 1, "init_cmd");
    luaL_checktype(L, -1, LUA_TTABLE);
    nInitCmdSize = luaL_getn(L, -1);
    if(nInitCmdSize%2 != 0)
    {
        return luaL_error(L, "disp_camera_open_ext nInitCmdSize=%d error,must be even", nInitCmdSize);
    }

	pInitCmd = malloc(sizeof(int)*nInitCmdSize);
    if(!pInitCmd)
    {
        return luaL_error(L, "disp_camera_open_ext malloc=%d fail", sizeof(int)*nInitCmdSize);
    }

	for(nIndex = 0; nIndex < nInitCmdSize; nIndex++)
    {
        lua_rawgeti(L, -1, nIndex+1);
        pInitCmd[nIndex] = lua_tointeger(L, -1);
        lua_remove(L,-1);
    }

	initRegTable_p = (PPLATFORM_CAMERA_REG)malloc(sizeof(PLATFORM_CAMERA_REG) * nInitCmdSize/2);
	
	if (!initRegTable_p)
	{
		return luaL_error(L, "disp_camera_open_ext malloc=%d fail", sizeof(int)*nInitCmdSize);
	}
	
	for(nIndex = 0; nIndex < nInitCmdSize/2; nIndex++)
    {
        initRegTable_p[nIndex].addr = pInitCmd[nIndex*2];
		initRegTable_p[nIndex].value = pInitCmd[nIndex*2+1];
    }

	param.i2cSlaveAddr = i2c_addr;
	param.idReg.addr = id_reg;
	param.idReg.value = id_value;
	param.sensorHeight = sensor_height;
	param.sensorWidth = sensor_width;
	param.spi_mode = spi_mode;
	param.spi_speed = spi_speed;
	param.spi_yuv_out = spi_yuv_out;
	param.initRegTable_p = initRegTable_p;
	param.initRegTableCount = nInitCmdSize/2;
    lua_pushboolean(L, platform_camera_poweron_ext(&param, zbar_scan));

    free(pInitCmd);
	free(initRegTable_p);
    
    return 1;
}
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
int disp_camera_close(lua_State *L)
{
    lua_pushboolean(L, platform_camera_poweroff());
    return 1;
}


int disp_camera_preview_close(lua_State *L)
{
    lua_pushboolean(L, platform_camera_preview_close());
    return 1;
}

int disp_camera_capture(lua_State *L)
{
	int n = lua_gettop(L);
    u16 width = luaL_checkinteger(L, 1);
    u16 height = luaL_checkinteger(L, 2);
	u16 quality;
	
	if (n == 3)
	{
	 	quality = luaL_checkinteger(L, 3);
	}
	else
	{
		quality =  50;
	}
    lua_pushboolean(L, platform_camera_capture(width, height, quality));
    return 1;
}

int encodeJpegBuffer(lua_State *L)
{
    int inBufferLen;
    int  inFormat, inWidth, inHeight, outWidth, outHeight, inQuality;
    char *inFilename;
    char *outFilename;
    u8 arg_index = 1;
    int n = 0;

    n = lua_gettop(L);

    if (n != 8)
    {
      lua_pushinteger(L, platform_encodeJpegBuffer(inFilename, inFormat, inWidth, inHeight, outWidth, outHeight, inQuality, outFilename));
      return 1;
    }

    inFilename  = luaL_checkstring(L, arg_index++);
    inFormat  = luaL_checkinteger(L, arg_index++);
    inWidth  = luaL_checkinteger(L, arg_index++);
    inHeight  = luaL_checkinteger(L, arg_index++);
    outWidth  = luaL_checkinteger(L, arg_index++);
    outHeight  = luaL_checkinteger(L, arg_index++);
    inQuality  = luaL_checkinteger(L, arg_index++);
    outFilename  = luaL_checkstring(L, arg_index++);

    lua_pushinteger(L, platform_encodeJpegBuffer(inFilename, inFormat, inWidth, inHeight, outWidth, outHeight, inQuality, outFilename));
    return 1;

}
int disp_camera_save_photo(lua_State *L)
{

    const char* filename  = luaL_checkstring(L, 1);
    lua_pushboolean(L, platform_camera_save_photo(filename));
    return 1;
}
#endif

/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
// 瑞恒 =========================================================================================================================================
/* 返回字符或字符串占用字节(byte)宽度 */
static int disp_getstringwidth(lua_State *L)
{
    const char *str;
    unsigned char m;

    str = luaL_checkstring(L, 1);
    m   = cast(unsigned char,luaL_checkinteger(L, 2));
    
    lua_pushinteger(L, disp_getcharwidth(str,m)); // lua_pushinteger表示要给LUA返回一个INT类型的值

    return 1; // 表示要给LUA返回多少个值
}
//==============================================================================================================================================

static int disp_set_act_layer(lua_State *L)
{
    int id;

	id = luaL_checkinteger(L, 1);

	lua_pushinteger(L, platform_disp_set_act_layer(id));

	return 1;
}

static int disp_destroy_layer(lua_State *L)
{
    int id;

	id = luaL_checkinteger(L, 1);

	platform_disp_destroy_layer(id);

	return 0;
}

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE disp_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( disp_init ) },
  { LSTRKEY( "close" ),  LFUNCVAL( disp_close ) },
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "update" ), LFUNCVAL( disp_update ) },
  /*+\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
  { LSTRKEY( "new" ),	LFUNCVAL( disp_new ) },
  { LSTRKEY( "getframe" ),	LFUNCVAL( disp_getframe ) },
  /*-\BUG2739\lijiaodi\2020.08.06\添加disp.new disp.getframe接口\*/ 
  { LSTRKEY( "puttext" ), LFUNCVAL( disp_puttext ) },
/*+\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */
  { LSTRKEY( "putimage" ), LFUNCVAL( disp_putimage ) },

  { LSTRKEY( "playgif" ), LFUNCVAL( disp_playgif) },

/*-\NEW\liweiqiang\2013.11.4\增加BMP图片显示支持 */
  { LSTRKEY( "stopgif" ), LFUNCVAL( disp_stopgif ) },

/*+\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */
  { LSTRKEY( "drawrect" ), LFUNCVAL( disp_drawrect ) },
/*-\NEW\liweiqiang\2013.12.7\增加矩形显示支持 */

/*+\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */
  { LSTRKEY( "putqrcode" ), LFUNCVAL( disp_qrcode ) },
/*-\NEW\shenyuanyuan\2020.3.31\开发移植disp的二维码显示接口 */

/*+\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */
  { LSTRKEY( "setcolor" ), LFUNCVAL( disp_setcolor ) },
  { LSTRKEY( "setbkcolor" ), LFUNCVAL( disp_setbkcolor ) },
/*-\NEW\liweiqiang\2013.12.9\增加前景色\背景色设置 */

/*+\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
  { LSTRKEY( "loadfont" ), LFUNCVAL( disp_loadfont ) },
  { LSTRKEY( "setfont" ), LFUNCVAL( disp_setfont ) },
/*-\NEW\liweiqiang\2013.12.9\增加非中文字体设置 */
/*+NEW\brezen\2016.05.13\字体缩放*/  
  { LSTRKEY( "setfontheight" ), LFUNCVAL( disp_setfontHeight ) },
  { LSTRKEY( "getfontheight" ), LFUNCVAL( disp_getfontHeight ) },
/*-NEW\brezen\2016.05.13\字体缩放*/  
  /*+\NewReq NEW\zhutianhua\2014.11.14\增加disp.sleep接口*/
  { LSTRKEY( "sleep" ), LFUNCVAL( disp_sleep ) },
  /*-\NewReq NEW\zhutianhua\2014.11.14\增加disp.sleep接口*/
  /*+\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp库没有完全兼容2G的disp库*/
  { LSTRKEY( "getlcdinfo" ), LFUNCVAL( disp_get_lcd_info ) },
  { LSTRKEY( "getlayerinfo" ), LFUNCVAL( disp_get_layer_info ) },
  /*-\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp库没有完全兼容2G的disp库*/
  /*+\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/
  { LSTRKEY( "write" ), LFUNCVAL( disp_write ) },
  /*-\BUG\shenyuanyuan\2020.06.02\BUG_1983\添加disp.write()接口，解决刷屏不正常的问题*/

  { LSTRKEY( "setactlayer" ), LFUNCVAL( disp_set_act_layer) },
  { LSTRKEY( "destroylayer" ), LFUNCVAL( disp_destroy_layer) },
  
#ifdef AM_LAYER_SUPPORT
  { LSTRKEY( "createuserlayer" ), LFUNCVAL( disp_createuserlayer) },
  { LSTRKEY( "destroyuserlayer" ), LFUNCVAL( disp_destroyuserlayer) },
  { LSTRKEY( "setactivelayer" ), LFUNCVAL( disp_setactivelayer) },
  { LSTRKEY( "copylayer" ), LFUNCVAL( disp_copy_layer) },
  { LSTRKEY( "preloadpng" ), LFUNCVAL(disp_preloadpng) },
  { LSTRKEY( "layerdisplay" ), LFUNCVAL( disp_layer_display ) },
  { LSTRKEY( "getimageresolution"), LFUNCVAL( disp_get_image_resolution ) },
#endif

/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#ifdef AM_LUA_CAMERA_SUPPORT
  { LSTRKEY( "cameraopen" ),     LFUNCVAL( disp_camera_open ) },
  { LSTRKEY( "cameraopen_ext" ),     LFUNCVAL( disp_camera_open_ext ) },
  { LSTRKEY( "cameraclose" ),     LFUNCVAL( disp_camera_close ) },
  { LSTRKEY( "camerapreview" ),  LFUNCVAL( disp_camera_preview_open ) },
  /*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
  { LSTRKEY( "camerapreviewzoom" ),  LFUNCVAL( disp_camera_preview_zoom ) },
  { LSTRKEY( "camerapreviewrotation" ),  LFUNCVAL( disp_camera_preview_rotation ) },
  /*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
  { LSTRKEY( "camerapreviewclose" ),  LFUNCVAL( disp_camera_preview_close ) },
  { LSTRKEY( "cameracapture" ),  LFUNCVAL( disp_camera_capture ) },
  { LSTRKEY( "camerasavephoto" ),  LFUNCVAL( disp_camera_save_photo ) },
  { LSTRKEY( "encodeJpeg" ), LFUNCVAL( encodeJpegBuffer ) },
   /*+\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/
  { LSTRKEY( "camerawritereg" ),     LFUNCVAL( camera_write_reg ) },
  /*-\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/
#endif
/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/

// 瑞恒 ========================================================================================================================================
  { LSTRKEY( "getstringwidth"), LFUNCVAL( disp_getstringwidth ) }, // 注册函数至Lua层
//==============================================================================================================================================

#ifdef TOUCH_PANEL_SUPPORT
  /*-\NEW\zhuwangbin\2015.2.23\LUA 操作图层平移，改成底层操作 */
  { LSTRKEY( "layermovestart"), LFUNCVAL(disp_layer_start_move) },
  /*-\NEW\zhuwangbin\2015.2.23\LUA 操作图层平移，改成底层操作 */

  /*-\NEW\zhuwangbin\2015.2.26\lua 图层悬停改到底层做*/
  { LSTRKEY( "layermovehangstart"), LFUNCVAL(disp_layer_hang_start) },
  { LSTRKEY( "layersetpicture"), LFUNCVAL(disp_layer_set_picture) },
  { LSTRKEY( "layersetext"), LFUNCVAL(disp_layer_set_text) },
  { LSTRKEY( "layersetqrcode"), LFUNCVAL(disp_layer_set_RQcode)},
  { LSTRKEY( "layersetdrawrect"), LFUNCVAL(disp_layer_set_drawRect)},
  { LSTRKEY( "layermovehangstop"), LFUNCVAL(disp_layer_hang_stop)},
  /*-\NEW\zhuwangbin\2015.2.26\lua 图层悬停改到底层做*/
#endif

  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_disp( lua_State *L )
{
  luaL_register( L, AUXLIB_DISP, disp_map );

  MOD_REG_NUMBER(L, "BUS_SPI4LINE", PLATFORM_LCD_BUS_SPI4LINE);
  MOD_REG_NUMBER(L, "BUS_PARALLEL", PLATFORM_LCD_BUS_PARALLEL);
/*+\new\liweiqiang\2014.10.22\lcd不同接口信息定义 */
  MOD_REG_NUMBER(L, "BUS_I2C", PLATFORM_LCD_BUS_I2C);
  MOD_REG_NUMBER(L, "BUS_SPI", PLATFORM_LCD_BUS_SPI);
/*-\new\liweiqiang\2014.10.22\lcd不同接口信息定义 */

  MOD_REG_NUMBER(L, "BASE_LAYER", BASIC_LAYER_ID);
  MOD_REG_NUMBER(L, "USER_LAYER1", USER_LAYER_1_ID);
  MOD_REG_NUMBER(L, "USER_LAYER2", USER_LAYER_2_ID);
  MOD_REG_NUMBER(L, "INVALID_LAYER", INVALID_LAYER_ID);

/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
  MOD_REG_NUMBER(L, "CAMERA_SPI_MODE_LINE1", PLATFORM_SPI_MODE_MASTER2_1);
  MOD_REG_NUMBER(L, "CAMERA_SPI_MODE_LINE2", PLATFORM_SPI_MODE_MASTER2_2);
  MOD_REG_NUMBER(L, "CAMERA_SPI_MODE_LINE4", PLATFORM_SPI_MODE_MASTER2_4);

  MOD_REG_NUMBER(L, "CAMERA_SPEED_SDR", PLATFORM_SPI_SPEED_SDR);
  MOD_REG_NUMBER(L, "CAMERA_SPEED_DDR", PLATFORM_SPI_SPEED_DDR);
  MOD_REG_NUMBER(L, "CAMERA_SPEED_QDR", PLATFORM_SPI_SPEED_QDR);

  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_Y0_U0_Y1_V0", PLATFORM_SPI_OUT_Y0_U0_Y1_V0);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_Y0_V0_Y1_U0", PLATFORM_SPI_OUT_Y0_V0_Y1_U0);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_U0_Y0_V0_Y1", PLATFORM_SPI_OUT_U0_Y0_V0_Y1);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_U0_Y1_V0_Y0", PLATFORM_SPI_OUT_U0_Y1_V0_Y0);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_V0_Y0_U0_Y1", PLATFORM_SPI_OUT_V0_Y0_U0_Y1);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_Y1_V0_Y0_U0", PLATFORM_SPI_OUT_Y1_V0_Y0_U0);
  MOD_REG_NUMBER(L, "CAMERA_SPI_OUT_Y1_U0_Y0_V0", PLATFORM_SPI_OUT_Y1_U0_Y0_V0);
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/

  return 1;
}

#ifdef LUA_LVGL_SUPPORT
#include "osi_compiler.h"
// OSI_STRONG_ALIAS(disp_init, lv_disp_init);
int lv_disp_init(lua_State *L)
{
	return disp_init(L);
}
// OSI_STRONG_ALIAS(disp_close, lv_disp_close);
int lv_disp_close(lua_State *L)
{
	return disp_close(L);
}
// OSI_STRONG_ALIAS(disp_get_lcd_info, lv_disp_get_lcd_info);
int lv_disp_get_lcd_info(lua_State *L)
{
	return disp_get_lcd_info(L);
}
// OSI_STRONG_ALIAS(disp_loadfont, lv_font_load_font);
int lv_font_load_font(lua_State *L)
{
	return disp_loadfont(L);
}
// OSI_STRONG_ALIAS(disp_setfont, lv_font_set_font);
int lv_font_set_font(lua_State *L)
{
	return disp_setfont(L);
}
#endif

#endif
