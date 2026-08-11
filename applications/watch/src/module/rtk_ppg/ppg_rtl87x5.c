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
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "hub_task.h"
#include "gsensor_stk8321.h"
#include "RTL87x5PPG.h"
#include "ppg_rtl87x5.h"
#include "os_mem.h"
#include "os_sched.h"
#include "os_queue.h"
#include "ppg_flash_handle.h"
#include "ppg_trans_handle.h"
#include "communicate_parse_health.h"
#include "hrs_gatt_svc.h"
#include "rtl876x_rtc.h"
#include "app_report.h"
#include "app_timer.h"
#include "module_global_data.h"


#define FIFO_THRESH   (32)
#define READ_FIFO_MS ((FIFO_THRESH / 33 + 1)*5)
#define HRS_SAMPLE_PERIOD    800
#define GLB_PPI_ARRAY_MAX_SIZE 36


#define HRS_I2C_DEV_NODE            DT_ALIAS(i2c0)
#define HRS_INT_NODE                DT_ALIAS(hrs_int)
#define HRS_INT_CTLR                DT_GPIO_CTLR(HRS_INT_NODE, gpios)
#define HRS_GPIO_PIN                DT_GPIO_PIN(HRS_INT_NODE, gpios)

static const struct device *hrs_i2c_dev = DEVICE_DT_GET(HRS_I2C_DEV_NODE);
static const struct device *hrs_int = DEVICE_DT_GET(HRS_INT_CTLR);
static struct gpio_callback hrs_cb_data;

uint8_t *hrs_mem = NULL;
static bool sensor_enable = false;
Hrm_rsult_rem_t hrm_display;
Check_cnt_t check_cnt;
uint8_t serial_no_old = 0;
uint8_t glb_ppi_len = 0;
uint16_t glb_ppi_array[GLB_PPI_ARRAY_MAX_SIZE];
T_PPG_SETTING_FLAG ppg_setting_flag;

uint8_t ppg_hrm_timer_id = 0;
uint8_t timer_handle_send_hrm = 0;
uint8_t timer_handle_read_hrm = 0;
typedef enum
{
    APP_PPG_SEND_HRM             = 0x00,
    APP_PPG_READ_HRM             = 0x01,
} T_APP_PPG_TIMER;

void hrs_int_set(gpio_flags_t trig);
void app_ppg_hrm_timeout_cb(uint8_t timer_evt, uint16_t param);
bool hrs_report_data(uint8_t heart_rate, uint8_t ppi_len, uint16_t *ppi_array);

void hrs_int_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    T_HUB_MSG_HRM state = HRM_SENSOR_INT_RELEASE;

    int hrs_pin_state = gpio_pin_get(hrs_int, HRS_GPIO_PIN);
    if (hrs_pin_state < 0)
    {
        return;
    }

    if (hrs_pin_state)
    {
        hrs_int_set(GPIO_INT_EDGE_FALLING);
        state = HRM_SENSOR_INT_TRIGGER;
    }
    else
    {
        hrs_int_set(GPIO_INT_EDGE_RISING);
        state = HRM_SENSOR_INT_RELEASE;
    }

    T_IO_MSG hrs_msg;
    hrs_msg.type = HUB_MSG_HRM;
    hrs_msg.subtype = state;

    send_msg_to_hub_task(&hrs_msg, __LINE__);
}

void hrs_int_set(gpio_flags_t trig)
{
    if (!device_is_ready(hrs_int))
    {
        APP_PRINT_ERROR0("HRS int is not ready\n");
        return;
    }

    if (gpio_pin_configure(hrs_int, HRS_GPIO_PIN, GPIO_INPUT | GPIO_PULL_DOWN))
    {
        APP_PRINT_ERROR0("Error: Failed to configure hrs int pin\n");
        return;
    }

    if (gpio_pin_interrupt_configure(hrs_int, HRS_GPIO_PIN, trig))
    {
        APP_PRINT_ERROR0("Error: Failed to configure hrs interrupt\n");
        return;
    }

    gpio_init_callback(&hrs_cb_data, hrs_int_callback, BIT(HRS_GPIO_PIN));
    gpio_add_callback(hrs_int, &hrs_cb_data);
}

void hrs_driver_init(void)
{
    if (!device_is_ready(hrs_i2c_dev))
    {
        APP_PRINT_ERROR0("Error: i2c device is not not ready!\n");
        return;
    }

    uint32_t i2c_cfg = I2C_SPEED_SET(I2C_SPEED_FAST) | I2C_MODE_CONTROLLER;

    if (i2c_configure(hrs_i2c_dev, i2c_cfg))
    {
        APP_PRINT_ERROR0("Error: Failed to configure tp i2c\n");
        return;
    }

    uint8_t ret = RTL87x5BootUpProc(0, 4, 4, 0);
    APP_PRINT_INFO1("hrs init ret %d", ret);
    Version_info_t version = RTL87x5GetLibVersion();
    APP_PRINT_INFO3("hrs lib_version: %d.%d.%d\n", version.major, version.minor, version.revision);

    app_timer_reg_cb(app_ppg_hrm_timeout_cb, &ppg_hrm_timer_id);
}

void hrs_reg_dump(void)
{
    //uint32_t l = os_lock();
    uint8_t data[4];
    uint32_t word;
    for (uint16_t i = 0x00; i <= 0x4BC; i += 4)
    {
        memset(data, 0, sizeof(data));
        RTL87x5ReadMultiReg_8bit(i, 1, data);
        word = 0;
        word |= ((uint32_t) data[0]) << 24;
        word |= ((uint32_t) data[1]) << 16;
        word |= ((uint32_t) data[2]) << 8;
        word |= ((uint32_t) data[3]);
        os_delay(10);
        APP_PRINT_TRACE6("CHI dump addr %x, word %x, byte %x %x %x %x", i, word, data[0], data[1], data[2],
                         data[3]);
    }
    //os_unlock(l);
}

void hrs_power_start_01(void)
{
    APP_PRINT_TRACE0("Cheat HRS 1, CMCHO1218");
    sensor_enable = true;
    memset(&check_cnt, 0, sizeof(Check_cnt_t));

    RTL87x5Init_t init_para;
    memset(&init_para, 0, sizeof(RTL87x5Init_t));

    init_para.en_word =
        0x3F; // bit0(D_HR_algo enable), bit1(S_HR_algo_enable), bit2(Amb_ctrl_enable), bit3(LED_ctrl_enable), bit4(Prox_det_enable)
    //init_para.en_word = 3; // bit0(D_HR_algo enable), bit1(S_HR_algo_enable), bit2(Amb_ctrl_enable), bit3(LED_ctrl_enable), bit4(Prox_det_enable)

    init_para.tx_idac_max_grn = 63;  // 0~63, Maximum TX IDAC of green LED
    init_para.tx_idac_max_red = 63;  // 0~63, Maximum TX IDAC of red LED
    init_para.tx_idac_max_ir  = 63;  // 0~63, Maximum TX IDAC of IR LED
    init_para.acc_lsb_per_g   =
        512;  // Accelerometer(g-sensor) sensortivity, LSB/g; set to 0 if no accelerometer is available

    init_para.sig_info_type = 0;    // signal information type, 0xFF(reserved for debug)
    init_para.sig_num[0] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.sig_num[1] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.sig_num[2] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    // [Note]: sig_num[0]<=8, sig_num[1]<=2, sig_num[2]<=2
    init_para.pd_usage[0] = 2;      // PD usage for 2 RX channel. [0]: RX1, [1]: RX2
    init_para.pd_usage[1] = 2;      // PD usage for 2 RX channel. [0]: RX1, [1]: RX2
    // Set bit0 if PD current input A is used, and set bit1 if PD current input B is used.
    // ex1. pd_usage[0]=0x1 if RX1 use RX1A only
    // ex2. pd_usage[1]=0x3 if RX2 use both RX2A and RX2B
    init_para.rx_sel[0] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.rx_sel[1] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.rx_sel[2] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    //For x-th signal of specific light y, bit x of rx_sel[y] is 0(use RX1) or 1(use RX2)
    //[Note]: 2 consecutive signals, like (2n)-th and (2n+1)-th, are recommended to share both LED placement and use RX1, RX2 separately.
    //  In that way, these two signals can be received at the same time for power saving.

    init_para.led_grn[0] = 2;       //LED placement for 1st green PPG signals.
    init_para.led_grn[1] = 3;       //LED placement for 2nd green PPG signals.
    //For x-th signal, led_grn[x]: 0(TX_LED1), 1(TX_LED2), 2(TX_LED3), 3(TX_LED4);
    init_para.led_red[0] =
        0;       //LED placement for 1st red PPG signals. (same format with green signals)
    init_para.led_red[1] =
        0;       //LED placement for 2nd red PPG signals. (same format with green signals)
    init_para.led_ir[0]  =
        1;       //LED placement for 1st red PPG signals. (same format with green signals)
    init_para.led_ir[1]  =
        1;       //LED placement for 2nd red PPG signals. (same format with green signals)

    uint16_t size = RTL87x5CalMemSize(&init_para.sig_num[0], init_para.en_word);
    hrs_mem = os_mem_alloc(OS_MEM_TYPE_DATA, size);
    if (hrs_mem == NULL)
    {
        APP_PRINT_ERROR0("hrs start 01 malloc fail");
        return;
    }
    RTL87x5SWRst(init_para, (uint32_t *)hrs_mem);
    RTL87x5SetFifoThd(FIFO_THRESH);
    RTL87x5StartProc();

    hrs_int_set(GPIO_INT_EDGE_RISING);
}

void hrs_power_start_02(void)
{
    APP_PRINT_TRACE0("Cheat HRS 2, CMCHO1218");
    sensor_enable = true;
    memset(&check_cnt, 0, sizeof(Check_cnt_t));

    RTL87x5Init_t init_para;
    memset(&init_para, 0, sizeof(RTL87x5Init_t));

    init_para.en_word =
        0x3F; // bit0(D_HR_algo enable), bit1(S_HR_algo_enable), bit2(Amb_ctrl_enable), bit3(LED_ctrl_enable), bit4(Prox_det_enable)

    init_para.tx_idac_max_grn = 63;  // 0~63, Maximum TX IDAC of green LED
    init_para.tx_idac_max_red = 63;  // 0~63, Maximum TX IDAC of red LED
    init_para.tx_idac_max_ir  = 16;  // 0~63, Maximum TX IDAC of IR LED
    init_para.acc_lsb_per_g   =
        512;  // Accelerometer(g-sensor) sensortivity, LSB/g; set to 0 if no accelerometer is available

    init_para.sig_info_type = 0;    // signal information type, 0xFF(reserved for debug)
    init_para.sig_num[0] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.sig_num[1] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.sig_num[2] = 2;       // number of PPG signals, [0]: green LED, [1]: red LED, [2]: IR LED
    // [Note]: sig_num[0]<=8, sig_num[1]<=2, sig_num[2]<=2
    init_para.pd_usage[0] = 2;      // PD usage for 2 RX channel. [0]: RX1, [1]: RX2
    init_para.pd_usage[1] = 2;      // PD usage for 2 RX channel. [0]: RX1, [1]: RX2
    // Set bit0 if PD current input A is used, and set bit1 if PD current input B is used.
    // ex1. pd_usage[0]=0x1 if RX1 use RX1A only
    // ex2. pd_usage[1]=0x3 if RX2 use both RX2A and RX2B
    init_para.rx_sel[0] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.rx_sel[1] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    init_para.rx_sel[2] =
        0x2;      //Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    //For x-th signal of specific light y, bit x of rx_sel[y] is 0(use RX1) or 1(use RX2)
    //[Note]: 2 consecutive signals, like (2n)-th and (2n+1)-th, are recommended to share both LED placement and use RX1, RX2 separately.
    //  In that way, these two signals can be received at the same time for power saving.

    init_para.led_grn[0] = 2;       //LED placement for 1st green PPG signals.
    init_para.led_grn[1] = 3;       //LED placement for 2nd green PPG signals.
    //For x-th signal, led_grn[x]: 0(TX_LED1), 1(TX_LED2), 2(TX_LED3), 3(TX_LED4);
    init_para.led_red[0] =
        0;       //LED placement for 1st red PPG signals. (same format with green signals)
    init_para.led_red[1] =
        0;       //LED placement for 2nd red PPG signals. (same format with green signals)
    init_para.led_ir[0]  =
        1;       //LED placement for 1st red PPG signals. (same format with green signals)
    init_para.led_ir[1]  =
        1;       //LED placement for 2nd red PPG signals. (same format with green signals)


    uint16_t size = RTL87x5CalMemSize(&init_para.sig_num[0], init_para.en_word);
    hrs_mem = os_mem_alloc(OS_MEM_TYPE_DATA, size);
    if (hrs_mem == NULL)
    {
        APP_PRINT_ERROR0("hrs start 02 malloc fail");
        return;
    }
    RTL87x5SWRst(init_para, (uint32_t *)hrs_mem);
    RTL87x5SetFifoThd(FIFO_THRESH);
    RTL87x5StartProc();

    hrs_int_set(GPIO_INT_EDGE_RISING);
}

void app_ppg_hrm_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case APP_PPG_SEND_HRM:
        {
            hrs_report_data(hrm_display.heart_rate, glb_ppi_len, glb_ppi_array);
            memset(&glb_ppi_array[0], 0, GLB_PPI_ARRAY_MAX_SIZE);
            glb_ppi_len = 0;
        }
        break;

    case APP_PPG_READ_HRM:
        {
            T_IO_MSG hrs_msg;
            hrs_msg.type = HUB_MSG_HRM;
            hrs_msg.subtype = HRM_SENSOR_HRM_READ;
            send_msg_to_hub_task(&hrs_msg, __LINE__);
        }
        break;
    default:
        break;
    }
}

void hrs_read_ppg_sensor(void)
{
    uint16_t word_len = RTL87x5GetFifoDataCnt();
    uint16_t byte_len = word_len * 4;

    uint8_t *data = os_mem_alloc(RAM_TYPE_DATA_ON, byte_len + 9);
    data[0] = 0xA5;
    data[1] = 0x5A;
    data[2] = (uint8_t)((byte_len + 5) >> 8);
    data[3] = (uint8_t)(byte_len + 5);
    RTL87x5ReadMultiReg(0xF00, word_len, (uint32_t *)&data[9]);

    uint32_t tick = RTC_GetCounter();
    data[4] = (uint8_t)(tick >> 24);
    data[5] = (uint8_t)(tick >> 16);
    data[6] = (uint8_t)(tick >> 8);
    data[7] = (uint8_t) tick;
    data[8] = (uint8_t) check_cnt.ppg_raw_cnt;

    RTL87x5PpgTopProc((uint32_t *)&data[9], word_len, tick);

    if (ppg_data_send_flag == START_FLAG)
    {
        ppg_send_data(0, EVENT_TRANS_PPG_DATA_CHECK, KEY_PPG_TRANS_TRANS_PPG_DATA, byte_len + 9,
                      data);
    }

    if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
    {
        ppg_flash_save_data(byte_len + 9, data,  false);
    }
//    APP_PRINT_TRACE5("Cheat hrs data 32b %x, 8b %x %x %x %x", *((uint32_t *)&data[8]), data[8], data[9], data[10], data[11]);

    os_mem_free(data);
    check_cnt.ppg_raw_cnt += 1;
}

void hrs_fill_gsensor_data(void)
{
    uint8_t fifo_len = gsensor_get_fifo_length();

    if ((fifo_len > 0) && (fifo_len <= 32))
    {
        uint8_t *acc_data_send = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(AxesRaw_t) * fifo_len + 9);
        if (acc_data_send == NULL)
        {
            return;
        }

        gsensor_get_fifo_data(fifo_len, (AxesRaw_t *)(acc_data_send + 9));

        uint8_t *acc_data_in = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(int32_t) * 3 * fifo_len);
        if (acc_data_in == NULL)
        {
            os_mem_free(acc_data_send);
            return;
        }

        uint32_t tick = RTC_GetCounter();

        acc_data_send[0] = 0xA7;
        acc_data_send[1] = 0x7A;
        acc_data_send[2] = (uint8_t)((fifo_len * 6 + 5) >> 8);
        acc_data_send[3] = (uint8_t)(fifo_len * 6 + 5);
        acc_data_send[4] = (uint8_t)(tick >> 24);
        acc_data_send[5] = (uint8_t)(tick >> 16);
        acc_data_send[6] = (uint8_t)(tick >> 8);
        acc_data_send[7] = (uint8_t) tick;
        acc_data_send[8] = (uint8_t) check_cnt.acc_raw_cnt;

        uint8_t *g_ptr = &acc_data_send[9];
        AxesRaw_t *acc_data = (AxesRaw_t *)&acc_data_send[9];
        int32_t *x_in = (int32_t *)&acc_data_in[0];
        int32_t *y_in = (int32_t *)&acc_data_in[0 + fifo_len * sizeof(int32_t)];
        int32_t *z_in = (int32_t *)&acc_data_in[0 + 2 * fifo_len * sizeof(int32_t)];

        for (uint8_t i = 0; i < fifo_len; i ++)
        {
            x_in[i] = acc_data[i].AXIS_X;
            y_in[i] = acc_data[i].AXIS_Y;
            z_in[i] = acc_data[i].AXIS_Z;

            uint16_t x_temp = acc_data[i].AXIS_X;
            uint16_t y_temp = acc_data[i].AXIS_Y;
            uint16_t z_temp = acc_data[i].AXIS_Z;

            BE_UINT16_TO_STREAM(g_ptr, x_temp);
            BE_UINT16_TO_STREAM(g_ptr, y_temp);
            BE_UINT16_TO_STREAM(g_ptr, z_temp);
        }

        RTL87x5AccTopProc(x_in, y_in, z_in, fifo_len, tick);

        if (ppg_data_send_flag == START_FLAG)
        {
            ppg_send_data(0, EVENT_TRANS_GSENSOR_DATA_CHECK, KEY_PPG_TRANS_TRANS_PPG_DATA,
                          (fifo_len * 6) + 9, acc_data_send);
        }

        if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
        {
            ppg_flash_save_data((fifo_len * 6) + 9, acc_data_send,  false);
        }
        check_cnt.acc_raw_cnt += 1;

        os_mem_free(acc_data_send);
        os_mem_free(acc_data_in);
    }
}

bool hrs_report_data(uint8_t heart_rate, uint8_t ppi_len, uint16_t *ppi_array)
{
    bool op_result;

    T_HEART_RATE_MEASUREMENT_VALUE_FLAG flag;
    flag.heart_rate_value_format_bit = 0; //HEART_RATE_VALUE_FORMAT_UINT8  0
    flag.sensor_contact_status_bits = 0;
    flag.energy_expended_status_bit = 0;

    if (ppi_len == 0)
    {
        flag.rr_interval_bit = 0;
    }
    else
    {
        flag.rr_interval_bit = 1;
    }
    flag.rfu = 0;

    T_HEART_RATE_MEASUREMENT_VALUE temp;

    temp.flag = flag;
    temp.heart_rate_measurement_value = heart_rate;
    if (ppi_len != 0)
    {
        temp.rr_interval_len = ppi_len * 2;
        temp.p_rr_interval = ppi_array;
    }

    extern T_SERVER_ID hrs_gatt_srv_id;
    uint16_t cid;
    uint8_t cid_num;
    uint16_t conn_handle = le_get_conn_handle(RtkWristbandSys.wristband_conn_id);
    gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
    op_result = hrs_heart_rate_measurement_value_notify(conn_handle, cid, hrs_gatt_srv_id, temp);
    return op_result;
}

Hrm_rsult_rem_t get_hrs_result(void)
{
    Hrm_rsult_rem_t *result;
    result = RTL87x5GetRsult();
    memcpy(&hrm_display, result, sizeof(Hrm_rsult_rem_t));

    return hrm_display;
}

void hrs_ppg_int_event_handle(void)
{
    uint8_t intr = RTL87x5GetIntrType();
    APP_PRINT_TRACE1("Cheat HRS int tri %d", intr);
    uint16_t data_size = RTL87x5GetFifoDataCnt();
    if (intr & 0b10)
    {
        RTL87x5ClrProxIntr();
    }

    if (intr & 0b01)
    {
        if (!sensor_enable)
        {
            return;
        }

        hrs_read_ppg_sensor();
        hrs_fill_gsensor_data();
    }
}

void hrs_command_stop_event_handle(void)
{
    if (!sensor_enable)
    {
        return;
    }

    app_stop_timer(&timer_handle_read_hrm);

    if (ppg_setting_flag == PPG_REAL_TIME_SEND_MODE)
    {
        app_stop_timer(&timer_handle_send_hrm);
    }

    if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
    {
        ppg_flash_save_data(0, NULL, true);
    }
    hrs_reg_dump();
    RTL87x5StopProc();
    if (hrs_mem != NULL)
    {
        os_mem_free(hrs_mem);
        hrs_mem = NULL;
    }
    sensor_enable = false;
}

void hrs_command_start_type1_event_handle(void)
{
    if (sensor_enable)
    {
        return;
    }
    uint8_t ret = RTL87x5BootUpProc(0, 4, 4, 0);
    APP_PRINT_INFO1("hrs init ret %d", ret);
    ppg_setting_flag = PPG_REAL_TIME_SEND_MODE;
    if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
    {
        ppg_flash_save_data_init();
    }
    hrs_power_start_01();
    app_start_timer(&timer_handle_read_hrm, "ppg_read_hrm", ppg_hrm_timer_id, APP_PPG_READ_HRM, 0, true,
                    800);
    if (ppg_setting_flag == PPG_REAL_TIME_SEND_MODE)
    {
        app_start_timer(&timer_handle_send_hrm, "ppg_send_hrm", ppg_hrm_timer_id, APP_PPG_SEND_HRM, 0, true,
                        1000);
    }
}

void hrs_command_start_type2_event_handle(void)
{
    if (sensor_enable)
    {
        return;
    }
    uint8_t ret = RTL87x5BootUpProc(0, 4, 4, 0);
    APP_PRINT_INFO1("hrs init ret %d", ret);
    if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
    {
        ppg_flash_save_data_init();
    }
    hrs_power_start_02();
    app_start_timer(&timer_handle_read_hrm, "ppg_read_hrm", ppg_hrm_timer_id, APP_PPG_READ_HRM, 0, true,
                    800);
}

void hrs_result_read_event(void)
{
    Hrm_rsult_rem_t *result;
    result = RTL87x5GetRsult();
    memcpy(&hrm_display, result, sizeof(Hrm_rsult_rem_t));
    uint16_t byte_len = sizeof(Hrm_rsult_rem_t);

    APP_PRINT_INFO2("read result heart rate = %d, wearing = %d", hrm_display.heart_rate,
                    hrm_display.wearing_st);

    uint8_t *data = os_mem_alloc(RAM_TYPE_DATA_ON, byte_len + 9);
    data[0] = 0xA3;
    data[1] = 0x3A;
    data[2] = (uint8_t)((byte_len + 5) >> 8);
    data[3] = (uint8_t)(byte_len + 5);

    uint32_t tick = RTC_GetCounter();
    data[4] = (uint8_t)(tick >> 24);
    data[5] = (uint8_t)(tick >> 16);
    data[6] = (uint8_t)(tick >> 8);
    data[7] = (uint8_t) tick;
    data[8] = (uint8_t) check_cnt.ppg_result_cnt;
    memcpy(&data[9], result, sizeof(Hrm_rsult_rem_t));

    if (serial_no_old != result->serial_no)
    {
        if (glb_ppi_len + result->ppi_array_len <= GLB_PPI_ARRAY_MAX_SIZE)
        {
            memcpy(&glb_ppi_array[glb_ppi_len], result->ppi_array, (result->ppi_array_len * sizeof(uint16_t)));

            glb_ppi_len += result->ppi_array_len;
            serial_no_old = result->serial_no;
        }
        else
        {
            APP_PRINT_ERROR0("PPG hrm glb_ppi_array is not enough!");
        }
    }

    if (ppg_setting_flag == PPG_DATA_SAVE_MODE)
    {
        hrs_report_data(result->heart_rate, result->ppi_array_len, result->ppi_array);
        ppg_flash_save_data(byte_len + 9, data,  false);
    }

    if (ppg_data_send_flag == START_FLAG)
    {
        ppg_send_data(0, EVENT_TRANS_HRS_DATA_CHECK, KEY_PPG_TRANS_TRANS_PPG_DATA, byte_len + 9,
                      data);
    }

    os_mem_free(data);
    check_cnt.ppg_result_cnt += 1;
}
