#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#
# Shared building blocks for every tinyml application:
#   - ts_mem.c       unified PSRAM-backed ts_realtek heap adapter
#   - psRam_heap.c   heap-4 allocator over an external-PSRAM ucHeap[]
#   - tinyml_psram.c external-PSRAM controller bring-up
#   - the prebuilt ts_realtek (TFLite-Micro) engine archive
# Headers used across apps (ts_realtek.h, ts_mem.h, model_tflite.h,
# psRam_heap.h, psram_section.h, trace.h, tinyml_psram.h) live here too.

target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ts_mem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/psRam_heap.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyml_psram.c
    )

# Prebuilt inference (ts_realtek) library.  GCC / Cortex-M55 archive built for
# the target platform with a DOUBLE-precision FPU
# (-march=armv8.1-m.main+dsp+mve+fp.dp -mfpu=fpv5-d16).  Wrap in a link group
# so cyclic references against libm / libstdc++ (pulled in by
# CONFIG_NEWLIB_LIBC / CONFIG_GLIBCXX_LIBCPP) resolve.
target_link_libraries(app PRIVATE
    -Wl,--start-group
    ${CMAKE_CURRENT_SOURCE_DIR}/libts_realtek_gcc_cm55.a
    -Wl,--end-group
    )
