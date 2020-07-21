
set(target lua_newlib)
add_library(${target} STATIC)

set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})

include_directories(libc/include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE include/machine)
target_include_directories(${target} PRIVATE include/sys)
target_include_directories(${target} PRIVATE src)
target_include_directories(${target} PRIVATE ../newlib/libc/include)
target_include_directories(${target} PRIVATE ../platform/include)
target_include_directories(${target} PRIVATE ../platform/openat/include)
target_include_directories(${target} PRIVATE ../lua/include)
target_include_directories(${target} PRIVATE ../lua/src)
target_include_directories(${target} PRIVATE ../lib/zlib/include)
target_include_directories(${target} PRIVATE ../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../modules/include)

# target_include_targets(${target} PRIVATE  openat )
# target_link_libraries(${target} PRIVATE  openat
# lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_modules lua_libc lua_platform res shell lua_openat
# )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc )

# target_link_libraries(${target} PRIVATE  
# lua_crypto lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_modules lua_libc lua_platform res shell lua_openat
# )

add_subdirectory_if_exist(libc)

target_link_libraries(${target} PRIVATE  
lua_libc lua_openat
)
set(newlib_src
src/devman.c  
src/genstd.c  
src/stubs.c   
)

target_sources(${target} PRIVATE
    ${newlib_src}
)


# target_link_libraries(${target} PRIVATE
#     kernel driver ml hal calclib atr cfw fs fsmount
#     aworker net lwip http mupnp mbedtls libcoap
#     nvm cis_sdk fupdate audio service tts
# )