/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include "gap_le_types.h"
#include "gap.h"

typedef struct
{
    uint16_t scan_interval;                   // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 sec
    uint16_t scan_window;                     // Range: 0x0004 to 0x4000 Time = N * 0.625 msec Time Range: 2.5 msec to 10.24 seconds
    uint16_t timeout;                         // Scan timeout between 0x0001 and 0xFFFF in seconds, 0x0000 disables timeout.
} T_FM_GAP_SCAN_PARAM;

typedef struct
{
    uint16_t adv_interval_min;               // Range: 0x0020 to 0x4000  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec
    uint16_t adv_interval_max;               // Range: 0x0020 to 0x4000  Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec
    T_GAP_ADTYPE adv_type;

    struct
    {
        uint8_t ch_37_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 37 */
        uint8_t ch_38_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 38 */
        uint8_t ch_39_off : 1;  /**< Setting this bit to 1 will turn off advertising on channel 39 */
    } ch_mask;
} T_FM_GAP_ADV_PARAM;

