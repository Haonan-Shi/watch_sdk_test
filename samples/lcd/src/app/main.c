/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/display.h>
#include <ctype.h>
#include <stdio.h>

#include "touch_CHSC6417_zephyr.h"
#include "trace.h"

#ifdef CONFIG_ARCH_POSIX
#include <unistd.h>
#else
#include <zephyr/posix/unistd.h>
#endif

LOG_MODULE_REGISTER(app);

#include <zephyr/device.h>
#include "app_lower_init.h"

static const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
static const struct device *touch_dev = DEVICE_DT_GET(DT_NODELABEL(touch_device));

static struct display_capabilities cfg;
static uint8_t bpp;

static int64_t last_activity_time = 0;
#define SCREEN_TIMEOUT_MS 5000  /* 5s no touch auto off screen */

/* screen state */
static bool is_screen_on = true;

/* touch count */
static uint32_t touch_count = 0;

int main(void)
{
    DBG_DIRECT("\n=== LCD Touch Demo ===");
    DBG_DIRECT("Task: %s", k_thread_name_get(k_current_get()));

    app_system_lower_init();

    /* display */
    if (!device_is_ready(dev))
    {
        DBG_DIRECT("ERROR: Display device not ready!");
        return -1;
    }
    DBG_DIRECT("[OK] Display device ready");

    /* touch device */
    if (!device_is_ready(touch_dev))
    {
        DBG_DIRECT("WARNING: Touch device not ready!");
        DBG_DIRECT("  Touch functionality will be disabled.");
    }
    else
    {
        DBG_DIRECT("[OK] Touch device ready");
    }

    /* display capabilities */
    display_get_capabilities(dev, &cfg);
    DBG_DIRECT("Display: %dx%d", cfg.x_resolution, cfg.y_resolution);

    switch (cfg.current_pixel_format)
    {
    case PIXEL_FORMAT_RGB_565: bpp = 2; break;
    case PIXEL_FORMAT_RGB_888: bpp = 3; break;
    case PIXEL_FORMAT_ARGB_8888: bpp = 4; break;
    default: bpp = 2; break;
    }
    DBG_DIRECT("Pixel format: %d bpp", bpp);

    /* initial screen on */
    display_blanking_off(dev);
    is_screen_on = true;
    last_activity_time = k_uptime_get();

    /* draw test pattern */
    uint32_t block_w = 100;
    uint32_t block_h = 100;
    size_t buf_size = block_w * block_h * bpp;

    uint8_t *image = malloc(buf_size);
    if (image)
    {
        memset(image, 0xF8, buf_size);  /* red block */
        struct display_buffer_descriptor desc =
        {
            .height = block_h,
            .pitch = block_w,
            .width = block_w,
            .buf_size = buf_size
        };
        display_write(dev, 50, 50, &desc, image);
        free(image);
        DBG_DIRECT("[OK] Test pattern drawn");
    }

    DBG_DIRECT("\n=== System Ready ===");
    DBG_DIRECT("Touch to wake screen, auto sleep after %d ms", SCREEN_TIMEOUT_MS);

    /* main loop: poll touch and manage screen state */
    uint8_t *test_image = malloc(buf_size);
    if (test_image)
    {
        memset(test_image, 0xF8, buf_size);
    }

    while (1)
    {
        int64_t now = k_uptime_get();
        bool touch_detected = false;
        static bool last_press_state = false;

        /* loop: poll touch data */
        if (device_is_ready(touch_dev))
        {
            TOUCH_DATA t_data = get_raw_touch_data(touch_dev);

            if (t_data.is_press)
            {
                touch_detected = true;

                /* check new touch event (press edge) */
                if (!last_press_state)
                {
                    touch_count++;
                    DBG_DIRECT("Touch #%d: x=%d, y=%d", touch_count, t_data.x, t_data.y);
                }
                last_press_state = true;
            }
            else
            {
                last_press_state = false;
            }
        }

        /* screen state management */
        if (touch_detected)
        {
            /* touch activity detected, refresh timestamp */
            last_activity_time = now;

            if (!is_screen_on)
            {
                DBG_DIRECT(">>> Screen WAKE UP by touch <<<");
                display_blanking_off(dev);

                /* redraw test pattern */
                if (test_image)
                {
                    struct display_buffer_descriptor desc =
                    {
                        .height = block_h,
                        .pitch = block_w,
                        .width = block_w,
                        .buf_size = buf_size
                    };
                    display_write(dev, 50, 50, &desc, test_image);
                }

                is_screen_on = true;
            }
        }
        else
        {
            /* no touch activity, check timeout */
            if (is_screen_on && (now - last_activity_time > SCREEN_TIMEOUT_MS))
            {
                DBG_DIRECT(">>> Screen AUTO OFF (timeout) <<<");
                display_blanking_on(dev);
                is_screen_on = false;
            }
        }

        /* sleep 50ms, balance response speed and CPU usage */
        k_sleep(K_MSEC(50));
    }

    if (test_image)
    {
        free(test_image);
    }

    return 0;
}


