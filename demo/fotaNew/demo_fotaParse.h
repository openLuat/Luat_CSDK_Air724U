#ifndef __DEMO_FOTAPARSE_H__
#define __DEMO_FOTAPARSE_H__

#define OTA_APP_MAGIN ("APPUPDATE")
#define OTA_APP_FILE_PATH "fota/app"
#define OTA_CORE_FILE_PATH "fota/core"
#define OTA_FILE_INVALID_FD (0xffffffff)

#define OTA_HEAD 0x7e
#define OTA_APP_HEAD 0X7c
#define OTA_CORE_HEAD 0x7d
#define OTA_HEAD_LEN (5)

typedef enum
{
	OTA_FILE_NULL,
	OTA_FILE_APP,
	OTA_FILE_CORE
} E_OTA_FILE_TYPE;

typedef enum 
{
	OTA_STATE_NULL,
	OTA_STATE_OK,
	OTA_STATE_ERROR
} OTA_STATE;

typedef struct
{
	unsigned char head[OTA_HEAD_LEN];
	unsigned int readLen;
	unsigned int fileNum;
} T_OTA_HEAD_CTX;

typedef struct
{
	E_OTA_FILE_TYPE type;
	unsigned char head[OTA_HEAD_LEN];
	unsigned int readLen;
	unsigned int size;
} T_OTA_FILE_CTX;

typedef struct
{

	T_OTA_HEAD_CTX head;
	T_OTA_FILE_CTX file[2];
	unsigned int readLen;
	unsigned int fileCur;
}otaCtx;

typedef enum
{
  OTA_SUCCESS = 0,
  OTA_INIT,
  OTA_PROCESS,
  OTA_END,
  OTA_ERROR_FLASH_NOT_SUPPORT = -100,
  OTA_ERROR_NO_MEMORY,
  OTA_ERROR_FBF_ERROR,
  OTA_ERROR_FW_NOT_ENOUGH,
  OTA_ERROR_VERIFY_ERROR,
  OTA_ERROR_WRITE,
  OTA_ERROR_NOT_INIT,
}otaResult_t;

typedef enum
{
	OTA_APP_FILE,
	OTA_CODE_FILE,
	OTA_FILE_MAX
}otaFileIndex_t;

typedef struct
{
	int fd;
	int totalLen;
	int writeLen;
	otaResult_t state;
} otaFile_t;

typedef struct
{
	otaFile_t file[OTA_FILE_MAX];
}otaCtx_t;

otaResult_t otaInit(void);
otaResult_t otaProcess(char* data, unsigned int len, unsigned int total);
otaResult_t otaDone(void);


#endif
