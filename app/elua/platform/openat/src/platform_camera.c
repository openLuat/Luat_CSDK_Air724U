/*+\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
#ifdef AM_LUA_CAMERA_SUPPORT
#include "string.h"
#include "malloc.h"
/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
#include "osi_api.h"
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
#include "assert.h"
#include "am_openat.h"

#include "platform.h"
#include "platform_conf.h"
#include "platform_rtos.h"
#include "platform_disp.h"
#include "platform_malloc.h"
#include "platform_camera.h"

#define CAM_SENSOR_WIDTH (320)
#define CAM_SENSOR_HEIGHT (240)
#define CAM_DISP_WIDTH (132)
#define CAM_DISP_HEIGHT (162)
#define CAM_FUNC() OPENAT_print("[CCAM]:%s line:%d",__func__, __LINE__)

extern kal_uint8 *workingbuffer;
extern int find_dm_entry( const char* name, char **pactname );
typedef enum
{
	CCAM_TYPE_OPEN,
	CCAM_TYPE_CLOSE,
	CCAM_TYPE_SCANNER_START,
	CCAM_TYPE_SCANNER_STOP
}camType_t;

typedef struct 
{
	BOOL video_mode;
	int nCamType;
	BOOL bZbarScan;
	BOOL bMirror;
	BOOL bJump;
	int nInitCmdSize;
	int *pInitCmd;
}camOpenParam_t;
/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
typedef struct
{
	int sensorWidth;
	int sensorHeight;
	BOOL bZbarScan;
}camExternParam_t;

typedef struct
{
	osiWork_t *work;
    osiWorkQueue_t *wq;
	UINT8* scannerBuff;
}camScanParan_t;

typedef struct
{
	camOpenParam_t openParam;
	T_AMOPENAT_CAM_PREVIEW_PARAM previewParam;
	camScanParan_t scan;
	camExternParam_t exCamera;
}platCamCtx_t;
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
platCamCtx_t platCamCtx;

/*+\NEW\zhuwangbin\2020.07.16\重新整理camera寄存器和配置逻辑*/
static AMOPENAT_CAMERA_REG prvCameraInitReg[]  =
{
	
	//GC0310_OUTPUT_MODE_PACKET_DDR_2BIT
	//VGA setting	
	/////////////////////////////////////////////////
	/////////////////   system reg  /////////////////
	/////////////////////////////////////////////////
	{0xfe,0xf0},
	{0xfe,0xf0},
	{0xfe,0x00},

	{0xfc,0x16}, //4
	{0xfc,0x16}, //4 
	{0xf2,0x07}, //sync output
	{0xf3,0x83}, //ff//1f//01 data output
	{0xf5,0x07}, //sck_dely

	{0xf7,0x89}, //f8//fd   88
	{0xf8,0x01},           // 00
	{0xf9,0x4f}, //0f//01   4d
	{0xfa,0x11}, //32

	{0xfc,0xce},
	{0xfd,0x00},
	/////////////////////////////////////////////////
	/////////////////   CISCTL reg  /////////////////
	/////////////////////////////////////////////////
	{0x00,0x2f}, 
	{0x01,0x0f}, 
	{0x02,0x04},


	{0x03,0x02},
	{0x04,0x12},

	{0x09,0x00}, 
	{0x0a,0x00}, 
	{0x0b,0x00}, 
	{0x0c,0x04}, 
	{0x0d,0x01}, 
	{0x0e,0xe8}, 
	{0x0f,0x02}, 
	{0x10,0x88}, 
	{0x16,0x00},
	{0x17,0x14},
	{0x18,0x1a}, 
	{0x19,0x14}, 
	{0x1b,0x48},
	{0x1c, 0x6c},
	{0x1e,0x6b}, 
	{0x1f,0x28},			

	{0x20,0x8b}, 

	{0x21,0x49}, 
	{0x22, 0xd0}, //b0 travis 20140929
	{0x23,0x04}, 
	{0x24,0xff}, 
	{0x34,0x20}, 
	/////////////////////////////////////////////////
	////////////////////   BLK   ////////////////////
	/////////////////////////////////////////////////
	{0x26,0x23}, //[1]dark_current_en [0]offset_en
	{0x28,0xff}, //BLK_limie_value
	{0x29,0x00}, //global offset
	{0x32, 0x04},//00  travis 20140929
	{0x33,0x10}, //offset_ratio
	{0x37,0x20}, //dark_current_ratio
	{0x38,0x10}, 
	{0x47,0x80}, //a7
	{0x4e,0x66}, //select_row
	{0xa8,0x02}, //win_width_dark, same with crop_win_width
	{0xa9,0x80},
	/////////////////////////////////////////////////
	//////////////////   ISP reg  ///////////////////
	/////////////////////////////////////////////////
	{0x40,0xff}, 
	{0x41,0x21},//00//[0]curve_en
	{0x42,0xcf},  //0a//[1]awn_en
	{0x44,0x02}, 
	{0x45,0xa8}, 
	{0x46,0x02}, //03//sync
	{0x4a,0x11}, 
	{0x4b,0x01}, 
	{0x4c,0x20},//00[5]pretect exp
	{0x4d,0x05},//update gain mode
	{0x4f,0x01}, 
	{0x50,0x01},//crop enable
	{0x55,0x01},//crop window height
	{0x56,0xe0}, 
	{0x57,0x02},//crop window width
	{0x58,0x80}, 


	/////////////////////////////////////////////////  
	///////////////////   GAIN   ////////////////////
	/////////////////////////////////////////////////
	{0x70,0x70},
	{0x5a,0x84},
	{0x5b,0xc9},
	{0x5c,0xed},
	{0x77,0x74},
	{0x78,0x40},
	{0x79,0x5f}, 


	///////////////////////////////////////////////// 
	///////////////////   DNDD  /////////////////////
	///////////////////////////////////////////////// 
	{0x82,0x08},//0x14 
	{0x83,0x0b},
	{0x89,0xf0},
	///////////////////////////////////////////////// 
	//////////////////   EEINTP  ////////////////////
	///////////////////////////////////////////////// 
	{0x8f,0xaa},
	{0x90,0x8c},
	{0x91,0x90},
	{0x92,0x03},
	{0x93,0x03},
	{0x94,0x05},
	{0x95,0x43}, //0x65
	{0x96,0xf0}, 
	///////////////////////////////////////////////// 
	/////////////////////  ASDE  ////////////////////
	///////////////////////////////////////////////// 
	{0xfe,0x00},

	{0x9a, 0x20}, 
	{0x9b, 0x80}, 
	{0x9c, 0x40}, 
	{0x9d, 0x80}, 
	{0xa1, 0x30}, 
	{0xa2, 0x32}, 
	{0xa4, 0x80},//30 travis 20140929
	{0xa5, 0x28},//30 travis 20140929
	{0xaa, 0x30},//10 travis 20140929 				
	{0xac, 0x22}, 

	/////////////////////////////////////////////////
	///////////////////   GAMMA   ///////////////////
	/////////////////////////////////////////////////
	{0xfe, 0x00}, 
	{0xbf,0x08}, 
	{0xc0,0x16}, 
	{0xc1,0x28}, 
	{0xc2,0x41}, 
	{0xc3,0x5a}, 
	{0xc4,0x6c}, 
	{0xc5,0x7a}, 
	{0xc6,0x96}, 
	{0xc7,0xac}, 
	{0xc8,0xbc}, 
	{0xc9,0xc9}, 
	{0xca,0xd3}, 
	{0xcb,0xdd}, 
	{0xcc,0xe5}, 
	{0xcd,0xf1}, 
	{0xce,0xfa}, 
	{0xcf,0xff},
	/* 
	{0xfe,0x00},//big gamma
	{0xbf,0x08},
	{0xc0,0x1d},
	{0xc1,0x34},
	{0xc2,0x4b},
	{0xc3,0x60},
	{0xc4,0x73},
	{0xc5,0x85},
	{0xc6,0x9f},
	{0xc7,0xb5},
	{0xc8,0xc7},
	{0xc9,0xd5},
	{0xca,0xe0},
	{0xcb,0xe7},
	{0xcc,0xec},
	{0xcd,0xf4},
	{0xce,0xfa},
	{0xcf,0xff},
	*/	

	/*
	{0xfe,0x00},//small gamma
	{0xbf,0x08},
	{0xc0,0x18},
	{0xc1,0x2c},
	{0xc2,0x41},
	{0xc3,0x59},
	{0xc4,0x6e},
	{0xc5,0x81},
	{0xc6,0x9f},
	{0xc7,0xb5},
	{0xc8,0xc7},
	{0xc9,0xd5},
	{0xca,0xe0},
	{0xcb,0xe7},
	{0xcc,0xec},
	{0xcd,0xf4},
	{0xce,0xfa},
	{0xcf,0xff},
	*/
				/////////////////////////////////////////////////
	///////////////////   YCP  //////////////////////
	/////////////////////////////////////////////////
	{0xd0,0x40}, 
	{0xd1,0x38}, //0x34
	{0xd2,0x38}, //0x34
	{0xd3,0x50},//0x40 
	{0xd6,0xf2}, 
	{0xd7,0x1b}, 
	{0xd8,0x18}, 
	{0xdd,0x03}, 

	/////////////////////////////////////////////////
	////////////////////   AEC   ////////////////////
	/////////////////////////////////////////////////
	{0xfe,0x01},
	{0x05,0x30},
	{0x06,0x75},
	{0x07,0x40},
	{0x08,0xb0},
	{0x0a,0xc5},
	{0x0b,0x11},
	{0x0c,0x00},
	{0x12,0x52},
	{0x13,0x38},
	{0x18,0x95},
	{0x19,0x96},
	{0x1f, 0x20}, 
	{0x20, 0xc0}, 
	{0x3e,0x40}, 
	{0x3f,0x57}, 
	{0x40,0x7d}, 

	{0x03,0x60}, 

	{0x44, 0x02}, 
	/////////////////////////////////////////////////
	////////////////////   AWB   ////////////////////
	/////////////////////////////////////////////////
	{0xfe,0x01},
	{0x1c,0x91},
	{0x21,0x15},
	{0x50,0x80},
	{0x56,0x04},
	{0x59,0x08},
	{0x5b,0x02},
	{0x61,0x8d},
	{0x62,0xa7},
	{0x63,0xd0},
	{0x65,0x06},
	{0x66,0x06},
	{0x67,0x84},
	{0x69,0x08},
	{0x6a,0x25},
	{0x6b,0x01},
	{0x6c,0x00},
	{0x6d,0x02},
	{0x6e,0xf0},
	{0x6f,0x80},
	{0x76,0x80},
	{0x78,0xaf},


	{0x79,0x75},
	{0x7a,0x40},
	{0x7b,0x50},
	{0x7c,0x0c}, 

	{0x90,0xc9},//stable AWB 
	{0x91,0xbe},
	{0x92,0xe2},
	{0x93,0xc9},
	{0x95,0x1b},
	{0x96,0xe2},
	{0x97,0x49},
	{0x98,0x1b},
	{0x9a,0x49},
	{0x9b,0x1b},
	{0x9c,0xc3},
	{0x9d,0x49},
	{0x9f,0xc7},
	{0xa0,0xc8},
	{0xa1,0x00},
	{0xa2,0x00},
	{0x86,0x00},
	{0x87,0x00},
	{0x88,0x00},
	{0x89,0x00},
	{0xa4,0xb9},
	{0xa5,0xa0},
	{0xa6,0xba},
	{0xa7,0x92},
	{0xa9,0xba},
	{0xaa,0x80},
	{0xab,0x9d},
	{0xac,0x7f},
	{0xae,0xbb},
	{0xaf,0x9d},
	{0xb0,0xc8},
	{0xb1,0x97},
	{0xb3,0xb7},
	{0xb4,0x7f},
	{0xb5,0x00},
	{0xb6,0x00},
	{0x8b,0x00},
	{0x8c,0x00},
	{0x8d,0x00},
	{0x8e,0x00},
	{0x94,0x55},
	{0x99,0xa6},
	{0x9e,0xaa},
	{0xa3,0x0a},
	{0x8a,0x00},
	{0xa8,0x55},
	{0xad,0x55},
	{0xb2,0x55},
	{0xb7,0x05},
	{0x8f,0x00},
	{0xb8,0xcb},
	{0xb9,0x9b},  

	/*	 
	{0xa4,0xb9}, //default AWB
	{0xa5,0xa0},
	{0x90,0xc9}, 
	{0x91,0xbe},                                 
	{0xa6,0xb8}, 
	{0xa7,0x95}, 
	{0x92,0xe6}, 
	{0x93,0xca},                                
	{0xa9,0xbc}, 
	{0xaa,0x95}, 
	{0x95,0x23}, 
	{0x96,0xe7},                                  
	{0xab,0x9d}, 
	{0xac,0x80},
	{0x97,0x43}, 
	{0x98,0x24},                                
	{0xae,0xb7}, 
	{0xaf,0x9e}, 
	{0x9a,0x43},
	{0x9b,0x24},                                 
	{0xb0,0xc8}, 
	{0xb1,0x97},
	{0x9c,0xc4}, 
	{0x9d,0x44},                               
	{0xb3,0xb7}, 
	{0xb4,0x7f},
	{0x9f,0xc7},
	{0xa0,0xc8},                               
	{0xb5,0x00}, 
	{0xb6,0x00},
	{0xa1,0x00},
	{0xa2,0x00},                         
	{0x86,0x60},
	{0x87,0x08},
	{0x88,0x00},
	{0x89,0x00},
	{0x8b,0xde},
	{0x8c,0x80},
	{0x8d,0x00},
	{0x8e,0x00},                               
	{0x94,0x55},
	{0x99,0xa6},
	{0x9e,0xaa},
	{0xa3,0x0a},
	{0x8a,0x0a},
	{0xa8,0x55},
	{0xad,0x55},
	{0xb2,0x55},
	{0xb7,0x05},
	{0x8f,0x05},                              
	{0xb8,0xcc},
	{0xb9,0x9a},
	*/  

	/////////////////////////////////////////////////
	////////////////////   CC    ////////////////////
	/////////////////////////////////////////////////
	{0xfe,0x01},

				//red skin
	{0xd0, 0x38}, //red
	{0xd1, 0x00}, 
	{0xd2, 0x02}, 
	{0xd3, 0x04}, 
	{0xd4, 0x38}, 
	{0xd5, 0x12}, 

	{0xd6,0x30},
	{0xd7,0x00},
	{0xd8,0x0a},
	{0xd9,0x16},
	{0xda,0x39},
	{0xdb,0xf8},
	/////////////////////////////////////////////////
	////////////////////   LSC   ////////////////////
	/////////////////////////////////////////////////
	{0xfe,0x01}, 
	{0xc1,0x3c},
	{0xc2,0x50},
	{0xc3,0x00},
	{0xc4,0x40},
	{0xc5,0x30},
	{0xc6,0x30},
	{0xc7,0x10},
	{0xc8,0x00},
	{0xc9,0x00},
	{0xdc,0x20},
	{0xdd,0x10},
	{0xdf,0x00}, 
	{0xde,0x00}, 


	/////////////////////////////////////////////////
	///////////////////  Histogram  /////////////////
	/////////////////////////////////////////////////
	{0x01,0x10}, 
	{0x0b, 0x31}, //11
	{0x0e,0x50}, 
	{0x0f,0x0f}, 
	{0x10,0x6e}, 
	{0x12,0xa0}, 
	{0x15,0x60}, 
	{0x16,0x60}, 
	{0x17,0xe0}, 


	/////////////////////////////////////////////////
	//////////////   Measure Window   ///////////////
	/////////////////////////////////////////////////
	{0xcc,0x0c},//aec window size 
	{0xcd,0x10}, 
	{0xce,0xa0}, 
	{0xcf,0xe6}, 


	/////////////////////////////////////////////////
	/////////////////   dark sun   //////////////////
	/////////////////////////////////////////////////
	{0x45,0xf7},
	{0x46,0xff}, //f0//sun vaule th
	{0x47,0x15},
	{0x48,0x03}, //sun mode
	{0x4f,0x60}, //sun_clamp

	{0xfe,0x00},
	{0x05, 0x01}, 
	{0x06, 0x89}, //HB
	{0x07, 0x00}, 
	{0x08, 0x2a}, //VB
	{0xfe, 0x01}, 
	{0x25, 0x00}, //step
	{0x26, 0x6d}, 
	{0x27, 0x01}, //20.2fps
	{0x28, 0xb4}, 
	{0x29, 0x02}, //16.6fps
	{0x2a, 0x8e}, 
	{0x2b, 0x02}, //14.3fps
	{0x2c, 0xfb}, 
	{0x2d, 0x07}, //5.5fps
	{0x2e, 0x3d}, 

	{0x3c,0x20},
};

 
static AMOPENAT_CAMERA_REG prvCamVgaReg[] =
{
	/////////////////////  SPI   ////////////////////
	/////////////////////////////////////////////////
	{0x50,0x01},	// Crop en
	{0x51,0x00},
	{0x52,0x00},
	{0x53,0x00},
	{0x54,0x00},
	{0x55,0x01},
	{0x56,0xe0},
	{0x57,0x02},
	{0x58,0x80},
		
	{0xfe,0x03},
	{0x01,0x00},
	{0x02,0x00},
	{0x10,0x00},
	{0x15,0x00},
	{0x17,0x00}, //01//03
	{0x04,0x10},
	{0x05,0x00},
	{0x40,0x00},


	{0x52,0xa0}, //a2 //a0//80//00 //22

	{0x53,0x24}, //24
	{0x54,0x20},
	{0x55,0x20}, //QQ//01
	{0x5a,0x00}, //00 //yuv 
	{0x5b,0x80},
	{0x5c,0x02},
	{0x5d,0xe0},
	{0x5e,0x01},
	{0x51,0x03},
	{0x64,0x06},
	{0xfe,0x00},
};

static AMOPENAT_CAMERA_REG prvCamQvgaReg[] =
{
	/////////////////////  SPI   ////////////////////
	/////////////////////////////////////////////////
	{0x50,0x01},	// Crop en
	{0x51,0x00},
	{0x52,0x78},
	{0x53,0x00},
	{0x54,0xa0},
	{0x55,0x00},
	{0x56,0xf0},
	{0x57,0x01},
	{0x58,0x40},
		
	{0xfe,0x03},
	{0x01,0x00},
	{0x02,0x00},
	{0x10,0x00},
	{0x15,0x00},
	{0x17,0x00}, //01//03
	{0x04,0x10},
	{0x05,0x00},
	{0x40,0x00},


	{0x52,0xa0}, //a2 //a0//80//00 //22

	{0x53,0x24}, //24
	{0x54,0x20},
	{0x55,0x20}, //QQ//01
	{0x5a,0x00}, //00 //yuv 
	{0x51,0x03},
	{0x64,0x06},
	{0xfe,0x00},
};

static AMOPENAT_CAMERA_REG prvCambMirrorTReg[] =
{
	{0x17, 0x15}, //bMirror = true
};
static AMOPENAT_CAMERA_REG prvCambMirrorFReg[] =
{
 	{0x17, 0x14}, //bMirror = false
};
/*-\NEW\zhuwangbin\2020.07.16\重新整理camera寄存器和配置逻辑*/


/*提取yuv数据中的y*/
static unsigned char * prvZbarScannerY(unsigned char *data, int width, int height)
{
	unsigned char *src = data,*end = data + (width * height * 2);
	unsigned char *dst = workingbuffer;

	if (!src || !dst)
		return NULL;

	while (src < end)
	{
		src ++;
		*dst = *src;
		src ++;
		dst++;
	}//End of while;

    return workingbuffer;
}

/*解析二维码中的数据*/
static void prvZbarScannerRun(int width, int height, int size, unsigned char *dataInput)
{
	int len;
	char *data;
	
	//创建句柄， handle != 0 表示解码成功
	int handle = OPENAT_zbar_scanner_open(width, height, size, dataInput);
    
	if (handle)
	{
		do
		{    
			CAM_FUNC();
			
			// 解码成功获取二维码信息
			data = OPENAT_zbar_get_data(handle, &len);
			data[len] = 0;

			//OPENAT_CameraPreviewClose();
			OPENAT_print("prvZbarScannerRun %s,%d,%s", OPENAT_zbar_get_type(handle), len, data);
			
			//发送消息通知lua
			PlatformMsgData rtosmsg;

			rtosmsg.zbarData.result = TRUE;
			rtosmsg.zbarData.pType = (u8 *)platform_malloc(strlen(OPENAT_zbar_get_type(handle))+1);
			memset(rtosmsg.zbarData.pType, 0, strlen(OPENAT_zbar_get_type(handle))+1);
			memcpy(rtosmsg.zbarData.pType, OPENAT_zbar_get_type(handle), strlen(OPENAT_zbar_get_type(handle)));
			rtosmsg.zbarData.pData = (u8 *)platform_malloc(len+1);
			memset(rtosmsg.zbarData.pData, 0, len+1);
			memcpy(rtosmsg.zbarData.pData, data, len);
			        		
	        platform_rtos_send(MSG_ID_RTOS_MSG_ZBAR, &rtosmsg);	
			
		 // 判断是否有下一个数据
		}while(OPENAT_zbar_find_nextData(handle) > 0);

		// 释放句柄
		OPENAT_zbar_scanner_close(handle);
	}
}
/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
static void prvCamScanProcess(void *param)
{
	OPENAT_print("prvCamScanProcess scannerBuff %x", platCamCtx.scan.scannerBuff);
	
	CAM_FUNC();
	if (platCamCtx.scan.scannerBuff == NULL)
		return;
	
	if (platCamCtx.exCamera.bZbarScan)
	{
		prvZbarScannerRun(platCamCtx.exCamera.sensorWidth, platCamCtx.exCamera.sensorHeight,
				platCamCtx.exCamera.sensorHeight*platCamCtx.exCamera.sensorWidth,
				platCamCtx.scan.scannerBuff);
	}
	else
	{
		prvZbarScannerRun(CAM_SENSOR_WIDTH, CAM_SENSOR_HEIGHT,
				CAM_SENSOR_WIDTH*CAM_SENSOR_HEIGHT,
					platCamCtx.scan.scannerBuff);
	}
}

static BOOL prvCamScanStart(void)
{
	osiWorkEnqueue(platCamCtx.scan.work, platCamCtx.scan.wq);
}

static BOOL prvCamScanOpen(void)
{

	if(platCamCtx.scan.wq)
	{
		return FALSE;
	}
	
	platCamCtx.scan.wq = osiWorkQueueCreate("cam-scan", 1, OSI_PRIORITY_NORMAL, 4096);
	if (!platCamCtx.scan.wq)
	{
		CAM_FUNC();
		return FALSE;
	}
	platCamCtx.scan.work = osiWorkCreate(prvCamScanProcess, NULL, NULL);
	if (!platCamCtx.scan.work)
	{
		CAM_FUNC();
		osiWorkQueueDelete(platCamCtx.scan.wq);
		platCamCtx.scan.wq = NULL;
		return FALSE;
	}	
	
	CAM_FUNC();
	return TRUE;
}

static BOOL prvCamScanClose(void)
{
	/*删除work queue*/
	if (platCamCtx.scan.wq)
	{
		osiWorkQueueDelete(platCamCtx.scan.wq);
		platCamCtx.scan.wq = NULL;
	}

	/*删除rx_work*/
	if (platCamCtx.scan.work)
	{
		osiWorkDelete(platCamCtx.scan.work);
		platCamCtx.scan.work = NULL;
	}

	platCamCtx.scan.scannerBuff = NULL;

	CAM_FUNC();
	return TRUE;
}


/*预览回调函数, 获取预览数据*/
static void prvCmaCallback(T_AMOPENAT_CAMERA_MESSAGE *pMsg)
{
	CAM_FUNC();
    switch(pMsg->evtId)
    {
        case OPENAT_DRV_EVT_CAMERA_DATA_IND:
        {
            // 获取camera得到的数据, 先提取y向量，然后在scan work中解析
            if (platCamCtx.exCamera.bZbarScan)
			{
				platCamCtx.scan.scannerBuff = prvZbarScannerY((unsigned char *)pMsg->dataParam.data, 
							platCamCtx.exCamera.sensorWidth, platCamCtx.exCamera.sensorHeight);
			}
			else
			{
				platCamCtx.scan.scannerBuff = prvZbarScannerY((unsigned char *)pMsg->dataParam.data, 
							CAM_SENSOR_WIDTH, CAM_SENSOR_HEIGHT);
			}

			prvCamScanStart();
			break;
        }
        default:
            break;
    }
}
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
static BOOL prvCamScannerStart(void)
{
	BOOL ret;
	T_AMOPENAT_CAM_PREVIEW_PARAM previewParam;
	
	previewParam.startX = platCamCtx.previewParam.startX;
	previewParam.startY = platCamCtx.previewParam.startY;
	previewParam.endX = platCamCtx.previewParam.endX;
	previewParam.endY = platCamCtx.previewParam.endY;
	/*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
	previewParam.zoom = platCamCtx.previewParam.zoom;
	previewParam.rotation = platCamCtx.previewParam.rotation;
	/*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
	ret = OPENAT_CameraPreviewOpen(&previewParam);
	if (!ret)
	{
		CAM_FUNC();
		return FALSE;
	}
	
	CAM_FUNC();
	return TRUE;
}

static BOOL prvCamScannerStop(void)
{
	/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	prvCamScanClose();
	/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	return OPENAT_CameraPreviewClose();
}

static BOOL prvCamOpen(void)
{
	static BOOL init = FALSE;
	BOOL ret;
	
	T_AMOPENAT_CAMERA_PARAM initParam =
	{
		NULL,
		OPENAT_I2C_1, 
		0x21,
		AMOPENAT_CAMERA_REG_ADDR_8BITS|AMOPENAT_CAMERA_REG_DATA_8BITS,
		TRUE,
		TRUE,
		TRUE, 
		/*+\NEW\zhuwangbin\2020.07.16\添加VGA拍照功能，不支持VGA扫码*/
		(platCamCtx.openParam.bJump) ? CAM_SENSOR_WIDTH : CAM_SENSOR_WIDTH *2,
		(platCamCtx.openParam.bJump) ? CAM_SENSOR_HEIGHT : CAM_SENSOR_HEIGHT *2,
		/*-\NEW\zhuwangbin\2020.07.16\添加VGA拍照功能，不支持VGA扫码*/
		CAMERA_IMAGE_FORMAT_YUV422,
		prvCameraInitReg,
        sizeof(prvCameraInitReg)/sizeof(AMOPENAT_CAMERA_REG),
		{0xf1, 0x10},
		{2,3,TRUE},
		1,
		OPENAT_SPI_MODE_MASTER2_2,
		OPENAT_SPI_OUT_Y1_V0_Y0_U0
	};
	
	/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	/*是否添加扫码功能*/
	if (platCamCtx.openParam.bZbarScan)
	{
		if (!prvCamScanOpen())
		{
			CAM_FUNC();
			return FALSE;
		}
		initParam.messageCallback = prvCmaCallback;
	}	
	/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	CAM_FUNC();

	if(!init)
	{
		ret = OPENAT_InitCamera(&initParam);
		if (!ret)
		{
			CAM_FUNC();
			return FALSE;
		}

		//init = TRUE;
		CAM_FUNC();
	}
	ret = OPENAT_CameraPoweron(FALSE);
	if (!ret)
	{
		CAM_FUNC();
		return FALSE;
	}
	
	/*+\NEW\zhuwangbin\2020.07.16\重新整理camera寄存器和配置逻辑*/
	if (platCamCtx.openParam.bJump)
	{
		OPENAT_CameraWriteReg(prvCamQvgaReg, sizeof(prvCamQvgaReg)/sizeof(AMOPENAT_CAMERA_REG));
	}
	else
	{
		OPENAT_CameraWriteReg(prvCamVgaReg, sizeof(prvCamVgaReg)/sizeof(AMOPENAT_CAMERA_REG));
	}
	
	/**是否使能bMiior**/
	if (platCamCtx.openParam.bMirror)
	{
		OPENAT_CameraWriteReg(prvCambMirrorTReg, sizeof(prvCambMirrorTReg)/sizeof(AMOPENAT_CAMERA_REG));
		
	}
	else
	{
		OPENAT_CameraWriteReg(prvCambMirrorFReg, sizeof(prvCambMirrorFReg)/sizeof(AMOPENAT_CAMERA_REG));
	}
	/*-\NEW\zhuwangbin\2020.07.16\重新整理camera寄存器和配置逻辑*/
	CAM_FUNC();
	return TRUE;
}

/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
static BOOL prvCamOpenExt(T_PLATFORM_CAMERA_PARAM *param)
{
	static BOOL init = FALSE;
	BOOL ret;
	int sInitCmdSize = 0;
	static AMOPENAT_CAMERA_REG* sInitCmd = NULL;
	
	T_AMOPENAT_CAMERA_PARAM initParam =
	{
		NULL,
		OPENAT_I2C_1, 
		param->i2cSlaveAddr,
		AMOPENAT_CAMERA_REG_ADDR_8BITS|AMOPENAT_CAMERA_REG_DATA_8BITS,
		TRUE,
		TRUE,
		TRUE, 
		param->sensorWidth,
		param->sensorHeight,
		CAMERA_IMAGE_FORMAT_YUV422,
		param->initRegTable_p,
        param->initRegTableCount,
		{param->idReg.addr, param->idReg.value},
		{2,3,TRUE},
		1,
		param->spi_mode,
		param->spi_yuv_out,
		FALSE,
		param->spi_speed
	};

	CAM_FUNC();

	/*是否添加扫码功能*/
	if (platCamCtx.exCamera.bZbarScan)
	{
		if (!prvCamScanOpen())
		{
			CAM_FUNC();
			return FALSE;
		}
		initParam.messageCallback = prvCmaCallback;
	}	

	if(!init)
	{
		ret = OPENAT_InitCamera(&initParam);
		if (!ret)
		{
			CAM_FUNC();
			return FALSE;
		}

		//init = TRUE;
		CAM_FUNC();
	}
	
	ret = OPENAT_CameraPoweron(FALSE);
	if (!ret)
	{
		CAM_FUNC();
		return FALSE;
	}
	
	CAM_FUNC();
	return TRUE;
}
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
static BOOL prvCamClose(void)
{
	/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	prvCamScanClose();
	/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
	
	OPENAT_CameraPreviewClose();
	OPENAT_CameraPowerOff();
	
	return TRUE;
}

static BOOL prvZbarCamCtl(camType_t type)
{
	BOOL ret = FALSE;

	CAM_FUNC();
	switch(type)
	{
		case CCAM_TYPE_OPEN:
			ret = prvCamOpen();
			break;
		case CCAM_TYPE_CLOSE:
			ret = prvCamClose();
			break;
		case CCAM_TYPE_SCANNER_START:
			ret = prvCamScannerStart();
			break;
		case CCAM_TYPE_SCANNER_STOP:
			ret = prvCamScannerStop();
			break;
		default:
			break;
	}

	CAM_FUNC();
	return ret;
}

/*+\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/
BOOL platform_CameraWriteReg(int *pInitCmd, int nInitCmdSize)
{
	AMOPENAT_CAMERA_REG *sInitCmd;
	int i;
	/*+\bug\rww\2020.11.18\修改一处内存泄漏*/
	BOOL retval;
	/*-\bug\rww\2020.11.18\修改一处内存泄漏*/

	if (nInitCmdSize / 2 <= 0)
	{
		OPENAT_print("platform_CameraWriteRegList error nInitCmdSize %d", nInitCmdSize);
		return FALSE;
	}
	
	sInitCmd = (AMOPENAT_CAMERA_REG*)platform_malloc(sizeof(AMOPENAT_CAMERA_REG)*nInitCmdSize/2);
	if(!sInitCmd)
	{
		OPENAT_print("camera_poweron_gc0310_ext malloc=%d fail",sizeof(AMOPENAT_CAMERA_REG)*nInitCmdSize/2);
		return FALSE;
	}

	for(i=0; i<nInitCmdSize; i+=2)
	{
		sInitCmd[i/2].addr = (unsigned char)pInitCmd[i];
		sInitCmd[i/2].value = (unsigned char)pInitCmd[i+1];
		//OPENAT_print("platform_CameraWriteRegList %d, {%x,%x}",nInitCmdSize, sInitCmd[i/2].addr, sInitCmd[i/2].value);
	}
	/*+\bug\rww\2020.11.18\修改一处内存泄漏*/
	retval = OPENAT_CameraWriteReg(sInitCmd, nInitCmdSize/2);

	platform_free(sInitCmd);

	return retval;
	/*-\bug\rww\2020.11.18\修改一处内存泄漏*/
}
/*-\NEW\zhuwangbin\2020.7.14\添加camera sensor写寄存器接口*/

// 获取camera得到的数据， 发送到zbartask 去解析
BOOL platform_camera_poweron(BOOL video_mode, int nCamType, BOOL bZbarScan, BOOL bMirror, BOOL bJump)
{
	OPENAT_print("[%s,%d] faile video_mode %d, nCamType %d, bzbar %d, bMirror %d, bJump %d",__FUNCTION__, __LINE__,video_mode, nCamType, bZbarScan, bMirror, bJump);
	if (video_mode || (nCamType != 1))
	{
		OPENAT_print("[%s,%d] faile video_mode %d, nCamType %d",__FUNCTION__, __LINE__,video_mode, nCamType);
		return FALSE;
	}
	
	/*+\NEW\zhuwangbin\2020.07.16\添加VGA拍照功能，不支持VGA扫码*/
	if (!bJump && bZbarScan)
	{
		OPENAT_print("camera_poweron !jump & zbarScan error");
		return FALSE;
	}
	/*-\NEW\zhuwangbin\2020.07.16\添加VGA拍照功能，不支持VGA扫码*/
	platCamCtx.openParam.video_mode = video_mode;
	platCamCtx.openParam.nCamType = nCamType;
	platCamCtx.openParam.bZbarScan = bZbarScan;
	platCamCtx.openParam.bMirror = bMirror;
	platCamCtx.openParam.bJump = bJump;
    return prvZbarCamCtl(CCAM_TYPE_OPEN);
}

/*+\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/
BOOL platform_camera_poweron_ext(T_PLATFORM_CAMERA_PARAM *param, BOOL bZbarScan)
{
	platCamCtx.exCamera.bZbarScan = bZbarScan;
	platCamCtx.exCamera.sensorHeight = param->sensorHeight;
	platCamCtx.exCamera.sensorWidth = param->sensorWidth;
    return prvCamOpenExt(param);
}
/*-\NEW\zhuwangbin\2020.8.22\ lua版本的camera寄存器由脚本配置*/

BOOL platform_camera_poweroff(void)
{
	/*+\NEW\zhuwangbin\2020.06.1\使用camera 扫码demo ，放置大概半小时，屏幕卡在扫描失败，不会继续执行了*/
	platCamCtx.openParam.nInitCmdSize = 0;
	platCamCtx.openParam.bZbarScan = 0;
	memset(&platCamCtx.previewParam, 0, sizeof(platCamCtx.previewParam));
	//memset(&platCamCtx, 0, sizeof(platCamCtx));
	/*-\NEW\zhuwangbin\2020.06.1\使用camera 扫码demo ，放置大概半小时，屏幕卡在扫描失败，不会继续执行了*/
    return prvZbarCamCtl(CCAM_TYPE_CLOSE);
}

BOOL platform_camera_preview_open(u16 offsetx, u16 offsety, u16 startx, u16 starty, u16 endx, u16 endy)
{
    T_AMOPENAT_CAM_PREVIEW_PARAM previewParam;
    previewParam.startX = startx;
    previewParam.startY = starty;
    previewParam.endX = endx;
    previewParam.endY = endy;
    previewParam.recordAudio = FALSE;
	/*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
	previewParam.rotation = platCamCtx.previewParam.rotation;
	previewParam.zoom = platCamCtx.previewParam.zoom;
	/*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
	memcpy(&platCamCtx.previewParam, &previewParam, sizeof(previewParam));
    return prvZbarCamCtl(CCAM_TYPE_SCANNER_START);
}

/*+\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/
BOOL platform_camera_preview_zoom(int zoom)
{
	platCamCtx.previewParam.zoom = zoom;

	return TRUE;
}

BOOL platform_camera_preview_rotation(int rotation)
{
	if (rotation != 0 && rotation != 90)
	{
		OPENAT_print("rotation != 0 && rotation != 90 ERROR");
		return FALSE;
	}
	
	platCamCtx.previewParam.rotation = rotation;

	return TRUE;
}
/*-\NEW\zhuwangbin\2020.7.20\添加camera 翻转放缩功能*/


BOOL platform_camera_preview_close(void)
{
    return prvZbarCamCtl(CCAM_TYPE_SCANNER_STOP);
}

/*+\NEW\zhuwangbin\2020.05.19\添加camera拍照功能*/
BOOL platform_camera_capture(u16 width, u16 height, u16 quality)
{
    T_AMOPENAT_CAM_CAPTURE_PARAM captureParam;
    captureParam.imageWidth = width;
    captureParam.imageHeight = height;
    captureParam.quality = quality;
		
    return OPENAT_CameraCapture(&captureParam);
}

BOOL platform_camera_save_photo(const char* filename)
{
    char* actname;
    int fb;
    BOOL result;
	
    if((find_dm_entry( filename, &actname ) ) == -1 )
    {
        return -1;
    }
    
    OPENAT_delete_file(actname);
	
    fb = OPENAT_create_file(actname, 0);

    result = OPENAT_CameraSavePhoto(fb);
	
    OPENAT_close_file(fb);
	
    return result;
}
/*-\NEW\zhuwangbin\2020.05.19\添加camera拍照功能*/
int platform_encodeJpegBuffer(UINT8 *inFilename,
                                int inFormat,
                                int inWidth,
                                int inHeight,
                                int outWidth,
                                int outHeight,
                                int inQuality,
                                char *outFilename)
{
#if 0
    T_AMOPENAT_DECODE_INPUT_PARAM inputParam;
    UINT32 inFIleSize;
    UINT32 readLen = 0;
    UINT8 *output;
    UINT32 outputLen;
    int err, inFb, outFb;
    char* inActname, *outActname;

   IVTBL(print)("img Imgs_encodeJpegBuffer %s,%d,%d,%d,%d,%d,%d,%s", 
        inFilename, inFormat, 
        inWidth, inHeight, outWidth, outHeight, 
        inQuality, outFilename);

    if(find_dm_entry(inFilename, &inActname ) == -1 
          || find_dm_entry(outFilename, &outActname) == -1)
    {
        return -1;
    }

    inFb =  IVTBL(open_file)(inActname, O_RDONLY, 0);
    if (inFb < 0)
    {
      return -2;
    }

    inFIleSize = IVTBL(seek_file)(inFb, 0, SEEK_END);
    
    if (inFIleSize <= 0)
    {
      IVTBL(close_file)(inActname);
      return -3;
    }
    IVTBL(seek_file)(inFb, 0, SEEK_SET);

    inputParam.inBuffer = IVTBL(malloc)(inFIleSize);

    if (!inputParam.inBuffer)
    {
      IVTBL(close_file)(inActname);
      IVTBL(delete_file)(inActname);
      return -4;
    }
    
    readLen = IVTBL(read_file)(inFb, inputParam.inBuffer, inFIleSize);
   
    inputParam.inFormat = inFormat;
    inputParam.inHeight = inHeight;
    inputParam.inWidth = inWidth;
    inputParam.outHeight = outHeight;
    inputParam.outWidth = outWidth;
    inputParam.inQuality = inQuality;

    IVTBL(delete_file)(outActname);
    outFb = IVTBL(create_file)(outActname, NULL);
  
    if (outFb < 0)
    {
      return -5;
    } 

    output = inputParam.inBuffer;
    err = IVTBL(Imgs_encodeJpegBuffer)(
        &inputParam,
        output,
        &outputLen);

    IVTBL(write_file)(outFb, output+1, outputLen);
    
    if (err != 0)
    {
      return err;
    }
    
    IVTBL(close_file)(outFb);
    IVTBL(print)("img Imgs_encodeJpegBuffer %d, err %d", outputLen,  err);
    IVTBL(free)(inputParam.inBuffer);
    
    return err;
#else
	return 0;
#endif
}
#endif
/*-\NEW\zhuwangbin\2020.05.01\添加disp camera功能*/
