

#/*+\NEW\chenzhimin\2020.07.21\ ssl*/
if(DEFINED ENV{BAT_BUILD_APP_SSL})
    if($ENV{BAT_BUILD_APP_SSL} STREQUAL "ON")
        set(CONFIG_BUILD_APP_SSL ON)
    else()
        set(CONFIG_BUILD_APP_SSL OFF)
    endif()
else()
    set(CONFIG_BUILD_APP_SSL OFF)
endif()
message("BUILD_SSL:" ${CONFIG_BUILD_APP_SSL})
#/*-\NEW\chenzhimin\2020.07.21\ ssl*/


#/*+\NEW\chenzhimin\2020.07.21\ ftp*/
if(DEFINED ENV{BAT_BUILD_APP_FTP})
    if($ENV{BAT_BUILD_APP_FTP} STREQUAL "ON")
        set(CONFIG_BUILD_APP_FTP ON)
    else()
        set(CONFIG_BUILD_APP_FTP OFF)
    endif()
else()
    set(CONFIG_BUILD_APP_FTP OFF)
endif()
message("BUILD_FTP:" ${CONFIG_BUILD_APP_FTP})
#/*-\NEW\chenzhimin\2020.07.21\ ftp*/


#/*+\NEW\chenzhimin\2020.07.21\ http*/
if(DEFINED ENV{BAT_BUILD_APP_HTTP})
    if($ENV{BAT_BUILD_APP_HTTP} STREQUAL "ON")
        set(CONFIG_BUILD_APP_HTTP ON)
    else()
        set(CONFIG_BUILD_APP_HTTP OFF)
    endif()
else()
    set(CONFIG_BUILD_APP_HTTP OFF)
endif()
message("BUILD_HTTP:" ${CONFIG_BUILD_APP_HTTP})
#/*-\NEW\chenzhimin\2020.07.21\ http*/


#/*+\NEW\chenzhimin\2020.07.21\ littlevgl*/
if(DEFINED ENV{BAT_BUILD_APP_LITTLEVGL})
    if($ENV{BAT_BUILD_APP_LITTLEVGL} STREQUAL "ON")
        set(CONFIG_BUILD_APP_LITTLEVGL ON)
    else()
        set(CONFIG_BUILD_APP_LITTLEVGL OFF)
    endif()
else()
    set(CONFIG_BUILD_APP_LITTLEVGL OFF)
endif()
message("BUILD_LITTLEVGL:" ${CONFIG_BUILD_APP_LITTLEVGL})
#/*-\NEW\chenzhimin\2020.07.21\ littlevgl*/


#/*+\NEW\chenzhimin\2020.07.21\ mqtt*/
if(DEFINED ENV{BAT_BUILD_APP_MQTT})
    if($ENV{BAT_BUILD_APP_MQTT} STREQUAL "ON")
        set(CONFIG_BUILD_APP_MQTT ON)
    else()
        set(CONFIG_BUILD_APP_MQTT OFF)
    endif()
else()
    set(CONFIG_BUILD_APP_MQTT OFF)
endif()
message("BUILD_MQTT:" ${CONFIG_BUILD_APP_MQTT})
#/*-\NEW\chenzhimin\2020.07.21\ mqtt*/


#/*+\NEW\chenzhimin\2020.07.21\ elua工程专用*/
#DEFINED BAT_BUILD_LUA
if(DEFINED ENV{BAT_BUILD_LUA})
    if($ENV{BAT_BUILD_LUA} STREQUAL "ON")
        set(CONFIG_BUILD_LUA ON)
        # 下面的没事最好不要动
        set(CONFIG_LUA_FLASH_OFFSET 0x2D8000)
        set(CONFIG_LUA_FLASH_SIZE 0x68000)
    else()
        set(CONFIG_BUILD_LUA OFF)
    endif()
else()
    set(CONFIG_BUILD_LUA OFF)
endif()
message("BUILD_LUA:" ${CONFIG_BUILD_LUA})
#/*-\NEW\chenzhimin\2020.07.21\ elua工程专用*/

