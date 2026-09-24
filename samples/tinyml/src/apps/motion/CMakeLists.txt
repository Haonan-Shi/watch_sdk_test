#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Motion-recognition app: C glue (main + preprocess + embedded window) plus the
# model flat-buffer (C++).  No feature library is needed (the z-score + int8
# quantization is plain C).  Shared engine lib / heap / PSRAM bring-up come from
# src/common.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

FILE(GLOB motion_c_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE
    ${motion_c_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model_tflite.cpp
    )

target_precompile_headers(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
    )
