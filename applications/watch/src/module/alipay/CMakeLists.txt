#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

if(CONFIG_ALIPAY)

# CMAKE_CXX_LINK_GROUP_USING_RESCAN_SUPPORTED is already set at the top of
# applications/watch/CMakeLists.txt (before find_package(Zephyr)); not
# repeated here.

set(SOURCE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_ble
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_hed_driver
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_port
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_quickJs
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_sdk
    ${CMAKE_CURRENT_SOURCE_DIR}/alipay_tlsf
)

if(CONFIG_ALIPAY_TEST)
list(APPEND SOURCE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/alipay_test)
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

set(ALIPAY_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/alipay_lib")

set(MBEDTLS_LIB_DIR "${ZEPHYR_BASE}/../modules/hal/realtek/rtl87x3g/subsys/mbedtls/lib")

file(GLOB ALIPAY_LIBS "${ALIPAY_LIB_DIR}/*.a")

file(GLOB ALIPAY_LIBS_MBEDTLS "${MBEDTLS_LIB_DIR}/*.a")

set(ALIPAY_GROUP_LIBS ${ALIPAY_LIBS} ${ALIPAY_LIBS_MBEDTLS})

# Wrap the alipay .a files into a real link group via the
# LINK_GROUP:RESCAN generator expression. The RESCAN feature is enabled for
# CXX above; CMake expands this at generate time into
# `LINKER:--start-group <libs> LINKER:--end-group` as a single unit. The
# linker rescans the group until all inter-archive cyclic dependencies are
# resolved.
target_link_libraries(app PRIVATE "$<LINK_GROUP:RESCAN,${ALIPAY_GROUP_LIBS}>")

zephyr_code_relocate(FILES SOURCE_DIRS LOCATION SRAM_DATA)

zephyr_code_relocate(FILES SOURCE_DIRS LOCATION SRAM_BSS)

endif()
