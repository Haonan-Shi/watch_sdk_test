/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_GFPS_CFG_H_
#define _APP_GFPS_CFG_H_

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" { /* __cplusplus */
#endif

/** @defgroup APP_RWS_GFPS App Gfps
  * @brief App Gfps
  * @{
  */
/** @brief  Read only configurations for gfps */
typedef struct
{
    //GFPS
    uint8_t gfps_model_id[3];

    uint8_t gfps_support : 1;
    uint8_t gfps_enable_tx_power : 1;
    uint8_t gfps_battery_info_enable : 1;
    uint8_t gfps_battery_remain_time_enable : 1;
    uint8_t gfps_battery_show_ui : 1;
    uint8_t gfps_left_ear_batetry_support : 1;
    uint8_t gfps_right_ear_batetry_support : 1;
    uint8_t gfps_case_battery_support : 1;

    int8_t  gfps_tx_power;
    uint8_t gfps_account_key_num;

    uint8_t gfps_sass_support : 1;
    uint8_t gfps_sass_support_dyamic_multilink_switch : 1;
    uint8_t gfps_sass_rsv : 5;
    uint8_t gfps_finder_support : 1;

    uint8_t gfps_le_device_support : 1;
    uint8_t gfps_le_disconn_force_enter_pairing_mode : 1;
    uint8_t gfps_le_device_mode : 2;
    uint8_t disable_finder_adv_when_power_off : 1;
    uint8_t gfps_rsv : 3;

    uint16_t gfps_discov_adv_interval;
    uint16_t gfps_not_discov_adv_interval;
    uint8_t  gfps_public_key[64];
    uint8_t  gfps_private_key[32];

    uint8_t tone_gfps_findme;

    uint8_t tone_gfps_dult;

    uint8_t gfps_company_name[64];
    uint8_t gfps_device_name[64];
    uint8_t gfps_device_type;


    uint8_t gfps_version[10];
    //gfps finder adv interval in power on state, unit 0.625ms, range (32,3200), default value 800, time = 800*0.625 = 500ms;
    uint32_t gfps_power_on_finder_adv_interval;
    //gfps finder adv interval in power off state, unit 0.625ms, range (32,3200), default value 1600, time = 1600*0.625 = 1000ms;
    uint32_t gfps_power_off_finder_adv_interval;
    //timeout value to start timer for finder adv in power off state, unit 1s, Range(60,3600), default value 600s
    uint16_t gfps_power_off_start_finder_adv_timer_timeout_value;
    //finder adv duration in power off state, unit 1s, range(10s,600s), default value 10s
    uint16_t gfps_power_off_finder_adv_duration;

    // unit 1s, Range(0,3600),   0: Disable
    uint16_t power_off_rtc_wakeup_timeout;
    // Range(0, 100)
    uint16_t gfps_finder_adv_skip_count_when_wakeup;

} T_APP_GFPS_CFG;


extern T_APP_GFPS_CFG app_gfps_cfg;


/**
    * @brief  GFPS config module init
    * @param  void
    * @return void
    */
void app_gfps_cfg_init(void);


/** End of APP_RWS_GFPS
* @}
*/


#ifdef __cplusplus
} /* __cplusplus */
#endif

#endif
