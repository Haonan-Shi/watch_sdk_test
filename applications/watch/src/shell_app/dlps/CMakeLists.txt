#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

# DLPS power-consumption test module (CONFIG_SHELL_APP_DLPS)
file(GLOB dlps_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
target_sources(app PRIVATE ${dlps_sources})

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/event_bus
)
