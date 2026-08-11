/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <gap_vendor.h>
#include "trace.h"

static void app_set_pta(uint8_t type, uint8_t priority, uint16_t handle)
{
    uint8_t policy[4] = {0};
    policy[0] = type;
    policy[1] = priority;
    policy[2] = (handle) & 0xff;
    policy[3] = (handle >> 8) & 0xff;
    gap_vendor_cmd_req(0xFDC0, 4, policy);
    APP_PRINT_TRACE1("app_set_pta, priority %d", priority);
}
