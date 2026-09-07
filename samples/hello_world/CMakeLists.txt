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
project(hello_world)

target_sources(app PRIVATE src/main.c)


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