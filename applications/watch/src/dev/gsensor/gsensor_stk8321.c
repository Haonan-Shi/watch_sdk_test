/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <trace.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/gpio.h>
#include "string.h"
#include "hub_gsensor.h"
#include "platform_utils.h"


#define GSENSOR_I2C_DEV_NODE                DT_ALIAS(i2c0)
#define GSENSOR_ADDR                        0x1F

static const struct device *gsensor_i2c_dev = DEVICE_DT_GET(GSENSOR_I2C_DEV_NODE);

void gsensor_read(uint8_t reg, uint8_t *p_data, uint16_t len)
{
    int ret = 0;
    ret = i2c_write(gsensor_i2c_dev, &reg, 1, GSENSOR_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! gsensor_read i2c_write: %d", ret);
    }
    platform_delay_us(1);

    ret = i2c_read(gsensor_i2c_dev, p_data, len, GSENSOR_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! gsensor_read i2c_read: %d", ret);
    }
}

void gsensor_write(uint8_t reg, uint8_t data)
{
    int ret = 0;
    uint8_t i2c_writebuf[2];
    i2c_writebuf[0] = reg;
    i2c_writebuf[1] = data;

    ret = i2c_write(gsensor_i2c_dev, i2c_writebuf, 2, GSENSOR_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! gsensor_write i2c_write: %d", ret);
    }
}

uint8_t gsensor_get_fifo_length(void)
{
    uint8_t len;
    gsensor_read(0x0C, &len, 1);
    len = len & 0x7f;

    return len;
}

bool gsensor_get_fifo_data(uint8_t len, AxesRaw_t *buf)
{
    uint8_t shift_num = 4;

    if ((len > 0) && (len <= 32))
    {
        gsensor_read(0x3F, (uint8_t *)buf, 6 * len);
        for (uint8_t i = 0; i < len; i++)
        {
            buf[i].AXIS_X = buf[i].AXIS_X >> shift_num;
            buf[i].AXIS_Y = buf[i].AXIS_Y >> shift_num;
            buf[i].AXIS_Z = buf[i].AXIS_Z >> shift_num;
        }
        return true;
    }
    else
    {
        return false;
    }
}


void gsensor_init(void)
{
    if (!device_is_ready(gsensor_i2c_dev))
    {
        APP_PRINT_ERROR0("Error: i2c device is not not ready!\n");
        return;
    }

    uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;

    if (i2c_configure(gsensor_i2c_dev, i2c_cfg))
    {
        APP_PRINT_ERROR0("Error: Failed to configure tp i2c\n");
        return;
    }

    /* reset stk8321 reg table to its default */
    /* fifo cfg defautl: bypass mode */
    /* filtered ODR default = 1kHz */
    gsensor_write(0x14, 0xb6);
    gsensor_write(0x11, 0x76);
    gsensor_write(0x12, 0x08);
    gsensor_write(0x0F, 0x05);
    gsensor_write(0x10, 0x0F);
    gsensor_write(0x34, 0x04);
    gsensor_write(0x3D, 0x20);
    gsensor_write(0x3E, 0x20);
    gsensor_write(0x19, 0x01);
    gsensor_write(0x1A, 0x40);
    gsensor_write(0x21, 0x8F);
    gsensor_write(0x20, 0x05);
    gsensor_write(0x17, 0x00);
}

void gsensor_enable(void)
{
    gsensor_init();
}

void gsensor_disable(void)
{
    gsensor_write(0x11, 0x80);// Deep Supend Mode
}
