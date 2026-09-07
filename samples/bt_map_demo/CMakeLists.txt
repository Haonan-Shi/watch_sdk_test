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
project(bt_map_demo)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE
  ${app_sources}
  )

target_precompile_headers(app PUBLIC 
	          ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
			  )
    
target_include_directories(app PRIVATE src)

