#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_WIFI_RTSP)

FILE(GLOB
     RTSP_SOURCES
     "*.c"
)

# Link the static library even if no .c files exist
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/liblive_min-cm55.a)

target_sources(app PRIVATE ${RTSP_SOURCES})
zephyr_code_relocate(FILES ${RTSP_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif()
