#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_REALTEK_SUBSYS_CONSOLE)

target_sources(app PRIVATE console_uart.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

endif()

