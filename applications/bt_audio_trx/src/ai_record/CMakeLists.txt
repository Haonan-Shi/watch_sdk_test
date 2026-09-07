#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

target_sources(app PRIVATE ${sources})

# The WiFi transport (UART/SDIO to an external module) lives in wifi/. Its
# wifi_sdio.c binds a GPIO via DT_ALIAS(wifi_int), so it can only be built when
# the board/overlay defines the `wifi_int` node. The rtl8773gtp record_pen build
# streams over SPI (src/wifi_8711), has no SDIO interface, and defines no
# wifi_int, so wifi/ is skipped there. Boards that DO define wifi_int (e.g.
# rtl8783gbf record_pen) still build it. Keep F_APP_WIFI_UART_CMD (UART path) in
# src/app/app_flags.h and CONFIG_WIFI_8711 / CONFIG_WIFI_8711_CMD (SPI transport
# module) consistent with the selected board.
dt_nodelabel(wifi_int_node NODELABEL wifi_int)
if(wifi_int_node)
    add_subdirectory(wifi)
else()
    message(STATUS "ai_record: no 'wifi_int' DT node -> skipping wifi/ (SPI-only build)")
endif()
