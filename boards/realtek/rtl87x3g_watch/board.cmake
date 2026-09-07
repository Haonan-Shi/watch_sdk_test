#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

board_runner_args(jlink "--device=RTL87X3G" "--speed=4000")
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
