#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE  sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(app PRIVATE ${sources})
zephyr_code_relocate(FILES ./app_gui_port/gui_port_os.c LOCATION SRAM_BSS_DATA NOKEEP)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_db/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_init/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_msg/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_port/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_ui/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_gui_ui/chargebox/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/app_module_port/)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/user_data/RTL8773g/)
