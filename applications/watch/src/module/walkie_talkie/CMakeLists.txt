#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_WALKIE_TALKIE)

FILE(GLOB
     WALKIE_TALKIE_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${WALKIE_TALKIE_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/codec2)

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/codec2/libcodec2_lib.a)

zephyr_code_relocate(FILES ${WALKIE_TALKIE_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif(CONFIG_WALKIE_TALKIE)

