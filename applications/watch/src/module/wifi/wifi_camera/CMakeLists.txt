#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_WIFI_CAMERA)

FILE(GLOB
     WIFI_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${WIFI_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

zephyr_code_relocate(FILES ${WIFI_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

if(CONFIG_WIFI_RTSP)
    add_subdirectory(rtsp)
    target_include_directories(app PRIVATE rtsp)
endif()

if(CONFIG_WIFI_JPEG_PARSER)
    add_subdirectory(jpeg_parser)
    target_include_directories(app PRIVATE jpeg_parser)
endif()


endif()
