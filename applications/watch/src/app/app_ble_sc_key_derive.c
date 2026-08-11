/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_SC_KEY_DERIVE
#include "gap_bond_le.h"
#include "trace.h"

//Distribution of keys
#define SMP_DIST_ENC_KEY        0x01/*distribute LTK*/
#define SMP_DIST_ID_KEY         0x02/*distribute IRK*/
#define SMP_DIST_SIGN_KEY       0x04/*distribute CSRK*/
#define SMP_DIST_LINK_KEY       0x08/*distribute link key*/

void app_ble_key_derive_init(void)
{
    APP_PRINT_INFO0("app_ble_key_derive_init");
    uint8_t init_key = SMP_DIST_ENC_KEY | SMP_DIST_ID_KEY | SMP_DIST_LINK_KEY;
    uint8_t repsonse_key = SMP_DIST_ENC_KEY | SMP_DIST_LINK_KEY;
    le_bond_cfg_local_key_distribute(init_key, repsonse_key);

    /*set convert LTK to linkkey*/
    uint8_t key_convert_flag = GAP_SC_KEY_CONVERT_LE_TO_BREDR_FLAG;
    gap_set_param(GAP_PARAM_BOND_LINK_KEY_CONVERT, sizeof(key_convert_flag), &key_convert_flag);
}
#endif
