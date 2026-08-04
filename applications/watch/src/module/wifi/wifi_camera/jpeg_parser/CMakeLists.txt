#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_WIFI_JPEG_PARSER)

FILE(GLOB JPEG_PARSER_SOURCES "*.c")

target_sources(app PRIVATE ${JPEG_PARSER_SOURCES})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
zephyr_code_relocate(FILES ${JPEG_PARSER_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif()
