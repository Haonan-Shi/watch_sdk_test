/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_MALLEUS_H_
#define _APP_MALLEUS_H_

#include "malleus.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief  audio effect config reset.
 *
 * @param  void
 * @return void
*/
void app_malleus_cfg_rst(void);

/**
    * @brief  App process malleus command.
    * @param  cmd_ptr command pointer
    * @param  cmd_len command length
    * @param  cmd_path command path use for distinguish uart,or le,or spp channel.
    * @param  app_idx received rx command device index
    * @param  ack_pkt command ack
    * @return void
*/
void app_malleus_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx,
                            uint8_t *ack_pkt);

/**
 * @brief  audio effect init.
 *
 * @param  normal_cycle normal mode cycle.
 * @param  gaming_cycle gaming mode cycle.
 * @return void
*/
void app_malleus_init(uint8_t normal_cycle, uint8_t gaming_cycle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
