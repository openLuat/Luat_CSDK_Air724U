# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

set(target app)
add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_hex_dir})
target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})

#/*+\ADD\chenzhimin\2020.07.9\ */
# 以下功能在config.cmake中配置是否启用
if(CONFIG_BUILD_APP_SSL)
    add_subdirectory_if_exist(ssl)
endif()

if(CONFIG_BUILD_APP_MBEDTLS)
	add_subdirectory_if_exist(mbedtls)
endif()

if(CONFIG_BUILD_APP_FTP)
    add_subdirectory_if_exist(ftp)
endif()

if(CONFIG_BUILD_APP_HTTP)
    add_subdirectory_if_exist(http)
endif()

if(CONFIG_BUILD_APP_MQTT)
    add_subdirectory_if_exist(mqtt)
endif()

if(CONFIG_BUILD_APP_LITTLEVGL)
    add_subdirectory_if_exist(littlevgl)
endif()

if(CONFIG_BUILD_APP_OLED_SSD1306)
    add_subdirectory_if_exist(oled_ssd1306)
endif()

if(CONFIG_BUILD_APP_OneWire)
    add_subdirectory_if_exist(OneWire)
endif()

if(CONFIG_BUILD_APP_AT_ENGINE)
    add_subdirectory_if_exist(at_engine)
endif()

if(CONFIG_BUILD_APP_SMS)
    add_subdirectory_if_exist(sms)
endif()

if(CONFIG_BUILD_APP_RTMP)
	add_subdirectory_if_exist(librtmp)
endif()
#/*-\ADD\chenzhimin\2020.07.9\ */