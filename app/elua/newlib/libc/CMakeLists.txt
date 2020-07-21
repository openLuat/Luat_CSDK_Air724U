
set(target lua_libc)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel openat 
# lua_crypto  lua_pbc lua_iconv lua_json
#  lua_lzma lua_zlib lua_platform res shell lua_modules lua_newlib
# lua_openat lua_lua ${libc_file_name} ${libm_file_name}
# )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)

# target_link_libraries(${target} PRIVATE   
# lua_crypto  lua_pbc lua_iconv lua_json
#  lua_lzma lua_zlib lua_platform res shell lua_modules lua_newlib
# lua_openat lua_lua ${libc_file_name} ${libm_file_name}
# )

target_link_libraries(${target} PRIVATE   
lua_crypto lua_platform res shell lua_modules lua_newlib
lua_openat ${libc_file_name} ${libm_file_name}
)
# target_link_libraries(${target} PRIVATE   
# lua_platform res shell lua_modules lua_newlib
# ${libc_file_name} ${libm_file_name}
# )
set(libc_src  
src/btea.c    
src/errno.c   
src/findfp.c  
src/locale.c  
src/malloc.c  
src/math.c    
src/signal.c  
src/stdio.c   
src/stdlib.c  
src/string.c  
src/time.c
)


target_sources(${target} PRIVATE
    ${libc_src}
)
