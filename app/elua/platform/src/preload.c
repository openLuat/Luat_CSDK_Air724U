/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    preload.c
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/7/1
 *
 * Description:
 *          预置文件解析处理
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "assert.h"
#include "LzmaLib.h"
#include "preload.h"
#include "LzmaLib.h"
#include "LzmaDec.h"
#include "ut_md5.h"
#include "btea.h"
#include "am_openat.h"




static int decodeFile(const u8 *pData, DbFileInfo *pFileInfo, u32 *pOffset);
static int decodeHeadInfo(const u8 *pData, DbHeadInfo *pHeadInfo, u32 *pOffset);
#ifdef WIN32
int writeCustomResData(unsigned char* data_ptr, unsigned int addr, unsigned int len)
{
	return 0;
}


int eraseCustomResData(unsigned int addr, unsigned int len)
{
	return 0;
}

BOOL file_exist(char* name)
{

	FILE* fp = fopen(name, "rb");

	if(fp)
	{
		fclose(fp);
		return TRUE;
	}

    fp = fopen_ext(name, "rb");
	if(fp)
	{
		fclose_ext(fp);
		return TRUE;
	}	
	return FALSE;
}
#endif


static T_UNCOMPRESS_FILE_TABLE_ITEM *pResFlashFileTable = NULL;
static T_UNCOMPRESS_FILE_TABLE_ITEM *pScriptFlashFileTable = NULL;


//extern char _lua_script_section_start[LUA_SCRIPT_SIZE - CUSTOMER_FILE_OFFSET];
extern char _lua_res_section_start[LUA_RES_SIZE - CUSTOMER_FILE_OFFSET];

int writeCustomResData(unsigned char* data_ptr, unsigned int addr, unsigned int len);
int eraseCustomResData(unsigned int addr, unsigned int len);
void OPENAT_watchdog_restart(void);

static BOOL AddUncompressFileItem(E_LUA_SCRIPT_TABLE_SECTION nSection, const char *pFile, s32 nOffset, s32 nLen)
{
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;
    
    if((nSection >= LUA_SCRIPT_TABLE_MAX_SECTION) || (NULL == pFile) || (strlen(pFile) == 0) || (nLen == 0))
    {
        OPENAT_print("[AddUncompressFileItem]: para error!\n");
        return FALSE;
    }

    pItem = L_CALLOC(1,sizeof(T_UNCOMPRESS_FILE_TABLE_ITEM));
    if(NULL == pItem)
    {
        OPENAT_print("[AddUncompressFileItem]: L_MALLOC item error!\n");
        return FALSE;
    }
    pItem->pFile = L_CALLOC(1,strlen(pFile) + 1);
    if(NULL == pItem->pFile)
    {
        OPENAT_print("[AddUncompressFileItem]: L_MALLOC file error!\n");
        L_FREE(pItem);
        pItem = NULL;
        return FALSE;
    }
    
    memcpy(pItem->pFile, pFile, strlen(pFile));

	pItem->pFile[strlen(pFile)] = '\0';
	pItem->pNext = NULL;

    pItem->nOffset = nOffset;
    pItem->nLen = nLen;

    if(LUA_RES_TABLE_FLASH_SECTION == nSection)
    {
#if 0    
        if(NULL == pResFlashFileTable)
        {
            pResFlashFileTable = pItem;
            return TRUE;
        }
        pItem->pNext = pResFlashFileTable;
        pResFlashFileTable = pItem;
#endif
        ASSERT(0);
    }
    else if(LUA_SCRIPT_TABLE_FLASH_SECTION == nSection)
    {
        if(NULL == pScriptFlashFileTable)
        {
            pScriptFlashFileTable = pItem;
            return TRUE;
        }
        pItem->pNext = pScriptFlashFileTable;
        pScriptFlashFileTable = pItem;
    }

    
    return TRUE;
}

static BOOL FindUncompressFileItem(E_LUA_SCRIPT_TABLE_SECTION *pSection, T_UNCOMPRESS_FILE_TABLE_ITEM **ppItem, const char *pFile)
{
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;
    short idx = 0;
    int fileNameLen;
    T_UNCOMPRESS_FILE_TABLE_ITEM* table = NULL;
    if((!pSection) || (!ppItem) || (!pFile) || (strlen(pFile) == 0))
    {
        OPENAT_print("[FindUncompressFileItem]: para error!\n");
        return FALSE;
    }

    fileNameLen = strlen(pFile);
#if 0    
    if(strncmp(&pFile[fileNameLen - 5],".luac", 5) == 0 ||
        strncmp(&pFile[fileNameLen - 4],".lua", 4) == 0)
#endif        
    {
        table = pScriptFlashFileTable;
        *pSection = LUA_SCRIPT_TABLE_FLASH_SECTION;
    }
#if 0      
    else
    {
        table = pResFlashFileTable;
        *pSection = LUA_RES_TABLE_FLASH_SECTION;
    }
#endif    
    if(table)
    {
        pItem = table;
        idx = 0;
        
        while(pItem)
        {
            if((fileNameLen == strlen(pItem->pFile)) && (memcmp(pFile, pItem->pFile, fileNameLen) == 0))
            {
                *ppItem = pItem;
                return TRUE;
            }
            idx++;
            pItem = pItem->pNext;
        }
    }


    return FALSE;
}

static void PrintUncompressFileTable(E_LUA_SCRIPT_TABLE_SECTION section)
{
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;

    if(section == LUA_RES_TABLE_FLASH_SECTION)
    {
        pItem = pResFlashFileTable;
    }
    else
    {
        pItem = pScriptFlashFileTable;
    }
    
    while(pItem)
    {
        OPENAT_print("FH:file=%s,len=%d,offset=%d\n", pItem->pFile, pItem->nLen, pItem->nOffset);
        pItem = pItem->pNext;
    }

}


#ifndef WIN32
char *fgets_ext(char *buf, int n, FILE *fp)
{
    int character = 0;
    int idx = 0;
    
    if((NULL == buf) || (n <= 1) || (NULL == fp))
    {
        return NULL;
    }

    character = getc_ext(fp);

    if(EOF_EXT == character)
    {
        return NULL;
    }
    
    while(EOF_EXT != character)
    {
        if(idx >= (n-1))
        {
            break;
        }

        buf[idx] = character;
        if(0x0A == character)
        {
            buf[idx+1] = 0;
            break;
        }
        
        idx++;
        character = getc_ext(fp);
    }

    if(EOF_EXT == character)
    {
        buf[idx] = 0;
    }
    
    buf[n-1] = 0;
    return buf;
}


FILE *fopen_ext(const char *file, const char *mode)
{
    FILE *fp = NULL;
    E_LUA_SCRIPT_TABLE_SECTION section = LUA_SCRIPT_TABLE_MAX_SECTION;
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;
    int fileNameLen = strlen(file);
    
    if((!file) || (strlen(file) == 0))
    {
        OPENAT_print("[fopen_ext]: para error!\n");
        return fp;
    }

    if(FindUncompressFileItem(&section, &pItem, file))
    {        
        fp = L_CALLOC(1,sizeof(FILE));
        if(fp)
        {
            fp->_flags = section;
            fp->_cookie = pItem;
        }
        fp->_type = LUA_UNCOMPRESS_FILE;

    #ifdef AM_LUA_CRYPTO_SUPPORT
        if(strncmp(&file[fileNameLen - 5],".luac", 5) == 0)
        {
            fp->_type |= ENC_FILE;
        }
    #endif

        //printf("[fopen_ext]: %s %d!\n", file, fp->_type);

    }

    return fp;
}


void*  fread_addr_ext(FILE *fp)
{
    size_t resid;
    int len;
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;

    if((!fp) || (fp->_flags != LUA_RES_TABLE_FLASH_SECTION &&  fp->_flags !=LUA_SCRIPT_TABLE_FLASH_SECTION))
    {
        return 0;
    }

    pItem = (T_UNCOMPRESS_FILE_TABLE_ITEM *)(fp->_cookie);
    len = ((pItem->nLen - fp->_offset) >= resid) ? resid : (pItem->nLen - fp->_offset);

    if(len <= 0)
    {
        return 0;
    }

    if(fp->_flags == LUA_RES_TABLE_FLASH_SECTION)
    {
        //return &_lua_res_section_start[pItem->nOffset + fp->_offset]; 
        ASSERT(0);
    }
    else if (fp->_flags == LUA_SCRIPT_TABLE_FLASH_SECTION)
    {
        //return &_lua_script_section_start[pItem->nOffset + fp->_offset]; 
        ASSERT(0);
    }
    return 0;

}


int fclose_ext(FILE *fp)
{
    if(!fp)
    {
        return -1;
    }    

    L_FREE(fp);
    
    return 0;
}

int getc_ext(FILE *fp)
{
    char c;
    
    if(fread_ext((void *)&c, 1, 1, fp) != 1){
        return (EOF_EXT);
    }

    return c;
}

int ungetc_ext(int c, FILE *fp)
{
    fseek_ext(fp, -1, SEEK_CUR);

    return 0;
}

extern const uint32_t enc_code[4];

//#define CRYPTO_DEBUG
#define DEC_BUFF_SIZE 512

static int decode_file(void *buf, size_t size, size_t count, FILE *fp)
{

    unsigned int act_low_boundary;  /*以512对齐的读取文件的起始位置*/
    unsigned int read_count;        /*需要从文件中读取的长度*/
    unsigned int act_up_boundary;   /*以512对齐的读取文件的结束位置*/
    unsigned int act_count;         /*读取到的有效数据长度*/
    unsigned char* temp;
    unsigned int* data = NULL;
    size_t resid;
    int len;
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;
    unsigned int offset  = ftell_ext(fp);

    int decCount;
    int i = 0;
    int real_size;

    resid = count * size;
    
    pItem = (T_UNCOMPRESS_FILE_TABLE_ITEM *)(fp->_cookie);

    act_low_boundary = (offset & 0xFFFFFE00);
    act_up_boundary = ((offset + resid + DEC_BUFF_SIZE - 1) & 0xFFFFFE00);
    read_count = act_up_boundary - act_low_boundary; 


    /*多申请8个字节的内存，以保证能4字节对齐*/
    data = (unsigned int*)L_MALLOC(4 + read_count + 4);
    
    /*保证4字节对齐*/
    temp = (unsigned char*)((((unsigned int)data + 3) >> 2) << 2);

    /*把文件指针移到以512对齐的位置*/
    fseek_ext(fp, act_low_boundary, SEEK_SET);

    len = ((pItem->nLen - offset) >= read_count) ? read_count : (pItem->nLen - offset);

    ASSERT(OPENAT_MEMD_ERR_NO == OPENAT_flash_read(OPENAT_turn_addr(LUA_SCRIPT_ADDR) + CUSTOMER_FILE_OFFSET + pItem->nOffset + offset, len, NULL, temp));
    //memcpy(temp, &_lua_script_section_start[pItem->nOffset + offset], len);
    
    
    act_count = resid;
    decCount = len / DEC_BUFF_SIZE;
   
    /*如果没有读到足够多的数据，意味着快到文件的末尾了*/
    if(read_count > len)
    {
        real_size = pItem->nLen;

        if(real_size - offset < resid)
        {
            act_count = real_size - offset;
        }
    }
    
    /*把文件指针移到真实的位置*/
    fseek_ext(fp, offset + act_count, SEEK_SET);

#ifdef CRYPTO_DEBUG
    OPENAT_print("liulean decode info  %d %d %d %d %d %d\r\n", 
        act_low_boundary, 
        act_up_boundary, 
        offset + count,
        offset - act_low_boundary,
        count,
        decCount);
#endif

    while(i < decCount)
    {
        btea((unsigned int*)(temp + DEC_BUFF_SIZE * i), -((DEC_BUFF_SIZE) / 4), enc_code);
        i++;
    }

    btea((unsigned int*)(temp + DEC_BUFF_SIZE * i), -((len % DEC_BUFF_SIZE) / 4), enc_code);
    
    memcpy(buf, &temp[offset - act_low_boundary], act_count);
    L_FREE(data);

    return (act_count / size);

}



size_t fread_ext(void *buf, size_t size, size_t count, FILE *fp)
{
    size_t resid;
    int len;
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;

    if((!buf) || (!fp))
    {
        return 0;
    }

    if((resid = count * size) == 0)
    {
        OPENAT_print("[fread_ext]: size 0!\n");
        return 0;
    }

    
    pItem = (T_UNCOMPRESS_FILE_TABLE_ITEM *)(fp->_cookie);
    len = ((pItem->nLen - fp->_offset) >= resid) ? resid : (pItem->nLen - fp->_offset);

    if(len <= 0)
    {
        return 0;
    }

    if(fp->_flags == LUA_RES_TABLE_FLASH_SECTION)
    {
        //memcpy(buf, &_lua_res_section_start[pItem->nOffset + fp->_offset], len);
        ASSERT(0);
    }
    else if(fp->_flags == LUA_SCRIPT_TABLE_FLASH_SECTION)
    {
    #ifdef AM_LUA_CRYPTO_SUPPORT
        if(fp->_type == LUA_UNCOMPRESS_ENC_FILE)
        {
            return decode_file(buf, size, count, fp);
        }
        else
    #endif
        {
            //memcpy(buf, &_lua_script_section_start[pItem->nOffset + fp->_offset], len);
            ASSERT(OPENAT_MEMD_ERR_NO == OPENAT_flash_read(
                          OPENAT_turn_addr(LUA_SCRIPT_ADDR) + CUSTOMER_FILE_OFFSET + pItem->nOffset + fp->_offset, 
                            len, NULL, buf));

        }
    }
    else
    {
        return 0;
    }

    fp->_offset += len;
    
    return (len/size);
}

int fseek_ext(FILE *fp, long offset, int whence)
{
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;

    if(!fp)
    {
        OPENAT_print("[fseek_ext]: fp error!\n");
        return -1;
    }

    pItem = (T_UNCOMPRESS_FILE_TABLE_ITEM *)(fp->_cookie);
    
    if(SEEK_SET == whence)
    {
        if((offset > pItem->nLen) || (offset < 0))
        {
            OPENAT_print("[fseek_ext]: set(%d) error!\n", offset);
            return -1;
        }
        fp->_offset = offset;
    }
    else if(SEEK_CUR == whence)
    {
        if(((fp->_offset + offset) > pItem->nLen) || ((fp->_offset + offset) < 0))
        {
            OPENAT_print("[fseek_ext]: cur(%d) error!\n", (fp->_offset + offset));
            return -1;
        }
        fp->_offset += offset;
    }
    else if(SEEK_END == whence)
    {
        if((offset < -(pItem->nLen)) || (offset > 0))
        {
            OPENAT_print("[fseek_ext]: end(%d),(%d) error!\n", offset,-(pItem->nLen));
            return -1;
        }
        fp->_offset = pItem->nLen - offset;
    }
    else
    {
        OPENAT_print("[fseek_ext]: whence error!\n");
        return -1;
    }
    
    return 0;
}

long ftell_ext(FILE *fp)
{
    if(!fp)
    {
        OPENAT_print("[ftell_ext]: fp error!\n");
        return -1;
    }

    return fp->_offset;    
}

int feof_ext(FILE *fp)
{
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;

    if(!fp)
    {
        OPENAT_print("[feof_ext]: fp error!\n");
        return 0;
    }

    pItem = (T_UNCOMPRESS_FILE_TABLE_ITEM *)(fp->_cookie);

    if(fp->_offset >= pItem->nLen)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


extern int file_exist(const char *name);
#else
#define getenv(string) get_luaenv(string)

char* get_luaenv(const char* string)
{
	return "lua";
}
#endif

//参数(需要解压的文件名，解压放的文件地方)
/*+\NEW\wangyong\2016.1.28\添加解压合成的.bin文件*/
int parse_compre_bin_file(DbFileInfo *pFileInfo, char* fileBuf, u32 offset,char *Decomdfilename)
{
    char filename[256];
     int lzmaRet = 0;
     u8 withoutZipLen;
    if(pFileInfo->nameLen > 4)
    {
        OPENAT_print("parse_compre_bin_file 2\n");
         withoutZipLen = pFileInfo->nameLen - 4;
        memset(filename, 0, sizeof(filename));
        if(strcmp(&pFileInfo->name[withoutZipLen], ".zip") == 0)
         {
            strcpy(filename,Decomdfilename); 
            strcat(filename,"/");
            strncat(filename, pFileInfo->name, withoutZipLen);
            lzmaRet = LzmaDecodeBufToFile(pFileInfo->data, pFileInfo->length, filename);
            if(lzmaRet != 0)
            {
              return LUADB_ERR_UNZIP_FILE;
            }
         }
    }
 return 0;
}



int decompress_file(const char *Comfilename,char *Decomdfilename)
{
    #define HEAD_MAX_LEN 100
    #define FILE_MAX_LEN  (250*1024)  /* 单个文件最大限制, 文件最大限制250K */
    FILE *fp = NULL;
    int total_size,res = -1;
    u8 *buff = NULL;
    u8 *filebuf = NULL;

    DbHeadInfo headInfo;
    DbFileInfo *pFileInfo = NULL;

    BOOL bZipFile = FALSE;
    
    u32 offset = 0;
    u16 fileIndex; 
    u32 readlen = 0;
    if((fp = fopen(Comfilename, "rb")) == NULL)
    {
        return LUADB_ERR_NONE;
    }
     buff = L_MALLOC(HEAD_MAX_LEN);
    if(buff == NULL)
    {
        return LUADB_ERR_MALLOC_FAIL;
    }
    
    fread(buff, 1, HEAD_MAX_LEN, fp);
     
    /* 解析headInfo */
    res = decodeHeadInfo(buff, &headInfo, &offset);
    if(res != LUADB_ERR_NONE)
    {
       L_FREE(buff);
       return res;
    }
    

    fseek(fp, offset - HEAD_MAX_LEN+1, SEEK_CUR);  

    pFileInfo = L_CALLOC(1, sizeof(DbFileInfo)*headInfo.filecount);
    if(pFileInfo == NULL)
    {
        return LUADB_ERR_MALLOC_FAIL;
    }
    filebuf = (u8*)L_MALLOC(FILE_MAX_LEN);
    if(NULL == filebuf)
    {
        L_FREE(buff);
        return LUADB_ERR_MALLOC_FAIL;
    }
  
    for(fileIndex = 0; fileIndex < headInfo.filecount; fileIndex++)
    {
        offset = 0;
		readlen = fread(filebuf,1,FILE_MAX_LEN,fp);
        
        if((res = decodeFile(&filebuf[0], &pFileInfo[fileIndex], &offset)) != LUADB_ERR_NONE)
        {
            L_FREE(buff);
            return res;
        }
        
        if(pFileInfo->nameLen > 4)
        {
            u8 withoutZipLen;

            withoutZipLen = pFileInfo[fileIndex].nameLen - 4;
            if(strcmp(&pFileInfo[fileIndex].name[withoutZipLen], ".zip") == 0)
            {
                bZipFile = TRUE;
            }
			else
			{
				bZipFile = FALSE;
			}
        }

        if(bZipFile)
        {
            if((res = parse_compre_bin_file(&pFileInfo[fileIndex], filebuf, offset,Decomdfilename)) != LUADB_ERR_NONE)
            {
                L_FREE(buff);
                return res;
            }
        }
        else
        {
            char filename[256];
            FILE* outfilep;

            memset(filename, 0, sizeof(filename));

            strcpy(filename,Decomdfilename); 
            strcat(filename,"/");
            strcat(filename, pFileInfo[fileIndex].name);

            OPENAT_print("decompress file %s", filename);
            
            outfilep = fopen(filename, "wb");
            
			/*整个文件已经读完了*/
			if(readlen - pFileInfo[fileIndex].offset >= pFileInfo[fileIndex].length)
			{
                fwrite(&filebuf[pFileInfo[fileIndex].offset], 1, pFileInfo[fileIndex].length, outfilep);
	            fclose(outfilep);

			}
			else /*还要继续读*/
			{
				int length = pFileInfo[fileIndex].length - (readlen - pFileInfo[fileIndex].offset);
                fwrite(&filebuf[pFileInfo[fileIndex].offset], 1, readlen - pFileInfo[fileIndex].offset, outfilep);

				while(length > FILE_MAX_LEN)
				{
					readlen = fread(filebuf, 1, FILE_MAX_LEN, fp);
					fwrite(filebuf, 1, readlen, outfilep);
					length -= readlen;
				}

				readlen = fread(filebuf, 1, length, fp);
				fwrite(filebuf, 1, readlen, outfilep);
				fclose(outfilep);
				continue;
			}

        }


        /* 文件游标回到真实的读取位置, 最后一个文件确实有点浪费, 多走了几次 */
		if(readlen != FILE_MAX_LEN)
		{
		    fseek(fp, offset-readlen, SEEK_END);  
		}
		else
		{
			fseek(fp, offset-FILE_MAX_LEN, SEEK_CUR);  
		}
    }
    
    fclose(fp);
    L_FREE(filebuf);
    L_FREE(buff);
    return res;
}

/*-\NEW\wangyong\2016.1.28\添加解压合成的.bin文件*/



/* 暂时不支持压缩的资源文件的远程升级 */
int parse_luadb_update_file(DbFileInfo *pFileInfo, char* fileBuf, u32 offset, u32* pos)
{
    int err = LUADB_ERR_NONE;
    char filename[256];
    int lzmaRet = 0;
    FILE *fout;

    if(pFileInfo->nameLen > 4)
    {
        u8 withoutZipLen = pFileInfo->nameLen - 4;
        u8 withoutLuaLen = withoutZipLen - 4;
        memset(filename, 0, sizeof(filename));

        if(strcmp(&pFileInfo->name[withoutZipLen], ".zip") == 0)
        {
            /* 压缩的脚本 */
            if(strncmp(&pFileInfo->name[withoutLuaLen],".lua", 4) == 0 ||
                strncmp(&pFileInfo->name[withoutLuaLen -1],".luac", 5) == 0)
            {
                strcpy(filename, getenv("LUA_DIR")); 
            }
            strcat(filename,"/");
            strncat(filename, pFileInfo->name, withoutZipLen);

            lzmaRet = LzmaDecodeBufToFile(pFileInfo->data, pFileInfo->length, filename);

            if(lzmaRet != 0)
            {
                err = LUADB_ERR_UNZIP_FILE;
                goto exit_err;
            }

        }
        else
        {
            /* 没有压缩的脚本 */
            if(strncmp(&pFileInfo->name[pFileInfo->nameLen-4],".lua", 4) == 0 ||
                strncmp(&pFileInfo->name[pFileInfo->nameLen-5],".luac", 5) == 0)
            {                    
                strcpy(filename, getenv("LUA_DIR"));
                strcat(filename,"/");
                strncat(filename, pFileInfo->name, pFileInfo->nameLen);
                
                fout = fopen(filename, "wb");

                if(!fout)
                {
                    err = LUADB_ERR_WRITE_FILE;
                    goto exit_err;
                }

                if(fwrite(pFileInfo->data, 1, pFileInfo->length, fout) != pFileInfo->length)
                {
                    OPENAT_print("[parse_luadb_data]: write file(%s) error!\n", filename);
                    fclose(fout);
                    fout = NULL;
                    remove(filename); //写文件失败,删除已写的文件.
                    err = LUADB_ERR_WRITE_FILE;
                    goto exit_err;
                }
                
                fclose(fout);

            
            }
            else  /* 剩下的都是资源了 */
            {
                /* 直接写flash */
                if(OPENAT_flash_write(((unsigned int)_lua_res_section_start + *pos), offset, NULL, fileBuf) != OPENAT_MEMD_ERR_NO)
                {
                    err = LUADB_ERR_WRITE_CUSTOMER;
                    goto exit_err;
                }

                /* 写flash成功, flash 的游标 */
                *pos += offset;
            }

        }

    }
    
exit_err:
    return err;
}


typedef enum LUADB_SECTION_FILE_TYPE_type
{
    LUADB_SECTION_CODE_FILE,
    LUADB_SECTION_RES_FILE,
    BL_SECTION_CODE_FILE,
}LUADB_SECTION_FILE_TYPE;


#ifndef WIN32
#define LUA_UPDATE_CODE_FILE "/luazip/update.bin"
#define LUA_UPDATE_RES_FILE "/luazip/update_res.bin"
#define BL_UPDATE_CODE_FILE "/luazip/EXT_BOOTLOADER.bin"
#else
#define LUA_UPDATE_CODE_FILE "luazip/update.bin"
#define LUA_UPDATE_RES_FILE "luazip/update_res.bin"
#define BL_UPDATE_CODE_FILE "luazip/EXT_BOOTLOADER.bin"
#endif

#define IN_BUFF_SIZE (120*1024)

#define OUT_BUFF_SIZE (240*1024)

#define BL_VERSION_LEN 24


typedef struct BOOTLOADER_INFO_TAG
{
    u8   version[24];
    u32 restart_to_download;
    u32 download_status;
    u8   reserved[64];
}BOOTLOADER_INFO;


extern const BOOTLOADER_INFO bootloader_info;
extern u32 Load$$BOOT_INFO_GFH$$Base;
int parse_BL_section_file()
{
    FILE *fp = NULL;
    int total_size;
    u8 *in_buff = NULL;
    u8 *out_buff = NULL;

    u32 offset = 0;
    u32 pos = 0;  
    u32 readlen = 0;
    int res = LUADB_ERR_NONE;

    u32 section_start;
    u32 section_size;
    char* update_file;
    int len;
    SizeT inProcessed, outProcessed, inLen;
#if 0    
    u32 customerfilehead[(CUSTOMER_FILE_OFFSET + 3)/4];
#endif
    u32   decodeLen = 0;
    u32   readSize = 0;

    u32   writeSize = 0;

    BOOTLOADER_INFO bootloader_info_bak;
#if 0    
    update_file = BL_UPDATE_CODE_FILE;
    
    /* 这里一定存在, 调用这个函数之前已经判断了, 为了保持一个函数内打开关闭文件 */
    if((fp = fopen(update_file, "rb")) == NULL)
    {
        return LUADB_ERR_NONE;
    }

    memcpy(&customerfilehead, &Load$$BOOT_INFO_GFH$$Base,  CUSTOMER_FILE_OFFSET);
    memcpy(&bootloader_info_bak, &bootloader_info,  sizeof(BOOTLOADER_INFO));

    
    {

#ifndef WIN32
        OPENAT_watchdog_restart();
#endif
        
        OPENAT_print("xxxxx 3333\n");

        in_buff = L_MALLOC(IN_BUFF_SIZE);

        if(in_buff == NULL)
        {
            res = LUADB_ERR_MALLOC_FAIL;
            goto exit_error;
        }

        out_buff  = L_MALLOC(OUT_BUFF_SIZE);

        if(out_buff == NULL)
        {
            res = LUADB_ERR_MALLOC_FAIL;
            goto exit_error;
        }
        
        fseek(fp, 0, SEEK_END);

        total_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

	{
		Md5Context   context;
		u8         output[MD5_DIGEST_LENGTH];
		u32         md5_len = 0;
		u32        read_size = 0;

		Md5Init (&context);

		while(1)
		{
			read_size = (total_size - MD5_DIGEST_LENGTH - md5_len > IN_BUFF_SIZE) ? IN_BUFF_SIZE : (total_size - MD5_DIGEST_LENGTH - md5_len);

			len = fread(in_buff, 1, read_size, fp);
			Md5Update (&context, in_buff, len);
			md5_len += len;

			if(read_size < IN_BUFF_SIZE)
			{
				Md5Final (output, &context);
				break;
			}

		}
		
		fread(in_buff, 1, MD5_DIGEST_LENGTH, fp);  
		
		if(memcmp(in_buff, output, MD5_DIGEST_LENGTH) != 0)
		{
                    printf("bin file md5 check failed!");
                    fclose(fp);
                    res = LUADB_ERR_CHECK_FAIL;
                    goto exit_error;
		}

               fseek(fp, 0, SEEK_SET);
	}



            
        OPENAT_print("xxxxx file size %d\n" ,total_size);

	if(total_size - MD5_DIGEST_LENGTH > IN_BUFF_SIZE)
	{
		len = fread(in_buff, 1, IN_BUFF_SIZE, fp);
	}
	else
	{
		len = fread(in_buff, 1, total_size - MD5_DIGEST_LENGTH, fp);
	}

        memcpy(&bootloader_info_bak.version, in_buff, sizeof(bootloader_info_bak.version));

        section_start   = in_buff[0x1c + BL_VERSION_LEN];
        section_start |=  (in_buff[0x1d + BL_VERSION_LEN] << 8);
        section_start |=  (in_buff[0x1e + BL_VERSION_LEN] << 16);
        section_start |=  (in_buff[0x1f + BL_VERSION_LEN] << 24);

        section_size = total_size - MD5_DIGEST_LENGTH - BL_VERSION_LEN;
        
        res = eraseCustomResData(section_start, section_size);

        if(res != 0)
        {
            res = LUADB_ERR_EARSE_CUSTOMER;
            fclose(fp);

            goto exit_error;
        }

        writeCustomResData(&in_buff[BL_VERSION_LEN], section_start, len - BL_VERSION_LEN);
        OPENAT_print("xxxxx 11111 %d %d %d\n", section_size,  len, total_size);

        writeSize = len;

        while(writeSize + MD5_DIGEST_LENGTH < total_size)
        {
            if(total_size - writeSize - MD5_DIGEST_LENGTH > IN_BUFF_SIZE)
            {
            	len = fread(in_buff, 1, IN_BUFF_SIZE, fp);
            }
            else
            {
            	len = fread(in_buff, 1, (total_size - writeSize - MD5_DIGEST_LENGTH), fp);
            }

            writeCustomResData(in_buff, section_start + writeSize , len);
            writeSize += len;
        }
        

        OPENAT_print("xxxxx 2222\n");

        fclose(fp);
    }

   
    eraseCustomResData(customerfilehead[0x1c / 4], 
                                        customerfilehead[0x20 / 4]);

    writeCustomResData((unsigned char *)customerfilehead, 
                                        (u32)&Load$$BOOT_INFO_GFH$$Base, 
                                        CUSTOMER_FILE_OFFSET);
    
    writeCustomResData((unsigned char *)&bootloader_info_bak, 
                                      (u32)&Load$$BOOT_INFO_GFH$$Base + CUSTOMER_FILE_OFFSET, 
                                      sizeof(bootloader_info_bak));

#endif
exit_error:
    if(in_buff != NULL)
    {
        L_FREE(in_buff);
    }

    if(out_buff != NULL)
    {
        L_FREE(out_buff);
    }

    return res;
}


int parse_luadb_update_section_file(LUADB_SECTION_FILE_TYPE fileType)
{
    FILE *fp = NULL;
    int total_size;
    u8 *in_buff = NULL;
    u8 *out_buff = NULL;

    u32 offset = 0;
    u32 pos = 0;  
    u32 readlen = 0;
    int res = LUADB_ERR_NONE;

    char * section_start;
    u32 section_size;
    char* update_file;
    int len;
    SizeT inProcessed, outProcessed, inLen;
#if 0    
    u8 customerfilehead[CUSTOMER_FILE_OFFSET];
#endif
    u32   decodeLen = 0;
	u32   readSize = 0;
	
    update_file = (fileType == LUADB_SECTION_RES_FILE) ? LUA_UPDATE_RES_FILE : LUA_UPDATE_CODE_FILE;

    /* 这里一定存在, 调用这个函数之前已经判断了, 为了保持一个函数内打开关闭文件 */
    if((fp = fopen(update_file, "rb")) == NULL)
    {
        return LUADB_ERR_NONE;
    }


    if(fileType == LUADB_SECTION_RES_FILE)
    {
        section_start = _lua_res_section_start  - CUSTOMER_FILE_OFFSET;
        section_size = LUA_RES_SIZE ;
    }
    else
    {
        //section_start = _lua_script_section_start  - CUSTOMER_FILE_OFFSET;
        section_start = OPENAT_turn_addr(LUA_SCRIPT_ADDR);
        section_size = LUA_SCRIPT_SIZE;
    }
    
    {

#ifndef WIN32
        OPENAT_watchdog_restart();
#endif
        
        OPENAT_print("xxxxx 3333\n");


        in_buff = L_MALLOC(IN_BUFF_SIZE);

        if(in_buff == NULL)
        {
            res = LUADB_ERR_MALLOC_FAIL;
            goto exit_error;
        }

        out_buff  = L_MALLOC(OUT_BUFF_SIZE);

        if(out_buff == NULL)
        {
            res = LUADB_ERR_MALLOC_FAIL;
            goto exit_error;
        }
        
        fseek(fp, 0, SEEK_END);

        total_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

		{
			Md5Context   context;
			u8         output[MD5_DIGEST_LENGTH];
			u32         md5_len = 0;
			u32        read_size = 0;

			Md5Init (&context);

			while(1)
			{
				read_size = (total_size - 16 - md5_len > IN_BUFF_SIZE) ? IN_BUFF_SIZE : (total_size - 16 - md5_len);

				len = fread(in_buff, 1, read_size, fp);
				Md5Update (&context, in_buff, len);
				md5_len += len;

				if(read_size < IN_BUFF_SIZE)
				{
					Md5Final (output, &context);
					break;
				}

			}
			
			fread(in_buff, 1, 16, fp);  
			
			if(memcmp(in_buff, output, 16) != 0)
			{
				OPENAT_print("bin file md5 check failed!");
                fclose(fp);
                res = LUADB_ERR_CHECK_FAIL;
           	    goto exit_error;
			}

	        fseek(fp, 0, SEEK_SET);
		}

        OPENAT_print("xxxxx 11111\n");
        res = OPENAT_flash_erase(((unsigned int)section_start), section_start + section_size);
        if(res != 0)
        {
            res = LUADB_ERR_EARSE_CUSTOMER;
            fclose(fp);

            goto exit_error;
        }

        OPENAT_print("xxxxx 2222\n");
            
        OPENAT_print("xxxxx file size %d\n" ,total_size);

		if(total_size - 16 > IN_BUFF_SIZE)
		{
			len = fread(in_buff, 1, IN_BUFF_SIZE, fp);
		}
		else
		{
			len = fread(in_buff, 1, total_size - 16, fp);
		}

		readSize += len;
		
		offset = 13;
        inProcessed = inLen = len - offset;
        outProcessed = OUT_BUFF_SIZE;
        lzmaDecodeStreamInit(in_buff, len);
		LzmaDecodeStream(in_buff + offset, OUT_BUFF_SIZE, out_buff, LZMA_FINISH_ANY, &inProcessed, &outProcessed);

		decodeLen += inProcessed;
		
        OPENAT_print("xxxxx decode %d\n" ,outProcessed);

    OPENAT_flash_write(((unsigned int)section_start +pos), outProcessed, NULL, out_buff);
		pos += outProcessed;

		while(outProcessed > 0)
		{
			memcpy(in_buff, &in_buff[inProcessed + offset], inLen - inProcessed);
			offset = 0;

			if(readSize + 16 + IN_BUFF_SIZE - inLen + inProcessed > total_size)
			{
				len = fread(&in_buff[inLen - inProcessed], 1, total_size - 16 - readSize, fp);
			}
			else
			{
				len = fread(&in_buff[inLen - inProcessed], 1, (IN_BUFF_SIZE - inLen + inProcessed), fp);
			}

			readSize += len;

#if 0
			if(len == 0)
			{
				break;
			}
#endif
			inProcessed = inLen = len + inLen - inProcessed;
	        outProcessed = OUT_BUFF_SIZE;

			LzmaDecodeStream(in_buff, OUT_BUFF_SIZE, out_buff, LZMA_FINISH_ANY, &inProcessed, &outProcessed);
            OPENAT_print("xxxxx decode %d\n" ,outProcessed);

      OPENAT_flash_write(((unsigned int)section_start +pos), outProcessed, NULL, out_buff);
			pos += outProcessed;
			decodeLen += inProcessed;

			if(decodeLen + 13 >= total_size - 16)
			{
				break;
			}
		}

        fclose(fp);
     //   OPENAT_print("xxxxx 4444\n");   
    }

exit_error:
    if(in_buff != NULL)
    {
        L_FREE(in_buff);
    }

    if(out_buff != NULL)
    {
        L_FREE(out_buff);
    }


    return res;

}


int parse_luadb_update_data()
{
    int result;
    
    if((result = parse_luadb_update_section_file(LUADB_SECTION_CODE_FILE)) != LUADB_ERR_NONE)
    {
        return result;
    }

    return parse_luadb_update_section_file(LUADB_SECTION_RES_FILE);
}

static u16 calcCheckCode(const u8 *data, u32 length)
{
    u32 checksum = 0;

    while(length--)
    {
        checksum += data[length];
    }

    return (u16)checksum;
}

static int decodeFile(const u8 *pData, DbFileInfo *pFileInfo, u32 *pOffset)
{
    u8 type;
    u8 length;
    int err = 0;
    u32 pos = 0;
    u16 checkCode = 0;
    BOOL loop = TRUE;
    u32 magic = 0;
    
    while(loop && !err)
    {
        type = pData[pos++];
        length = pData[pos++];
        
        switch(type)
        {
        case FileHeadReserved0x00:
        case FileHeadReserved0xFF:
            err = ASSERT_LEN(length, 0);
            break;
            
        case FileHeadMagicNumber:
            err = ASSERT_LEN(length, 4);
            magic = DBMAKEU32(pData, pos);
            break;
            
        case FileHeadName:
            pFileInfo->name = L_CALLOC(1, length+1);
            memcpy(pFileInfo->name, &pData[pos], length);
			pFileInfo->name[length] ='\0';
            pFileInfo->nameLen = length;
            break;
            
        case FileHeadLength:
            err = ASSERT_LEN(length, 4);
            pFileInfo->length = DBMAKEU32(pData, pos);
            break;

        case FileHeadCRC:
            err = ASSERT_LEN(length, 2);
            checkCode = DBMAKEU16(pData, pos);
            loop = FALSE;
            break;

        default:
            err = LUADB_ERR_UNKNOWN_TYPE;
            break;
        }
        
        pos += length;
    }
    
    if(err != 0)
    {
        goto decode_file_exit;
    }
    
    if(magic != 0xA55AA55A)
    {
        err = LUADB_ERR_MAGIC;
        goto decode_file_exit;
    }

    if(calcCheckCode(pData, pos - 2/*check data 2 byte*/) != checkCode)
    {
        err = LUADB_ERR_CHECK_FAIL;
        goto decode_file_exit;
    }
    
    pFileInfo->data = &pData[pos];
    pFileInfo->offset = *pOffset + pos;

    // 计算偏移 +文件内容与填充内容
    pos += pFileInfo->length;
    *pOffset += pos;
    
decode_file_exit:
    return err;
}

static int decodeHeadInfo(const u8 *pData, DbHeadInfo *pHeadInfo, u32 *pOffset)
{
    u8 type;
    u8 length;
    int err = 0;
    u32 pos = 0;
    u16 checkCode = 0;
    BOOL loop = TRUE;
    u32 magic = 0;

    while(loop && !err)
    {
        type = pData[pos++];
        length = pData[pos++];

        switch(type)
        {
        case HeadInfoMagicNumber:
            err = ASSERT_LEN(length, 4);
            magic = DBMAKEU32(pData, pos);
            break;

        case HeadInfoVersion:
            err = ASSERT_LEN(length, 2);
            pHeadInfo->verNum = DBMAKEU16(pData, pos);
            break;
        
        case HeadInfoLength:
            err = ASSERT_LEN(length, 4);
            pHeadInfo->length = DBMAKEU32(pData, pos);
            break;

        case HeadInfoFilecount:
            err = ASSERT_LEN(length, 2);
            pHeadInfo->filecount = DBMAKEU16(pData, pos);
            break;

        case HeadInfoReserved0x00:
        case HeadInfoReserved0xFF:
            err = ASSERT_LEN(length, 0);
            break;
        
        case HeadInfoCRC:
            err = ASSERT_LEN(length, 2);
            checkCode = DBMAKEU16(pData, pos);
            loop = FALSE;
            break;

        default:
            err = LUADB_ERR_UNKNOWN_TYPE;
            break;
        }

        pos += length;
    }

    if(err != 0)
    {
        goto decode_head_exit;
    }

    if(magic != 0xA55AA55A)
    {
        err = LUADB_ERR_MAGIC;
        goto decode_head_exit;
    }

    if(calcCheckCode(pData, pos - 2/*check data 2 byte*/) != checkCode)
    {
        err = LUADB_ERR_CHECK_FAIL;
        goto decode_head_exit;
    }

/*+\NEW\liweiqiang\2013.12.9\去掉luadb最多100个文件的限制 */
#if 0    
    if(pHeadInfo->filecount > 100)
    {
        err = LUADB_ERR_TOO_MANY_FILES;
        goto decode_head_exit;
    }
#endif
/*-\NEW\liweiqiang\2013.12.9\去掉luadb最多100个文件的限制 */

    *pOffset += pHeadInfo->length;

decode_head_exit:
    return err;
}


int parse_luadb_data(const u8 *pData, u32 length, BOOL override, E_LUA_SCRIPT_TABLE_SECTION section, BOOL *pRestart)
{
#define LUA_UPDATE_FILE "/luazip/update.bin"
    int err;
    u32 offset = 0;
    DbHeadInfo headInfo;
    u16 fileIndex;
    DbFileInfo *pFileInfo = NULL;
    char filename[256];
    int lzmaRet = 0;

    E_LUA_SCRIPT_TABLE_SECTION nSection = LUA_SCRIPT_TABLE_MAX_SECTION;
    T_UNCOMPRESS_FILE_TABLE_ITEM *pItem = NULL;
    BOOL exist = FALSE;


    if(!(pData[0] == 0x01 || pData[1] == 0x04))
        return LUADB_ERR_NOT_DBDATA;

    memset(&headInfo, 0, sizeof(headInfo));

    err = decodeHeadInfo(pData, &headInfo, &offset);

    if(LUADB_ERR_NONE != err)
    {
        goto decode_exit;
    }

    pFileInfo = L_CALLOC(1, sizeof(DbFileInfo)*headInfo.filecount);

    if(!pFileInfo)
        goto decode_exit;

    for(fileIndex = 0; fileIndex < headInfo.filecount && offset < length; fileIndex++)
    {
        if((err = decodeFile(&pData[offset], &pFileInfo[fileIndex], &offset)) != LUADB_ERR_NONE)
        {
            goto decode_exit;
        }
    }

    if(offset > length)
    {
        err = LUADB_ERR_OUT_OF_RANGE;
        goto decode_exit;
    }

    if(fileIndex != headInfo.filecount)
    {
        err = LUADB_ERR_FILE_COUNT;
        goto decode_exit;
    }

    OPENAT_print("[parse_luadb_data]:section %d, override %d  %d\n", section, override, headInfo.filecount);


    switch(section)
    {
        case LUA_SCRIPT_TABLE_FLASH_SECTION:
            {
                // write file to fs
                for(fileIndex = 0; fileIndex < headInfo.filecount; fileIndex++)
                {   
                    memset(filename, 0, sizeof(filename));
                    strcpy(filename, getenv("LUA_DIR")); 
                    strcat(filename,"/");
                    strncat(filename, pFileInfo[fileIndex].name, strlen(pFileInfo[fileIndex].name));
                
                    if(FALSE == AddUncompressFileItem(section, filename, pFileInfo[fileIndex].offset, pFileInfo[fileIndex].length))
                    {
                        err = LUADB_ERR_ADD_TABLE_ITEM;
                        goto decode_exit;
                    }
                    else
                    {
                        continue;
                    }
                }          
                
            }
            break;
        default:
            break;
    }


decode_exit:
    if(pFileInfo)
    {
        for(fileIndex = 0; fileIndex < headInfo.filecount; fileIndex++)
        {
            if(pFileInfo[fileIndex].name)
                L_FREE(pFileInfo[fileIndex].name);
        }

        L_FREE(pFileInfo);
    }

    if(err != LUADB_ERR_NONE)
        OPENAT_print("[parse_luadb_data]: luadb error %d\n", err);

#if 1
    PrintUncompressFileTable(section);
#endif

    *pRestart = FALSE;

    return err;
}


