#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Benchmark app: C glue (main, UART driver, protocol parser, queue, CRC) plus
# the C++ model-process helper.  Unlike the other tinyml apps there is NO
# embedded model_tflite: the model and input data are streamed in from the host
# over the data UART at runtime.  The shared engine lib / heap / PSRAM bring-up
# come from src/common.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

FILE(GLOB bench_c_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE
    ${bench_c_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model_process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ts_micro_time.cpp
    )

target_precompile_headers(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
    )
