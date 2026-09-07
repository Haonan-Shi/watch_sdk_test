/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "os_mem.h"
#include "string.h"
#include "trace.h"
#include "section.h"
#include "rtl876x_pinmux.h"
#include "hw_tim.h"
#include "hal_gpio.h"
#include "hal_gpio_int.h"
#include "sw_keyscan.h"

typedef struct
{
    T_KEYSCAN_INIT_PARA keyscan_para;
    T_KEYSCAN_DATA keyscan_data;
    bool keyscan_debounce_timer_status;
    bool keyscan_scan_interval_timer_status;
    bool keyscan_release_timer_status;
    uint16_t *filter_data;
    uint8_t filter_data_size;
    bool keyscan_any_key_pressed;
    T_HW_TIMER_HANDLE keyscan_timer_handle;
} T_KEYSCAN_MANAGER;

static T_KEYSCAN_MANAGER keyscan_manager;

RAM_TEXT_SECTION
static void hal_gpio_open_drian_output_high(uint8_t pin_index)
{
    hal_gpio_change_direction(pin_index, GPIO_DIR_INPUT);
}

RAM_TEXT_SECTION
static void hal_gpio_open_drian_output_low(uint8_t pin_index)
{
    hal_gpio_change_direction(pin_index, GPIO_DIR_OUTPUT);
    hal_gpio_set_level(pin_index, GPIO_LEVEL_LOW);
}

ISR_TEXT_SECTION
static void keyscan_gpio_cb(uint32_t context)
{
    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.row_size; i++)
    {
        hal_gpio_irq_disable(keyscan_manager.keyscan_para.keyscan_row_pins[i]);
    }

    keyscan_manager.keyscan_debounce_timer_status = true;
    hw_timer_restart(keyscan_manager.keyscan_timer_handle, KEYSCAN_DEBOUNCE_TIME * 1000);
}

RAM_TEXT_SECTION
static bool keyscan_scan(void)
{
    uint16_t key_index = 0;
    bool keyscan_filter_flag = false;
    keyscan_manager.keyscan_data.length = 0;
    memset(keyscan_manager.keyscan_data.key_data, 0,
           keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size * sizeof(
               uint16_t));

    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.column_size; i++)
    {
        hal_gpio_open_drian_output_high(keyscan_manager.keyscan_para.keyscan_column_pins[i]);
    }

    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.column_size; i++)
    {
        hal_gpio_open_drian_output_low(keyscan_manager.keyscan_para.keyscan_column_pins[i]);

        for (uint8_t j = 0; j < keyscan_manager.keyscan_para.row_size; j++)
        {
            key_index = ((uint16_t)j << 8) | i;
            if (keyscan_manager.filter_data_size)
            {
                for (uint8_t k = 0; k < keyscan_manager.filter_data_size; k++)
                {
                    if (key_index == keyscan_manager.filter_data[k])
                    {
                        keyscan_filter_flag = true;
                        break;
                    }
                }
                if (keyscan_filter_flag)
                {
                    keyscan_filter_flag = false;
                    continue;
                }
            }
            if (hal_gpio_get_input_level(keyscan_manager.keyscan_para.keyscan_row_pins[j]) == GPIO_LEVEL_LOW)
            {
                keyscan_manager.keyscan_data.key_data[keyscan_manager.keyscan_data.length] = key_index;
                keyscan_manager.keyscan_data.length++;
            }
        }

        hal_gpio_open_drian_output_high(keyscan_manager.keyscan_para.keyscan_column_pins[i]);
    }

    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.column_size; i++)
    {
        hal_gpio_open_drian_output_low(keyscan_manager.keyscan_para.keyscan_column_pins[i]);
    }

    return (keyscan_manager.keyscan_data.length == 0) ? false : true;
}

RAM_TEXT_SECTION
static void keyscan_debounce_timer_event_handle(void)
{
    bool key_is_press = false;

    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.row_size; i++)
    {
        if (hal_gpio_get_input_level(keyscan_manager.keyscan_para.keyscan_row_pins[i]) == GPIO_LEVEL_LOW)
        {
            key_is_press = true;
            break;
        }
    }

    if (key_is_press == false)
    {
        for (uint8_t i = 0; i < keyscan_manager.keyscan_para.row_size; i++)
        {
            hal_gpio_irq_enable(keyscan_manager.keyscan_para.keyscan_row_pins[i]);
        }
    }
    else
    {
        bool key_press_status = keyscan_scan();
        keyscan_manager.keyscan_scan_interval_timer_status = true;
        hw_timer_restart(keyscan_manager.keyscan_timer_handle, KEYSCAN_SCAN_INTERVAL_TIME * 1000);
        if (key_press_status == true)
        {
            keyscan_manager.keyscan_any_key_pressed = true;
            if (keyscan_manager.keyscan_para.keyscan_scan_end_cb != NULL)
            {
                keyscan_manager.keyscan_para.keyscan_scan_end_cb(keyscan_manager.keyscan_data);
            }
        }
    }

    keyscan_manager.keyscan_debounce_timer_status = false;
}

RAM_TEXT_SECTION
static void keyscan_scan_interval_timer_event_handle(void)
{
    if (keyscan_scan() == true)
    {
        keyscan_manager.keyscan_any_key_pressed = true;
        hw_timer_restart(keyscan_manager.keyscan_timer_handle, KEYSCAN_SCAN_INTERVAL_TIME * 1000);
        if (keyscan_manager.keyscan_para.keyscan_scan_end_cb != NULL)
        {
            keyscan_manager.keyscan_para.keyscan_scan_end_cb(keyscan_manager.keyscan_data);
        }
    }
    else
    {
        keyscan_manager.keyscan_scan_interval_timer_status = false;
        keyscan_manager.keyscan_release_timer_status = true;
        hw_timer_restart(keyscan_manager.keyscan_timer_handle, KEYSCAN_SCAN_RELEASE_TIME * 1000);
    }
}

RAM_TEXT_SECTION
static void keyscan_release_timer_event_handle(void)
{
    keyscan_manager.keyscan_release_timer_status = false;

    if (keyscan_scan() == true)
    {
        keyscan_manager.keyscan_any_key_pressed = true;
        keyscan_manager.keyscan_scan_interval_timer_status = true;
        hw_timer_restart(keyscan_manager.keyscan_timer_handle, KEYSCAN_SCAN_INTERVAL_TIME * 1000);
        if (keyscan_manager.keyscan_para.keyscan_scan_end_cb != NULL)
        {
            keyscan_manager.keyscan_para.keyscan_scan_end_cb(keyscan_manager.keyscan_data);
        }
    }
    else
    {
        for (uint8_t i = 0; i < keyscan_manager.keyscan_para.row_size; i++)
        {
            hal_gpio_irq_enable(keyscan_manager.keyscan_para.keyscan_row_pins[i]);
        }
        if ((keyscan_manager.keyscan_para.keyscan_release_cb != NULL) &&
            (keyscan_manager.keyscan_any_key_pressed == true))
        {
            keyscan_manager.keyscan_para.keyscan_release_cb();
        }
        keyscan_manager.keyscan_any_key_pressed = false;
    }
}

RAM_TEXT_SECTION
static void keyscan_timer_callback(T_HW_TIMER_HANDLE handle)
{
    if (keyscan_manager.keyscan_debounce_timer_status == true)
    {
        keyscan_debounce_timer_event_handle();
    }
    else if (keyscan_manager.keyscan_scan_interval_timer_status == true)
    {
        keyscan_scan_interval_timer_event_handle();
    }
    else if (keyscan_manager.keyscan_release_timer_status == true)
    {
        keyscan_release_timer_event_handle();
    }
}

static void keyscan_gpio_init(void)
{
    hal_gpio_init();
    hal_gpio_int_init();
    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.column_size; i++)
    {
        hal_gpio_init_pin(keyscan_manager.keyscan_para.keyscan_column_pins[i], GPIO_TYPE_AON,
                          GPIO_DIR_OUTPUT, GPIO_PULL_UP);
        hal_gpio_open_drian_output_low(keyscan_manager.keyscan_para.keyscan_column_pins[i]);
    }

    for (uint8_t i = 0; i < keyscan_manager.keyscan_para.row_size; i++)
    {
        hal_gpio_init_pin(keyscan_manager.keyscan_para.keyscan_row_pins[i], GPIO_TYPE_AUTO, GPIO_DIR_INPUT,
                          GPIO_PULL_UP);
        hal_gpio_set_up_irq(keyscan_manager.keyscan_para.keyscan_row_pins[i], GPIO_IRQ_LEVEL,
                            GPIO_IRQ_ACTIVE_LOW, false);
        hal_gpio_register_isr_callback(keyscan_manager.keyscan_para.keyscan_row_pins[i], keyscan_gpio_cb,
                                       keyscan_manager.keyscan_para.keyscan_row_pins[i]);
        hal_gpio_irq_enable(keyscan_manager.keyscan_para.keyscan_row_pins[i]);
    }
}

static void keyscan_timer_init(void)
{
    keyscan_manager.keyscan_timer_handle = hw_timer_create("keyscan_timer",
                                                           KEYSCAN_DEBOUNCE_TIME * 1000, false,
                                                           keyscan_timer_callback);
    if (keyscan_manager.keyscan_timer_handle == NULL)
    {
        IO_PRINT_ERROR0("keyscan_timer_init: fail to create keyscan timer");
    }
}

void sw_keyscan_filter_data_config(uint16_t filter_data, bool filter_data_enable)
{
    uint8_t i, j;

    if ((((uint8_t)(filter_data >> 8)) >= keyscan_manager.keyscan_para.row_size) ||
        (((uint8_t)filter_data) >= keyscan_manager.keyscan_para.column_size))
    {
        return;
    }

    if (filter_data_enable)
    {
        for (i = 0; i < keyscan_manager.filter_data_size; i++)
        {
            if (keyscan_manager.filter_data[i] == filter_data)
            {
                return;
            }
        }
        keyscan_manager.filter_data[keyscan_manager.filter_data_size] = filter_data;
        keyscan_manager.filter_data_size++;
    }
    else
    {
        for (i = 0; i < keyscan_manager.filter_data_size; i++)
        {
            if (keyscan_manager.filter_data[i] == filter_data)
            {
                for (j = i; j < keyscan_manager.filter_data_size - 1; j++)
                {
                    keyscan_manager.filter_data[j] = keyscan_manager.filter_data[j + 1];
                }
                keyscan_manager.filter_data_size--;
                keyscan_manager.filter_data[keyscan_manager.filter_data_size] = 0;
                return;
            }
        }
    }
}

void sw_keyscan_clear_all_filter_data(void)
{
    keyscan_manager.filter_data_size = 0;
    memset(keyscan_manager.filter_data, 0,
           keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size * sizeof(
               uint16_t));
}

void sw_keyscan_init(T_KEYSCAN_INIT_PARA *keyscan_init_para)
{
    memset(&keyscan_manager, 0, sizeof(T_KEYSCAN_MANAGER));
    keyscan_manager.keyscan_para.row_size = keyscan_init_para->row_size;
    keyscan_manager.keyscan_para.column_size = keyscan_init_para->column_size;
    keyscan_manager.keyscan_para.keyscan_scan_end_cb = keyscan_init_para->keyscan_scan_end_cb;
    keyscan_manager.keyscan_para.keyscan_release_cb = keyscan_init_para->keyscan_release_cb;
    keyscan_manager.keyscan_para.keyscan_row_pins = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                                            keyscan_manager.keyscan_para.row_size);
    keyscan_manager.keyscan_para.keyscan_column_pins = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                                               keyscan_manager.keyscan_para.column_size);
    keyscan_manager.keyscan_data.key_data = (uint16_t *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                                     keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size *
                                                                     sizeof(uint16_t));
    keyscan_manager.filter_data = (uint16_t *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                           keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size *
                                                           sizeof(uint16_t));
    memset(keyscan_manager.keyscan_data.key_data, 0,
           keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size * sizeof(
               uint16_t));
    memset(keyscan_manager.filter_data, 0,
           keyscan_manager.keyscan_para.row_size * keyscan_manager.keyscan_para.column_size * sizeof(
               uint16_t));
    memcpy(keyscan_manager.keyscan_para.keyscan_row_pins, keyscan_init_para->keyscan_row_pins,
           keyscan_manager.keyscan_para.row_size);
    memcpy(keyscan_manager.keyscan_para.keyscan_column_pins, keyscan_init_para->keyscan_column_pins,
           keyscan_manager.keyscan_para.column_size);

    keyscan_gpio_init();

    keyscan_timer_init();
}
