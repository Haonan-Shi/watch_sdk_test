#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_DFU)

FILE(GLOB
     DFU_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${DFU_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

zephyr_code_relocate(FILES ${DFU_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif(CONFIG_DFU)
