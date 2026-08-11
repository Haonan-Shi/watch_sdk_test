#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

target_sources(app PRIVATE hub_task.c)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_HUB_CHARGER)
	target_sources(app PRIVATE hub_charger.c)
endif()

if(CONFIG_HUB_CLOCK)
	target_sources(app PRIVATE hub_clock.c)
endif()

if(CONFIG_HUB_GSENSOR)
	target_sources(app PRIVATE hub_gsensor.c)
endif()

if(CONFIG_HUB_HRS)
	target_sources(app PRIVATE hub_hrs.c)
endif()

if(CONFIG_HUB_USB)
	target_sources(app PRIVATE hub_usb.c)
endif()