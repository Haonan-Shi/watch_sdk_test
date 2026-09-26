#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Keyword-spotting app: C glue (main + static PCM clip) plus the model
# flat-buffer (C++).  Adds the prebuilt mel-feature (kws_feature) archive on top
# of the shared engine lib / heap / PSRAM bring-up from src/common.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

FILE(GLOB kws_c_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
target_sources(app PRIVATE
    ${kws_c_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model_tflite.cpp
    )

target_precompile_headers(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
    )

# Prebuilt mel-feature library (GCC / Cortex-M55, double-precision FPU).
# Wrap in a link group so its cyclic references against libm / libstdc++ and the
# shared ts_realtek archive resolve.
target_link_libraries(app PRIVATE
    -Wl,--start-group
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libkws_feature_cm55.a
    -Wl,--end-group
    )
