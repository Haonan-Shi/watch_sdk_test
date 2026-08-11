#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

#these files should be build on some feature
list(REMOVE_ITEM sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/communicate_sync_gps.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/communicate_sync_heart_rate.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/communicate_sync_pedo.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/communicate_sync_sleep.c"
)

target_sources(app PRIVATE ${sources})

