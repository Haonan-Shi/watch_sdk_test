/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef RTL87X5PPG_H
#define RTL87X5PPG_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    uint8_t en_word;            // Functional enable word; Each bit maps to corresponding function, and set to 1 for enable, 0 for disable.
    // bit 0(DHR), bit 1(SHR), bit2(AmbCtrl), bit 3(LEDCtrl), bit 4(ProxDet), bit 5(SpO2Est), bit 6(SpO2FitMode), others(reseved)
    uint8_t tx_idac_max_grn;    // 0~63, Maximum TX IDAC of green LED
    uint8_t tx_idac_max_red;    // 0~63, Maximum TX IDAC of red LED
    uint8_t tx_idac_max_ir;     // 0~63, Maximum TX IDAC of IR LED

    uint8_t sig_info_type;      // Signal information type, 0xFF(reserved for debug)
    uint8_t sig_num[3];         // Number of PPG signals for 3 light color, [0]: green LED, [1]: red LED, [2]: IR LED
    //    [Note]: sig_num[0]<=8, sig_num[1]<=2, sig_num[2]<=2
    uint8_t pd_usage[2];        // PD usage for 2 RX channel. [0]: Rx0, [1]: Rx1
    // For each channel, set bit0 if PD current input A is used, and set bit1 if PD current input B is used.
    //    ex1. pd_usage[0]=0x1 if Rx0 use Rx0A only
    //    ex2. pd_usage[1]=0x3 if Rx1 use both Rx1A and Rx1B
    uint8_t rx_sel[3];          // Rx channel selection for all signals, [0]: green LED, [1]: red LED, [2]: IR LED
    // For x-th signal of specific light y, bit x of rx_sel[y] is 0(use Rx0) or 1(use Rx1)
    //    [Note]: 2 consecutive signals, like (2n)-th and (2n+1)-th, will be determinate to share same ��Meas�� if the corresponding 2 bits of rx_sel[y]=0x2.
    //            Otherwise, 2 ��Meas��s will be allocated for them.
    //    Here, sharing means 2 optical results will be captured at the same ��Meas�� time use Rx0 and Rx1 individually.
    int32_t acc_lsb_per_g;      // Accelerometer(g-sensor) sensortivity, LSB/g; set to 0 if no accelerometer is available
    uint32_t led_grn[8];        // LED assignments for all green PPG signals.
    // For x-th signal, bit0~1 of led_grn[x] indicate the connection of LED output: 0(TX_LED0), 1(TX_LED1), 2(TX_LED2), 3(TX_LED3);
    // bit2~3 indicate the usage of 2 Tx drivers: 0(use single Tx driver), 3(use both).
    uint32_t led_red[2];        // LED assignments for all red PPG signals. (same format with green signals)
    uint32_t led_ir[2];         // LED assignments for all IR PPG signals. (same format with green signals)
} RTL87x5Init_t;

typedef struct
{
    uint8_t serial_no;              //Serial number, used to determine that the result updated or not.
    uint8_t wearing_st;             //Wearing status, 0(Not ready), 1(un-wearing), 2(wearing)
    uint8_t heart_rate;             //Heart rate, 30~210, unit: BPM
    uint8_t hr_sqi;                 //Heart rate SQI(signal quality index), 0~100
    uint8_t ppi_array_len;          //Number of valid data in PPI(peak-to-peak interval) array, 0(Not ready), max=6
    uint8_t ppi_sqi_array[6];       //SQI of reported PPI, 0~100
    int8_t  ppi_outlier_detect[6];  //Outlier detection result of reported PPI. For each element: -1(Not ready), 0(normal), 1(missed), 2(extra)
    uint8_t motion_st;              //Motion status, 0(Not ready), 1(static), 2(in motion)
    uint16_t ppi_array[6];          //PPI within one second, 285~2000, unit: ms. [0]:the oldest interval
    uint16_t ror[4];                //ROR(Ratio of ratio) from Red and IR signal, unit: 2^-14
    uint16_t spo2;                  //Estimated SpO2 Measurement, 700~1000, unit: 0.1%
    uint8_t sat_cnt[4];             //Saturation counters for wearing detection signals. [0]:green-CH0, [1]:green-CH1, [2]:IR-CH0, [3]:IR-CH1
    uint8_t spo2_sqi;               //SpO2 SQI(signal quality index), 0(Not ready), 1(bad), 2(good)
    uint32_t rsv[5];
} Hrm_rsult_rem_t;

typedef struct
{
    union
    {
        uint32_t version_info;
        struct
        {
            uint32_t revision: 8;  // [0:7]   revision version
            uint32_t minor: 8;     // [8:15]  minor version
            uint32_t major: 8;     // [16:23] major version
            uint32_t reserve: 8;   // [24:31] reserved

        };
    };
} Version_info_t;

//Init. and Control
uint32_t RTL87x5CalMemSize(uint8_t *p_sig_num, uint8_t en_word);
uint8_t RTL87x5BootUpProc(uint8_t alc_mode, uint8_t u_pre_wid, uint8_t u_mp_exp,
                          uint8_t sam_r_type);
uint8_t RTL87x5SWRst(RTL87x5Init_t settings, uint32_t *p_mem_pool);
void RTL87x5StartProc(void);
void RTL87x5StopProc(void);

//Interrupt Handling
uint8_t RTL87x5GetIntrType(void);
void RTL87x5ClrProxIntr(void);
void RTL87x5SetFifoThd(uint8_t num_word);
uint16_t RTL87x5GetFifoDataCnt(void);

//Heart Rate Monitor(HRM) Data Process
void RTL87x5PpgTopProc(uint32_t *ppg_data, uint32_t data_size, uint32_t rtc_cnt);
void RTL87x5AccTopProc(int32_t *data_x, int32_t *data_y, int32_t *data_z, uint32_t data_size,
                       uint32_t rtc_cnt);

//Proximity Detection
void RTL87x5SetProxThd(uint16_t led_thd, uint16_t amb_thd, uint8_t tx_idac_spec_grn,
                       uint8_t tx_idac_spec_ir, uint8_t lh_thd, uint8_t out_format);
uint8_t RTL87x5StartPutOnDet(uint8_t intr_type, uint8_t rx_sel, uint8_t pd_usage, uint8_t led_sel,
                             uint16_t sam_r);

//Adjust the ratio to achieve an specific DC target range
void RTL87x5SetDCTargetRatio(uint8_t ratio);

//Passing SpO2 fitting parameter
void RTL87x5SpO2FitPmt(uint16_t *fit_pmt);

//Get Result
Hrm_rsult_rem_t *RTL87x5GetRsult(void);

//Get Lib Version infomation
Version_info_t RTL87x5GetLibVersion(void);

//Implement by user
//Register R/W
extern void RTL87x5WriteMultiReg(uint16_t start_addr, uint16_t len, uint32_t *p_word_data);
extern void RTL87x5ReadMultiReg(uint16_t start_addr, uint16_t len, uint32_t *p_word_data);
extern void RTL87x5ReadMultiReg_8bit(uint16_t startAddress, uint16_t len, uint8_t *p_word_data);

//Delay
extern void RTL87x5DelayMS(uint32_t t);
extern void RTL87x5DelayUS(uint32_t t);

#ifdef __cplusplus
}
#endif

#endif /*RTL87X5PPG_H */
