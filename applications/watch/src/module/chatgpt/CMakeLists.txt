#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_CHATGPT)

FILE(GLOB
     CHATGPT
     "*.c"
)

target_sources(app PRIVATE ${CHATGPT})

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

zephyr_code_relocate(FILES ${CHATGPT} LOCATION SRAM_BSS_DATA NOKEEP)

endif(CONFIG_CHATGPT)
