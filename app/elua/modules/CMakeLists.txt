
set(target lua_modules)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

target_include_directories(${target} PRIVATE ../lua/include)
target_include_directories(${target} PRIVATE ../lua/src)
target_include_directories(${target} PRIVATE ../lib/iconv/include)
target_include_directories(${target} PRIVATE ../lib/zlib/include)
target_include_directories(${target} PRIVATE ../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../lib/qr_encode/include)
target_include_directories(${target} PRIVATE ../newlib/include)
target_include_directories(${target} PRIVATE ../newlib/libc/include)
target_include_directories(${target} PRIVATE ../platform/openat/include)
target_include_directories(${target} PRIVATE ../platform/include)
target_include_directories(${target} PRIVATE ../modules/include)

target_include_directories(${target} PRIVATE ../lib/OneWire)
target_include_directories(${target} PRIVATE ../lib/OneWire/ds18b20)
target_include_directories(${target} PRIVATE ../lib/OneWire/dht11)
# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel openat 
# lua_openat lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_newlib lua_libc lua_platform res shell 
# )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)
# target_link_libraries(${target} PRIVATE  
# lua_openat lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_newlib lua_libc lua_platform res shell 
# )

target_link_libraries(${target} PRIVATE  
lua_openat lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
lua_newlib lua_libc lua_platform lua_qr_encode lua_OneWire
)

set(moudles_src
src/adc.c            
src/audio.c          
src/bit.c            
src/bitarray.c       
src/cpu.c            
src/disp.c           
src/factory.c        
src/gpio_i2c.c       
src/gps.c            
src/hrsensor.c       
src/i2c.c            
src/l_uart.c         
src/lpack.c          
src/luaiconv.c       
src/lzlib.c          
src/pio.c            
src/pmd.c            
src/pwm.c            
src/qr_encode.c      
src/rtos.c           
src/tcpipsock.c      
src/ttsply.c         
src/watchdog.c       
src/spi.c
src/wifi.c
src/luaOneWire.c
)


target_sources(${target} PRIVATE
    ${moudles_src}
)
