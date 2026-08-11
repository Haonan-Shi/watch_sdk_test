#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

cmake_minimum_required(VERSION 3.10)

# Define a variable for the library name
if(CONFIG_REALTEK_HONEYGUI_410_502_HMI_SMARTWATCH_DEMO AND CONFIG_REALTEK_HONEYGUI) #set to 1 to enable or variable to for dependency

    file(GLOB_RECURSE SOURCES "*.c" "*.cpp")
    file(GLOB_RECURSE HEADERS "*.h" "*.hpp")

    set(ALL_INCLUDE_DIRS "")

    foreach(header_path ${HEADERS})
        get_filename_component(dir_path ${header_path} PATH)
        list(APPEND ALL_INCLUDE_DIRS ${dir_path})
    endforeach()

    if(ALL_INCLUDE_DIRS)
        list(REMOVE_DUPLICATES ALL_INCLUDE_DIRS)
    endif()

    message(STATUS "Recursive Include Dirs: ${ALL_INCLUDE_DIRS}\n")

    if((NOT HEADERS) AND (NOT SOURCES))
        message(STATUS "No source or header files found in ${CMAKE_CURRENT_SOURCE_DIR}.")
    endif()

    if(HEADERS AND (NOT SOURCES))
        target_include_directories(app PUBLIC ${ALL_INCLUDE_DIRS})
    endif()

    if(SOURCES AND (NOT HEADERS))
        target_sources(app PRIVATE ${SOURCES})
    endif()

    if(SOURCES AND HEADERS)
        target_sources(app PRIVATE ${SOURCES})
        target_include_directories(app PUBLIC ${ALL_INCLUDE_DIRS})
    endif()

endif()
