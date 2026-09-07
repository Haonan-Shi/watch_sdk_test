/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_PWM_OUTPUT_SUPPORT

#include <stddef.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "app_key_detect.h"
#include "app_pwm_output.h"
#include "trace.h"
#include "rtl876x_pinmux.h"
#include "usbh_mgr.h"
#include "pm.h"
//P5_0: input with pull-up, detect host connection
static const struct gpio_dt_spec key_input =
{
    .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)),
    .pin = 15,
    .dt_flags = GPIO_PULL_UP,
};
//P5_3: output, HIGH when host connected, LOW when host disconnected
static const struct gpio_dt_spec key_output =
{
    .port = DEVICE_DT_GET(DT_NODELABEL(gpiob)),
    .pin = 18,
    .dt_flags = GPIO_ACTIVE_HIGH,
};

static struct gpio_callback key_gpio_cb;
static struct k_timer confirm_timer;
static volatile bool falling_pending;

typedef enum
{
    KEY_STATE_IDLE,
    KEY_STATE_CONFIRMED,
} key_state_t;

static key_state_t key_state = KEY_STATE_IDLE;

bool app_key_detect_get_host_status(void)
{
    return (key_state == KEY_STATE_CONFIRMED);
}

void app_key_detect_host_connected(void)
{
    app_pwm_output_stop();
    gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                 GPIO_INT_MODE_DISABLE_ONLY);
}

void app_key_detect_host_disconnected(void)
{
    app_pwm_output_start();
    gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                 GPIO_INT_EDGE_FALLING);
}

void app_key_detect_get_init_status(void)
{
    int level;

    level = gpio_pin_get(key_input.port, key_input.pin);
    if (level == 0)
    {
        APP_PRINT_INFO0("key_detect: key_detect_isr falling edge, start confirm timer init");
        falling_pending = true;
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_MODE_DISABLE_ONLY);
    }
}
static void key_detect_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);
    uint32_t actual_mhz = 0;
    if (key_state == KEY_STATE_CONFIRMED)
    {
        /* rising edge: device removed, restore everything */
        key_state = KEY_STATE_IDLE;
        falling_pending = false;
        gpio_pin_set_dt(&key_output, 0);
        usbh_mgr_stop();
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_EDGE_FALLING);
        // pm_cpu_freq_set(40, &actual_mhz);

    }
    else
    {
        /* falling edge: device insert pending */
        APP_PRINT_INFO0("key_detect: key_detect_isr falling edge, start confirm timer");

        falling_pending = true;
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_MODE_DISABLE_ONLY);
    }
}

static void confirm_timer_cb(struct k_timer *timer_id)
{
    int level;
    uint32_t actual_mhz = 0;
    level = gpio_pin_get(key_input.port, key_input.pin);
    if (level == 0)
    {
        APP_PRINT_INFO0("key_detect: confirmed, P5_3 HIGH");
        key_state = KEY_STATE_CONFIRMED;

        pm_cpu_freq_set(200, &actual_mhz);

        usbh_mgr_start();
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_EDGE_RISING);
        gpio_pin_set_dt(&key_output, 1);

    }
    else
    {
        APP_PRINT_INFO0("key_detect: false alarm, P5_3 LOW");
        falling_pending = false;
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_EDGE_FALLING);
    }
}

void app_key_detect_poll(void)
{
    int level;

    level = gpio_pin_get(key_input.port, key_input.pin);
    APP_PRINT_INFO2("key_detect: app_key_detect_poll %d %d", falling_pending, level);
    if (level == 0 && key_state == KEY_STATE_IDLE)
    {
        k_timer_start(&confirm_timer, K_USEC(10000), K_NO_WAIT);
    }
    if (falling_pending && level != 0)
    {
        gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                     GPIO_INT_EDGE_FALLING);
    }
    falling_pending = false;

}

void app_key_detect_init(void)
{
    int ret;

    APP_PRINT_INFO0("key_detect: init P5_0 input pull-up, P5_3 output low");

    if (!device_is_ready(key_input.port))
    {
        APP_PRINT_ERROR0("key_detect: GPIO device not ready");
        return;
    }

    /* P5_0: input with pull-up */
    ret = gpio_pin_configure_dt(&key_input, GPIO_INPUT);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_detect: P5_0 configure failed %d", ret);
        return;
    }

    /* P5_3: output, initially low */
    ret = gpio_pin_configure_dt(&key_output, GPIO_OUTPUT_LOW);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_detect: P5_3 configure failed %d", ret);
        return;
    }

    k_timer_init(&confirm_timer, confirm_timer_cb, NULL);

    falling_pending = false;

    gpio_init_callback(&key_gpio_cb, key_detect_isr, BIT(key_input.pin));
    ret = gpio_add_callback(key_input.port, &key_gpio_cb);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_detect: add callback failed %d", ret);
        return;
    }

    ret = gpio_pin_interrupt_configure(key_input.port, key_input.pin,
                                       GPIO_INT_EDGE_FALLING);
    if (ret < 0)
    {
        APP_PRINT_ERROR1("key_detect: interrupt configure failed %d", ret);
        return;
    }

    APP_PRINT_INFO0("key_detect: init done, waiting for falling edge on P5_0");
}


#endif /* F_APP_PWM_OUTPUT_SUPPORT */
