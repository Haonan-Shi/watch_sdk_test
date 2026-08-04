#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_RTK_PPG)

FILE(GLOB
     RTK_PPG_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${RTK_PPG_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/RTL87x5PPG.a)

zephyr_code_relocate(FILES ${RTK_PPG_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif()
