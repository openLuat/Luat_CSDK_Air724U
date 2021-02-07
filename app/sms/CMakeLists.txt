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



#configure_file(include/fota_config.h.in ${BINARY_TOP_DIR}/include/fota_config.h)



#设置包含头文�?
target_include_directories(${target} PUBLIC ../../api/include)
target_include_directories(${target} PUBLIC ../components/ril/inc)
target_include_directories(${target} PUBLIC inc)
target_link_libraries(${target} PUBLIC ril)

target_sources(${target} PUBLIC
	src/sms.c
)

