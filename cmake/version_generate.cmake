#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

set(TOOLS_DIR "${CMAKE_CURRENT_LIST_DIR}/../../tools")
set(VERSION_H "${CMAKE_BINARY_DIR}/../VERSION")
set(VERSION_GEN "${TOOLS_DIR}/Gadgets/version_gen_config.sh")

option(ENABLE_GENERATE_VERSION "Enable version generation" OFF)

function(generate_version)
  if(NOT ENABLE_GENERATE_VERSION)
    message(STATUS "[VERSION] skip generate version")
    return()
  endif()
  message(STATUS "[VERSION] generate version...")
  find_program(Bash_EXECUTABLE bash NO_CMAKE_FIND_ROOTO_PATH)
  message(STATUS "[VERSION]${Bash_EXECUTABLE} ${VERSION_GEN} ${VERSION_H}")
  execute_process(
    COMMAND ${Bash_EXECUTABLE} ${VERSION_GEN} ${VERSION_H}
  )
endfunction(generate_version)