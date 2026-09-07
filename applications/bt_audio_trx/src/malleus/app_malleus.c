/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_MALLEUS_SUPPORT
#include "malleus.h"
#include "app_malleus.h"
#include "app_cfg.h"
#include "app_cmd.h"
#include "app_a2dp.h"
#include "app_report.h"
#include "app_main.h"

static const uint8_t malleus_ack_map[] = {CMD_SET_STATUS_COMPLETE, CMD_SET_STATUS_DISALLOW, CMD_SET_STATUS_UNKNOW_CMD, CMD_SET_STATUS_PARAMETER_ERROR};

static void app_malleus_report_event(uint8_t cmd_path, uint8_t app_index, uint8_t *data,
                                     uint16_t len)
{
    app_report_event(cmd_path, EVENT_MALLEUS_CUSTOMIZED_REPORT, app_index, data, len);
}

static void app_malleus_param_init(void)
{
    malleus.normal_type = &app_cfg_nv.malleus_effect_normal_type;

    malleus.factory_addr = app_db.factory_addr;
    malleus.key_flash_offset = APP_RW_MALLEUS_KEY_VAL_ADDR;
    malleus.key_flash_len = APP_RW_MALLEUS_KEY_VAL_SIZE;

    malleus.report_event = app_malleus_report_event;
}

void app_malleus_cfg_rst(void)
{
    malleus_cfg_rst(MALLEUS_MUSIC_MODE, MALLEUS_MUSIC_MODE);
}

void app_malleus_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                            uint8_t *ack_pkt)
{
    uint8_t ack_idx = malleus_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx);

    ack_pkt[2] = malleus_ack_map[ack_idx];
    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
}

void app_malleus_init(uint8_t normal_cycle, uint8_t gaming_cycle)
{
    app_malleus_param_init();
    malleus_init(normal_cycle, gaming_cycle);
}
#endif
