/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include "platform_reg.h"
#include "mfb_api.h"
#include "mfb.h"
#include "app_dlps.h"

#define MFB_LEVEL_HIGH 1
#define MFB_LEVEL_LOW 0

struct mfb_keys_data
{
    struct k_work_delayable work;
    k_work_handler_t handler;
    uint32_t debounce_interval_ms;
    uint8_t level;
    T_MFB_KEY value_report;
    T_MFB_KEY_CALLBACK user_callback;
};
static struct mfb_keys_data mfb_key;

#include "trace.h"
static void app_mfb_callback(void)
{
    if (mfb_key.value_report.current_state == MFB_KEY_RELEASED ||
        mfb_key.value_report.current_state == MFB_KEY_INVALID)
    {
        mfb_key.level = mfb_get_level();
        if (mfb_key.level == MFB_LEVEL_LOW)
        {
            mfb_irq_disable();
            app_dlps_disable(APP_DLPS_ENTER_CHECK_BUTTON);
            k_work_reschedule(&mfb_key.work, K_MSEC(mfb_key.debounce_interval_ms));
        }
    }

    if (mfb_key.value_report.current_state == MFB_KEY_PRESSED)
    {
        mfb_key.level = mfb_get_level();
        if (mfb_key.level == MFB_LEVEL_HIGH)
        {
            mfb_irq_disable();
            k_work_reschedule(&mfb_key.work, K_MSEC(mfb_key.debounce_interval_ms));
        }
    }

    // DBG_DIRECT("app_mfb_callback %d mfb_key.value_report.current_state %d", mfb_key.level, mfb_key.value_report.current_state);
}


static void mfb_work_handler(struct k_work *work)
{
    uint8_t new_value;
    new_value = mfb_get_level();
    // printk("last level = %d\n", mfb_key.level);
    if (mfb_key.value_report.current_state == MFB_KEY_RELEASED ||
        mfb_key.value_report.current_state == MFB_KEY_INVALID)
    {
        if (new_value == MFB_LEVEL_LOW)
        {
            mfb_key.value_report.current_state = MFB_KEY_PRESSED;
            mfb_key.value_report.press_timestamp = k_uptime_get_32();
            mfb_irq_enable();
        }
        else
        {
            mfb_irq_enable();
        }
        // app_dlps_enable(APP_DLPS_ENTER_CHECK_BUTTON);
    }

    if (mfb_key.value_report.current_state == MFB_KEY_PRESSED)
    {
        if (new_value == MFB_LEVEL_HIGH)
        {
            mfb_key.value_report.current_state = MFB_KEY_RELEASED;
            mfb_key.value_report.release_timestamp = k_uptime_get_32();
            mfb_irq_enable();
            if (mfb_key.user_callback)
            {
                mfb_key.user_callback(&mfb_key.value_report);
            }
            app_dlps_enable(APP_DLPS_ENTER_CHECK_BUTTON);
        }
        else
        {
            mfb_irq_enable();
        }
    }

    // DBG_DIRECT("mfb_work_handler: new_value = %d current_state = %d press_timestamp = %d release_timestamp = %d\n",
    //     new_value, mfb_key.value_report.current_state, mfb_key.value_report.press_timestamp, mfb_key.value_report.release_timestamp);

}

void app_mfb_init(void)
{
    mfb_init(app_mfb_callback);
    mfb_key.handler = mfb_work_handler;
    mfb_key.debounce_interval_ms = 30;

    k_work_init_delayable(&mfb_key.work, mfb_key.handler);
}

T_MFB_KEY app_mfb_get_level(void)
{
    return mfb_key.value_report;
}

void app_mfb_register_callback(T_MFB_KEY_CALLBACK user_callback)
{
    mfb_key.user_callback = user_callback;
}

