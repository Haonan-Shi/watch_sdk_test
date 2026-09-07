/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <stdbool.h>
#include "trace.h"

#include "mfb.h"
#include "key_button_8773g_zephyr.h"
#include "app_key_button.h"
#include "app_msg.h"
#include "app_io_msg.h"
#include "app_dlps.h"

static const struct device *gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpio_keys));
static uint16_t key2 = DT_PROP(DT_NODELABEL(key2), key_id);

#define LONG_PRESS_TIME         2500
#define VERY_LONG_PRESS_TIME    6000
#define ULTRA_LONG_PRESS_TIME   9000


static void key_action_handler(uint32_t key_id, KEY_ACTION action)
{
    APP_PRINT_INFO2("key_action_handler: key_id 0x%x, action %d", key_id, action);

    T_IO_MSG key_msg;
    key_msg.type = IO_MSG_TYPE_GPIO_BUTTON;
    key_msg.subtype = key_id;
    key_msg.u.param = action;
    app_io_msg_send(&key_msg);
}

static void gpio_button_callback(T_GPIO_KEY *key)
{
    if (key->current_state == GPIO_KEY_PRESSED)
    {
        app_dlps_disable(APP_DLPS_ENTER_CHECK_GPIO);
        return;
    }
    if (key->current_state == GPIO_KEY_RELEASED)
    {
        app_dlps_enable(APP_DLPS_ENTER_CHECK_GPIO);
    }

    uint32_t press_duration = key->release_timestamp - key->press_timestamp;

    if (press_duration >= ULTRA_LONG_PRESS_TIME)
    {
        key_action_handler(key->key_id, KEY_ACTION_ULTRA_LONG_PRESS);
    }
    else if (press_duration >= VERY_LONG_PRESS_TIME)
    {
        key_action_handler(key->key_id, KEY_ACTION_VERY_LONG_PRESS);
    }
    else if (press_duration >= LONG_PRESS_TIME)
    {
        key_action_handler(key->key_id, KEY_ACTION_LONG_PRESS);
    }
    else
    {
        key_action_handler(key->key_id, KEY_ACTION_SINGLE_CLICK);
    }
}

void app_key_gpio_button_init(void)
{
    int32_t ret = gpio_button_register_callback(gpio_dev, key2, gpio_button_callback);
    if (ret != 0)
    {
        APP_PRINT_INFO2("gpio_button_register_callback failed dev name %s, key %d", gpio_dev->name, key2);
    }
    app_mfb_init();
    app_mfb_register_callback(gpio_button_callback);
}
