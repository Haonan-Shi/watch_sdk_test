#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")
if(CONFIG_BOARD_RTL87X3G_WATCH_RTL8773GTP)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8773gtp)
endif()

if(CONFIG_BOARD_RTL87X3G_WATCH_RTL8773GTP_NANDBOOT)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8773gtp)
endif()

if(CONFIG_BOARD_RTL87X3G_WATCH_RTL8783GBF)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8783gbf)
endif()

