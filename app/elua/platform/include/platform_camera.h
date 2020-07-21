/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#ifndef __PLATFORM_CAMERA_H__
#define __PLATFORM_CAMERA_H__

BOOL platform_camera_poweron(BOOL video_mode, int nCamType, BOOL bZbarScan, BOOL bMirror, BOOL bJump);


BOOL platform_camera_poweroff(void);


BOOL platform_camera_preview_open(u16 offsetx, u16 offsety,u16 startx, u16 starty, u16 endx, u16 endy);



BOOL platform_camera_preview_close(void);



BOOL platform_camera_capture(u16 width, u16 height, u16 quality);


BOOL platform_camera_save_photo(const char* filename);

#endif
/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
