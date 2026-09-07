#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

# SPI transport for the bt_audio_transceiver demo (device1 / device2).
#
# Gated by F_APP_SPI_ROLE_MASTER / F_APP_SPI_ROLE_SLAVE (app_flags.h, set per
# build variant in JenkinsScript/build_flags.py). These sources are the
# transceiver-owned copy of the SPI transport and are fully decoupled from the
# wifi_8711 module in src/wifi_8711.
#
# Build only when the wifi_8711 module is NOT selected: src/spi and
# src/wifi_8711 ship the same header names (app_spi_api.h, app_spi_common.h,
# ...) so exactly one of the two include directories may be on the path at a
# time. When CONFIG_WIFI_8711=y the wifi_8711 module owns the transport; the
# transceiver files here self-gate to empty via F_APP_SPI_ROLE_* anyway, but we
# also drop them from the build to keep the include path unambiguous.
if(NOT CONFIG_WIFI_8711)
    file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    target_sources(app PRIVATE ${sources})
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
