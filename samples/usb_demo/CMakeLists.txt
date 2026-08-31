#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.20.0)

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
project(usb_demo)

FILE(GLOB app_sources
		  src/*.c
		  src/fatfs/*.c
		  src/key/*.c
		  src/usb/*.c
		  src/usb_audio/*.c
		  src/usb/audio/*.c
		  src/usb_host/*.c
		  )
target_sources(app PRIVATE
  ${app_sources}
  )



if(CONFIG_REALTEK_USB_AUDIO_ENABLE)
target_sources(app PRIVATE 
			src/usb/hid/usb_hid.c
			src/usb_hid/app_usb_hid.c)
elseif(CONFIG_REALTEK_USB_HID_MOUSE_ENABLE)
add_compile_definitions(USB_HID_MOUSE_EN=1)
target_sources(app PRIVATE 
			src/usb/hid/usb_hid_mouse.c
			src/usb_hid_mouse/app_usb_hid_mouse.c)
elseif(CONFIG_REALTEK_USB_HID_KEYBOARD_ENABLE)
add_compile_definitions(USB_HID_KEYBOARD_EN=1)
target_sources(app PRIVATE 
			src/usb/hid/usb_hid.c
			src/usb_hid/app_usb_hid.c)
endif()
  
set(headers
	  src/
	  src/fatfs/
	  src/key/
	  src/usb/
	  src/usb/audio/
	  src/usb/hid/
	  src/usb_audio/
	  src/usb_hid/
	  src/usb_hid_mouse/
	  src/usb_host/
	)


foreach(header ${headers})
    if(EXISTS ${header})
        zephyr_include_directories(${header})
    else()
        message(FATAL_ERROR "header file directory does not exist: ${header}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

foreach(src ${sources})
    if(EXISTS ${src})
        target_sources(app PRIVATE ${src})
    else()
        message(FATAL_ERROR "source file does not exist: ${src}. Current cmakelist: ${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endforeach()

target_precompile_headers(app PUBLIC
	          ${CMAKE_CURRENT_SOURCE_DIR}/src/app_flags.h
			  )

target_include_directories(app PRIVATE src)
