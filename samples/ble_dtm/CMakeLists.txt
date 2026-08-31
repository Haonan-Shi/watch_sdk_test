#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

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
project(ble_dtm)

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

set(directories
    src/app)





foreach(subdir ${directories})
    if(EXISTS ${subdir})
        add_subdirectory(${subdir})
    else()
        message(FATAL_ERROR "subdirectory does not exist: ${subdir}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(src ${sources})
    if(EXISTS ${src})
        target_sources(app PRIVATE ${src})
    else()
        message(FATAL_ERROR "source file does not exist: ${src}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(header ${headers})
    if(EXISTS ${header})
    target_include_directories(app PRIVATE ${header})
    else()
        message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

zephyr_include_directories(src/app)

zephyr_compile_options(
    -Wno-unused-variable
    -Wno-maybe-uninitialized
    -Wno-double-promotion
    -Wno-stringop-truncation
    -Wno-unused-function
    -Wno-missing-braces
    -Wno-use-after-free
    -Wno-parentheses
    -Wno-address
    -Wno-format-overflow
    -Wno-overflow
    -Wno-char-subscripts
    -Wno-enum-compare
)

#workaround for os_mem.h type define
target_compile_definitions(app PRIVATE T_OS_MEM_TYPE=RAM_TYPE)
target_compile_definitions(app PRIVATE OS_MEM_TYPE_DATA=RAM_TYPE_DATA_ON)

#marcos from app_flags.h
target_compile_definitions(app PRIVATE CONFIG_SOC_SERIES_RTL8773G=1)
target_compile_definitions(app PRIVATE F_BT_LE_5_0_SUPPORT=1)


