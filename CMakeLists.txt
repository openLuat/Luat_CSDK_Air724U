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

cmake_minimum_required(VERSION 3.13)

set(SOURCE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_TOP_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)

if(NOT BUILD_TARGET)
	set(BUILD_TARGET $ENV{BUILD_TARGET})
	#/*+\NEW\zhuwangbin\2020.07.1\add fotaNew */
	add_definitions(-DCSDK_PRO="${BUILD_TARGET}")
	#/*-\NEW\zhuwangbin\2020.07.1\add fotaNew */
	#/*+\NEW\zhuwangbin\2020.07.1\add dofta.bin */
	message("BUILD_TARGET: ${BUILD_TARGET}")
	#/*-\NEW\zhuwangbin\2020.07.1\add dofta.bin */
endif()

#/*+\NEW\zhuwangbin\2020.07.1\add dofta.bin */

if(NOT CSDK_VER)
	set(CSDK_VER $ENV{CSDK_VER})
	#/*+\NEW\zhuwangbin\2020.07.1\add fotaNew */
	add_definitions(-DCSDK_VER="${CSDK_VER}")
	#/*-\NEW\zhuwangbin\2020.07.1\add fotaNew */
	message("CSDK_VER: ${CSDK_VER} ")
endif()

if(DEFINED ENV{FOTA_FLAG})
	set(FOTA_FLAG $ENV{FOTA_FLAG})
	set(FOTA_TYPE $ENV{FOTA_TYPE})
	
	if (NOT (${FOTA_FLAG} STREQUAL  "fota"))
		message(FATAL_ERROR "FOTA_FLAG: ${FOTA_FLAG} != fota")
	endif()
	
	if (NOT ((${FOTA_TYPE} STREQUAL  "app") OR (${FOTA_TYPE} STREQUAL  "core") OR (${FOTA_TYPE} STREQUAL "all")))
		message(FATAL_ERROR "FOTA_TYPE: ${FOTA_TYPE} != app or core or all")
	endif()
	
	message("${FOTA_FLAG}=${FOTA_TYPE}")
endif()
#/*-\NEW\zhuwangbin\2020.07.1\add dofta.bin */

if (${BUILD_TARGET} STREQUAL "tts")
set(TTS_ENABLE ON)
message("TTS_ENABLE SET  ON")
else()
set(TTS_ENABLE OFF)
message("TTS_ENABLE SET OFF")
endif()

set(out_hex_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_rel_dir ${CMAKE_CURRENT_BINARY_DIR}/rel)
set(out_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
set(tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/tools)

if(TTS_ENABLE)
add_definitions(-DTTS_ENABLE)
set(core_stub_o ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/core_stub_tts.o)
else()
set(core_stub_o ${CMAKE_CURRENT_SOURCE_DIR}/components/lib/core_stub.o)
endif()

set(flash_ldscript ${SOURCE_TOP_DIR}/ldscripts/app_flashimg.ld)
set(file_ldscript ${SOURCE_TOP_DIR}/ldscripts/app_fileimg.ld)
set(cmd_mkappimg dtools mkappimg)
set(pacgen_py ${tools_dir}/pacgen.py)
set(pacgen_core_py ${tools_dir}/combine_core.py)

if(TTS_ENABLE)
set(core_name CSDK_V757_RDA8910_TTS)
else()
set(core_name CSDK_V757_RDA8910)
endif()

if(TTS_ENABLE)
include(cmake/core_config_tts.cmake)
else()
include(cmake/core_config.cmake)
endif()
include(cmake/toolchain-gcc.cmake)
set(BUILD_SHARED_LIBS OFF)

project(core C CXX ASM)

include_directories(components/include components/newlib/include components/openat_inc)

function(cpp_only target file)
    add_library(${target} OBJECT ${file})
    set_source_files_properties(${file} PROPERTIES LANGUAGE C)
    target_compile_options(${target} PRIVATE -E -P -x c)
endfunction()

function(add_appimg target ldscript)
    set(gen_ldscript ${target}_ldscript)
    set(target_map_file ${out_hex_dir}/${target}.map)
    set(target_img_file ${out_hex_dir}/${target}.img)
    cpp_only(${gen_ldscript} ${ldscript})
    add_executable(${target} ${ARGN} ${core_stub_o})
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${out_hex_dir})
    target_link_libraries(${target} PRIVATE -T $<TARGET_OBJECTS:${gen_ldscript}>)
    target_link_libraries(${target} PRIVATE -Wl,-Map=${target_map_file} -nostdlib -Wl,--gc-sections)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${cmd_mkappimg} $<TARGET_FILE:${target}> ${target_img_file}
        BYPRODUCTS ${target_img_file} ${target_map_file}
    )
endfunction()

configure_file(${SOURCE_TOP_DIR}/components/lib/fdl1.img ${out_hex_dir}/fdl1.img COPYONLY)
configure_file(${SOURCE_TOP_DIR}/components/lib/fdl2.img ${out_hex_dir}/fdl2.img COPYONLY)
macro(pac_init_fdl cmd pac_config)
    set(${cmd}
        cfg-init --pname UIX8910_MODEM --palias "APPIMG"
            --pversion "8910 MODULE" --version "BP_R1.0.0"
            --flashtype 1 ${pac_config}
        cfg-host-fdl -a ${CONFIG_FDL1_IMAGE_START} -s ${CONFIG_FDL1_IMAGE_SIZE}
            -p ${out_hex_dir}/fdl1.img ${pac_config}
        cfg-fdl2 -a ${CONFIG_FDL2_IMAGE_START} -s ${CONFIG_FDL2_IMAGE_SIZE}
            -p ${out_hex_dir}/fdl2.img ${pac_config}
    )
endmacro()

if(CONFIG_APPIMG_LOAD_FLASH)
    set(target appimg_flash_delete)
    set(pac_config ${out_hex_dir}/${target}.json)
    set(pac_file ${out_hex_dir}/${target}.pac)
    pac_init_fdl(init_fdl ${pac_config})
    execute_process(
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-erase-flash -i ERASE_APPIMG -a ${CONFIG_APPIMG_FLASH_ADDRESS}
                -s ${CONFIG_APPIMG_FLASH_SIZE} ${pac_config}
            dep-gen --base ${SOURCE_TOP_DIR} ${pac_config}
        OUTPUT_VARIABLE pac_dep
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )

    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} pac-gen ${pac_config} ${pac_file}
        DEPENDS ${pacgen_py} ${pac_config} ${pac_dep}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()

if(CONFIG_APPIMG_LOAD_FILE)
    set(target appimg_file_delete)
    set(pac_config ${out_hex_dir}/${target}.json)
    set(pac_file ${out_hex_dir}/${target}.pac)
    pac_init_fdl(init_fdl ${pac_config})
    execute_process(
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-del-appimg -i DEL_APPIMG ${pac_config}
            dep-gen --base ${SOURCE_TOP_DIR} ${pac_config}
        OUTPUT_VARIABLE pac_dep
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )

    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} pac-gen ${pac_config} ${pac_file}
        DEPENDS ${pacgen_py} ${pac_config} ${pac_dep}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file})
endif()
add_subdirectory_if_exist(api)
add_subdirectory_if_exist(app)
if(TTS_ENABLE)
set(target Air720U_CSDK_TTS_demo_${BUILD_TARGET})
else()
set(target Air720U_CSDK_demo_${BUILD_TARGET})
endif()

set(hex_bin_dir ${CMAKE_SOURCE_DIR}/hex/${target})
set(hex_map_dir ${CMAKE_SOURCE_DIR}/hex/${target}_map)
if(TTS_ENABLE)
set(core_bin_dir ${CMAKE_SOURCE_DIR}/core/iot_SDK_720U_TTS)
else()
set(core_bin_dir ${CMAKE_SOURCE_DIR}/core/iot_SDK_720U)
endif()
set(core_map_dir ${core_bin_dir}_map)
set(hex_app_file ${hex_bin_dir}/${target}_APP.pac)
if(CONFIG_APPIMG_LOAD_FILE)
set(pac_delete_file ${out_hex_dir}/appimg_file_delete.pac) 
set(hex_delete_file ${hex_bin_dir}/appimg_file_delete.pac)
elseif(CONFIG_APPIMG_LOAD_FLASH)
set(pac_delete_file ${out_hex_dir}/appimg_flash_delete.pac) 
set(hex_delete_file ${hex_bin_dir}/appimg_flash_delete.pac)
endif()
set(hex_app_with_core_file ${hex_bin_dir}/${target}.pac)
set(hex_map_file ${hex_map_dir}/${core_name}.map)
set(hex_elf_file ${hex_map_dir}/${core_name}.elf)

if(CONFIG_APPIMG_LOAD_FLASH)
	add_subdirectory_if_exist(demo/${BUILD_TARGET})
	target_link_libraries(${target} PRIVATE api)
	target_link_libraries(${target} PRIVATE app)
	target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})
    set(pac_config ${out_hex_dir}/${target}.json)
    set(pac_file ${out_hex_dir}/${target}_APP.pac)
	set(pac_file_with_core ${out_hex_dir}/${target}.pac)
    pac_init_fdl(init_fdl ${pac_config})
    execute_process(
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-image -i APPIMG -a ${CONFIG_APPIMG_FLASH_ADDRESS} -s ${CONFIG_APPIMG_FLASH_SIZE}
                -p ${out_hex_dir}/${target}.img ${pac_config}
            dep-gen --base ${SOURCE_TOP_DIR} ${pac_config}
        OUTPUT_VARIABLE pac_dep
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )

    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} pac-gen ${pac_config} ${pac_file}
        DEPENDS ${pacgen_py} ${pac_config} ${pac_dep}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )
	message(!!!!!!!!!!!!!!!!!!!!!!${pac_file_with_core})
	
	##+ zhuwangbin 2020-6-30 add sffs.img
	if (EXISTS ${out_hex_dir}/sffs.img)
		add_custom_command(OUTPUT ${pac_file_with_core}
			COMMAND python3 ${pacgen_core_py} ${out_hex_dir}/${target}.img ${pac_file_with_core} ${TTS_ENABLE} ${out_hex_dir}/sffs.img
			DEPENDS ${pacgen_core_py} ${pac_dep}
			WORKING_DIRECTORY ${SOURCE_TOP_DIR})
	else()
		add_custom_command(OUTPUT ${pac_file_with_core}
			COMMAND python3 ${pacgen_core_py} ${out_hex_dir}/${target}.img ${pac_file_with_core} ${TTS_ENABLE} NULL
			DEPENDS ${pacgen_core_py} ${pac_dep}
			WORKING_DIRECTORY ${SOURCE_TOP_DIR})
	endif()
	##- zhuwangbin 2020-6-30 add sffs.img
	
	add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file} ${pac_file_with_core})
endif()

if(CONFIG_APPIMG_LOAD_FILE)
    add_subdirectory_if_exist(demo/${BUILD_TARGET})
    target_link_libraries(${target} PRIVATE ${libc_file_name} ${libm_file_name} ${libgcc_file_name})
    set(pac_config ${out_hex_dir}/${target}.json)
    set(pac_file ${out_hex_dir}/${target}_APP.pac)
	set(pac_file_with_core ${out_hex_dir}/${target}.pac)
	
    pac_init_fdl(init_fdl ${pac_config})
    execute_process(
        COMMAND python3 ${pacgen_py} ${init_fdl}
            cfg-pack-file -i APPIMG -p ${out_hex_dir}/${target}.img
                -n ${CONFIG_APPIMG_LOAD_FILE_NAME} ${pac_config}
            dep-gen --base ${SOURCE_TOP_DIR} ${pac_config}
        OUTPUT_VARIABLE pac_dep
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )

    add_custom_command(OUTPUT ${pac_file}
        COMMAND python3 ${pacgen_py} pac-gen ${pac_config} ${pac_file}
        DEPENDS ${pacgen_py} ${pac_config} ${pac_dep}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
    )

	add_custom_command(OUTPUT ${pac_file_with_core}
		COMMAND python3 ${pacgen_core_py} ${out_hex_dir}/${target}.img ${pac_file_with_core} TTS_ENABLE
		DEPENDS ${pacgen_core_py} ${pac_dep}
        WORKING_DIRECTORY ${SOURCE_TOP_DIR}
	)
    add_custom_target(${target}_pacgen ALL DEPENDS ${pac_file} ${pac_file_with_core})
endif()

add_custom_command(OUTPUT ${hex_app_file} ${hex_app_with_core_file} ${hex_delete_file} ${hex_map_file} ${hex_elf_file}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${hex_bin_dir}
	COMMAND ${CMAKE_COMMAND} -E copy ${pac_file} ${hex_app_file}
	COMMAND ${CMAKE_COMMAND} -E copy ${pac_file_with_core} ${hex_app_with_core_file}
	COMMAND ${CMAKE_COMMAND} -E make_directory ${hex_map_dir}
	COMMAND ${CMAKE_COMMAND} -E copy ${core_map_dir}/${core_name}.map ${hex_map_file}
	COMMAND ${CMAKE_COMMAND} -E copy ${core_map_dir}/${core_name}.elf ${hex_elf_file}
	DEPENDS ${pac_file} ${pac_file_with_core} 
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
	
#/*+\NEW\zhuwangbin\2020.07.1\add dofta.bin */
if(DEFINED ENV{FOTA_TYPE})
		set(hex_fota_file ${BUILD_TARGET}_${CSDK_VER}_${core_name}.dfota.bin)
		
		if (${FOTA_TYPE} STREQUAL  "all")
			add_custom_command(OUTPUT ${hex_fota_file}
				COMMAND ${CMAKE_COMMAND} -E copy ${out_hex_dir}/${target}.img ${target}.img
				COMMAND ${CMAKE_COMMAND} -E copy ${core_bin_dir}/${core_name}.pac  ${core_name}.pac
				COMMAND ${CMAKE_COMMAND} -E rename ${target}.img app.bin
				COMMAND ${CMAKE_COMMAND} -E rename ${core_name}.pac ${core_name}.bin
				COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${hex_bin_dir}.zip" "--format=zip" "${core_name}.bin" "app.bin"
				COMMAND ${CMAKE_COMMAND} -E rename ${hex_bin_dir}.zip ${hex_fota_file}
				COMMAND ${CMAKE_COMMAND} -E remove [-f] ${core_name}.bin app.bin
				DEPENDS ${out_hex_dir}/${target}.img ${hex_app_file} ${hex_app_with_core_file}
				WORKING_DIRECTORY ${hex_bin_dir})
				add_custom_target(${target}_release ALL DEPENDS ${hex_app_file} ${hex_app_with_core_file} ${hex_map_file} ${hex_elf_file} ${hex_fota_file})
		endif()
		
		if (${FOTA_TYPE} STREQUAL  "core")
			add_custom_command(OUTPUT ${hex_fota_file}
			COMMAND ${CMAKE_COMMAND} -E copy ${core_bin_dir}/${core_name}.pac  ${core_name}.pac
			COMMAND ${CMAKE_COMMAND} -E rename ${core_name}.pac ${core_name}.bin
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${hex_bin_dir}.zip" "--format=zip" "${core_name}.bin" 
			COMMAND ${CMAKE_COMMAND} -E rename ${hex_bin_dir}.zip ${hex_fota_file}
			COMMAND ${CMAKE_COMMAND} -E remove [-f] ${core_name}.bin
			DEPENDS ${out_hex_dir}/${target}.img ${hex_app_file} ${hex_app_with_core_file}
			WORKING_DIRECTORY ${hex_bin_dir})
			add_custom_target(${target}_release ALL DEPENDS ${hex_app_file} ${hex_app_with_core_file} ${hex_map_file} ${hex_elf_file} ${hex_fota_file})

		endif()
		
		if (${FOTA_TYPE} STREQUAL  "app")
			add_custom_command(OUTPUT ${hex_fota_file}
			COMMAND ${CMAKE_COMMAND} -E copy ${out_hex_dir}/${target}.img ${target}.img
			COMMAND ${CMAKE_COMMAND} -E rename ${target}.img app.bin
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${hex_bin_dir}.zip" "--format=zip" "app.bin" 
			COMMAND ${CMAKE_COMMAND} -E rename ${hex_bin_dir}.zip ${hex_fota_file}
			COMMAND ${CMAKE_COMMAND} -E remove [-f] app.bin
			DEPENDS ${out_hex_dir}/${target}.img ${hex_app_file} ${hex_app_with_core_file}
			WORKING_DIRECTORY ${hex_bin_dir})
			add_custom_target(${target}_release ALL DEPENDS ${hex_app_file} ${hex_app_with_core_file} ${hex_map_file} ${hex_elf_file} ${hex_fota_file})
		endif()
else()
	add_custom_target(${target}_release ALL DEPENDS ${hex_app_file} ${hex_app_with_core_file} ${hex_map_file} ${hex_elf_file})
endif()
#/*-\NEW\zhuwangbin\2020.07.1\add dofta.bin */

