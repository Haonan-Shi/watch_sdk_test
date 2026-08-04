#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

#these files should be build on some feature

# zephyr_code_relocate(FILES ${sources} LOCATION SRAM_BSS_DATA NOKEEP)

target_sources(app PRIVATE ${sources})
