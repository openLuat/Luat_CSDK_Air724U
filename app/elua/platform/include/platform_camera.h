/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#ifndef __PLATFORM_CAMERA_H__
#define __PLATFORM_CAMERA_H__

/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
typedef struct T_PLATFORM_CAMERA_REG_TAG
{
    UINT16      addr;
    UINT16      value;
}PLATFORM_CAMERA_REG, *PPLATFORM_CAMERA_REG;

typedef enum
{
    PLATFORM_SPI_MODE_NO = 0,         // parallel sensor in use
    PLATFORM_SPI_MODE_SLAVE ,        // SPI sensor as SPI slave
    PLATFORM_SPI_MODE_MASTER1,     // SPI sensor as SPI master, 1 sdo output with SSN 
    PLATFORM_SPI_MODE_MASTER2_1, // SPI sensor as SPI master, 1 sdo output without SSN
    PLATFORM_SPI_MODE_MASTER2_2, // SPI sensor as SPI master, 2 sdos output 
    PLATFORM_SPI_MODE_MASTER2_4, // SPI sensor as SPI master, 4 sdos output
    PLATFORM_SPI_MODE_UNDEF,
} PLATFORM_CAMERA_SPI_MODE_E;

typedef enum
{
    PLATFORM_SPI_OUT_Y0_U0_Y1_V0 = 0,
    PLATFORM_SPI_OUT_Y0_V0_Y1_U0,
    PLATFORM_SPI_OUT_U0_Y0_V0_Y1,
    PLATFORM_SPI_OUT_U0_Y1_V0_Y0,
    PLATFORM_SPI_OUT_V0_Y1_U0_Y0,
    PLATFORM_SPI_OUT_V0_Y0_U0_Y1,
    PLATFORM_SPI_OUT_Y1_V0_Y0_U0,
    PLATFORM_SPI_OUT_Y1_U0_Y0_V0,
} PLATFORM_CAMERA_SPI_YUV_OUT_E;

typedef enum
{
	PLATFORM_SPI_SPEED_DEFAULT,
	PLATFORM_SPI_SPEED_SDR, /*单倍速率*/
	PLATFORM_SPI_SPEED_DDR, /*双倍速率*/
	PLATFORM_SPI_SPEED_QDR, /*四倍速率 暂不支持*/
}PLATFORM_SPI_SPEED_MODE_E;

/*camera sensor属性配置*/
typedef struct T_PLATFORM_CAMERA_PARAM_TAG
{
    UINT8       i2cSlaveAddr;               /* 摄像头i2c访问地址 */
    UINT16      sensorWidth;                /* 摄像头的宽 */
    UINT16      sensorHeight;				/* 摄像头的高 */    
    PPLATFORM_CAMERA_REG initRegTable_p;  /* 摄像头初始化指令表 */
    UINT16 initRegTableCount;          /* 摄像头初始化指令数 */
    PLATFORM_CAMERA_REG idReg;          /* 摄像头ID寄存器与值 */
    PLATFORM_CAMERA_SPI_MODE_E       spi_mode; /*摄像头SPI模式*/
    PLATFORM_CAMERA_SPI_YUV_OUT_E  spi_yuv_out; /*摄像头YUV格式*/
	PLATFORM_SPI_SPEED_MODE_E spi_speed; /*摄像头采集速率*/
}T_PLATFORM_CAMERA_PARAM;
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/

BOOL platform_camera_poweron(BOOL video_mode, int nCamType, BOOL bZbarScan, BOOL bMirror, BOOL bJump);


BOOL platform_camera_poweroff(void);


BOOL platform_camera_preview_open(u16 offsetx, u16 offsety,u16 startx, u16 starty, u16 endx, u16 endy);



BOOL platform_camera_preview_close(void);



BOOL platform_camera_capture(u16 width, u16 height, u16 quality);


BOOL platform_camera_save_photo(const char* filename);


/*+\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/
BOOL platform_CameraWriteReg(int *pInitCmd, int nInitCmdSize);
/*+\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/

/*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
BOOL platform_camera_preview_zoom(int zoom);
BOOL platform_camera_preview_rotation(int rotation);
/*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/


#endif
/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
