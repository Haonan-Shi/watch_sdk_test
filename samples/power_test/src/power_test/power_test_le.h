/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _POWER_TEST_LE_H_
#define _POWER_TEST_LE_H_

#include <stdint.h>
#include <stdbool.h>
#include "gap_ext_scan.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "app_msg.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    POWER_TEST_CMD_LE_ADV_START   = 0x00,
    POWER_TEST_CMD_LE_ADV_STOP    = 0x01,
    POWER_TEST_CMD_LE_CONN_START  = 0x02,
    POWER_TEST_CMD_LE_CONN_STOP   = 0x03,
    POWER_TEST_CMD_LE_SCAN_START  = 0x04,
    POWER_TEST_CMD_LE_SCAN_STOP   = 0x05,
    POWER_TEST_CMD_LE_CREATE_CONN = 0x06,
    POWER_TEST_CMD_LE_CANCEL_CONN = 0x07,
    POWER_TEST_CMD_LE_UNKNOW      = 0xFF,
} T_POWER_TEST_GAP_LE_CMD;

typedef struct
{
    bool le_adv_pending;
    bool le_conn_adv_pending;
    bool le_conn_update_pending;
    bool le_scan_pending;
} T_POWER_LE_PENDING_ACTION;
/**
 * @brief used for le adv power test
 * adv_phy @ref T_GAP_PHYS_PRIM_ADV_TYPE
 * adv_event_prop @ref T_LE_EXT_ADV_LEGACY_ADV_PROPERTY
 * advertising interval for undirected and low duty directed advertising.In units of 0.625ms,
 * range: 0x000020 to 0xFFFFFF.
 */
typedef struct
{
    uint16_t adv_interval;
    uint16_t adv_data_length;
} T_POWER_LE_ADV;

/**
 * @brief used for le conn power test
 * conn_phy   GAP_PHYS_PREFER_1M_BIT, GAP_PHYS_PREFER_2M_BIT, GAP_PHYS_PREFER_CODED_BIT
 * conn_slave_latency  Range: 0x0000 to 0x01F3
 * conn_interval_min/conn_interval_max  In units of 1.25ms, Range: 0x0006 to 0x0C80.
 */
typedef struct
{
    uint8_t conn_phy;
    uint16_t conn_slave_latency;
    uint16_t conn_interval_min;
    uint16_t conn_interval_max;
} T_POWER_LE_CONN;

/**
 * @brief  used for le scan power test
 * scan_mode @ref T_GAP_SCAN_MODE
 * scan_phy @ref T_LE_EXT_SCAN_PHY_TYPE
 * scan_interval  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * scan_window  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * duplicate_enable @ref T_GAP_SCAN_FILTER_DUPLICATE
 *
 */
typedef struct
{
    uint8_t scan_phy;
    uint8_t scan_mode;
    uint16_t scan_interval;
    uint16_t scan_window;
    uint8_t  duplicate_enable;
} T_POWER_LE_SCAN;

/**
 * @brief
 *
 * @param cmd_str
 * @param buf
 * @param buf_len
 * @return true
 * @return false
 * adv_phy @ref T_GAP_PHYS_PRIM_ADV_TYPE
 * adv_event_prop @ref T_LE_EXT_ADV_LEGACY_ADV_PROPERTY
 * adv_interval_min/adv_interval_max In units of 0.625ms, range: 0x000020 to 0xFFFFFF.
 * adv_data_length 0-31
 *
 * conn_phy   GAP_PHYS_PREFER_1M_BIT, GAP_PHYS_PREFER_2M_BIT, GAP_PHYS_PREFER_CODED_BIT
 * conn_slave_latency  Range: 0x0000 to 0x01F3
 * conn_interval_min/conn_interval_max  In units of 1.25ms, Range: 0x0006 to 0x0C80.
 *
 * scan_mode @ref T_GAP_SCAN_MODE
 * scan_phy @ref T_LE_EXT_SCAN_PHY_TYPE
 * scan_interval  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * scan_window  In units of 0.625ms, range: 0x0004 to 0xFFFF.
 * duplicate_enable @ref T_GAP_SCAN_FILTER_DUPLICATE
 */

void power_handle_le_cmd(uint8_t action);

void power_le_init_adv_param(void);

T_APP_RESULT power_ble_gap_cb(uint8_t cb_type, void *p_cb_data);

void power_ble_handle_gap_msg(T_IO_MSG *p_io_msg);

void power_ble_gap_param_init(void);
#ifdef __cplusplus
}
#endif

#endif /* _POWER_TEST_LE_H_ */
