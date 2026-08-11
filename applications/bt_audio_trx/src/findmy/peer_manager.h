/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "stdbool.h"
#include "app_msg.h"
#include "fmna_constants_platform.h"

bool pm_is_conn_handle_excluded(T_IO_MSG *p_gap_msg);
void pm_exclude_list_init(void);
fmna_ret_code_t pm_conn_exclude_add(uint16_t conn_handle);
fmna_ret_code_t pm_conn_exclude_delete(uint16_t conn_handle);
