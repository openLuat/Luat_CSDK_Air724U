if(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${flash_ldscript} coreTest.c)
else(CONFIG_APPIMG_LOAD_FLASH)
add_appimg(${target} ${file_ldscript} coreTest.c)
endif()

target_include_directories(${target} PRIVATE ../components/ril/inc)
target_link_libraries(${target} PRIVATE ril)

aux_source_directory(Testlib DIR_SRCS)
target_sources(${target} PUBLIC ${DIR_SRCS})

