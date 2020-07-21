
set(target lua_json)
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
target_include_directories(${target} PRIVATE ../../platform/include)
target_include_directories(${target} PRIVATE ../../modules/include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)
target_link_libraries(${target} PRIVATE  
lua_libc
)
set(json_src
	src/fpconv.c 
	src/lua_cjson.c 
	src/strbuf.c
)


target_sources(${target} PRIVATE
    ${json_src}
)