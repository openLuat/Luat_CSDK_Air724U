
#/*+\NEW\chenzhimin\2020.07.21\ elua工程专用*/
if(CONFIG_BUILD_LUA)
    set(CONFIG_LUA_FLASH_SIZE 0x68000)
    #set(CONFIG_LUA_FLASH_OFFSET ${CONFIG_FS_SYS_FLASH_ADDRESS}-${CONFIG_BOOT_FLASH_ADDRESS}-${CONFIG_LUA_FLASH_SIZE})
    math(EXPR CONFIG_LUA_FLASH_OFFSET "${CONFIG_FS_SYS_FLASH_ADDRESS}-${CONFIG_BOOT_FLASH_ADDRESS}-${CONFIG_LUA_FLASH_SIZE}" OUTPUT_FORMAT HEXADECIMAL)
    
else()
    set(CONFIG_BUILD_LUA OFF)
endif(CONFIG_BUILD_LUA)

message("BUILD_LUA:" ${CONFIG_BUILD_LUA})
#/*-\NEW\chenzhimin\2020.07.21\ elua工程专用*/

if(
CONFIG_BUILD_APP_SSL OR 
CONFIG_BUILD_APP_FTP OR
CONFIG_BUILD_APP_HTTP OR
CONFIG_BUILD_APP_LITTLEVGL OR
CONFIG_BUILD_APP_MQTT OR
CONFIG_BUILD_APP_OLED_SSD1306 OR
CONFIG_BUILD_APP_OneWire OR
CONFIG_BUILD_APP_AT_ENGINE OR
CONFIG_BUILD_APP_SMS OR
CONFIG_BUILD_APP_MBEDTLS OR
CONFIG_BUILD_APP_RTMP

)
    set(CONFIG_BUILD_APP_EN ON)
endif()
