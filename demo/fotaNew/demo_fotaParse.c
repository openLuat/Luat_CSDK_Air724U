#include <string.h>
#include "iot_sys.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_socket.h"
#include "iot_network.h"
#include "iot_fs.h"
#include "iot_vat.h"
#include <sys/stdio.h>
#include <stdlib.h>
#include "demo_fotaParse.h"


otaCtx_t gOtaCtx;
static otaCtx g_s_otaCtx;

static void _ota_ls(char *dirName)
{
    AMOPENAT_FS_FIND_DATA findResult;
    INT32 iFd = -1;
	char path[1024];
	
    iFd = iot_fs_find_first(dirName, &findResult);
	if (iFd < 0)
		return;
	memset(path, 0, sizeof(path));
	snprintf(path, sizeof(path), "%s/%s", dirName, findResult.st_name);
	iot_debug_print("[ota_parse] %s ls:", dirName);
    iot_debug_print("[ota_parse] \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode&E_FS_ATTR_ARCHIVE)? "FILE":"DIR"),    
                                                findResult.st_name,
                                                iot_fs_file_size(path),
                                                findResult.mtime);

    while(iot_fs_find_next(iFd, &findResult) == 0)
    {
    	memset(path, 0, sizeof(path));
		snprintf(path, sizeof(path), "%s/%s", dirName, findResult.st_name);
        iot_debug_print("[ota_parse] \t%s:\t%s\t%d\t%d\t", ((findResult.st_mode&E_FS_ATTR_ARCHIVE)? "FILE":"DIR"),    
                                                findResult.st_name,
                                                iot_fs_file_size(path),
                                                findResult.mtime);
    }

    if(iFd >= 0)
    {
        iot_fs_find_close(iFd);
    }
    
}

static otaResult_t _OtaDownloadInit(void)
{
	int i;

	_ota_ls("fota");
	for (i=0; i<OTA_FILE_MAX; i++)
	{
		gOtaCtx.file[i].fd = OTA_FILE_INVALID_FD;
		gOtaCtx.file[i].writeLen = 0;
		gOtaCtx.file[i].totalLen = 0;
		gOtaCtx.file[i].state = OTA_INIT;
	}

	iot_debug_print("ota_parse OtaDownloadInit ");
	return OTA_SUCCESS;
}

static inline void _otaStateSet(otaFileIndex_t fileIdx, otaResult_t state)
{
	iot_debug_print("ota_parse [%s,%d] set fileIdx %d, state %d", __FUNCTION__, __LINE__, fileIdx, state);
	gOtaCtx.file[fileIdx].state = state;
}

static inline otaResult_t _otaStateGet(otaFileIndex_t fileIdx)
{
	iot_debug_print("ota_parse [%s,%d] get fileIdx %d, state %d", __FUNCTION__, __LINE__, fileIdx, gOtaCtx.file[fileIdx].state);
	return gOtaCtx.file[fileIdx].state;
}


static otaResult_t _otaDownloadProcess(const char* data, unsigned int len, unsigned int total, bool isCore)
{
	iot_debug_print("ota_parse otaDownloadProcess gFotaFd %d", len, total, isCore);
	otaFileIndex_t fdInx;
	int fd;
	int writeLen;
	
	if (isCore)
	{
		fdInx = OTA_CODE_FILE;
	}
	else
	{
		fdInx = OTA_APP_FILE;
	}

	/*写文件的第一包数据*/
	if (gOtaCtx.file[fdInx].state == OTA_INIT)
	{
		if (fdInx == OTA_CODE_FILE)
		{
			fd = iot_fs_open_file(OTA_CORE_FILE_PATH, FS_O_RDWR | FS_O_CREAT | FS_O_TRUNC);
		}
		else
		{
			fd = iot_fs_open_file(OTA_APP_FILE_PATH, FS_O_RDWR | FS_O_CREAT | FS_O_TRUNC);
		}
		
		if (fd < 0)
		{
			_otaStateSet(fdInx, OTA_ERROR_WRITE);
			return OTA_ERROR_WRITE;
		}

		gOtaCtx.file[fdInx].fd = fd;
		gOtaCtx.file[fdInx].totalLen = total;
		
		_otaStateSet(fdInx, OTA_PROCESS);
		
		writeLen = iot_fs_write_file(fd, (UINT8 *)data, len);
		gOtaCtx.file[fdInx].writeLen = writeLen;

		iot_debug_print("ota_parse otaDownloadProcess init %d/%d", gOtaCtx.file[fdInx].writeLen, total);
		if (writeLen != len)
		{
			_otaStateSet(fdInx, OTA_ERROR_NO_MEMORY);
			return OTA_ERROR_NO_MEMORY;
		}

		if (gOtaCtx.file[fdInx].totalLen == gOtaCtx.file[fdInx].writeLen)
		{
			_otaStateSet(fdInx, OTA_END);
			iot_fs_close_file(fd);
		}
	}
	/*剩余的数据*/
	else if (gOtaCtx.file[fdInx].state == OTA_PROCESS)
	{
		fd = gOtaCtx.file[fdInx].fd;
		writeLen = iot_fs_write_file(fd, (UINT8 *)data, len);
		gOtaCtx.file[fdInx].writeLen += writeLen;
		iot_debug_print("ota_parse otaDownloadProcess %d/%d", gOtaCtx.file[fdInx].writeLen, total);
		if (gOtaCtx.file[fdInx].totalLen == gOtaCtx.file[fdInx].writeLen)
		{
			_otaStateSet(fdInx, OTA_END);
		}
	}
	/*正常不会到这里*/
	else
	{
		_otaStateSet(fdInx, OTA_ERROR_FBF_ERROR);
	}

	return OTA_SUCCESS;
}

static otaResult_t _otaDownloadDone(void)
{
	int appState = _otaStateGet(OTA_APP_FILE);
	int CoreState = _otaStateGet(OTA_CODE_FILE);
	
	iot_debug_print("ota_parse otaDownloadDone appState %d CoreState %d", appState, CoreState);

	_ota_ls("fota");
	
	/**core和app 有一个状态不对就不准升级**/
	if (appState == OTA_PROCESS || appState < 0 || 
			CoreState == OTA_PROCESS || CoreState < 0)
	{
		iot_debug_print("ota_parse otaDownloadDone fail");

		iot_fs_delete_file(OTA_CORE_FILE_PATH);
		iot_fs_delete_file(OTA_APP_FILE_PATH);
		return OTA_ERROR_VERIFY_ERROR;
	}

	if (appState == OTA_INIT)
	{
		if (!iot_ota_newapp(NULL))
			return OTA_ERROR_VERIFY_ERROR;
	}
	else
	{
		if (!iot_ota_newapp(OTA_APP_FILE_PATH))
			return OTA_ERROR_VERIFY_ERROR;
	}

	if (CoreState == OTA_INIT)
	{
		if (!iot_ota_newcore(NULL))
			return OTA_ERROR_VERIFY_ERROR;
	}
	else
	{
		if (!iot_ota_newcore(OTA_CORE_FILE_PATH))
			return OTA_ERROR_VERIFY_ERROR;
	}

	iot_debug_print("ota_parse _otaDownloadDone ok");
	memset(&gOtaCtx, 0, sizeof(gOtaCtx));
	
	return OTA_SUCCESS;
}

otaResult_t otaInit(void)
{
	memset(&g_s_otaCtx, 0, sizeof(g_s_otaCtx));
	return _OtaDownloadInit();
}

otaResult_t otaProcess(char* data, unsigned int len, unsigned int total)
{

	char *cur = data;
	char *end = data+len;
	unsigned char *temp;
	unsigned int remain, tempLen, size;
	E_OTA_FILE_TYPE type;

	iot_debug_print("ota_parse otaProcess len %d, total %d", len, total);
	
	if (g_s_otaCtx.readLen > total)
	{
		iot_debug_print("ota_parse otaProcess ERROR readLen %d, total %d", g_s_otaCtx.readLen, total);
		return OTA_ERROR_VERIFY_ERROR;
	}

	/*1. 检测头信息head信息*/
	if (g_s_otaCtx.head.readLen < OTA_HEAD_LEN)
	{
		temp = g_s_otaCtx.head.head;
		tempLen = g_s_otaCtx.head.readLen;
	
		if (g_s_otaCtx.head.readLen + len < OTA_HEAD_LEN)
		{
			memcpy(temp+tempLen, cur, len);
			g_s_otaCtx.head.readLen += len;

			iot_debug_print("ota_parse otaProcess wait head.readLen %d, len %d", g_s_otaCtx.head.readLen, len);
			return OTA_SUCCESS;
		}
		
		memcpy(temp+tempLen, cur, OTA_HEAD_LEN-tempLen);
		cur += OTA_HEAD_LEN-tempLen;
		g_s_otaCtx.head.readLen = OTA_HEAD_LEN;
		
		if (temp[0] != OTA_HEAD)
		{
			iot_debug_print("ota_parse otaProcess ERROR HEAD %x", g_s_otaCtx.head.head[0]);
			return OTA_ERROR_VERIFY_ERROR;
		}

		g_s_otaCtx.head.fileNum = temp[1] << 24 | temp[2] <<16
					| temp[3] << 8 | temp[4] << 0;
		
		if (g_s_otaCtx.head.fileNum > OTA_FILE_MAX)
		{
			iot_debug_print("ota_parse otaProcess ERROR fileNum %x", g_s_otaCtx.head.fileNum);
			return OTA_ERROR_VERIFY_ERROR;
		}

		iot_debug_print("ota_parse otaProcess readLen %d, fileNum %d", g_s_otaCtx.head.readLen, g_s_otaCtx.head.fileNum);
	}

/*file Check*/
OTA_CHECK:
	/*2. 检测file 信息*/
	remain = end - cur;
	if (remain == 0)
		return OTA_SUCCESS;

	if (g_s_otaCtx.fileCur >= g_s_otaCtx.head.fileNum)
	{
		iot_debug_print("ota_parse otaProcess g_s_otaCtx.fileCur %x,%x", g_s_otaCtx.fileCur, g_s_otaCtx.head.fileNum);
		return OTA_ERROR_VERIFY_ERROR;
	}
		
	if (g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen < OTA_HEAD_LEN)
	{
		temp = g_s_otaCtx.file[g_s_otaCtx.fileCur].head;
		tempLen = g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen;
	
		if (tempLen + remain < OTA_HEAD_LEN)
		{
			memcpy(temp+tempLen, cur, remain);
			g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen += remain;

			iot_debug_print("ota_parse otaProcess wait file readLen %d", g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen, len);
			return OTA_SUCCESS;
		}

		memcpy(temp+tempLen, cur, OTA_HEAD_LEN-tempLen);
		cur += OTA_HEAD_LEN-tempLen;
		g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen = OTA_HEAD_LEN;

		if (temp[0] == OTA_APP_HEAD)
		{
			g_s_otaCtx.file[g_s_otaCtx.fileCur].type = OTA_FILE_APP;
		}
		else if (temp[0] == OTA_CORE_HEAD)
		{
			g_s_otaCtx.file[g_s_otaCtx.fileCur].type = OTA_FILE_CORE;
		}
		else
		{
			iot_debug_print("ota_parse otaProcess ERROR file_type %x", g_s_otaCtx.head.head[0]);
			return OTA_ERROR_VERIFY_ERROR;
		}

		g_s_otaCtx.file[g_s_otaCtx.fileCur].size = temp[1] << 24 | temp[2] <<16
					| temp[3] << 8 | temp[4] << 0;

		iot_debug_print("ota_parse otaProcess file size %d, type %d", 
				g_s_otaCtx.file[g_s_otaCtx.fileCur].size, g_s_otaCtx.file[g_s_otaCtx.fileCur].type);
	}
	

	/*3. 写数据*/
	remain = end - cur;
	if (remain)
	{
		type = g_s_otaCtx.file[g_s_otaCtx.fileCur].type;
		tempLen = g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen;
		size = g_s_otaCtx.file[g_s_otaCtx.fileCur].size;
		switch(type)
		{
			case OTA_FILE_CORE:
			{
				if (tempLen + remain < size + OTA_HEAD_LEN)
				{
					g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen += remain;
					return _otaDownloadProcess(cur, remain, size, TRUE);
				}
				else 
				{
					remain = size + OTA_HEAD_LEN - tempLen;
					g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen += remain;
					_otaDownloadProcess(cur, remain, size, TRUE);
					cur += remain;
					
					iot_debug_print("ota_parse OTA_FILE_CORE fileCur %d, fileSize %d,remain %d,%d", 
						g_s_otaCtx.fileCur, g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen,end - cur, remain);

					g_s_otaCtx.fileCur++;
					goto	OTA_CHECK;
				}
			}
			case OTA_FILE_APP:
			{
				if (tempLen + remain < size + OTA_HEAD_LEN)
				{
					g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen += remain;
					return _otaDownloadProcess(cur, remain, size, FALSE);
				}
				else 
				{
					remain = size + OTA_HEAD_LEN - tempLen;
					g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen += remain;
					_otaDownloadProcess(cur, remain, size, FALSE);
					cur += remain;
					
					iot_debug_print("ota_parse OTA_FILE_APP fileCur %d, fileSize %d,remain %d", 
						g_s_otaCtx.fileCur, g_s_otaCtx.file[g_s_otaCtx.fileCur].readLen,end - cur);

					g_s_otaCtx.fileCur++;
					goto	OTA_CHECK;
				}
			}
				break;
			default:
				return OTA_ERROR_VERIFY_ERROR;
		}
	}

	
	return OTA_SUCCESS;
}
/*\-NEW\zhuwangbin\2020.2.5\添加二次开发远程升级*/
otaResult_t otaDone(void)
{
	return _otaDownloadDone();
}


