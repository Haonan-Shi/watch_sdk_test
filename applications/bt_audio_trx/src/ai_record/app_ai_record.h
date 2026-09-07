/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AI_RECORD_H_
#define _APP_AI_RECORD_H_

#include <stdint.h>
#include <stdbool.h>
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define APP_AI_RECORD_PIN_FOR_EVB   1

#define APP_AI_RECORD_OTA_USE_EMMC   0

#define APP_AI_RECORD_PTA_SUPPORT           1

#define AI_RECORD_BATTERY_EN_WIFI_SOC_THRESHOLD     30

#if APP_AI_RECORD_PIN_FOR_EVB
#if APP_AI_RECORD_PTA_SUPPORT
#define PIN_WL_ACT                   P4_0 // P3_2
#define PIN_BT_ACT                   P3_3
#define PIN_BT_STAT                  P3_4
#endif

#define PIN_WIFI_POWER              P3_5  //EVB the pin connect with wifi soc chip_en
#define PIN_G_SENSOR_POWER          P5_4
#define PIN_AI_RECORD_ALS               P5_5

#define PIN_TMP110_I2C_SCL          P5_2
#define PIN_TMP110_I2C_SDA          P5_3

#define PIN_TOUCH_WEAR_DETECT       P5_7
#define PIN_TOUCH_SLIDE_DETECT      P5_6
#else
#if APP_AI_RECORD_PTA_SUPPORT
#define PIN_WL_ACT                   P4_0
#define PIN_BT_ACT                   P2_6
#define PIN_BT_STAT                  P2_7
#endif

#define PIN_WIFI_POWER              P5_0
#define PIN_G_SENSOR_POWER          P5_4
#define PIN_AI_RECORD_ALS               P5_5

#define PIN_TMP110_I2C_SCL          P1_1
#define PIN_TMP110_I2C_SDA          P1_2

#define PIN_TOUCH_WEAR_DETECT       P5_7
#define PIN_TOUCH_SLIDE_DETECT      P5_6
#endif

#define AI_RECORD_PTA_SET_PRIORITY_BREDR_ACL_LINK_TYPE            0x00
#define AI_RECORD_PTA_SET_PRIORITY_LE_LINK_TYPE                   0x01
#define AI_RECORD_PTA_SET_PRIORITY_BREDR_SCO_TYPE                 0x09
#define AI_RECORD_PTA_SET_PRIORITY_BREDR_ESCO_TYPE                0x0A

#define AI_RECORD_APP_EVENT_LEN_VOICE_START_INFO    15

typedef enum
{
    AI_RECORD_MMI_IDLE                  = 0,
    AI_RECORD_MMI_SNAPSHOT              = 1,
    AI_RECORD_MMI_RECORD_START          = 2,
    AI_RECORD_MMI_RECORD_STOP           = 3,
    AI_RECORD_MMI_AI_VOICE_START        = 4,
    AI_RECORD_MMI_AI_VOICE_STOP         = 5,
    AI_RECORD_MMI_AUDIO_START           = 6,
    AI_RECORD_MMI_AUDIO_STOP            = 7,
} T_AI_RECORD_MMI;

typedef enum
{
    AI_RECORD_AI_STATE_DISCONN          = 0x00,
    AI_RECORD_AI_STATE_CONN             = 0x01,
} T_AI_RECORD_AI_CONN_STATE;

typedef enum
{
    AI_RECORD_ACTION_BITMAP_SNAPSHOT       = 0x01,
    AI_RECORD_ACTION_BITMAP_AI_SNAPSHOT    = 0x02,
    AI_RECORD_ACTION_BITMAP_RECORD         = 0x04,
    AI_RECORD_ACTION_BITMAP_AUDIO          = 0x08,
    AI_RECORD_ACTION_BITMAP_ALL            = 0xFF,
} T_AI_RECORD_ACTION_BITMAP;

bool app_ai_record_get_vad_resume_flag(void);
uint8_t app_ai_record_get_ai_conn_state(void);

bool app_ai_record_get_als_state(void);

void app_ai_record_sensor_ct8504_handle_msg(T_IO_MSG *io_driver_msg_recv);

//void app_ai_record_scenario_set_state(T_AI_RECORD_STATE state);

//uint8_t app_ai_record_scenario_get_state(void);

void app_ai_record_connected_handle(uint8_t cmd_path, uint16_t mut_size);

void app_ai_record_disconnected_handle(uint8_t cmd_path);

bool app_ai_record_mmi_handle(uint8_t action);
void app_ai_record_mmi_action(uint8_t action);

bool app_ai_record_a2dp_check_pause(void);

void app_ai_record_a2dp_check_resume(void);

void app_ai_record_set_mode_cmd_handle(bool fast_mode);

/** @brief Drive the external WiFi module's chip_en (and G-sensor) power pins
 *  high. Same pin logic as the CMD_AI_RECORD_WIFI_POWER_ON path; safe to call
 *  repeatedly. Callers that talk to a cold module must allow it boot time
 *  before issuing commands. */
void app_ai_record_wifi_power_on(void);

void app_ai_record_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_AI_RECORD_H_ */
