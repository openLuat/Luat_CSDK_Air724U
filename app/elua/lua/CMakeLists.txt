
set(target lua_lua)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE src)
target_include_directories(${target} PRIVATE ../newlib/include)
target_include_directories(${target} PRIVATE ../newlib/libc/include)
target_include_directories(${target} PRIVATE ../platform/openat/include)
target_include_directories(${target} PRIVATE ../platform/include)
target_include_directories(${target} PRIVATE ../modules/include)

# target_include_targets(${target} PRIVATE kernel openat )
# target_link_libraries(${target} PRIVATE kernel openat 
# lua_openat lua_libc lua_platform lua_newlib
# )

target_include_directories(${target} PRIVATE  ${PROJECT_SOURCE_DIR}/components/openat_inc)

# target_link_libraries(${target} PRIVATE   
# lua_openat lua_libc lua_platform lua_newlib
# )
target_link_libraries(${target} PRIVATE   
lua_newlib
)

set(lua_src
src/lapi.c         
src/lauxlib.c      
src/lbaselib.c     
src/lcode.c        
src/ldblib.c       
src/ldebug.c       
src/ldo.c          
src/ldump.c        
src/lfunc.c        
src/lgc.c          
src/linit.c        
src/liolib.c       
src/llex.c         
src/lmathlib.c     
src/lmem.c         
src/loadlib.c      
src/lobject.c      
src/lopcodes.c     
src/loslib.c       
src/lparser.c      
src/lstate.c       
src/lstring.c      
src/lstrlib.c      
src/ltable.c       
src/ltablib.c      
src/ltm.c          
src/lua.c          
src/luac.c         
src/lundump.c      
src/lvm.c          
src/lzio.c         
src/print.c

)


target_sources(${target} PRIVATE
    ${lua_src}
)