
set(target lua_crypto)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

target_include_directories(${target} PRIVATE ../../newlib/include)
target_include_directories(${target} PRIVATE ../../newlib/libc/include)
target_include_directories(${target} PRIVATE ../../platform/include)
target_include_directories(${target} PRIVATE ../../platform/openat/include)
target_include_directories(${target} PRIVATE ../../lua/include)
target_include_directories(${target} PRIVATE ../../lua/src)
target_include_directories(${target} PRIVATE ../../lib/lzma/include)
target_include_directories(${target} PRIVATE ../../lib/zlib/include)
target_include_directories(${target} PRIVATE ../../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../../modules/include)
target_include_directories(${target} PRIVATE ../../../net/mbedtls/include/mbedtls)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel )
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/net_inc)

# target_link_libraries(${target} PRIVATE  
# lua_openat  lua_iconv lua_json lua_lzma lua_pbc lua_zlib lua_lua 
# lua_newlib lua_libc lua_platform res shell 
# )
target_link_libraries(${target} PRIVATE  
lua_platform
)
#target_link_libraries(${target} PRIVATE ${PROJECT_SOURCE_DIR}/components/lib/alllib/libkernel.a)

set(crypto_src
src/crypto.c   
src/aliyun_iot_common_base64.c  
src/I_aes.c  
src/xxtea.c
src/md5_hmac.c
src/sha1_hmac.c
src/sha2.c

)


target_sources(${target} PRIVATE
    ${crypto_src}
)

