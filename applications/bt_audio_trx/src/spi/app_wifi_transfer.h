/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_WIFI_TRANSFER_H
#define APP_WIFI_TRANSFER_H
#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    WIFI_CMD_AT_TEST,
    WIFI_CMD_AT_WLCONN,
    WIFI_CMD_AT_WLDISCONN,
    WIFI_CMD_AT_WLSTARTAP,
    WIFI_CMD_AT_WLSTOPAP,
} T_WIFI_AT_CMD;

typedef enum
{
    WIFI_CMD_AT_STATE_IDLE,
    WIFI_CMD_AT_STATE_BUSY,
} T_WIFI_AT_CMD_STATE;

typedef enum
{
    WIFI_AT_CMD_SEND_RESULT_SUCCESS,
    WIFI_AT_CMD_SEND_RESULT_SPI_ERR,
    WIFI_AT_CMD_SEND_RESULT_SEQ_ERR,
} T_WIFI_AT_CMD_SEND_RESULT;

void app_wifi_trans_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                   uint16_t cmd_len, uint8_t *ack_pkt);

#endif
