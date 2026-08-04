#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_WIFI)

FILE(GLOB
     WIFI_SOURCES
     "*.c"
)

# wifi_gui.c depends on HoneyGUI; only compile it when the GUI is enabled
if(NOT CONFIG_REALTEK_HONEYGUI)
    list(REMOVE_ITEM WIFI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/wifi_gui.c")
endif()


target_sources(app PRIVATE ${WIFI_SOURCES})

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

zephyr_code_relocate(FILES ${WIFI_SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

if(CONFIG_WIFI_CAMERA)
    add_subdirectory(wifi_camera)
    target_include_directories(app PRIVATE wifi_camera)
endif()

endif()
