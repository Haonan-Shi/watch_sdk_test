/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if  F_APP_EXT_AUDIO_AMP_SUPPORT
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "app_ext_audio_amp.h"
#include "app_cfg.h"
#include "audio_plugin.h"
#include "app_timer.h"
#include "trace.h"

#define EXT_AMP_NODE                DT_ALIAS(ext_amp)
#define EXT_AMP_GPIO_CTLR           DT_GPIO_CTLR(EXT_AMP_NODE, gpios)
#define EXT_AMP_GPIO_PIN            DT_GPIO_PIN(EXT_AMP_NODE, gpios)
#define EXT_AMP_GPIO_FLAGS          DT_GPIO_FLAGS(EXT_AMP_NODE, gpios)

static const struct device *ext_amp_gpio_port = DEVICE_DT_GET(EXT_AMP_GPIO_CTLR);

typedef enum t_app_amp_state
{
    APP_AMP_STATE_OFF = 0,
    APP_AMP_STATE_WAIT_OFF,
    APP_AMP_STATE_WAIT_ON,
    APP_AMP_STATE_ON,
    APP_AMP_STATE_NUM,
} T_APP_AMP_STATE;

typedef enum t_app_amp_timer
{
    APP_TIMER_AMP_PRE_ON_GUARD_TIME = 0,
    APP_TIMER_AMP_POST_ON_GUARD_TIME,
    APP_TIMER_AMP_OFF_GUARD_TIME,
    APP_TIMER_AMP_NUM,
} T_APP_AMP_TIMER;

typedef struct t_app_amp_db
{
    T_APP_AMP_STATE             state;
    uint8_t                     ref_cnt;
    T_AUDIO_PLUGIN_HANDLE       plugin_handle;
    void                       *poweron_context;
    void                       *poweroff_context;
} T_APP_AMP_DB;

typedef struct
{
    uint8_t amp_post_on_guard_time;
    uint8_t amp_pre_on_guard_time;
    uint8_t amp_off_guard_time;
} T_APP_AMP_TIMEOUT_PARAMETER;

static T_APP_AMP_DB app_amp_db = { .state = APP_AMP_STATE_OFF, 0 };

static uint8_t timer_idx_amp = 0;
static uint8_t amp_timer_id = 0;

const T_APP_AMP_TIMEOUT_PARAMETER     amp_const_timeout =
{
    .amp_post_on_guard_time = 1,
    .amp_pre_on_guard_time = 1,
    .amp_off_guard_time = 1,
};

static void app_amp_send_to_plugin(void *context)
{
    APP_PRINT_TRACE1("app_amp_send_to_plugin: context %p", context);
    audio_plugin_msg_send(app_amp_db.plugin_handle, context);
}

static void app_amp_control(uint8_t activate_fg)
{
    APP_PRINT_INFO1("app_amp_control activate_fg = %d", activate_fg);

    if (activate_fg) //Turn on pa, switch pinmux mode before gpio write
    {
        gpio_pin_set(ext_amp_gpio_port, EXT_AMP_GPIO_PIN, 1); //logical level, depends on EXT_AMP_GPIO_FLAGS
    }
    else //Turn off pa, switch sw mode for dlps
    {
        gpio_pin_set(ext_amp_gpio_port, EXT_AMP_GPIO_PIN, 0); //logical level, depends on EXT_AMP_GPIO_FLAGS
    }
}

static void app_amp_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("app_amp_timeout_cb: timer_evt 0x%02x, param %d", timer_evt, param);

    app_stop_timer(&timer_idx_amp);
    switch (timer_evt)
    {
    case APP_TIMER_AMP_PRE_ON_GUARD_TIME:
        {
            app_amp_control(1);
            app_start_timer(&timer_idx_amp, "amp_post_on_guard",
                            amp_timer_id, APP_TIMER_AMP_POST_ON_GUARD_TIME, 0, false,
                            amp_const_timeout.amp_post_on_guard_time * 100);
        }
        break;

    case APP_TIMER_AMP_POST_ON_GUARD_TIME:
        {
            app_amp_db.state = APP_AMP_STATE_ON; /* amp is ready to run */
            app_amp_send_to_plugin(app_amp_db.poweron_context);
        }
        break;

    case APP_TIMER_AMP_OFF_GUARD_TIME:
        {
            app_amp_db.state = APP_AMP_STATE_OFF;
            app_amp_send_to_plugin(app_amp_db.poweroff_context);
        }
        break;

    default:
        break;
    }
}

static void app_amp_poweron(T_AUDIO_PLUGIN_HANDLE handle, T_AUDIO_PLUGIN_PARAM param, void *context)
{
    /* Action description: Check whether amp is enabled. If so, inform back to audio path immediately.
                           Otherwise, start the ENABLE procedure and inform back when
                           amp becomes enabled. */
    app_amp_db.ref_cnt++;
    if (app_amp_db.state == APP_AMP_STATE_ON)
    {
        app_amp_send_to_plugin(context);
    }
    else
    {
        app_stop_timer(&timer_idx_amp);
        app_amp_db.state = APP_AMP_STATE_WAIT_ON;
        app_amp_db.poweron_context = context;
        app_start_timer(&timer_idx_amp, "amp_pre_on_guard",
                        amp_timer_id, APP_TIMER_AMP_PRE_ON_GUARD_TIME, 0, false,
                        amp_const_timeout.amp_pre_on_guard_time * 100);
    }
}

static void app_amp_run(T_AUDIO_PLUGIN_HANDLE handle, T_AUDIO_PLUGIN_PARAM param, void *context)
{
    /* Action description: Inform back to audio path when amp is enabled. */
    if (app_amp_db.state == APP_AMP_STATE_ON)
    {
        app_amp_send_to_plugin(context);
    }
}

static void app_amp_poweroff(T_AUDIO_PLUGIN_HANDLE handle, T_AUDIO_PLUGIN_PARAM param,
                             void *context)
{
    /* Action description: If the condition to power off is satisfied, start the POWEROFF procedure
                           and inform back when the procedure is done. Otherwise, inform back immediatelly.
    */
    if (app_amp_db.ref_cnt)
    {
        app_amp_db.ref_cnt--;
        if (app_amp_db.ref_cnt == 0)
        {
            app_amp_control(0);
            app_stop_timer(&timer_idx_amp);
            app_amp_db.state = APP_AMP_STATE_WAIT_OFF;
            app_amp_db.poweroff_context = context;
            app_start_timer(&timer_idx_amp, "amp_off_guard",
                            amp_timer_id, APP_TIMER_AMP_OFF_GUARD_TIME, 0, false,
                            amp_const_timeout.amp_off_guard_time * 100);
        }
        else
        {
            app_amp_send_to_plugin(context);
        }
    }
}

static const T_AUDIO_PLUGIN_POLICY app_amp_policies[] =
{
    /* category */          /* occasion */                           /* action handler */
    { AUDIO_CATEGORY_AUDIO, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_VOICE, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_LINE, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_TONE, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_VP, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_APT, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_LLAPT, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },
    { AUDIO_CATEGORY_ANC, AUDIO_PLUGIN_OCCASION_ANALOG_DOMAIN_ON, app_amp_poweron },

    { AUDIO_CATEGORY_AUDIO, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_VOICE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_LINE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_TONE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_VP, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_APT, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_LLAPT, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },
    { AUDIO_CATEGORY_ANC, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_ON, app_amp_run },

    { AUDIO_CATEGORY_AUDIO, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_VOICE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_LINE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_TONE, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_VP, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_APT, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_LLAPT, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
    { AUDIO_CATEGORY_ANC, AUDIO_PLUGIN_OCCASION_DIGITAL_DOMAIN_OFF, app_amp_poweroff },
};

bool app_ext_audio_amp_init(void)
{
    int ret = 0;
    if (!device_is_ready(ext_amp_gpio_port))
    {
        APP_PRINT_ERROR0("Ext AMP GPIO is not ready\n");
        ret = 1;
    }
    gpio_pin_configure(ext_amp_gpio_port, EXT_AMP_GPIO_PIN, GPIO_OUTPUT | EXT_AMP_GPIO_FLAGS);

    if (app_timer_reg_cb(app_amp_timeout_cb, &amp_timer_id) != 0)
    {
        ret = 2;
        goto fail_reg_amp_timer;
    }

    app_amp_db.plugin_handle = audio_plugin_create(app_amp_policies,
                                                   sizeof(app_amp_policies) / sizeof(app_amp_policies[0]));
    if (app_amp_db.plugin_handle == NULL)
    {
        ret = 3;
        goto fail_create_plugin;
    }

    return true;

fail_create_plugin:
fail_reg_amp_timer:
    APP_PRINT_TRACE1("app_amp_init: failed %d", -ret);

    return false;
}
#endif
