#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

#these files should be build on some feature
list(REMOVE_ITEM sources 
    "${CMAKE_CURRENT_SOURCE_DIR}/ams.c" #not a source in keil
    "${CMAKE_CURRENT_SOURCE_DIR}/app_bt_policy_api.c" #not a source in keil
    "${CMAKE_CURRENT_SOURCE_DIR}/app_usb.c"# not a source in keil
)

target_sources(app PRIVATE ${sources})

