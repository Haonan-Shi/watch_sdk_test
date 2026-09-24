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
project(power_test)

# app sources (non-recursive glob)
FILE(GLOB app_sources src/*.c src/power_test/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

# flash_map.h selected by the applied flash-size snippet (e.g. -S flash_8M_bank0);
# defaults to 16M when no flash snippet is given, matching bt_audio_trx.
set(flash_map_source_dir src/flashmap/single_bank_16M)
foreach(snippet ${SNIPPET})
  if(snippet MATCHES "flash_4M")
    set(flash_map_source_dir src/flashmap/4M)
    break()
  elseif(snippet MATCHES "flash_8M")
    set(flash_map_source_dir src/flashmap/8M)
    break()
  elseif(snippet MATCHES "flash_16M")
    set(flash_map_source_dir src/flashmap/16M)
    break()
  endif()
endforeach()
message(STATUS "flash map dir: ${flash_map_source_dir}")

set(sources
   )

set(headers
    #header files from app
    src/
    src/power_test
    ${flash_map_source_dir}
    #header files from libs
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/../modules/hal/realtek/rtl87x3g/subsys/audio/inc
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/../modules/hal/realtek/rtl87x3g/subsys/console/inc
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/../modules/hal/realtek/rtl87x3g/subsys/bluetooth/ble_mgr/inc
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/common/console/transport
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/samples/power_test
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

target_include_directories(app PRIVATE src)
