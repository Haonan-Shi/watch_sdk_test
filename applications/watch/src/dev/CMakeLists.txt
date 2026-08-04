#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

if(CONFIG_HUB_CHARGER)
    add_subdirectory(charger)
    target_include_directories(app PRIVATE charger)
endif()

if(CONFIG_HUB_GSENSOR)
    add_subdirectory(gsensor)
    target_include_directories(app PRIVATE gsensor)
endif()

if(CONFIG_QDEC)
    add_subdirectory(qdec)
    target_include_directories(app PRIVATE qdec)
endif()

if(CONFIG_HUB_USB)
    add_subdirectory(usb)
    target_include_directories(app PRIVATE usb)
endif()

if(CONFIG_MFB)
    add_subdirectory(mfb)
    target_include_directories(app PRIVATE mfb)
endif()
