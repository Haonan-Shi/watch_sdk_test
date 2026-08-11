#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_AIRSYNC_SERVICE_SUPPORT)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_sources(app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/airsync_ble_service.c
    )
endif()