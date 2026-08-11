/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "trace.h"
#include "device_charger.h"
#include "charger_api.h"
#include "charger_utils.h"
#include "hub_task.h"
#include "os_timer.h"
#include "app_dlps.h"
#include "app_main.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "hal_adp.h"
#include "adc_manager.h"
#include "rtl876x_adc.h"
#include "string.h"
#include "os_sched.h"


#define CHR_INT_NODE                DT_ALIAS(charger_int)
#define CHR_INT_CTLR                DT_GPIO_CTLR(CHR_INT_NODE, gpios)
#define CHR_GPIO_PIN                DT_GPIO_PIN(CHR_INT_NODE, gpios)
#define VOLTAGE_ROW_NUM             101
#define DEBOUNCE_DELAY_MS           500

static const struct device *chr_int = DEVICE_DT_GET(CHR_INT_CTLR);
static struct gpio_callback chr_cb_data;
static T_ADP_STATE adp_status = ADP_STATE_UNKNOWN;
static int chr_pin_state = 0;
static int chr_pin_temp_state = 0;
static uint8_t chr_adc_channel_handler;
static struct k_timer debounce_timer;

const uint16_t battery_voltage_table[VOLTAGE_ROW_NUM][2] =
{
    {4125, 100},
    {4116, 99},
    {4107, 98},
    {4098, 97},
    {4090, 96},
    {4083, 95},
    {4074, 94},
    {4067, 93},
    {4058, 92},
    {4051, 91},
    {4044, 90},
    {4037, 89},
    {4031, 88},
    {4024, 87},
    {4016, 86},
    {4007, 85},
    {3998, 84},
    {3989, 83},
    {3980, 82},
    {3972, 81},
    {3963, 80},
    {3955, 79},
    {3948, 78},
    {3942, 77},
    {3937, 76},
    {3931, 75},
    {3926, 74},
    {3921, 73},
    {3915, 72},
    {3910, 71},
    {3904, 70},
    {3899, 69},
    {3894, 68},
    {3888, 67},
    {3883, 66},
    {3878, 65},
    {3873, 64},
    {3869, 63},
    {3864, 62},
    {3860, 61},
    {3855, 60},
    {3851, 59},
    {3847, 58},
    {3843, 57},
    {3839, 56},
    {3836, 55},
    {3832, 54},
    {3828, 53},
    {3825, 52},
    {3821, 51},
    {3818, 50},
    {3814, 49},
    {3811, 48},
    {3807, 47},
    {3804, 46},
    {3800, 45},
    {3797, 44},
    {3793, 43},
    {3790, 42},
    {3786, 41},
    {3783, 40},
    {3779, 39},
    {3776, 38},
    {3772, 37},
    {3769, 36},
    {3765, 35},
    {3761, 34},
    {3758, 33},
    {3754, 32},
    {3751, 31},
    {3747, 30},
    {3744, 29},
    {3740, 28},
    {3737, 27},
    {3733, 26},
    {3729, 25},
    {3725, 24},
    {3721, 23},
    {3717, 22},
    {3712, 21},
    {3707, 20},
    {3703, 19},
    {3699, 18},
    {3695, 17},
    {3690, 16},
    {3684, 15},
    {3679, 14},
    {3674, 13},
    {3670, 12},
    {3665, 11},
    {3661, 10},
    {3657, 9},
    {3652, 8},
    {3648, 7},
    {3643, 6},
    {3636, 5},
    {3625, 4},
    {3606, 3},
    {3578, 2},
    {3542, 1},
    {3500, 0},
};

const uint16_t battery_voltage_incharing_table[VOLTAGE_ROW_NUM][2] =
{
    {4145, 100},
    {4137, 99},
    {4130, 98},
    {4123, 97},
    {4115, 96},
    {4108, 95},
    {4101, 94},
    {4094, 93},
    {4086, 92},
    {4079, 91},
    {4072, 90},
    {4065, 89},
    {4059, 88},
    {4052, 87},
    {4046, 86},
    {4039, 85},
    {4033, 84},
    {4028, 83},
    {4022, 82},
    {4016, 81},
    {4010, 80},
    {4004, 79},
    {3998, 78},
    {3992, 77},
    {3987, 76},
    {3981, 75},
    {3976, 74},
    {3971, 73},
    {3965, 72},
    {3959, 71},
    {3954, 70},
    {3949, 69},
    {3944, 68},
    {3939, 67},
    {3934, 66},
    {3930, 65},
    {3925, 64},
    {3920, 63},
    {3916, 62},
    {3912, 61},
    {3908, 60},
    {3904, 59},
    {3900, 58},
    {3896, 57},
    {3892, 56},
    {3888, 55},
    {3884, 54},
    {3880, 53},
    {3876, 52},
    {3872, 51},
    {3868, 50},
    {3864, 49},
    {3860, 48},
    {3856, 47},
    {3852, 46},
    {3848, 45},
    {3845, 44},
    {3841, 43},
    {3837, 42},
    {3833, 41},
    {3829, 40},
    {3825, 39},
    {3821, 38},
    {3817, 37},
    {3814, 36},
    {3810, 35},
    {3806, 34},
    {3802, 33},
    {3798, 32},
    {3794, 31},
    {3790, 30},
    {3786, 29},
    {3782, 28},
    {3778, 27},
    {3774, 26},
    {3769, 25},
    {3765, 24},
    {3761, 23},
    {3756, 22},
    {3751, 21},
    {3747, 20},
    {3742, 19},
    {3737, 18},
    {3733, 17},
    {3728, 16},
    {3723, 15},
    {3719, 14},
    {3714, 13},
    {3710, 12},
    {3705, 11},
    {3700, 10},
    {3695, 9},
    {3689, 8},
    {3681, 7},
    {3671, 6},
    {3657, 5},
    {3638, 4},
    {3616, 3},
    {3591, 2},
    {3565, 1},
    {3535, 0},
};

void chr_int_set(gpio_flags_t trig);
T_CHARGER_STATE device_charger_read_state(void);

static void debounce_timer_handler(struct k_timer *timer)
{
    APP_PRINT_INFO0("charger int timeout");
    if (chr_pin_temp_state != gpio_pin_get(chr_int, CHR_GPIO_PIN))
    {
        if (chr_pin_state)
        {
            gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, GPIO_INT_EDGE_FALLING);
            APP_PRINT_INFO0("charger int debounce 0");
        }
        else
        {
            gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, GPIO_INT_EDGE_RISING);
            APP_PRINT_INFO0("charger int debounce 1");
        }
        return;
    }

    chr_pin_state = chr_pin_temp_state;
    if (chr_pin_state)
    {
        gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, GPIO_INT_EDGE_FALLING);
    }
    else
    {
        gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, GPIO_INT_EDGE_RISING);
    }

    T_IO_MSG intn_chgr_msg;
    intn_chgr_msg.type = HUB_MSG_INTERNAL_CHARGER;
    intn_chgr_msg.subtype = CHARGER_STATE_CHANGE;
    intn_chgr_msg.u.param = (uint32_t)device_charger_read_state();
    send_msg_to_hub_task(&intn_chgr_msg, __LINE__);

}

void chr_int_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, GPIO_INT_DISABLE);
    chr_pin_temp_state = gpio_pin_get(chr_int, CHR_GPIO_PIN);
    k_timer_start(&debounce_timer, K_MSEC(DEBOUNCE_DELAY_MS), K_NO_WAIT);
    APP_PRINT_INFO1("chr_int_callback  %d", chr_pin_temp_state);
}

void chr_int_set(gpio_flags_t trig)
{

    if (!device_is_ready(chr_int))
    {
        APP_PRINT_ERROR0("charger int is not ready\n");
        return;
    }

    if (gpio_pin_configure(chr_int, CHR_GPIO_PIN, GPIO_INPUT | GPIO_PULL_UP))
    {
        APP_PRINT_ERROR0("Error: Failed to configure charger int pin\n");
        return;
    }

    if (gpio_pin_interrupt_configure(chr_int, CHR_GPIO_PIN, trig))
    {
        APP_PRINT_ERROR0("Error: Failed to configure charger interrupt\n");
        return;
    }

    gpio_init_callback(&chr_cb_data, chr_int_callback, BIT(CHR_GPIO_PIN));
    gpio_add_callback(chr_int, &chr_cb_data);
}

static void chr_adp_plug_cb(T_ADP_PLUG_EVENT event, void *user_data)
{
    switch (event)
    {
    case ADP_EVENT_PLUG_IN:
        {
            APP_PRINT_INFO0("adp in");
            adp_status = adp_get_current_state(ADP_DETECT_5V);
        }
        break;

    case ADP_EVENT_PLUG_OUT:
        {
            APP_PRINT_INFO0("adp out");
            adp_status = adp_get_current_state(ADP_DETECT_5V);
        }
        break;

    default:
        break;
    } // switch (event)

    T_IO_MSG intn_chgr_msg;
    intn_chgr_msg.type = HUB_MSG_INTERNAL_CHARGER;
    intn_chgr_msg.subtype = CHARGER_STATE_CHANGE;
    intn_chgr_msg.u.param = (uint32_t)device_charger_read_state();
    send_msg_to_hub_task(&intn_chgr_msg, __LINE__);
}

static void chr_adc_cb(void *pvPara, uint32_t int_status)
{
    uint16_t adc_data = 0;
    uint16_t sched_bit_map = 0x0001;

    adc_mgr_read_data_req(chr_adc_channel_handler, &adc_data, sched_bit_map);
    app_db.batt.voltage = ADC_GetRes(adc_data, INTERNAL_VBAT_MODE);
    APP_PRINT_INFO2("chr_adc_cb: adc_data %d, voltage = %d", adc_data, app_db.batt.voltage);
}

T_CHARGER_STATE device_charger_read_state(void)
{
    T_CHARGER_STATE state = STATE_CHARGER_END;
    if (adp_status == ADP_STATE_IN)
    {
        if (chr_pin_state)
        {
            state = STATE_CHARGER_FINISH;
        }
        else
        {
            state = STATE_CHARGER_START;
        }
    }
    else if (adp_status == ADP_STATE_OUT)
    {
        state = STATE_CHARGER_END;
    }
    return state;
}

T_CHARGER_ERROR_CODE device_charger_read_error_code(void)
{
    return 0;
}

void device_charger_update_battery_state(void)
{
    adc_mgr_enable_req(chr_adc_channel_handler);
    os_delay(10);

    uint8_t tmp_level = 0;
    const uint16_t (*p_bat_tbl)[2] = NULL;
    static uint8_t bat_level_array[3] = {0xFF, 0xFF, 0xFF};
    static uint8_t bat_update_idx = 0;
    if (app_db.batt.charger_state == STATE_CHARGER_START)
    {
        p_bat_tbl = battery_voltage_incharing_table;
    }
    else
    {
        p_bat_tbl = battery_voltage_table;
    }
    for (uint8_t i = 0; i < VOLTAGE_ROW_NUM; i++)
    {
        if (app_db.batt.voltage >= p_bat_tbl[i][0])
        {
            tmp_level = p_bat_tbl[i][1];
            break;
        }
    }

    if (bat_level_array[0] == 0xFF)
    {
        memset(bat_level_array, tmp_level, sizeof(bat_level_array));
    }

    uint8_t last_level;
    if (bat_update_idx == 0)
    {
        last_level = bat_level_array[2];
    }
    else
    {
        last_level = bat_level_array[bat_update_idx - 1];
    }

    if (app_db.batt.charger_state == STATE_CHARGER_START)
    {
        if (tmp_level < last_level)
        {
            tmp_level = last_level;
        }
    }
    else
    {
        if (tmp_level > last_level)
        {
            tmp_level = last_level;
        }
    }
    bat_level_array[bat_update_idx] = tmp_level;
    bat_update_idx++;
    if (bat_update_idx == 3)
    {
        bat_update_idx = 0;
    }
    app_db.batt.level = (bat_level_array[0] + bat_level_array[1] + bat_level_array[2]) / 3;
    APP_PRINT_INFO4("bat = %d, array %d, %d, %d", app_db.batt.level, bat_level_array[0],
                    bat_level_array[1], bat_level_array[2]);
}


void device_charger_init(void)
{
    chr_int_set(GPIO_INT_EDGE_FALLING);
    adp_register_state_change_cb(ADP_DETECT_5V, chr_adp_plug_cb, NULL);
    k_timer_init(&debounce_timer, debounce_timer_handler, NULL);

    adp_status = adp_get_current_state(ADP_DETECT_5V);
    chr_pin_state = gpio_pin_get(chr_int, CHR_GPIO_PIN);
    app_db.batt.charger_state = device_charger_read_state();

    APP_PRINT_INFO3("adp_status = 0x%x, chr_pin_state = 0x%x, app_db.batt.charger_state = 0x%x",
                    adp_status, chr_pin_state, app_db.batt.charger_state);

    //get battery voltage, level
    ADC_InitTypeDef ADC_InitStruct;
    ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.adcClock = ADC_CLK_39K;
    ADC_InitStruct.bitmap = 0x0001;
    ADC_InitStruct.schIndex[0] = INTERNAL_VBAT_MODE;

    if (!adc_mgr_register_req(&ADC_InitStruct, (adc_callback_function_t)chr_adc_cb,
                              &chr_adc_channel_handler))
    {
        IO_PRINT_ERROR0("charger adc: adc_manager Register Request Fail");
    }
    else
    {
        adc_mgr_enable_req(chr_adc_channel_handler);
    }

}

