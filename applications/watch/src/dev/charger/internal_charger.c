/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "trace.h"
#include "device_charger.h"
#include "charger_api.h"
#include "charger_utils.h"
#include "hub_task.h"
#include "os_timer.h"
#include "app_dlps.h"
#include "app_main.h"


static void charger_state_callback(T_CHARGER_STATE charger_state)
{
    T_IO_MSG intn_chgr_msg;
    intn_chgr_msg.type = HUB_MSG_INTERNAL_CHARGER;
    intn_chgr_msg.subtype = CHARGER_STATE_CHANGE;
    intn_chgr_msg.u.param = (uint32_t)charger_state;
    send_msg_to_hub_task(&intn_chgr_msg, __LINE__);
}

void device_charger_update_battery_state(void)
{
    int16_t current;
    uint16_t voltage;
    if ((charger_utils_get_batt_volt(&voltage) == CHARGER_UTILS_SUCCESS) &&
        (charger_utils_get_batt_curr(&current) == CHARGER_UTILS_SUCCESS))
    {
        app_db.batt.current = current;
        app_db.batt.voltage = voltage;
    }
    app_db.batt.level = charger_api_get_state_of_charge();
}

T_CHARGER_ERROR_CODE device_charger_read_error_code(void)
{
    return charger_api_get_error_code();
}

void device_charger_init(void)
{
    // charger must be initialed before other module is power on, in case of other module draws current
    app_db.batt.charger_state = charger_api_get_charger_state();
    if (app_db.batt.charger_state == STATE_CHARGER_ERROR)
    {
        app_db.batt.err_code = charger_api_get_error_code();
        if (app_db.batt.err_code == CHARGER_ERROR_OPEN || app_db.batt.err_code == CHARGER_ERROR_SHORT)
        {
            app_db.batt.level = 0xff;
        }
        else
        {
            app_db.batt.level = charger_api_get_state_of_charge();
        }
    }
    else
    {
        app_db.batt.err_code = CHARGER_NO_ERROR;
        app_db.batt.level = charger_api_get_state_of_charge();
    }

    int16_t current;
    uint16_t voltage;
    if ((charger_utils_get_batt_volt(&voltage) == CHARGER_UTILS_SUCCESS) &&
        (charger_utils_get_batt_curr(&current) == CHARGER_UTILS_SUCCESS))
    {
        app_db.batt.current = current;
        app_db.batt.voltage = voltage;
    }

    APP_PRINT_INFO3("initial battery level: %d, current: %d, voltage: %d",
                    app_db.batt.level, app_db.batt.current, app_db.batt.voltage);
    APP_PRINT_INFO2("initial charger state: 0x%x, error code: 0x%x", app_db.batt.charger_state,
                    app_db.batt.err_code);

    charger_api_reg_charger_state_callback(charger_state_callback);
}

