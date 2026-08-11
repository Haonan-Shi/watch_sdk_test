#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

# Point to RTK root directory.
message(STATUS "CMAKELIST FOLDER: ${CMAKE_CURRENT_SOURCE_DIR}")
set(REALTEK_APP_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH "RTK root directory")

# Because display-driver and honeygui are Zephyr modules, no need to include modules file.
add_subdirectory(subsys)
add_subdirectory(common)
zephyr_include_directories(include)
