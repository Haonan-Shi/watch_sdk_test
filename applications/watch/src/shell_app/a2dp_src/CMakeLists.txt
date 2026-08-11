#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

# A2DP-source play-music test module (CONFIG_SHELL_APP_A2DP_SRC)
file(GLOB a2dp_src_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
target_sources(app PRIVATE ${a2dp_src_sources})

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/app
    ${CMAKE_SOURCE_DIR}/src/event_bus
    ${CMAKE_SOURCE_DIR}/src/module/audio
    ${CMAKE_SOURCE_DIR}/src/module/audio/audio_a2dp_src
)
