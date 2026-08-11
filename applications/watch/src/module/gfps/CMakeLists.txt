#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_REALTEK_GFPS_FEATURE_SUPPORT)

FILE(GLOB
     GFPS_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${GFPS_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

zephyr_code_relocate(FILES ${GFPS_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif(CONFIG_REALTEK_GFPS_FEATURE_SUPPORT)

