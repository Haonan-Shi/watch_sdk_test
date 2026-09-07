/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "trace.h"

#include <zephyr/dt-bindings/input/input-event-codes.h>

#include  "app_key_button.h"
#include  "app_key_process.h"
#include  "app_device.h"
#include  "app_mmi.h"

void app_gpio_button_handle_msg(T_IO_MSG *key_msg)
{
    uint32_t key_id;
    KEY_ACTION action;

    key_id = key_msg->subtype;
    action = (KEY_ACTION)key_msg->u.param;

    if (key_id == 0)
    {
        switch (action)
        {
        case KEY_ACTION_SINGLE_CLICK:
            /* code */
            break;
        case KEY_ACTION_LONG_PRESS:
            {
                if (app_device_is_power_on() == true)
                {
                    app_mmi_handle_action(MMI_DEV_POWER_OFF);
                }
                else
                {
                    app_mmi_handle_action(MMI_DEV_POWER_ON);
                }
            }
            break;
        case KEY_ACTION_VERY_LONG_PRESS:
            {
                if (app_device_is_power_on() == false)
                {
                    app_mmi_handle_action(MMI_DEV_ENTER_PAIRING_MODE);
                }
            }
            break;

        case KEY_ACTION_ULTRA_LONG_PRESS:
            {
                if (app_device_is_power_on() == false)
                {
                    app_mmi_handle_action(MMI_DEV_FACTORY_RESET);
                }
            }
            break;

        default:
            break;
        }

    }
    else if (key_id == INPUT_BTN_2)
    {
        switch (action)
        {
        case KEY_ACTION_SINGLE_CLICK:
            {
#if CONFIG_REALTEK_APP_AI_RECORD
                static bool voice_start = false;
                if (voice_start == false)
                {
                    app_mmi_handle_action(MMI_AI_VOICE_START);
                    voice_start = true;
                }
                else
                {
                    app_mmi_handle_action(MMI_AI_VOICE_STOP);
                    voice_start = false;
                }
#endif
            }

            break;
        case KEY_ACTION_LONG_PRESS:
            /* code */
            break;
        case KEY_ACTION_VERY_LONG_PRESS:
            /* code */
            break;

        default:
            break;
        }

    }

}
