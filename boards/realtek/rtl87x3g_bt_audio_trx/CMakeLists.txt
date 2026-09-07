#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_BOARD_RTL87X3G_BT_AUDIO_TRX_RTL8773GTP)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8773gtp)
endif()

if(CONFIG_BOARD_RTL87X3G_BT_AUDIO_TRX_RTL8763GWS)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8763gws)
endif()

if(CONFIG_BOARD_RTL87X3G_BT_AUDIO_TRX_RTL8783GBF)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8783gbf)
endif()

if(CONFIG_BOARD_RTL87X3G_BT_AUDIO_TRX_RTL8763GTS)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/board_pin_assignments/rtl8763gts)
endif()

