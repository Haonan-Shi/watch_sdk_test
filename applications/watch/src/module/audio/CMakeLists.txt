#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

set(SOURCE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_a2dp_sink
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_a2dp_src
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_hfp
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_record
    ${CMAKE_CURRENT_SOURCE_DIR}/audio_playback
)

if(CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG)
    list(APPEND SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/audio_cmd)
endif()

if(CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE)
    list(APPEND SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/audio_data_capture)
endif()

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

zephyr_code_relocate(FILES ${SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

