#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT)
    set(SOURCE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/ble_platform
        ${CMAKE_CURRENT_SOURCE_DIR}/ble_profile
        ${CMAKE_CURRENT_SOURCE_DIR}/fmna
        ${CMAKE_CURRENT_SOURCE_DIR}/fmna/platform
        ${CMAKE_CURRENT_SOURCE_DIR}/fmna/UARP
    )

    foreach(src ${SOURCE_DIRS})
        if(EXISTS ${src})
            file(GLOB dir_sources "${src}/*.c")
            list(APPEND SOURCES ${dir_sources})
        else()
            message(FATAL_ERROR "source file does not exist: ${src}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
    endforeach()

    target_sources(app PRIVATE ${SOURCES})

    foreach(header ${SOURCE_DIRS})
        if(EXISTS ${header})
        target_include_directories(app PRIVATE ${header})
        else()
            message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
    endforeach()
endif()

