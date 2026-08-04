#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_INTERNAL_CHARGER)
	target_sources(app PRIVATE internal_charger.c)
endif()

if(CONFIG_CJ4056_CHARGER)
	target_sources(app PRIVATE cj4056_charger.c)
endif()



