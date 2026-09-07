/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "usbh_mgr.h"
#include "app_usbh_msc.h"
#include "trace.h"

void app_usbh_init(void)
{
    usbh_mgr_init();
    app_usbh_msc_init();
//  extern int32_t usbh_cdc_driver_init(void);
//  usbh_cdc_driver_init();
    usbh_mgr_start();
}