# SPDX-License-Identifier: Apache-2.0

if(CONFIG_CYWEE_MOTION_ALGORITHM)

set(SOURCE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cwm_app_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/samples/RTL8773G
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

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Library/ALGO/Inc)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Library/DML/Inc)

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Library/ALGO/Lib/CWM_LIB_gcc_m55.a)
target_link_libraries(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Library/DML/Lib/CWM_LIB_DML_gcc_m55.a)

zephyr_code_relocate(FILES ${SOURCES} LOCATION SRAM_BSS_DATA NOKEEP)

endif()
