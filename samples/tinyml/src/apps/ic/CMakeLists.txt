#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Image-classification app: C glue (main + RGB normalize + embedded image) plus
# the model flat-buffer (C++).  Shared engine lib / heap / PSRAM bring-up come
# from src/common.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

FILE(GLOB ic_c_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE
    ${ic_c_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model_tflite.cpp
    )

target_precompile_headers(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
    )
