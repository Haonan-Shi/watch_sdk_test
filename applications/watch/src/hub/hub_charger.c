/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <stdlib.h>
#include "trace.h"
#include "hub_charger.h"
#include "charger_api.h"
#include "charger_utils.h"
#include "hub_task.h"
#include "os_timer.h"
#include "app_dlps.h"
#include "app_main.h"
#include "device_charger.h"

#define NORMAL_BATTERY_REFRESH_INTERVAL     (30)
#define FAST_BATTERY_REFRESH_INTERVAL       (10)

#define MAX_CHARGER_CB_NUM 16

bool   charger_inited = false;
extern uint32_t dlps_bitmap;
static uint8_t registered_cb_num = 0;
static T_INTERNAL_CHARGER_CB charger_cb[MAX_CHARGER_CB_NUM];
static struct k_timer charger_timer;


static void charger_timer_cb(struct k_timer *timer)
{
    T_IO_MSG intn_chgr_msg;
    intn_chgr_msg.type = HUB_MSG_INTERNAL_CHARGER;
    intn_chgr_msg.subtype = GET_BATTERY_LEVEL;
    send_msg_to_hub_task(&intn_chgr_msg, __LINE__);
}

bool charger_register_cb
(
    uint8_t low_batt_level,
    void (*low_batt_level_cb)(void),
    uint8_t high_batt_level,
    void (*high_batt_level_cb)(void)
)
{
    if (registered_cb_num < MAX_CHARGER_CB_NUM)
    {
        if (charger_inited)
        {
            if (app_db.batt.level <= low_batt_level)
            {
                charger_cb[registered_cb_num].in_noral_mode = false;
                if (low_batt_level_cb)
                {
                    low_batt_level_cb();
                }
            }
            else
            {
                charger_cb[registered_cb_num].in_noral_mode = true;
                if (high_batt_level_cb)
                {
                    high_batt_level_cb();
                }
            }
        }

        charger_cb[registered_cb_num].low_batt_level = low_batt_level;
        charger_cb[registered_cb_num].low_batt_level_cb = low_batt_level_cb;
        charger_cb[registered_cb_num].high_batt_level = high_batt_level;
        charger_cb[registered_cb_num].high_batt_level_cb = high_batt_level_cb;

        ++registered_cb_num;

        return true;
    }
    else
    {
        return false;
    }
}


static void charger_invoke_cbs(void)
{
    for (uint8_t i = 0; i < registered_cb_num; ++i)
    {
        if (charger_cb[i].in_noral_mode)
        {
            if (app_db.batt.level <= charger_cb[i].low_batt_level && charger_cb[i].low_batt_level_cb)
            {
                charger_cb[i].low_batt_level_cb();
                charger_cb[i].in_noral_mode = false;
            }

        }
        else
        {
            if (app_db.batt.level > charger_cb[i].high_batt_level && charger_cb[i].high_batt_level_cb)
            {
                charger_cb[i].high_batt_level_cb();
                charger_cb[i].in_noral_mode = true;
            }
        }
    }
}

void charger_event_handler(T_IO_MSG msg)
{
    switch (msg.subtype)
    {
    case CHARGER_STATE_CHANGE:
        app_db.batt.charger_state = (T_CHARGER_STATE)msg.u.param;
        if (STATE_CHARGER_ERROR == app_db.batt.charger_state)
        {
            app_db.batt.err_code = device_charger_read_error_code();
        }
        APP_PRINT_INFO2("new charger state: 0x%x, error code: 0x%x", app_db.batt.charger_state,
                        app_db.batt.err_code);
        break;

    case GET_BATTERY_LEVEL:
        if (!(dlps_bitmap & APP_DLPS_ENTER_CHECK_DISPLAY)) // lcd is not working
        {
            device_charger_update_battery_state();
            APP_PRINT_INFO3("get battery level: %d, current: %d, voltage: %d",
                            app_db.batt.level, app_db.batt.current, app_db.batt.voltage);
            charger_invoke_cbs();
        }
        else
        {
            APP_PRINT_INFO0("not a good time to get battery level");
        }
        k_timer_start(&charger_timer, K_SECONDS(NORMAL_BATTERY_REFRESH_INTERVAL), K_NO_WAIT);
        break;

    default:
        break;
    }
}

void charger_add_hub_task(void)
{
    device_charger_init();
    k_timer_init(&charger_timer, charger_timer_cb, NULL);
    k_timer_start(&charger_timer, K_SECONDS(NORMAL_BATTERY_REFRESH_INTERVAL), K_NO_WAIT);
    charger_inited = true;
    device_charger_update_battery_state();
    charger_invoke_cbs();
}
