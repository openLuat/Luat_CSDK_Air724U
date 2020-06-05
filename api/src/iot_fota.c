#include "iot_fota.h"


/**远程升级初始化
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_init(void)
{
    return openat_otaInit();
}

/**远程升级下载
*@param		data:				下载固件包数据
*@param		len:				下载固件包长度
*@param		total:				固件包总大小
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_download(const char* data, UINT32 len, UINT32 total)
{
    return openat_otaProcess(data, len,total);
}

/**远程升级
*@return	0:   表示成功
*           <0：  表示有错误
**/
E_OPENAT_OTA_RESULT iot_fota_done(void)
{
    return openat_otaDone();
}


