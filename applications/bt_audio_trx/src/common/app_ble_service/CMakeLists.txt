#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(app PRIVATE ${sources})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
