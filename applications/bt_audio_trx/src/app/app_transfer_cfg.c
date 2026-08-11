/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "rtl876x.h"
#include "rtl876x_uart.h"
#include "app_transfer_cfg.h"

T_APP_TRANSFER_CFG app_transfer_cfg;

void app_transfer_cfg_init(void)
{
    app_transfer_cfg.enable_rtk_vendor_cmd = 1;
    app_transfer_cfg.data_uart_baud_rate = BAUD_RATE_2000000;
#if F_APP_NXP_UWB_CALIBRATION_DATA_DUMP
    app_transfer_cfg.report_uart_event_only_once = true;
#else
    app_transfer_cfg.report_uart_event_only_once = false;
#endif
    app_transfer_cfg.resend_interval_ms = 100;
    app_transfer_cfg.resend_num = 10;
}
