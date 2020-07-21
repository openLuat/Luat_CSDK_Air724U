
set(target lua_platform)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PRIVATE include)
target_include_directories(${target} PRIVATE ../platform/openat/include)
target_include_directories(${target} PRIVATE ../lua/include)
target_include_directories(${target} PRIVATE ../lua/src)
target_include_directories(${target} PRIVATE ../lib/lpng/include)
target_include_directories(${target} PRIVATE ../lib/lpng/lpng_pal/include)
target_include_directories(${target} PRIVATE ../lib/lpng/src)
target_include_directories(${target} PRIVATE ../lib/qr_encode/include)
target_include_directories(${target} PRIVATE ../lib/qr_encode/src)
target_include_directories(${target} PRIVATE ../lib/zlib/include)
target_include_directories(${target} PRIVATE ../lib/lzma/include)
target_include_directories(${target} PRIVATE ../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../platform/include)
target_include_directories(${target} PRIVATE ../newlib/include)
target_include_directories(${target} PRIVATE ../newlib/libc/include)
target_include_directories(${target} PRIVATE ../modules/include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel openat
# lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_modules lua_newlib lua_libc  res shell lua_openat
# )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)

# target_link_libraries(${target} PRIVATE  
# lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_modules lua_newlib lua_libc  res shell lua_openat
# )
target_link_libraries(${target} PRIVATE  
lua_pbc lua_libc lua_newlib lua_lpng
)
set(platform_src
src/buf.c            
src/common.c         
src/common_disp.c    
src/common_tmr.c     
src/common_uart.c    
src/crc.c            
src/dlmalloc.c       
src/elua_int.c       
src/preload.c        
src/ut_md5.c         
	
)

add_subdirectory_if_exist(openat)

target_sources(${target} PRIVATE
    ${platform_src}
)