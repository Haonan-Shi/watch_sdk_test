#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.20.0)

#===========version generation===========
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/version_generate.cmake)
generate_version()
#===========version generation===========
#===========post build===========
# post build should be called before find_package
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/post_build.cmake)
rtk_post_build()
#===========post build===========

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ota_demo)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

set(sources 
   ${ZEPHYR_REALTEK_APP_MODULE_DIR}/applications/bt_audio_trx/src/common/app_ble_service/ota_service.c
   )

if(CONFIG_REALTEK_COMMON_SDK_4M)
set (flash_map_source_dir ${ZEPHYR_REALTEK_APP_MODULE_DIR}/applications/bt_audio_trx/src/flashmap/4M)
else()
set (flash_map_source_dir ${ZEPHYR_REALTEK_APP_MODULE_DIR}/applications/bt_audio_trx/src/flashmap/16M)
endif()
message(STATUS "flash map dir: ${flash_map_source_dir}")

set(headers 
    #header files from app
	src/
	${ZEPHYR_REALTEK_APP_MODULE_DIR}/applications/bt_audio_trx/src/common/app_ble_service
	${ZEPHYR_REALTEK_APP_MODULE_DIR}/common/stream/
	${ZEPHYR_REALTEK_MODULE_DIR}/rtl87x3g/subsys/bluetooth/bt_host/inc
	${flash_map_source_dir}
	# ${ZEPHYR_REALTEK_MODULE_DIR}/rtl87x3g/platform/hal_lib/inc/io
    #header files from libs

    )

foreach(header ${headers})
    if(EXISTS ${header})
        zephyr_include_directories(${header})
    else()
        message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(src ${sources})
    if(EXISTS ${src})
        target_sources(app PRIVATE ${src})
    else()
        message(FATAL_ERROR "source file does not exist: ${src}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

target_precompile_headers(app PUBLIC 
	          ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
			  )
    
target_include_directories(app PRIVATE src)

