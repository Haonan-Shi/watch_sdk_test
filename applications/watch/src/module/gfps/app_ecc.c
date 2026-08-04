/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "trace.h"
#include "ecc_enhanced.h"
#include "gfps.h"


void app_ecc_handle_msg(void)
{
    T_ECC_CAUSE ecc_cause = ecc_sub_proc();

    if (ecc_cause == ECC_CAUSE_SUCCESS)
    {
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
        gfps_handle_pending_char_kbp();
#endif
    }
};
#endif
