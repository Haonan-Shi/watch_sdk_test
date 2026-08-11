#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_OPUS)

FILE(GLOB
     OPUS_SOURCES
     "*.c"
)

target_sources(app PRIVATE ${OPUS_SOURCES})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

zephyr_include_directories(inc)

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/libopus-gcc-cortex-m55.a)

zephyr_code_relocate(FILES ${OPUS_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif(CONFIG_OPUS)
