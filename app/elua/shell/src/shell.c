
#include "lua.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"
#include "devman.h"
#include "lplatform.h"
#include "platform_conf.h"
#include "platform_fs.h"
#include "platform_rtos.h"
#include "platform_factory.h"

/*+\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
#if defined(AM_LZMA_SUPPORT)
#include "lzmalib.h"
#endif
/*-\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/

#include "preload.h"

/*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
/*+\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/
#define LUA_ENTRY_FILENAME "main.lua"
#define LUA_ENTRY_FILE "/lua/" LUA_ENTRY_FILENAME
/*-\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/

#define LUAC_ENTRY_FILENAME "main.luac"
#define LUAC_ENTRY_FILE "/lua/" LUAC_ENTRY_FILENAME
#define LUAE_ENTRY_FILENAME "main.luae"
#define LUAE_ENTRY_FILE "/lua/" LUAE_ENTRY_FILENAME

/*+\NEW\liulean\2015.8.5\解决产线概率性MP3播放无声音的问题*/
#define LUA_CHECK_INTEGRITY_FILE "/integrity.bin"
#define LUA_INTEGRITY_FLAG 0xABCD8765
/*-\NEW\liulean\2015.8.5\解决产线概率性MP3播放无声音的问题*/

/*+\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
#define LUA_ENTRY_FILE_ZIP "/luazip/" LUA_ENTRY_FILENAME ".zip"
/*-\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
#define LUAC_ENTRY_FILE_ZIP "/luazip/" LUAC_ENTRY_FILENAME ".zip"
#define LUAE_ENTRY_FILE_ZIP "/luazip/" LUAE_ENTRY_FILENAME ".zip"
/*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */

#define LUA_CFG_FILENAME "amcfg.ini"
#define LUA_CFG_FILE "/lua/"LUA_CFG_FILENAME



/*+\LIULEAN\2015.1.26\为LUA脚本分配一块固定的FLASH区域*/
/*CUSTOMER_FILE_OFFSET为CUSTOMER_GFH的长度*/
char* _lua_script_section_start = NULL;//[LUA_SCRIPT_SIZE -CUSTOMER_FILE_OFFSET ];
/*-\LIULEAN\2015.1.26\为LUA脚本分配一块固定的FLASH区域*/



/*+\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
void LuaDeleteMainFile(void)
{
    remove(LUA_ENTRY_FILE);
    remove(LUAC_ENTRY_FILENAME);
    remove(LUAE_ENTRY_FILENAME);
}
/*-\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/

/*+\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */
int file_exist(const char *name)
{
    FILE *fp;
    if((fp = fopen(name, "rb")) == NULL)
        return FALSE;

    fclose(fp);
    return TRUE;
}

/*-\NEW\liweiqiang\2013.10.25\lua脚本统一放在lua目录下,预置的非lua文件统一放在ldata文件下 */

static void load_luadbToBuf(char* buf, UINT32 srcAddr, UINT32 size)
{
  int offset = 0, len = 0;
  E_AMOPENAT_MEMD_ERR err;

    {
      err = OPENAT_flash_read(srcAddr + offset, size, NULL, buf);
      
      if(OPENAT_MEMD_ERR_NO != err)
      {
        IVTBL(print)("[lua] load luadb script read flash error\n");
        ASSERT(0);
      }
    }
}

static int load_updatebin(FILE* updateFp, UINT32 dest, UINT32 maxLen)
{
#define LOAD_FILE_BUF pageSize
  UINT32 offset = 0;
  char* buf;
  int result;
  UINT32 pageSize = OPENAT_flash_page();

  buf = malloc(LOAD_FILE_BUF);

  if(!buf)
  {
    return LUADB_ERR_MALLOC_FAIL;
  }
  do{
    result = fread(buf, 1, LOAD_FILE_BUF, updateFp);
    if(result <= 0)
    {
      break;
    }
    OPENAT_print("lua read len = %d\n", result);
    if(OPENAT_MEMD_ERR_NO != OPENAT_flash_erase(dest + offset, LOAD_FILE_BUF + dest + offset))
    {
      result = LUADB_ERR_EARSE_CUSTOMER;
      break;
    }
    if(OPENAT_MEMD_ERR_NO != OPENAT_flash_write(dest + offset, result, NULL, buf))
    {
      result = LUADB_ERR_WRITE_CUSTOMER;
      break;
    }
    offset += result;

    printf("[lua] load_updatebin write bin %d...\n", offset);

    OPENAT_print("load_updatebin write bin %u...\n", offset);
    
  }while(result != LUADB_ERR_NONE);

  free(buf);

  return result;
  
}



/*+\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */
static int load_luadb(void)
{
#define LUA_UPDATE_FILE "/luazip/update.bin"

    FILE *fp = NULL;
        
    u8 *buff;
    BOOL update = FALSE;
    BOOL restart1 = FALSE;
    BOOL restart2 = FALSE;
    int result = 0;

    DbHeadInfo headInfo;
    DbFileInfo *pFileInfo = NULL;
    u32 offset = 0;
    u16 fileIndex;
    u8* filebuf;

        
    /* 远程升级文件是否存在 */
    if((fp = fopen(LUA_UPDATE_FILE, "rb")) != NULL)
    {
      OPENAT_print("lua load update bin...\n", result);
      result = load_updatebin(fp, OPENAT_turn_addr(LUA_SCRIPT_ADDR),  LUA_SCRIPT_SIZE);
      if(result != LUADB_ERR_NONE)
      {
        OPENAT_print("lua write update bin error %d\n", result);
        restart1 = TRUE;
        fclose(fp);
        goto end;
      }
      fclose(fp);
      remove(LUA_UPDATE_FILE);
      load_luadbToBuf(_lua_script_section_start, OPENAT_turn_addr(LUA_SCRIPT_ADDR) + CUSTOMER_FILE_OFFSET, LUA_SCRIPT_SIZE);
      
    }    
    update = FALSE;  
    OPENAT_print("lua load luabin...\n");
    result = parse_luadb_data(_lua_script_section_start, LUA_SCRIPT_SIZE, update, LUA_SCRIPT_TABLE_FLASH_SECTION, &restart2);

    OPENAT_print("lua load luabin reuslt = %d\n", result);
end:
    /*+\NEW\zhuth\2014.8.14\开机如果成功执行了所有的写文件动作，则重启*/
    if(restart1 || restart2)
    {
        platform_rtos_restart();
    }
    /*-\NEW\zhuth\2014.8.14\开机如果成功执行了所有的写文件动作，则重启*/
    
    return result;
}
/*-\NEW\liweiqiang\2013.11.28\luadb方式远程升级支持 */

#ifdef MUXUSE_DLMALLOC_MEMORY_AS_LUA_SCRIPT_LOAD
BOOL bScriptLoaded;
#endif

int LuaAppTask(void)
{    
/*+\NEW\2013.7.11\liweiqiang\增加luadb预置文件处理*/
    int argc;
    char **argv;
    BOOL existScript = TRUE;
    BOOL existLuaDB = FALSE;
    BOOL existRes = FALSE;
    int dbret;
    UINT32 offset = 0;
    E_AMOPENAT_MEMD_ERR err = OPENAT_MEMD_ERR_NO;
    char trace_port = 0; //默认UART1

    u32 len;
    char* _lua_script = NULL;

    static const char *argv_null[] = {"lua", NULL};
#if 0    
    static const char *argv_script_const[] =
    {
        "lua",
        "-e",
        _lua_script_section_start,
        NULL
    };
#endif    
/*+\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/
    static const char *argv_script_file[] =
    {
        "lua",
        LUA_ENTRY_FILE,
        NULL
    };

    static const char *argv_luac_script_file[] =
    {
        "lua",
        LUAC_ENTRY_FILE,
        NULL
    };

    static const char *argv_luae_script_file[] =
    {
        "lua",
        LUAE_ENTRY_FILE,
        NULL
    };

    OPENAT_print("%s check stack %p, %p", __FUNCTION__, &offset, &len);

#ifdef MUXUSE_DLMALLOC_MEMORY_AS_LUA_SCRIPT_LOAD
    {
        extern char malloc_buf[800*1024];
         _lua_script_section_start = malloc_buf;
    }
#else
    _lua_script_section_start = L_MALLOC(LUA_SCRIPT_SIZE);
#endif

   

    load_luadbToBuf(_lua_script_section_start, OPENAT_turn_addr(LUA_SCRIPT_ADDR) + CUSTOMER_FILE_OFFSET, LUA_SCRIPT_SIZE);
    
    _lua_script = _lua_script_section_start;


/*-\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/
    OPENAT_print("LuaAppTask enter 1111");


	/*+\liulean\2015.1.29\解决LUA脚本打印语句无效的问题*/
	/************************************************************************************
	 这里不能用_lua_script_section_start,否则会导致编译器优化，使else语句中的代码永远不执行，
	 也就无法设置debug口，LUA的print函数就会打印不出任何东西
	 ************************************************************************************/
    
    if(_lua_script[0] == 0xff || _lua_script[0] == '\0')  
	/*-\liulean\2015.1.29\解决LUA脚本打印语句无效的问题*/
	{
        argc = sizeof(argv_null)/sizeof(argv_null[0]);
        argv = (char **)argv_null;

        existScript = FALSE;
    }
    else
    {
        //存在预置脚本时使用debug口作为命令行输出
        platform_set_console_port(PLATFORM_PORT_ID_DEBUG);
    }


    //初始化设备 stdio\fs\...
    if(platform_init() != PLATFORM_OK)
    {
        ASSERT(0);
    }

    lua_dm_init();

    // 注册平台文件系统接口
    dm_register(platform_fs_init());
   
    dbret = load_luadb();

    if(dbret != LUADB_ERR_NOT_DBDATA)
    {
        existLuaDB = TRUE;
    }

/*+\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/
    if(existScript || existLuaDB)
    {
        BOOL exitZipFile = FALSE;
        char* zipFileName;
        char* enterFile;
    /*+\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/
    #if defined(AM_LZMA_SUPPORT)
        // 保留旧的文件升级方式,以兼容旧版本
        if(file_exist(LUA_ENTRY_FILE_ZIP) == TRUE)
        {
            exitZipFile = TRUE;
            zipFileName = LUA_ENTRY_FILE_ZIP;
            enterFile = LUA_ENTRY_FILE;
        }
        else if(file_exist(LUAC_ENTRY_FILE_ZIP) == TRUE)
        {
            exitZipFile = TRUE;
            zipFileName = LUAC_ENTRY_FILE_ZIP;
            enterFile = LUAC_ENTRY_FILE;
        }
        else if(file_exist(LUAE_ENTRY_FILE_ZIP) == TRUE)
        {
            exitZipFile = TRUE;
            zipFileName = LUAE_ENTRY_FILE_ZIP;
            enterFile = LUAE_ENTRY_FILE;
        }
        if(exitZipFile)
        {
            // 只有在存在升级包文件的情况下才处理解压
            int lzmaret = 0;    
            if((lzmaret = LzmaUncompressFile(zipFileName, enterFile)) == 0)
            {
                /*+\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
                // 解压缩成功,删除压缩文件
                /*+\NEW\zhuth\2014.8.11\升级包解压缩成功后，删除升级包，并且重启*/
                remove(zipFileName);
                /*-\NEW\zhuth\2014.8.11\升级包解压缩成功后，删除升级包，并且重启*/
                /*-\NEW\rufei\2013.9.13\处理lua文件可能被破坏导致持续重启问题*/
                OPENAT_print("uncompress zip file success!\n", lzmaret);
                /*+\NEW\zhuth\2014.8.11\升级包解压缩成功后，删除升级包，并且重启*/
                platform_rtos_restart();
                /*-\NEW\zhuth\2014.8.11\升级包解压缩成功后，删除升级包，并且重启*/
            }
            else
            {
                printf("uncompress file error(%d) %s!\n", lzmaret, zipFileName);
            }
        }
    #endif   
    /*-\NEW\liweiqiang\2013.5.11\开机自解压luazip目录下文件支持,压缩算法lzma*/

        if(file_exist(LUA_CFG_FILE) == TRUE)
        {
         	OPENAT_print("[lua]: amcfg.ini exist ");
        	trace_port = (char)GetIniKeyInt("trace","port",LUA_CFG_FILE);
        	OPENAT_print("[lua]: trace port = %d", trace_port);
        	
        	if(trace_port > 2)
        	{
        	  trace_port = 0;
        	}

			/*+\NEW\shenyuanyuan\2020.4.16\修改工具设置的USB TRACE口和内部URAT3口冲突问题*/
			if(trace_port == 2)
			{
				trace_port = 3;	
			}
			/*-\NEW\shenyuanyuan\2020.4.16\修改工具设置的USB TRACE口和内部URAT3口冲突问题*/
        }
        platform_rtos_set_trace_port(trace_port,0);


        if(file_exist(LUA_ENTRY_FILE) == TRUE)
        {
            argc = sizeof(argv_script_file)/sizeof(argv_script_file[0]);
            argv = (char **)argv_script_file;
            OPENAT_print("[lua]: main.lua exist ");
        }
        else if(file_exist(LUAC_ENTRY_FILE) == TRUE)
        {
            argc = sizeof(argv_luac_script_file)/sizeof(argv_luac_script_file[0]);
            argv = (char **)argv_luac_script_file;
            OPENAT_print("[lua]: main.luac exist ");
        }
        else if(file_exist(LUAE_ENTRY_FILE) == TRUE)
        {
#define platform_dump_key(k) OPENAT_print("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", k[0], k[1], k[2], k[3], k[4], k[5])
            argc = sizeof(argv_luae_script_file)/sizeof(argv_luae_script_file[0]);
            argv = (char **)argv_luae_script_file;
            OPENAT_print("[lua]: main.luae exist ");
			int fd = vfs_open("/luat/keybin", 2/* O_RDWR */);
			ASSERT(fd != -1);
			UINT32 keyData[6];
			UINT32 backup[6];
			ssize_t readed = vfs_read(fd, keyData, sizeof(keyData));
			platform_dump_key(keyData);
			ASSERT(readed == sizeof(keyData));
			memcpy(backup, keyData, sizeof(backup));
			ASSERT(platform_set_key(keyData));
			platform_dump_key(keyData);
			if (backup[5] == 0xbabeface)
			{
				backup[5] = platform_get_uid();
				vfs_lseek(fd, 0, 0/* SEEK_SET */);
				size_t writed = vfs_write(fd, backup, sizeof(backup));
				ASSERT(sizeof(backup) == writed);
			}
			else
			{
				ASSERT(backup[5] == platform_get_uid());
			}
			vfs_close(fd);
        }      
        else if(existLuaDB)
        {
            // 若从预置数据无法解析出文件则无法从预置脚本运行
            OPENAT_print("[lua]: luadb parse ret %d\n", dbret);
            argc = sizeof(argv_null)/sizeof(argv_null[0]);
            argv = (char **)argv_null;
        }
        else
        {
            ASSERT(0);
#if 0            
            OPENAT_print("[lua]: main.lua not exist, excute from const script.\n");
            argc = sizeof(argv_script_const)/sizeof(argv_script_const[0]);
            argv = (char **)argv_script_const;
#endif            
        }
    }
/*-\NEW\liweqiang\2013.5.8\在文件系统存在main.lua文件时启动时自动加载文件系统文件*/
/*-\NEW\2013.7.11\liweiqiang\增加luadb预置文件处理*/
     printf("LuaAppTask enter 777777 :%d\n",OPENAT_get_system_tick());
     OPENAT_print("lua run lua script...\n");
     
     
 #if 0
    {
        INT32 fd ;
        INT32 result;
        UINT8 fileTest[10];
         result =  IVTBL(make_dir)("/file_test_dir", 0);

         printf("make dir %d\n", result);

         fd = IVTBL(open_file)("/file_test_dir/file1.txt", FS_O_RDONLY, 0);
         
         printf("FILE EXIST %d\n", fd);

         if(fd >= 0 )
         {
            IVTBL(close_file)(fd);
         }
         
         fd =  IVTBL(create_file)("/file_test_dir/file1.txt", 0);
         printf("create_file %d\n", fd);
        // fd = IVTBL(open_file)("/file_test_dir/file1.txt", FS_O_TRUNC, 0);
        IVTBL(write_file)(fd, "hello!", 6);
        IVTBL(close_file)(fd);
        
         fd = IVTBL(open_file)("/file_test_dir/file1.txt", FS_O_RDONLY, 0);
         printf("open_file %d\n", fd);
         
         IVTBL(read_file)(fd, fileTest, 6);
         if(memcmp(fileTest, "hello!", 6))
         {
            OPENAT_print("vfs test fail!");
         }
         else
         {
            OPENAT_print("vfs test success!");

         }
    }
#endif

#ifndef MUXUSE_DLMALLOC_MEMORY_AS_LUA_SCRIPT_LOAD    
    L_FREE(_lua_script_section_start);
#else
    bScriptLoaded = TRUE;	
#endif
     _lua_script_section_start = NULL;

     

    return lua_main(argc, argv);



    
}

