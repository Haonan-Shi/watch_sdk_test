#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

#===========version generation===========
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/version_generate.cmake)
generate_version()
#===========version generation===========
#===========post build===========
# post build should be called before find_package
include(${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/post_build.cmake)

# Extract bank name from snippet list (e.g. "flash_16M_bank0" -> "bank0")
set(RTK_BANK "")
foreach(snippet ${SNIPPET})
    if(snippet MATCHES "bank[0-9]+")
        string(REGEX MATCH "bank[0-9]+" RTK_BANK "${snippet}")
        break()
    endif()
endforeach()

# APP_NAME from project name, BANK from snippet if found
if(RTK_BANK)
    rtk_post_build(APP_NAME bt_audio_trx BANK ${RTK_BANK})
else()
    rtk_post_build(APP_NAME bt_audio_trx)
endif()
#===========post build===========

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(bt_audio_trx)

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")
set(APP_TARGET_NAME app)

set(directories
	src/ai_record
    src/app
    src/app_customer
    src/ble
    src/bond
    src/bqb
    src/bredr
	src/bt_audio
	src/bt_common
	src/cmd
    src/fs
    src/ai_record_lib
    src/key_button
	src/io
	src/ipc
    src/lea_snk
	src/lea_src
	src/malleus
    src/mfb
	src/mp
	src/ota
	src/power
	src/smart_pa
	src/source_play
    src/wifi_8711
    src/spi
	src/test
	src/throuput_server
	src/transfer_path
	src/uart_dfu
	src/usb
    src/usb/audio
    src/usb/cdc
    src/usb/hid
    src/usb/mass_storage
    src/common
	src/clk
	)


#ble profile source files
set(sources
   ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/src/client/ancs_client.c
   ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/src/server/bas.c
   ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/src/server/dis.c
   ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/src/server/tps.c)

if(CONFIG_REALTEK_COMMON_SDK_4M)
set (flash_map_source_dir "src/flashmap/4M")
elseif(CONFIG_REALTEK_COMMON_SDK_8M)
set (flash_map_source_dir "src/flashmap/8M")
else()
set (flash_map_source_dir "src/flashmap/16M")
endif()

message(STATUS "flash map dir: ${flash_map_source_dir}")

set(headers
    #header files from app
	src/ai_record
    src/ai_record/wifi
    src/app
    src/app_customer
    src/ble
    src/bond
    src/bqb
    src/bredr
	src/bt_audio
	src/bt_common
	src/cmd
    ${flash_map_source_dir}
    src/fs
	src/io
	src/ipc
    src/key_button
    src/lea_snk
	src/lea_src
	src/malleus
    src/mfb
	src/mp
	src/ota
	src/power
	src/smart_pa
	src/source_play
	src/test
	src/throuput_server
	src/transfer_path
	src/uart_dfu
	src/usb
    src/usb/audio
    src/usb/cdc
    src/usb/hid
    src/usb/mass_storage
    src/common
	src/clk
    #header files from libs
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/inc/bt_gatt_client
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/inc/bt_gatt_svc
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/inc/client
    ${ZEPHYR_REALTEK_APP_MODULE_DIR}/subsys/bluetooth/gatt_profile/inc/server)


if(CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT)
    add_subdirectory(src/findmy)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/findmy)
endif()

if(CONFIG_REALTEK_GFPS_FEATURE_SUPPORT)
    add_subdirectory(src/app_gfps)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/app_gfps)
endif()

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
    # Resolve relative entries against the source dir; a bare relative path in
    # if(EXISTS) is evaluated against the CMake process CWD, which is the build
    # dir during ninja-triggered reconfigures -> flashmap (headers-only, no
    # build-output mirror dir) would spuriously fail. ABSOLUTE BASE_DIR keeps
    # already-absolute entries (e.g. ${ZEPHYR_REALTEK_APP_MODULE_DIR}/...) intact.
    get_filename_component(header_abs "${header}" ABSOLUTE BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    if(EXISTS ${header_abs})
        zephyr_include_directories(${header})
    else()
        message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()


if (CONFIG_REALTEK_HONEYGUI)
    target_link_libraries(app PRIVATE realtek_display_driver_library)
    target_link_libraries(app PRIVATE guiLib)
endif()

if(CONFIG_REALTEK_BUILD_LVGL)
    add_subdirectory(${ZEPHYR_LVGL_MODULE_DIR} modules/rtk_lvgl)
    add_subdirectory(${ZEPHYR_LVGL_MODULE_DIR}/rtk/demos modules/rtk_lvgl_demos)
    target_link_libraries(app PRIVATE realtek_display_driver_library)
    target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/lvgl/lvgl_port)
    zephyr_code_relocate(FILES ${ZEPHYR_LVGL_MODULE_DIR}/src/stdlib/builtin/lv_mem_core_builtin.c LOCATION SRAM)
endif()

if(CONFIG_REALTEK_APP_GUI)
    add_subdirectory(src/panel)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/panel)
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

#zephyr_compile_options(-include app_flags.h -include app_lea_flags.h)

#zephyr_compile_options(-include "app_flags.h" -include "app_lea_flags.h")
#zephyr_compile_options(-include "app_lea_flags.h")
#zephyr_compile_options(-include "ble_audio_flags.h")

target_compile_definitions(app PRIVATE T_OS_MEM_TYPE=RAM_TYPE)
target_compile_definitions(app PRIVATE OS_MEM_TYPE_DATA=RAM_TYPE_DATA_ON)

target_precompile_headers(app PUBLIC
	          ${CMAKE_CURRENT_SOURCE_DIR}/src/app/app_flags.h
			  ${CMAKE_CURRENT_SOURCE_DIR}/src/app/app_lea_flags.h
			  ${CMAKE_CURRENT_SOURCE_DIR}/../../../modules/hal/realtek/rtl87x3g/subsys/bluetooth/leaudio/inc/ble_audio_flags.h
              ${CMAKE_CURRENT_SOURCE_DIR}/src/app_gfps/app_gfps_flags.h
			  )
