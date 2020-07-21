
set(target lua_openat)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

target_include_directories(${target} PRIVATE ../../newlib/libc/include)
target_include_directories(${target} PRIVATE ../../platform/openat/include)
target_include_directories(${target} PRIVATE ../../lua/include)
target_include_directories(${target} PRIVATE ../../lua/src)
target_include_directories(${target} PRIVATE ../../lib/zlib/include)
target_include_directories(${target} PRIVATE ../../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../../newlib/include)
target_include_directories(${target} PRIVATE ../../platform/include)
target_include_directories(${target} PRIVATE ../../modules/include)
target_include_directories(${target} PRIVATE ../../../atr/include)
target_include_directories(${target} PRIVATE ../../../driver/include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel openat
#  lua_crypto  lua_pbc lua_iconv lua_json
#  lua_lzma lua_zlib lua_platform res shell lua_modules lua_newlib
#  lua_libc lua_lua 
#  )
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/include)
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/driver_inc)
# target_link_libraries(${target} PRIVATE ${PROJECT_SOURCE_DIR}/components/driverlib/libdriver.a)
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/atr_inc)
# target_link_libraries(${target} PRIVATE ${PROJECT_SOURCE_DIR}/components/atrlib/libatr.a)

target_link_libraries(${target} PRIVATE
 shell 
)

# target_link_libraries(${target} PRIVATE 
# shell lua_newlib lua_libc lua_lua

# )

set(openat_src
src/ctype.c                     
src/cycle_queue.c               
src/libc_time.c                 
src/platform.c                  
src/platform_assert.c           
src/platform_audio.c            
src/platform_AW9523B.c          
src/platform_factory.c          
src/platform_fs.c               
src/platform_gps.c              
src/platform_hrsensor.c         
src/platform_i2c.c              
src/platform_int.c              
src/platform_lcd.c              
src/platform_main.c             
src/platform_malloc.c           
src/platform_pmd.c              
src/platform_rtos.c             
src/platform_SLI3108.c          
src/platform_socket.c           
src/platform_stdio.c            
src/platform_sys.c              
src/platform_tp.c               
src/platform_ttsply.c           
src/platform_uart.c             
src/platform_watchdog.c         
src/platform_spi.c
src/platform_camera.c
)


target_sources(${target} PRIVATE
    ${openat_src}
)

