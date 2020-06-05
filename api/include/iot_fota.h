#ifndef __IOT_FOTA_H__
#define __IOT_FOTA_H__

#include "openat_ota.h"

/**
 * @defgroup iot_sdk_fota 远程升级接口
 * @{
 */
	/**@example fota/demo_fota.c
	* OTA接口示例
	*/ 


/**远程升级初始化
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_init(void);

/**远程升级下载
*@param		data:				下载固件包数据
*@param		len:				下载固件包长度
*@param		total:				固件包总大小
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_download(const char* data, UINT32 len, UINT32 total);

/**远程升级
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_done(void);


#endif

