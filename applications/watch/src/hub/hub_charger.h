/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _HUB_CHARGER_H_
#define _HUB_CHARGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "app_msg.h"
#include "charger_api.h"


typedef struct
{
    uint8_t level;
    T_CHARGER_STATE charger_state;
    T_CHARGER_ERROR_CODE err_code;
    int16_t current;
    uint16_t voltage;
    struct
    {
        bool bt         : 1;
        bool ble        : 1;
        bool playback   : 1;
        bool lcd        : 1;
        // to be added
    } allow_open;
} T_BATTERY_INFO;

typedef struct
{
    bool in_noral_mode;
    uint8_t low_batt_level;
    uint8_t high_batt_level;
    void (*low_batt_level_cb)(void);
    void (*high_batt_level_cb)(void);
} T_INTERNAL_CHARGER_CB;

void charger_event_handler(T_IO_MSG msg);
void charger_add_hub_task(void);

/**
 * Callbacks will behave like a hysteresis comparator. low_batt_level_cb will be invoked
 * when battery level drops lower than low_batt_level, and high_batt_level_cb will be invoked
 * when battery level rises higher than high_batt_level.
 *
 * During rigister, if battery level is lower than low_batt_level, low_batt_level_cb will be
 * invoked, otherwise high_batt_level_cb will be invoked.
 *
 * @return true if register successfully, otherwise false.
 * @note  Callbacks are invoked in hub task. It is better to only use os_msg_send in callbacks.
 */
bool internal_charger_register_cb
(
    uint8_t low_batt_level,
    void (*low_batt_level_cb)(void),
    uint8_t high_batt_level,
    void (*high_batt_level_cb)(void)
);

#ifdef __cplusplus
}
#endif

#endif /* _HUB_CHARGER_H_ */
