
set(target shell)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

target_include_directories(${target} PRIVATE ../platform/openat/include)
target_include_directories(${target} PRIVATE ../lua/include)
target_include_directories(${target} PRIVATE ../lua/src)
target_include_directories(${target} PRIVATE ../lib/zlib/include)
target_include_directories(${target} PRIVATE ../lib/lzma/include)
target_include_directories(${target} PRIVATE ../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../platform/include)
target_include_directories(${target} PRIVATE ../newlib/include)
target_include_directories(${target} PRIVATE ../newlib/libc/include)
target_include_directories(${target} PRIVATE ../modules/include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel openat lua_libc)
target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)
target_link_libraries(${target} PRIVATE 
lua_libc lua_newlib lua_openat lua_lua
lua_lzma
)

set(shell_src
	src/shell.c
	src/ini_parser.c
)


target_sources(${target} PRIVATE
    ${shell_src}
)
