/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "string.h"
#include "gfps.h"
#include "app_gfps.h"
#include "app_gfps_cfg.h"

T_APP_GFPS_CFG app_gfps_cfg;

void app_gfps_cfg_init(void)
{
    app_gfps_cfg.gfps_model_id[0] = 0;
    app_gfps_cfg.gfps_model_id[1] = 0;
    app_gfps_cfg.gfps_model_id[2] = 0;

    app_gfps_cfg.gfps_support = 1;
    app_gfps_cfg.gfps_finder_support = 1;
    app_gfps_cfg.gfps_le_device_support = 1;
    app_gfps_cfg.gfps_enable_tx_power = 1;
    app_gfps_cfg.gfps_tx_power = -6;
    app_gfps_cfg.tone_gfps_findme = 0x9C;//power_on.wav
    app_gfps_cfg.gfps_account_key_num = 5;
    app_gfps_cfg.gfps_discov_adv_interval = 32;
    app_gfps_cfg.gfps_not_discov_adv_interval = 100;

    app_gfps_cfg.gfps_battery_info_enable = 0;
    app_gfps_cfg.gfps_le_disconn_force_enter_pairing_mode = 0;
    app_gfps_cfg.gfps_le_device_mode = GFPS_LE_DEVICE_MODE_LE_MODE_WITHOUT_LEA;

    app_gfps_cfg.gfps_power_on_finder_adv_interval = 800;
    app_gfps_cfg.disable_finder_adv_when_power_off = 1;
    uint8_t  gfps_public_key[64] = {0};
    uint8_t  gfps_private_key[32] = {0};
    memcpy(app_gfps_cfg.gfps_public_key, gfps_public_key, 64);
    memcpy(app_gfps_cfg.gfps_private_key, gfps_private_key, 32);

    app_gfps_cfg.tone_gfps_dult = 0x9C;//power_on.wav

    uint8_t  device_name[64] = {0};
    uint8_t  gfps_version[10] = "v1.3";
    uint8_t  company_name[64] = {0};
    memcpy(app_gfps_cfg.gfps_company_name, company_name, sizeof(company_name));
    memcpy(app_gfps_cfg.gfps_device_name, device_name, sizeof(device_name));

    app_gfps_cfg.gfps_device_type = GFPS_LOCATOR_TRACKER;
}
#endif
