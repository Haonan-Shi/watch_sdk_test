#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

# wifi_transport.h/.c is in the shared wifi_8711 directory (used by both SPI and
# UART Wi-Fi transports). Add the include path and source so UART-only builds
# (e.g. rtl8783gbf) can find/compile it even when CONFIG_WIFI_8711 is not set.
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../wifi_8711)
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../wifi_8711/wifi_transport.c
)

# Sources compiled for all WiFi-capable boards
target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/wifi_app.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wifi_bt_coexist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app_uart_atcmd.c
    ${CMAKE_CURRENT_SOURCE_DIR}/app_wifi_uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/wifi_transport_uart.c
)

# SDIO WiFi: only for boards with hardware WiFi SDIO (e.g. rtl8783gbf_record_pen)
if (CONFIG_APP_WIFI_SDIO)
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/wifi_sdio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/wifi_sdio_tx_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/wifi_file_upload.c
    )
endif()



