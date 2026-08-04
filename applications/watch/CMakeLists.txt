#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_C_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)
set(CMAKE_C_LINK_GROUP_USING_RESCAN
    "LINKER:--start-group"
    "LINKER:--end-group"
)
set(CMAKE_CXX_LINK_GROUP_USING_RESCAN_SUPPORTED TRUE)
set(CMAKE_CXX_LINK_GROUP_USING_RESCAN
    "LINKER:--start-group"
    "LINKER:--end-group"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/pre_build_process.cmake)
#===========version generation===========
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/version_generate.cmake)
generate_version()
#===========version generation===========
#===========post build===========
# post build should be called before find_package
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/post_build.cmake)
rtk_post_build()
#===========post build===========

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(watch)

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")
set(APP_TARGET_NAME app)

set(directories
    src/app
    src/comm
    src/dev
    src/event_bus
    src/hub
    src/module
    src/mp
    src/gui
    src/shell_app
    src/flashmap)

set(headers
    #header files from app
    src/app
    src/comm
    src/dev
    src/event_bus
    src/module
    src/hub
    src/mp
    src/gui
    src/shell_app
    src/flashmap)

foreach(subdir ${directories})
    if(EXISTS ${subdir})
        add_subdirectory(${subdir})
    else()
        message(FATAL_ERROR "subdirectory does not exist: ${subdir}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(src ${sources})
    if(EXISTS ${src})
        target_sources(app PRIVATE ${src})
    else()
        message(FATAL_ERROR "source file does not exist: ${src}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(header ${headers})
    if(EXISTS ${header})
        zephyr_include_directories(${header})
    else()
        message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

# HoneyGUI, display driver library link
# HoneyGUI ui resource preprocess and copy
if (CONFIG_REALTEK_DISPLAY)
  target_link_libraries(app PRIVATE realtek_display_driver_library)
endif()
if (CONFIG_REALTEK_HONEYGUI)
  target_link_libraries(app PRIVATE guiLib)

  set(only_copy_ui_resource FALSE)

  if(CONFIG_REALTEK_BUILD_GUI_410_502_DEMO)
    set(ui_demo 410_502)
  elseif(CONFIG_REALTEK_BUILD_GUI_466_466_DEMO)
    set(ui_demo 466_466)
  elseif(CONFIG_REALTEK_HONEYGUI_410_502_HMI_SMARTWATCH_DEMO)
    set(ui_demo none)
    set(only_copy_ui_resource TRUE)
  else()
    set(ui_demo none)
    message(WARNING "Please check prj.conf, gui demo is not selected")
  endif()

  if(only_copy_ui_resource)
    set(copy_input_file "${CMAKE_CURRENT_SOURCE_DIR}/src/gui/gui_app/bin/res.bin")
    honeygui_resource_copy(app ${copy_input_file} "${CMAKE_BINARY_DIR}/../honeygui_userdata/")
  endif()

  if(NOT ui_demo STREQUAL "none")
    dt_nodelabel(userdata1_node NODELABEL "partition_userdata1") 
    dt_nodelabel(flash_node NODELABEL "flash") 
    dt_prop(base_addr PATH ${flash_node} PROPERTY "reg" INDEX 0)
    dt_prop(offset_addr PATH ${userdata1_node} PROPERTY "reg" INDEX 0)
    set(image_header_len 0x400)
    math(EXPR userdata_addr "${base_addr} + ${offset_addr} + ${image_header_len}" OUTPUT_FORMAT HEXADECIMAL)
    message(STATUS "userdata1_node offset_addr: ${userdata1_node}, ${offset_addr}")
    message(STATUS "userdata_addr: ${userdata_addr}")
    honeygui_resource_preprocess(guiLib ${userdata_addr} ${ui_demo})
  endif()

endif() 

if(CONFIG_REALTEK_BUILD_LVGL)
  add_subdirectory(${ZEPHYR_LVGL_MODULE_DIR} modules/rtk_lvgl)
  if(CONFIG_REALTEK_BUILD_LVGL_SIMPLE_DEMO)
    add_subdirectory(${ZEPHYR_LVGL_MODULE_DIR}/rtk/demos modules/rtk_lvgl_demos)
  endif()
  target_link_libraries(app PRIVATE realtek_display_driver_library)
  target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/lvgl/lvgl_port)
  zephyr_code_relocate(FILES ${ZEPHYR_LVGL_MODULE_DIR}/src/stdlib/builtin/lv_mem_core_builtin.c LOCATION SRAM_BSS_DATA NOKEEP)
endif()


zephyr_compile_options(
    -Wno-unused-variable
    -Wno-maybe-uninitialized
    -Wno-double-promotion
    -Wno-stringop-truncation
    -Wno-unused-function
    -Wno-missing-braces
    -Wno-use-after-free
    -Wno-parentheses
    -Wno-address
    -Wno-format-overflow
    -Wno-overflow
    -Wno-char-subscripts
    -Wno-enum-compare
)

#workaround for os_mem.h type define
target_compile_definitions(app PRIVATE T_OS_MEM_TYPE=RAM_TYPE)
target_compile_definitions(app PRIVATE OS_MEM_TYPE_DATA=RAM_TYPE_DATA_ON)
#marcos from app_flags.h
target_compile_definitions(app PRIVATE F_APP_A2DP_SOURCE_SUPPORT=1)

target_compile_definitions(app PRIVATE F_APP_HID_SUPPORT=0)
target_compile_definitions(app PRIVATE F_APP_HID_KEYBOARD_MOUSE_SUPPORT=0)

target_compile_definitions(app PRIVATE F_APP_SUPPORT_USB=0)

if(DEFINED CONFIG_BLE_DEFAULT_LINKS)
    math(EXPR NEW_BLE_LINKS "${CONFIG_BLE_DEFAULT_LINKS} + ${CONFIG_FINDMY_MAX_LINKS} + ${CONFIG_REALTEK_GFPS_MAX_LINKS} + ${CONFIG_WALKIE_TALKIE_BLE_LINKS}")
    target_compile_definitions(app PRIVATE CONFIG_BLE_LINKS=${NEW_BLE_LINKS})
endif()


if(CONFIG_FAT_FILESYSTEM_ELM)
    zephyr_code_relocate(FILES ${ZEPHYR_BASE}/subsys/fs/fat_fs.c LOCATION SRAM_NOINIT)
endif()
zephyr_code_relocate(FILES ${ZEPHYR_BASE}/soc/realtek/bee/rtl87x3g/osif/zephyr/osif_zephyr.c LOCATION SRAM_NOINIT)
zephyr_linker_sources(SECTIONS app_module_init.ld)
zephyr_linker_sources(SECTIONS app_ble_service_info.ld)
