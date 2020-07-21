/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    disp.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/3/26
 *
 * Description:
 *          lua.disp��
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

static u8 putimage_assert_fail = FALSE;

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
    
/*+\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */
    //16λɫ����or�ڰ���
    if(!(param.bpp == 16 || param.bpp == 1 || param.bpp == 24))
    {
        return luaL_error(L, "disp.init: pixel depth must be 16 or 1!%d", param.bpp); 
    }
    
    // lcd����ӿ�
    param.bus = (PlatformLcdBus)getFiledInt(L, 1, "bus"); // panjun, 2015.04.21, Commit SSD1306's driver code.

    /*+\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */
    // ��ͬ����ӿڶ���
    if(param.bus == PLATFORM_LCD_BUS_I2C || param.bus == PLATFORM_LCD_BUS_SPI){
        lua_getfield(L, 1, "interface");
        luaL_checktype(L, -1, LUA_TTABLE);

        if(param.bus == PLATFORM_LCD_BUS_I2C){
            param.lcd_itf.bus_i2c.bus_id = checkFiledInt(L, -1, "bus_id");
            param.lcd_itf.bus_i2c.freq = checkFiledInt(L, -1, "freq");
            param.lcd_itf.bus_i2c.slave_addr = checkFiledInt(L, -1, "slave_addr");
            param.lcd_itf.bus_i2c.cmd_addr = checkFiledInt(L, -1, "cmd_addr");
            param.lcd_itf.bus_i2c.data_addr = checkFiledInt(L, -1, "data_addr");
        } else if(param.bus == PLATFORM_LCD_BUS_SPI){
            param.lcd_itf.bus_spi.bus_id = checkFiledInt(L, -1, "bus_id");
            param.lcd_itf.bus_spi.pin_rs = checkFiledInt(L, -1, "pin_rs");
            param.lcd_itf.bus_spi.pin_cs = optFiledInt(L, -1, "pin_cs", PLATFORM_IO_UNKNOWN_PIN);
            param.lcd_itf.bus_spi.freq = checkFiledInt(L, -1, "freq");
        }
    }
    /*-\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */    

    // lcd rst�ű��붨��
    param.pin_rst = checkFiledInt(L, 1, "pinrst");

    lua_getfield(L, 1, "pincs");

    if(lua_type(L,-1) != LUA_TNUMBER)
        param.pin_cs = PLATFORM_IO_UNKNOWN_PIN;
    else
        param.pin_cs = lua_tonumber(L,-1);

    // ����ƫ����Ĭ��0
    param.x_offset = getFiledInt(L, 1, "xoffset");
    param.y_offset = getFiledInt(L, 1, "yoffset");
    
    /*+\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */
    param.hwfillcolor = optFiledInt(L, 1, "hwfillcolor", -1);
    /*-\new\liweiqiang\2014.10.21\���Ӳ�ͬ�ڰ������ɫ���� */

    // .initcmd ��ʼ��ָ���
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
/*-\NEW\2013.4.10\���Ӻڰ�����ʾ֧�� */

/*+\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
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
/*-\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */

    platform_disp_init(&param);

    L_FREE(param.pLcdCmdTable);

    /*+\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */
    if(param.pLcdSleepCmd)
        L_FREE(param.pLcdSleepCmd);

    if(param.pLcdWakeCmd)
        L_FREE(param.pLcdWakeCmd);
    /*-\NEW\liweiqiang\2013.12.18\����lcd˯������֧�� */

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

/*+\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
//disp.putimage
static int disp_putimage(lua_State *L) {
    const char *filename;
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    u16 x, y, left, top, right, bottom;
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    int transcolor,transtype;

    ASSERT(putimage_assert_fail==FALSE);
    putimage_assert_fail = TRUE;

    filename   = luaL_checkstring(L, 1);
    x     = luaL_optint(L, 2, 0);
/*+\NEW\liweiqiang\2013.11.12\������ʾͼƬy�����޷����� */
    y     = luaL_optint(L, 3, 0);
/*-\NEW\liweiqiang\2013.11.12\������ʾͼƬy�����޷����� */

/*+\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    transcolor = luaL_optint(L, 4, -1); //Ĭ�ϲ�͸��
    left = luaL_optint(L, 5, 0);
    top = luaL_optint(L, 6, 0);
    right = luaL_optint(L, 7, 0);
    bottom = luaL_optint(L, 8, 0);
    transtype = luaL_optint(L, 9, 1);


    platform_disp_putimage(filename, x, y, transcolor, left, top, right, bottom,transtype);
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
/*-\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */

    putimage_assert_fail = FALSE;
    
    return 0; 
}
/*-\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */



static int disp_preloadpng(lua_State *L) {
    const char *filename;
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    u16  index;
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/

    filename   = luaL_checkstring(L, 1);
    index     = luaL_optint(L, 2, 0);
   

    platform_disp_preload_png_to_layer(filename, index);
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
/*-\NEW\liweiqiang\2013.12.6\����ͼƬ͸��ɫ���� */    
    return 0; 
}

#ifdef AM_LAYER_SUPPORT
static int disp_layer_display(lua_State *L) {
    /*+\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
    int layer_id1, layer_id2, layer_id3, x1,y1,x2,y2,x3,y3;
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/

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
    /*-\NewReq NEW\zhutianhua\2013.12.24\��ʾͼƬ��ָ������*/
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

/*-\NEW\zhuwangbin\2015.2.23\LUA ����ͼ��ƽ�ƣ��ĳɵײ���� */
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
/*-\NEW\zhuwangbin\2015.2.23\LUA ����ͼ��ƽ�ƣ��ĳɵײ���� */


/*-\NEW\zhuwangbin\2015.2.26\lua ͼ����ͣ�ĵ��ײ���*/
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
  BOOL colorSetBool = FALSE; 	//lua�Ƿ�����color��־
  int total;

  total = lua_gettop(L);
  file_id = luaL_optint(L, 1, 0);
  textString = luaL_checkstring(L, 2);
  x = luaL_optint(L, 3, 0);
  y  = luaL_optint(L, 4, 0);
  color = luaL_optint(L, 5, 0);

	// ֻ��lua����color����ʱ��������������ɫ
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
/*-\NEW\zhuwangbin\2015.2.26\lua ͼ����ͣ�ĵ��ײ���*/
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




/*+\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */
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
/*-\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */

/*+\NEW\shenyuanyuan\2020.3.31\������ֲdisp�Ķ�ά����ʾ�ӿ� */
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
/*-\NEW\shenyuanyuan\2020.3.31\������ֲdisp�Ķ�ά����ʾ�ӿ� */

/*+\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */
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
/*-\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */

/*+\NEW\liweiqiang\2013.12.9\���ӷ������������� */
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
/*-\NEW\liweiqiang\2013.12.9\���ӷ������������� */
/*+NEW\brezen\2016.05.13\��������*/  
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
/*-NEW\brezen\2016.05.13\��������*/  

/*+\BUG\shenyuanyuan\2020.06.02\BUG_1983\����disp.write()�ӿڣ����ˢ��������������*/
static int disp_write(lua_State *L)
{
    int cmd = luaL_checkinteger(L, 1);

	platform_disp_wrire(cmd);
	
    return 1;
}
/*-\BUG\shenyuanyuan\2020.06.02\BUG_1983\����disp.write()�ӿڣ����ˢ��������������*/

/*+\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/
extern void platform_lcd_powersave(int sleep_wake);
static int disp_sleep(lua_State *L) {    
    int sleep = luaL_checkinteger(L,1);

    platform_lcd_powersave(sleep);
    return 0; 
}
/*-\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/

/*+\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp��û����ȫ����2G��disp��*/
static int disp_get_lcd_info(lua_State *L)
{
    u16 width, height;
    u8 bpp;

    platform_get_lcd_info(&width, &height, &bpp);

    lua_pushinteger(L, width);
    lua_pushinteger(L, height);
    lua_pushinteger(L, bpp);

    return 3;
}
/*-\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp��û����ȫ����2G��disp��*/

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
/*+\NEW\zhuwangbin\2020.05.01\����disp camera����*/
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

int disp_camera_open_ext(lua_State *L)
{
    int nCamType = 1;
    int bZbarScan = FALSE;
    int bMirror = FALSE;
    int bJump = FALSE;
    int nInitCmdSize = 0;
    int *pInitCmd = NULL;
    int nIndex = 0;

    luaL_checktype(L, 1, LUA_TTABLE);

    nCamType = optFiledInt(L, 1, "camType", 1);
    bZbarScan = optFiledInt(L, 1, "zbarScan", 0);
    bMirror = optFiledInt(L, 1, "mirror", 0);
    bJump = optFiledInt(L, 1, "jump", 0);

    lua_getfield(L, 1, "initCmd");
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

    lua_pushboolean(L, platform_camera_poweron_ext(FALSE, nCamType, bZbarScan,bMirror,bJump, nInitCmdSize, pInitCmd));

    free(pInitCmd);
    
    return 1;
}

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

/*-\NEW\zhuwangbin\2020.05.01\����disp camera����*/
// ��� =========================================================================================================================================
/* �����ַ����ַ���ռ���ֽ�(byte)���� */
static int disp_getstringwidth(lua_State *L)
{
    const char *str;
    unsigned char m;

    str = luaL_checkstring(L, 1);
    m   = cast(unsigned char,luaL_checkinteger(L, 2));
    
    lua_pushinteger(L, disp_getcharwidth(str,m)); // lua_pushinteger��ʾҪ��LUA����һ��INT���͵�ֵ

    return 1; // ��ʾҪ��LUA���ض��ٸ�ֵ
}
//==============================================================================================================================================

#define MIN_OPT_LEVEL 2
#include "lrodefs.h"  

// Module function map
const LUA_REG_TYPE disp_map[] =
{ 
  { LSTRKEY( "init" ),  LFUNCVAL( disp_init ) },
  { LSTRKEY( "close" ),  LFUNCVAL( disp_close ) },
  { LSTRKEY( "clear" ), LFUNCVAL( disp_clear ) },
  { LSTRKEY( "update" ), LFUNCVAL( disp_update ) },
  { LSTRKEY( "puttext" ), LFUNCVAL( disp_puttext ) },
/*+\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
  { LSTRKEY( "putimage" ), LFUNCVAL( disp_putimage ) },

  { LSTRKEY( "playgif" ), LFUNCVAL( disp_playgif) },

/*-\NEW\liweiqiang\2013.11.4\����BMPͼƬ��ʾ֧�� */
  { LSTRKEY( "stopgif" ), LFUNCVAL( disp_stopgif ) },

/*+\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */
  { LSTRKEY( "drawrect" ), LFUNCVAL( disp_drawrect ) },
/*-\NEW\liweiqiang\2013.12.7\���Ӿ�����ʾ֧�� */

/*+\NEW\shenyuanyuan\2020.3.31\������ֲdisp�Ķ�ά����ʾ�ӿ� */
  { LSTRKEY( "putqrcode" ), LFUNCVAL( disp_qrcode ) },
/*-\NEW\shenyuanyuan\2020.3.31\������ֲdisp�Ķ�ά����ʾ�ӿ� */

/*+\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */
  { LSTRKEY( "setcolor" ), LFUNCVAL( disp_setcolor ) },
  { LSTRKEY( "setbkcolor" ), LFUNCVAL( disp_setbkcolor ) },
/*-\NEW\liweiqiang\2013.12.9\����ǰ��ɫ\����ɫ���� */

/*+\NEW\liweiqiang\2013.12.9\���ӷ������������� */
  { LSTRKEY( "loadfont" ), LFUNCVAL( disp_loadfont ) },
  { LSTRKEY( "setfont" ), LFUNCVAL( disp_setfont ) },
/*-\NEW\liweiqiang\2013.12.9\���ӷ������������� */
/*+NEW\brezen\2016.05.13\��������*/  
  { LSTRKEY( "setfontheight" ), LFUNCVAL( disp_setfontHeight ) },
  { LSTRKEY( "getfontheight" ), LFUNCVAL( disp_getfontHeight ) },
/*-NEW\brezen\2016.05.13\��������*/  
  /*+\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/
  { LSTRKEY( "sleep" ), LFUNCVAL( disp_sleep ) },
  /*-\NewReq NEW\zhutianhua\2014.11.14\����disp.sleep�ӿ�*/
  /*+\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp��û����ȫ����2G��disp��*/
  { LSTRKEY( "getlcdinfo" ), LFUNCVAL( disp_get_lcd_info ) },
  /*-\BUG\shenyuanyuan\2020.04.09\BUG_1459\disp��û����ȫ����2G��disp��*/
  /*+\BUG\shenyuanyuan\2020.06.02\BUG_1983\����disp.write()�ӿڣ����ˢ��������������*/
  { LSTRKEY( "write" ), LFUNCVAL( disp_write ) },
  /*-\BUG\shenyuanyuan\2020.06.02\BUG_1983\����disp.write()�ӿڣ����ˢ��������������*/
  
#ifdef AM_LAYER_SUPPORT
  { LSTRKEY( "createuserlayer" ), LFUNCVAL( disp_createuserlayer) },
  { LSTRKEY( "destroyuserlayer" ), LFUNCVAL( disp_destroyuserlayer) },
  { LSTRKEY( "setactivelayer" ), LFUNCVAL( disp_setactivelayer) },
  { LSTRKEY( "copylayer" ), LFUNCVAL( disp_copy_layer) },
  { LSTRKEY( "preloadpng" ), LFUNCVAL(disp_preloadpng) },
  { LSTRKEY( "layerdisplay" ), LFUNCVAL( disp_layer_display ) },
  { LSTRKEY( "getimageresolution"), LFUNCVAL( disp_get_image_resolution ) },
#endif

/*+\NEW\zhuwangbin\2020.05.01\����disp camera����*/
#ifdef AM_LUA_CAMERA_SUPPORT
  { LSTRKEY( "cameraopen" ),     LFUNCVAL( disp_camera_open ) },
  { LSTRKEY( "cameraopen_ext" ),     LFUNCVAL( disp_camera_open_ext ) },
  { LSTRKEY( "cameraclose" ),     LFUNCVAL( disp_camera_close ) },
  { LSTRKEY( "camerapreview" ),  LFUNCVAL( disp_camera_preview_open ) },
  { LSTRKEY( "camerapreviewclose" ),  LFUNCVAL( disp_camera_preview_close ) },
  { LSTRKEY( "cameracapture" ),  LFUNCVAL( disp_camera_capture ) },
  { LSTRKEY( "camerasavephoto" ),  LFUNCVAL( disp_camera_save_photo ) },
  { LSTRKEY( "encodeJpeg" ), LFUNCVAL( encodeJpegBuffer ) },
#endif
/*-\NEW\zhuwangbin\2020.05.01\����disp camera����*/

// ��� ========================================================================================================================================
  { LSTRKEY( "getstringwidth"), LFUNCVAL( disp_getstringwidth ) }, // ע�ắ����Lua��
//==============================================================================================================================================

#ifdef TOUCH_PANEL_SUPPORT
  /*-\NEW\zhuwangbin\2015.2.23\LUA ����ͼ��ƽ�ƣ��ĳɵײ���� */
  { LSTRKEY( "layermovestart"), LFUNCVAL(disp_layer_start_move) },
  /*-\NEW\zhuwangbin\2015.2.23\LUA ����ͼ��ƽ�ƣ��ĳɵײ���� */

  /*-\NEW\zhuwangbin\2015.2.26\lua ͼ����ͣ�ĵ��ײ���*/
  { LSTRKEY( "layermovehangstart"), LFUNCVAL(disp_layer_hang_start) },
  { LSTRKEY( "layersetpicture"), LFUNCVAL(disp_layer_set_picture) },
  { LSTRKEY( "layersetext"), LFUNCVAL(disp_layer_set_text) },
  { LSTRKEY( "layersetqrcode"), LFUNCVAL(disp_layer_set_RQcode)},
  { LSTRKEY( "layersetdrawrect"), LFUNCVAL(disp_layer_set_drawRect)},
  { LSTRKEY( "layermovehangstop"), LFUNCVAL(disp_layer_hang_stop)},
  /*-\NEW\zhuwangbin\2015.2.26\lua ͼ����ͣ�ĵ��ײ���*/
#endif

  { LNILKEY, LNILVAL }
};

LUALIB_API int luaopen_disp( lua_State *L )
{
  luaL_register( L, AUXLIB_DISP, disp_map );

  MOD_REG_NUMBER(L, "BUS_SPI4LINE", PLATFORM_LCD_BUS_SPI4LINE);
  MOD_REG_NUMBER(L, "BUS_PARALLEL", PLATFORM_LCD_BUS_PARALLEL);
/*+\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */
  MOD_REG_NUMBER(L, "BUS_I2C", PLATFORM_LCD_BUS_I2C);
  MOD_REG_NUMBER(L, "BUS_SPI", PLATFORM_LCD_BUS_SPI);
/*-\new\liweiqiang\2014.10.22\lcd��ͬ�ӿ���Ϣ���� */

  MOD_REG_NUMBER(L, "BASE_LAYER", BASIC_LAYER_ID);
  MOD_REG_NUMBER(L, "USER_LAYER1", USER_LAYER_1_ID);
  MOD_REG_NUMBER(L, "USER_LAYER2", USER_LAYER_2_ID);
  MOD_REG_NUMBER(L, "INVALID_LAYER", INVALID_LAYER_ID);

  return 1;
}  

#endif