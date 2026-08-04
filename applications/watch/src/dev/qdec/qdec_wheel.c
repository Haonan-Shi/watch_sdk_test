/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/sensor/qdec_rtl87x3g.h>
#include "qdec_wheel.h"

// get qdec device
const struct device *const dev = DEVICE_DT_GET(DT_ALIAS(qdec0));

// qdec data for wheel algo
T_QDEC_DATA qdec_data;

// qdec data read
T_QDEC_DATA qdec_wheel_read_data(void)
{
    return qdec_data;
}

void qdec_callback(const struct device *dev, const struct sensor_trigger *trigger)
{
    struct sensor_value val;

    sensor_sample_fetch(dev);
    sensor_channel_get(dev, SENSOR_ATTR_QDEC_X_ROTATION, &val);
    qdec_data.count = val.val1;
    qdec_data.timestamp = k_uptime_get();
}

// qdec data init
void qdec_wheel_init(void)
{
    memset((uint8_t *)&qdec_data, 0x00, sizeof(qdec_data));

    if (!device_is_ready(dev))
    {
        //APP_PRINT_ERROR0("Qdec device is not ready\n");
        return;
    }

    struct sensor_trigger trig;

    trig.type = SENSOR_TRIG_DATA_READY;
    trig.chan = SENSOR_ATTR_QDEC_X_ROTATION;

    sensor_trigger_set(dev, &trig, qdec_callback);

}

