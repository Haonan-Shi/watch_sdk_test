#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

if(CONFIG_WIFI_8711)
    # Module core (task, message queue, init) - always compiled with the module.
    # wifi_transport.c is the generic transport dispatcher (wifi_transport_get /
    # wifi_transport_register_callback). For UART boards it is compiled via
    # src/ai_record/wifi; for the SPI build (no wifi_int node -> wifi/ is skipped)
    # it must be compiled here so those symbols resolve. The two are mutually
    # exclusive (SPI board sets CONFIG_WIFI_8711 + has no wifi_int; UART board has
    # wifi_int + no CONFIG_WIFI_8711), so wifi_transport.c is never compiled twice.
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/wifi_8711_app.c
        ${CMAKE_CURRENT_SOURCE_DIR}/wifi_transport.c
    )
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    # SPI master role (drive the external 8711 slave).
    if(CONFIG_WIFI_8711_ROLE_MASTER)
        target_sources(app PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/app_spi_master_zephyr.c
            ${CMAKE_CURRENT_SOURCE_DIR}/app_spi_common.c
        )
    endif()

    # AT-command / TCP throughput test path (enlarges SPI_XMIT_SIZE to 16K).
    # wifi_transport_spi.c (g_spi_transport vtable) and spi_file_upload.c
    # (AT+SKTSENDRAW file upload) are both internally gated on CONFIG_WIFI_8711_CMD.
    if(CONFIG_WIFI_8711_CMD)
        target_sources(app PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/app_spi_atcmd.c
            ${CMAKE_CURRENT_SOURCE_DIR}/spi_tcp_tp_test.c
            ${CMAKE_CURRENT_SOURCE_DIR}/app_spi_sd_source.c
            ${CMAKE_CURRENT_SOURCE_DIR}/wifi_transport_spi.c
            ${CMAKE_CURRENT_SOURCE_DIR}/spi_file_upload.c
        )
    endif()
endif()
