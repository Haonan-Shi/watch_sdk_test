#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#


# BLE-throughput monitor shell module (CONFIG_SHELL_APP_BLE_THROUGHPUT)
file(GLOB ble_throughput_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
target_sources(app PRIVATE ${ble_throughput_sources})

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/event_bus
)
