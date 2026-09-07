#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

file(GLOB source_files "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
set(AI_RECORD_LIB_NAME ai_record_lib)

set(PUBLIC_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/inc")

if(source_files)
    # Dev Mode
    message(STATUS "[${AI_RECORD_LIB_NAME}] Build from SOURCE")

    add_library(${AI_RECORD_LIB_NAME} STATIC ${source_files})

    target_include_directories(${AI_RECORD_LIB_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PUBLIC_INC_DIR}
    )

    target_link_libraries(${AI_RECORD_LIB_NAME} PUBLIC zephyr_interface)

    target_link_libraries(app PUBLIC ${AI_RECORD_LIB_NAME})

    zephyr_link_libraries(${AI_RECORD_LIB_NAME})

else()
    # Release Mode
    set(PRECOMPILED_LIB "${CMAKE_CURRENT_SOURCE_DIR}/lib${AI_RECORD_LIB_NAME}.a")
    if(EXISTS ${PRECOMPILED_LIB})
        message(STATUS "[${AI_RECORD_LIB_NAME}] Link from PRE-COMPILED LIBRARY")
        add_library(${AI_RECORD_LIB_NAME} STATIC IMPORTED GLOBAL)
        set_target_properties(${AI_RECORD_LIB_NAME} PROPERTIES IMPORTED_LOCATION "${PRECOMPILED_LIB}")
        target_include_directories(${AI_RECORD_LIB_NAME} INTERFACE "${PUBLIC_INC_DIR}")

        target_link_libraries(app PUBLIC ${AI_RECORD_LIB_NAME})
        zephyr_link_libraries(${AI_RECORD_LIB_NAME})
    endif()
endif()
