#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_REALTEK_KEY_BUTTON_8773G_ZEPHYR_CS)
	target_sources(app PRIVATE key_button_8773g_zephyr.c)
endif()
