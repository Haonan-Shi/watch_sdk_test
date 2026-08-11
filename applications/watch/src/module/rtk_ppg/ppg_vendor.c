/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "string.h"
#include <trace.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include "RTL87x5PPG.h"
#include "os_mem.h"
#include "platform_utils.h"

#define HRS_I2C_DEV_NODE            DT_ALIAS(i2c0)
#define HRS_DEBUG                   0

#define RTL87X5_ADDR                0x54

static const struct device *hrs_i2c_dev = DEVICE_DT_GET(HRS_I2C_DEV_NODE);

bool hrs_read_reg(uint16_t reg, uint8_t *data, uint16_t len)
{
    int ret = 0;
    ret = i2c_write(hrs_i2c_dev, (const uint8_t *)&reg, sizeof(reg), RTL87X5_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! hrs_read i2c_write: %d", ret);
        return false;
    }
    k_busy_wait(1);

    ret = i2c_read(hrs_i2c_dev, data, len, RTL87X5_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! hrs_read i2c_read: %d", ret);
        return false;
    }
    return true;
}

bool hrs_write_data(uint8_t *data, uint16_t len)
{
    int ret = 0;

    ret = i2c_write(hrs_i2c_dev, data, len, RTL87X5_ADDR);
    if (ret != 0)
    {
        APP_PRINT_TRACE1("ERROR! hrs_write i2c_write: %d", ret);
        return false;
    }
    return true;
}

void RTL87x5WriteMultiReg(uint16_t startAddress, uint16_t len, uint32_t *p_word_data)
{
    uint8_t *write_buf = NULL;

    if (len > 0)
    {
        write_buf = os_mem_alloc(OS_MEM_TYPE_DATA, len * 4 + 2);
    }

    if (write_buf != NULL)
    {
        uint8_t *start_buf = write_buf;
        write_buf[0] = startAddress >> 8;
        write_buf[1] = startAddress & 0xFF;
        write_buf += 2;
        for (uint16_t i = 0; i < len; i++)
        {
            for (uint8_t j = 4; j > 0; j--)
            {
                *write_buf = (p_word_data[i] >> (8 * (j - 1)));
                write_buf += 1;
            }
        }
        //memcpy(write_buf + 2, p_word_data, len);
#if HRS_DEBUG
        uint8_t log_len = len * 4;
        uint8_t *log_buf = start_buf + 2;
        APP_PRINT_TRACE2("CHI I2C write, addr: %x, data: %b", startAddress, TRACE_BINARY(log_len, log_buf));
#endif
        hrs_write_data(start_buf, (len * 4) + 2);

        os_mem_free(start_buf);
    }
}

void RTL87x5ReadMultiReg(uint16_t startAddress, uint16_t len, uint32_t *p_word_data)
{
    if (len == 0)
    {
        return;
    }

    uint8_t *read_buf = NULL;
    uint8_t *free_read_buf = NULL;
    read_buf = os_mem_alloc(OS_MEM_TYPE_DATA, len * 4);
    free_read_buf = read_buf;
    uint16_t reg = (startAddress << 8) | (startAddress >> 8);

    if (read_buf != NULL)
    {
        if (hrs_read_reg(reg, read_buf, len * 4))
        {
#if HRS_DEBUG
            uint8_t log_len = len * 4;
            APP_PRINT_TRACE2("CHI I2C read, addr: %x, data: %b", startAddress, TRACE_BINARY(log_len, read_buf));
#endif
            for (uint16_t i = 0; i < len; i++)
            {
                //uint8_t *write_buf = (uint8_t*)&p_word_data[i];
                p_word_data[i] = 0;
                //APP_PRINT_INFO1("cheat read fifo %b", TRACE_BINARY(4, read_buf));
                for (uint8_t j = 4; j > 0; j--)
                {
                    p_word_data[i] |= (uint32_t)(*read_buf) << (8 * (j - 1));
                    read_buf++;
                }
            }
            os_mem_free(free_read_buf);
        }
    }
}

void RTL87x5ReadMultiReg_8bit(uint16_t startAddress, uint16_t len, uint8_t *p_word_data)
{
    if (len == 0)
    {
        return;
    }

    uint16_t reg = (startAddress << 8) | (startAddress >> 8);
    hrs_read_reg(reg, p_word_data, len * 4);
}

void print_dbg_msg(const char *format, ...)
{
#if HRS_DEBUG
    char tx_buffer[256];
    va_list args;
    va_start(args, format);
    int n = vsnprintf((char *)tx_buffer, 256, format, args);
    va_end(args);
    DBG_BUFFER_INTERNAL(SUBTYPE_FORMAT, MODULE_APP, LEVEL_ERROR, "[hrs] %s", 1,
                        TRACE_STRING(tx_buffer));
#endif
}

void RTL87x5DelayMS(uint32_t t)
{
    platform_delay_ms(t);
}

void RTL87x5DelayUS(uint32_t t)
{
    platform_delay_us(t);
}
