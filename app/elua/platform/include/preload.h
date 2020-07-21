/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    preload.h
 * Author:  liweiqiang
 * Version: V0.1
 * Date:    2013/7/1
 *
 * Description:
 *          预置文件解析处理
 **************************************************************************/

#ifndef _PRELOAD_H_
#define _PRELOAD_H_

#define LUADB_ERR_NONE              0
#define LUADB_ERR_TLV_LEN           -1
#define LUADB_ERR_CHECK_FAIL        -2
#define LUADB_ERR_TOO_MANY_FILES    -3
#define LUADB_ERR_FILE_COUNT        -4
#define LUADB_ERR_MAGIC             -5
#define LUADB_ERR_OUT_OF_RANGE      -6
#define LUADB_ERR_UNKNOWN_TYPE      -7
#define LUADB_ERR_UNZIP_FILE        -8
#define LUADB_ERR_WRITE_FILE        -9
#define LUADB_ERR_ADD_TABLE_ITEM        -10
#define LUADB_ERR_EARSE_CUSTOMER       -11
#define LUADB_ERR_WRITE_CUSTOMER        -12
#define LUADB_ERR_MALLOC_FAIL         -13


#define LUADB_ERR_NOT_DBDATA        -100 //非luadb数据

/*+\NEW\zhuth\2014.2.17\通过文件记录表访问luadb中未压缩的文件*/
typedef enum
{
    LUA_RES_TABLE_FLASH_SECTION,
    LUA_SCRIPT_TABLE_FLASH_SECTION,
    

    LUA_SCRIPT_TABLE_MAX_SECTION
}E_LUA_SCRIPT_TABLE_SECTION;
/*-\NEW\zhuth\2014.2.17\通过文件记录表访问luadb中未压缩的文件*/



#define ASSERT_LEN(len, defLen)     ((len) == (defLen) ? 0 : LUADB_ERR_TLV_LEN)

#define DBMAKEU16(d, p)             ((u16)(d[p]|((d[p+1])<<8)))
#define DBMAKEU32(d, p)             ((u32)(DBMAKEU16(d,p)|(DBMAKEU16(d,p+2)<<16)))

typedef struct DbHeadInfoTag
{
    u16         verNum;
    u16         filecount;
    u32         length;
}DbHeadInfo;

typedef struct DbFileInfoTag
{
    char       *name;
    u8          nameLen;
    u32         length;
    const u8   *data;
    u32 offset;
}DbFileInfo;

typedef enum DbHeadTypeTag
{
    HeadInfoReserved0x00 = 0x00,
    HeadInfoMagicNumber,
    HeadInfoVersion,
    HeadInfoLength,
    HeadInfoFilecount,

    HeadInfoCRC = 0xFE,
    HeadInfoReserved0xFF = 0xFF
}DbHeadType;

typedef enum DbFileHeadTypeTag
{
    FileHeadReserved0x00 = 0x00,
    FileHeadMagicNumber = 0x01,
    FileHeadName,
    FileHeadLength,

    FileHeadCRC = 0xFE,
    FileHeadReserved0xFF = 0xFF
}DbFileHeadType;


char *fgets_ext(char *buf, int n, FILE *fp);
FILE *fopen_ext(const char *file, const char *mode);
int fclose_ext(FILE *fp);
int getc_ext(FILE *fp);
int ungetc_ext(int c, FILE *fp);
size_t fread_ext(void *buf, size_t size, size_t count, FILE *fp);
int fseek_ext(FILE *fp, long offset, int whence);
long ftell_ext(FILE *fp);
int feof_ext(FILE *fp);
#define EOF_EXT (-1)
typedef struct UNCOMPRESS_FILE_TABLE_ITEM_TAG
{
    char *pFile;
    s32 nOffset;
    s32 nLen;
    struct UNCOMPRESS_FILE_TABLE_ITEM_TAG *pNext;
}T_UNCOMPRESS_FILE_TABLE_ITEM;


/*+\NEW\zhuth\2014.2.17\通过文件记录表访问luadb中未压缩的文件*/
int parse_luadb_data(const u8 *pData, u32 length, BOOL override, E_LUA_SCRIPT_TABLE_SECTION section, BOOL *pRestart);
/*-\NEW\zhuth\2014.2.17\通过文件记录表访问luadb中未压缩的文件*/



#endif/*_PRELOAD_H_*/