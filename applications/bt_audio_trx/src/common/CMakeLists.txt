#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

#file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

#target_sources(app PRIVATE ${sources})

target_sources(app PRIVATE
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_data_transfer.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_dsp_cfg.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_nrec.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_util.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_sidetone.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_cfu_transfer.c
			 ${CMAKE_CURRENT_SOURCE_DIR}/app_common_cfu.c
			 )

add_subdirectory(app_ble_service)
add_subdirectory(app_ble_client)
add_subdirectory(usb)