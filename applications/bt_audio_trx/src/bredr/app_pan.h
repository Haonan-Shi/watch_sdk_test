/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __APP_PAN_H
#define __APP_PAN_H


void app_pan_init(void);


bool app_pan_connect(uint8_t *bd_addr);


void app_pan_cmd(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                 uint8_t *ack_pkt);

#endif
