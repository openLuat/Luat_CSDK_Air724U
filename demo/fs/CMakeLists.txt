
## +zhuwangbin 2020-6-30 add sffs.img
execute_process(COMMAND cmd /c "sffs.bat ${out_hex_dir} ${CONFIG_FS_SYS_FLASH_OFFSET} ${CONFIG_FS_SYS_FLASH_SIZE}" ${CMAKE_SOURCE_DIR}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/demo/fs/SFFS
    OUTPUT_VARIABLE SFFS_VAR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("${SFFS_VAR}")
## -zhuwangbin 2020-6-30 add sffs.img

if(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${flash_ldscript} demo_fs.c)
else(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${file_ldscript} demo_fs.c)
endif()