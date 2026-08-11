#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

cmake_minimum_required(VERSION 3.10)

if(CONFIG_APP_NANDBOOT AND CONFIG_SOC_RTL8773GTP)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nand)
endif()

if(NOT CONFIG_APP_NANDBOOT AND CONFIG_SOC_RTL8773GTP)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nor/16M)
endif()
if(NOT CONFIG_APP_NANDBOOT AND CONFIG_SOC_RTL8783GBF)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/nor/8M)
endif()
