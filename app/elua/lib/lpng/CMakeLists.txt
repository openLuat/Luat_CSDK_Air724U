
set(target lua_lpng)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)

target_include_directories(${target} PRIVATE ../../newlib/libc/include)
target_include_directories(${target} PRIVATE ../../platform/openat/include)
target_include_directories(${target} PRIVATE ../../lua/include)
target_include_directories(${target} PRIVATE ../../lua/src)
target_include_directories(${target} PRIVATE ../../lib/lpng/include)
target_include_directories(${target} PRIVATE ../../lib/lpng/lpng_pal/include)
target_include_directories(${target} PRIVATE ../../lib/zlib/include)
target_include_directories(${target} PRIVATE ../../lib/zlib/zlib_pal/include)
target_include_directories(${target} PRIVATE ../../platform/include)
target_include_directories(${target} PRIVATE ../../modules/include)

# target_include_targets(${target} PRIVATE kernel openat)
# target_link_libraries(${target} PRIVATE kernel )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)

set(lpng_src
	src/png.c     
	src/pngerror.c    
	src/pngget.c       
	src/pngmem.c     
	src/pngpread.c     
	src/pngread.c       
	src/pngrio.c      
	src/pngrtran.c     
	src/pngrutil.c     
	src/pngset.c     
	src/pngtrans.c     
	src/pngwio.c    
	src/pngwrite.c       
	src/pngwtran.c     
	src/pngwutil.c
)


target_sources(${target} PRIVATE
    ${lpng_src}
)
