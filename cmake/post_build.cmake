#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#set property should be set before find_package(Zephyr) called

# tool path set
set(TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/../tools")
set(BIN_DIR "${CMAKE_BINARY_DIR}/../bin")

# Detect OS and set appropriate tool paths
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(PREPEND_HEADER "${TOOLS_DIR}/Gadgets/prepend_header.exe")
	set(MD5_SCRIPT "${TOOLS_DIR}/Gadgets/md5_generate.sh")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(PREPEND_HEADER "${TOOLS_DIR}/Gadgets/prepend_header")
	set(MD5_SCRIPT "${TOOLS_DIR}/Gadgets/md5_generate.sh")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(PREPEND_HEADER "${TOOLS_DIR}/Gadgets/prepend_header_mac/prepend_header")
    set(MD5_SCRIPT "${TOOLS_DIR}/Gadgets/md5_generate_mac.sh")
else()
    message(WARNING "Unsupported OS: ${CMAKE_HOST_SYSTEM_NAME}, using Linux tools")
    set(PREPEND_HEADER "${TOOLS_DIR}/Gadgets/prepend_header")
endif()

set(ECDSA_KEY "${TOOLS_DIR}/Gadgets/ecdsa_key.pem")
set(MP_INI "${CMAKE_BINARY_DIR}/../mp.ini")
set(VERSION_H "${CMAKE_BINARY_DIR}/../VERSION")
#set(MD5_SCRIPT "${TOOLS_DIR}/Gadgets/md5_generate.sh")
set(VERSION_GEN "${TOOLS_DIR}/Gadgets/version_gen_config.sh")

set(ZEPHYR_BIN "${CMAKE_BINARY_DIR}/zephyr/zephyr.bin")
set(ZEPHYR_LST "${CMAKE_BINARY_DIR}/zephyr/zephyr.lst")
set(ZEPHYR_MAP "${CMAKE_BINARY_DIR}/zephyr/zephyr.map")
set(ZEPHYR_TRACE "${CMAKE_BINARY_DIR}/zephyr/realtek_log.trace")
set(ZEPHYR_ELF "${CMAKE_BINARY_DIR}/zephyr/zephyr.elf")
set(ZEPHYR_CONFIG "${CMAKE_BINARY_DIR}/zephyr/.config")
set(ZEPHYR_DTS "${CMAKE_BINARY_DIR}/zephyr/zephyr.dts")

# rtk_post_build([APP_NAME <name>] [BANK <bank>])
#   APP_NAME : output file base name, default is "app"
#   BANK     : optional sub-folder under bin/ (e.g. bank0, bank1)
function(rtk_post_build)
  cmake_parse_arguments(RTK_PB "" "APP_NAME;BANK" "" ${ARGN})

  # Default app name
  if(NOT RTK_PB_APP_NAME)
    set(RTK_PB_APP_NAME "app")
  endif()

  # Output directory: bin/<BANK>/ or bin/
  if(RTK_PB_BANK)
    set(OUT_DIR "${BIN_DIR}/${RTK_PB_BANK}")
    set(OUT_NAME "${RTK_PB_APP_NAME}_${RTK_PB_BANK}")
  else()
    set(OUT_DIR "${BIN_DIR}")
    set(OUT_NAME "${RTK_PB_APP_NAME}")
  endif()

  set(OUT_BIN    "${OUT_DIR}/${OUT_NAME}.bin")
  set(OUT_MP_BIN "${OUT_DIR}/${OUT_NAME}_MP.bin")
  set(OUT_LST    "${OUT_DIR}/${OUT_NAME}.lst")
  set(OUT_MAP    "${OUT_DIR}/${OUT_NAME}.map")
  set(OUT_TRACE  "${OUT_DIR}/${OUT_NAME}.trace")
  set(OUT_ELF    "${OUT_DIR}/${OUT_NAME}.elf")
  set(OUT_CONFIG "${OUT_DIR}/${OUT_NAME}.config")
  set(OUT_DTS    "${OUT_DIR}/${OUT_NAME}.dts")

  set_property(GLOBAL APPEND PROPERTY extra_post_build_commands
    COMMAND echo "============start post build=============="
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${OUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUT_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_BIN}" "${OUT_BIN}"
    COMMAND "${PREPEND_HEADER}" /app_code "${OUT_BIN}" /ecdsa "${ECDSA_KEY}"
    COMMAND "${PREPEND_HEADER}" /app_code "${OUT_BIN}" /mp_ini "${MP_INI}"
    COMMAND bash "${MD5_SCRIPT}" "${OUT_MP_BIN}" "${VERSION_H}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_LST}" "${OUT_LST}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_MAP}" "${OUT_MAP}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_TRACE}" "${OUT_TRACE}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_ELF}" "${OUT_ELF}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_CONFIG}" "${OUT_CONFIG}"
    COMMAND ${CMAKE_COMMAND} -E copy "${ZEPHYR_DTS}" "${OUT_DTS}"
    COMMAND echo "============post build finished==========="
  )
endfunction()

