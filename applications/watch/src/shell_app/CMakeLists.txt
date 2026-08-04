#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

# Zephyr-shell app test scenarios (migrated from src/module/customer_app).
# Each module is gated by its CONFIG_SHELL_APP_* Kconfig symbol.

# Expose shared shell_app headers (shell_app_common.h) to every app source.
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_SHELL_APP_A2DP_SRC)
    add_subdirectory(a2dp_src)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/a2dp_src)
endif()

if(CONFIG_SHELL_APP_WIFI_BASIC)
    add_subdirectory(wifi_basic)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_basic)
endif()

if(CONFIG_SHELL_APP_DLPS)
    add_subdirectory(dlps)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/dlps)
endif()

if(CONFIG_SHELL_APP_BLE_THROUGHPUT)
    add_subdirectory(ble_throughput)
    target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ble_throughput)
endif()
