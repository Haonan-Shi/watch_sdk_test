/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_PWM_OUTPUT_SUPPORT

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include "app_pwm_output.h"
#include "app_dlps.h"
#include "app_key_detect.h"

LOG_MODULE_REGISTER(app_pwm_output, LOG_LEVEL_INF);
extern void app_key_detect_get_init_status(void);

#define PWM_CTLR_NODE  DT_NODELABEL(pwm6)
#define PWM_CHANNEL    0
#define PWM_POLARITY   0

static const struct device *const pwm_dev = DEVICE_DT_GET(PWM_CTLR_NODE);
static const struct device *const gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpiob));

#define PWM_DEFAULT_FREQ_HZ     100000
#define PWM_DEFAULT_DUTY_PCT    50
#define PWM_PHASE_MS            100   /* duration of PWM-output phase */
#define HIGH_PHASE_MS           200   /* duration of GPIO-HIGH phase */

typedef enum
{
    HOST_PHASE_PWM,
    HOST_PHASE_HIGH,
} host_phase_t;

static host_phase_t current_phase = HOST_PHASE_PWM;
static bool          module_running = false;

static uint64_t current_period_ns;
static uint64_t current_pulse_ns;

static struct k_timer phase_timer;

static void pin_set_pwm_func_high(void)
{
    int ret;

    ret = pwm_set(pwm_dev, PWM_CHANNEL, current_period_ns, current_period_ns, PWM_POLARITY);
    if (ret < 0)
    {
        LOG_ERR("pwm_set() failed: %d", ret);
    }
}

static void pin_set_pwm_func(void)
{
    int ret;

    ret = pwm_set(pwm_dev, PWM_CHANNEL, current_period_ns, current_pulse_ns, PWM_POLARITY);
    if (ret < 0)
    {
        LOG_ERR("pwm_set() failed: %d", ret);
    }
}

static void phase_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    if (current_phase == HOST_PHASE_PWM)
    {
        pin_set_pwm_func_high();
        current_phase = HOST_PHASE_HIGH;
        k_timer_start(&phase_timer, K_MSEC(HIGH_PHASE_MS), K_FOREVER);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_PWM);
    }
    else
    {
        app_dlps_disable(APP_DLPS_ENTER_CHECK_PWM);
        pin_set_pwm_func();
        app_key_detect_poll();
        current_phase = HOST_PHASE_PWM;
        k_timer_start(&phase_timer, K_MSEC(PWM_PHASE_MS), K_FOREVER);
    }

    LOG_INF("phase_switch_handler: current_phase %d", current_phase);
}

void app_pwm_output_init(void)
{
    int ret;

    if (module_running)
    {
        return;
    }

    if (!device_is_ready(pwm_dev))
    {
        LOG_ERR("PWM device is not ready");
        return;
    }

    if (!device_is_ready(gpio_dev))
    {
        LOG_ERR("GPIO device is not ready");
        return;
    }

    current_period_ns = PWM_HZ(PWM_DEFAULT_FREQ_HZ);
    current_pulse_ns  = (current_period_ns * PWM_DEFAULT_DUTY_PCT) / 100;
    app_dlps_disable(APP_DLPS_ENTER_CHECK_PWM);

    ret = pwm_set(pwm_dev, PWM_CHANNEL, current_period_ns, current_pulse_ns, PWM_POLARITY);
    if (ret < 0)
    {
        LOG_ERR("pwm_set_dt() init failed: %d", ret);
        return;
    }

    k_timer_init(&phase_timer, phase_timer_handler, NULL);

    current_phase = HOST_PHASE_PWM;
    k_timer_start(&phase_timer, K_MSEC(PWM_PHASE_MS), K_FOREVER);
    module_running = true;

    LOG_INF("pwm_output: init ok, freq=%uHz duty=%u%% "
            "cycle=%umsPWM+%umsHIGH",
            PWM_DEFAULT_FREQ_HZ, PWM_DEFAULT_DUTY_PCT,
            PWM_PHASE_MS, HIGH_PHASE_MS);

    app_key_detect_get_init_status();
}

void app_pwm_output_set(uint32_t freq_hz, uint8_t duty_percent)
{
    if (!device_is_ready(pwm_dev))
    {
        return;
    }

    if (duty_percent > 100)
    {
        duty_percent = 100;
    }

    current_period_ns = PWM_HZ(freq_hz);
    current_pulse_ns  = (current_period_ns * duty_percent) / 100;

    /* If module is running and currently in PWM phase, apply immediately */
    if (module_running && current_phase == HOST_PHASE_PWM)
    {
        int ret = pwm_set(pwm_dev, PWM_CHANNEL, current_period_ns, current_pulse_ns, PWM_POLARITY);
        if (ret < 0)
        {
            LOG_ERR("pwm_set() failed: %d", ret);
        }
    }

    LOG_INF("pwm_output: set freq=%uHz duty=%u%%", freq_hz, duty_percent);
}

void app_pwm_output_start(void)
{
    if (!device_is_ready(pwm_dev) || !device_is_ready(gpio_dev))
    {
        LOG_ERR("app_pwm_output_start: devices not ready");
        return;
    }

    app_dlps_disable(APP_DLPS_ENTER_CHECK_PWM);

    k_timer_stop(&phase_timer);
    current_phase = HOST_PHASE_PWM;
    pin_set_pwm_func();
    k_timer_start(&phase_timer, K_MSEC(PWM_PHASE_MS), K_NO_WAIT);
    module_running = true;
}

void app_pwm_output_stop(void)
{
    if (!device_is_ready(pwm_dev) || !device_is_ready(gpio_dev))
    {
        return;
    }

    k_timer_stop(&phase_timer);
    pin_set_pwm_func_high();   /* Hold pin HIGH when stopped */
    module_running = false;
    app_dlps_enable(APP_DLPS_ENTER_CHECK_PWM);

    LOG_INF("app_pwm_output_stop: module_running=%d", module_running);
}

bool app_pwm_output_is_active(void)
{
    return module_running;
}

#endif /* F_APP_PWM_OUTPUT_SUPPORT */
