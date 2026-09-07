/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AI_RECORD_H_
#define _AI_RECORD_H_

#include <stdint.h>
#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define AI_RECORD_PICTURE_NAME_MAX_LEN      48
#define AI_RECORD_VIDEO_NAME_MAX_LEN        48
#define AI_RECORD_AI_AND_HD_SNAPSHOT        1

#define AI_RECORD_WIFI_SOC_OTA_SUCCESS                      6
#define AI_RECORD_WIFI_SOC_OTA_SUCCESS_BT_OTA_IS_READY      7
#define AI_RECORD_WIFI_SOC_OTA_FAIL                         8


typedef void (*AI_RECORD_FUNC_CB)();

typedef enum
{
    AI_RECORD_CB_IDX_WIFI_POWER_ON_HANDLE = 0x00,
    AI_RECORD_CB_IDX_WIFI_POWER_DOWN_HANDLE,
    AI_RECORD_CB_IDX_WIFI_QUERY_INFO,
    AI_RECORD_CB_IDX_UART_SEND, // wi-fi
    AI_RECORD_CB_IDX_ATCMD_SEND, // wi-fi
    AI_RECORD_CB_IDX_REPORT_SEND,
    AI_RECORD_CB_IDX_TAKE_PICTURE_TONE,
    AI_RECORD_CB_IDX_RECORD_STOP,
    AI_RECORD_CB_IDX_SET_STATE,
    AI_RECORD_CB_IDX_GET_ALS_STATE,
    AI_RECORD_CB_IDX_AI_RECORDING_START,
    AI_RECORD_CB_IDX_AI_RECORDING_STOP,
    AI_RECORD_CB_IDX_AI_SNAPSHOT,
    AI_RECORD_CB_IDX_AI_GET_NEXT_FILE,
    AI_RECORD_CB_IDX_SET_GPS,
    AI_RECORD_CB_IDX_CHECK_BATTERY_STATE,

    AI_RECORD_CB_IDX_SET_WIFI_STA_MODE,
    AI_RECORD_CB_IDX_WIFI_STA_RESP,

    AI_RECORD_CB_IDX_WIFI_SOC_OTA_STATUS,

    AI_RECORD_CB_IDX_APP_GET_FILE_CNT,
    AI_RECORD_CB_IDX_WIFI_GET_FILE_CNT,
    AI_RECORD_CB_IDX_LIVE_START,
    AI_RECORD_CB_IDX_LIVE_STOP,
    AI_RECORD_CB_IDX_RTSP_START,
    AI_RECORD_CB_IDX_RTSP_STOP,

    AI_RECORD_CB_IDX_AUDIOING_SNAPSHOT_DONE,

    AI_RECORD_CB_IDX_AI_UPDATE_WIFI,

    AI_RECORD_CB_IDX_NUM,
} T_AI_RECORD_CB_IDX;

typedef enum
{
    AI_RECORD_STATE_IDLE                  = 0,
    AI_RECORD_STATE_SNAPSHOT              = 1,
    AI_RECORD_STATE_RECORD_VIDEO          = 2,
    AI_RECORD_STATE_AI_VOICEING           = 3,
    AI_RECORD_STATE_OTA                   = 4,
    AI_RECORD_STATE_WIFI_AP               = 5,
    AI_RECORD_STATE_WIFI_STA              = 6,
    AI_RECORD_STATE_LIVEING               = 7,
    AI_RECORD_STATE_RTSP_STREAM           = 8,
    AI_RECORD_STATE_RECORD_AUDIO          = 9,
    AI_RECORD_STATE_AI_SNAPSHOT           = 10,
    AI_RECORD_STATE_NONE,
} T_AI_RECORD_STATE;

typedef enum
{
    AI_RECORD_ACTION_STATE_IDLE                  = 0,
    AI_RECORD_ACTION_STATE_SNAPSHOT              = 1,
    AI_RECORD_ACTION_STATE_RECORD_VIDEO          = 2,
    AI_RECORD_ACTION_STATE_AI_VOICEING           = 3,
    AI_RECORD_ACTION_STATE_OTA                   = 4,
    AI_RECORD_ACTION_STATE_WIFI_AP               = 5,
    AI_RECORD_ACTION_STATE_WIFI_STA              = 6,
    AI_RECORD_ACTION_STATE_WIFI_OTA              = 7,

    AI_RECORD_ACTION_STATE_LOW_POWER             = 0x10,
    AI_RECORD_ACTION_STATE_BLOCK_ALS             = 0x11,
} T_AI_RECORD_ACTION_STATE;
/**  @brief  cmd set status to phone
  */
typedef enum
{
    AI_RECORD_ERR_SUCCESS       = 0x00,
    AI_RECORD_ERR_SET_WIFI      = 0x01,
    AI_RECORD_ERR_APP_TRANS     = 0x02,
    AI_RECORD_ERR_APP_OFFSET    = 0x03,

    AI_RECORD_ERR_WIFI_TRANS    = 0x10,
    AI_RECORD_ERR_WIFI_TRANS_SEQ = 0x11,
    AI_RECORD_ERR_WIFI_PKT_NUM  = 0x12,
    AI_RECORD_ERR_WIFI_SOC_OTA  = 0x13,

} T_AI_RECORD_ERR;

typedef enum
{
    AI_RECORD_CMD_AI_STATE_DISCONN          = 0x00,
    AI_RECORD_CMD_AI_STATE_CONN             = 0x01,
} T_AI_RECORD_CMD_AI_CONN_STATE;

typedef struct
{
    uint8_t ai_conn_state;
    uint8_t trans_mode;
    uint16_t mtu_size;
} T_AI_RECORD_SYNC_DATA;

void ai_record_sync_data_get(T_AI_RECORD_SYNC_DATA *data);

void ai_record_sync_data_set(T_AI_RECORD_SYNC_DATA *data);

void ai_record_app_idx_set(uint8_t app_idx);

uint8_t ai_record_app_idx_get(void);

uint8_t ai_record_cmd_get_ai_conn_state(void);

/**
 * \brief  set side role
 * \param  side  side role
*/
void ai_record_side_role_set(uint8_t side);
/**
 * \brief  set scenario state
 * \param  state  scenario state \ref T_AI_RECORD_SEND
*/
void ai_record_scenario_set_state(T_AI_RECORD_STATE state);

/**
 * ai_record.h
 *
 * \brief  ai record get scenario state.
 *
 */
uint8_t ai_record_scenario_get_state(void);

/**
 * \brief  The main function to handle all the wifi command
 * \param  path    transmission path
 * \param  length  length of command id and data
 * \param  p_value data addr
 * \param  app_idx received rx command device index
*/
void ai_record_cmd_wifi_handle(uint8_t path, uint16_t length, uint8_t *p_value, uint8_t app_idx);

/**
 * \brief  The main function to handle all the wifi command for RTL8720C
 * \param  path    transmission path
 * \param  length  length of command id and data
 * \param  p_value data addr
 * \param  app_idx received rx command device index
*/
void ai_record_cmd_wifi_8720c_handle(uint8_t path, uint16_t length, uint8_t *p_value,
                                     uint8_t app_idx);

/**
 * \brief  The main function to handle all the ai record app command
 * \param  path    transmission path
 * \param  length  length of command id and data
 * \param  p_value data addr
 * \param  app_idx received rx command device index
*/
void ai_record_cmd_app_handle(uint8_t path, uint16_t length, uint8_t *p_value, uint8_t app_idx);


/**
 * ai_record.h
 *
 * \brief  ai record ai app connected handle.
 *
 */
void ai_record_cmd_ai_connected_handle(uint8_t path, uint16_t mtu_size);

/**
 * ai_record.h
 *
 * \brief  ai record ai app disconnected handle.
 *
 */
void ai_record_cmd_ai_disconnected_handle(void);

/**
 * ai_record.h
 *
 * \brief  ai record ai snapshot cmd handle.
 *
 */
uint8_t ai_record_cmd_ai_snapshot(uint8_t *data, uint16_t len, uint8_t *name_buf, uint8_t name_len);

/**
 * ai_record.h
 *
 * \brief  ai record snapshot.
 * \param  p_name    file name
 * \param  name_len  length of name, \ref AI_RECORD_PICTURE_NAME_MAX_LEN
 *
 */
uint8_t ai_record_cmd_snapshot(uint8_t *p_name, uint8_t name_len);

/**
 * ai_record.h
 *
 * \brief  update ai record snapshot isp info.
 * \param  p_data    isp info
 * \param  data_len  length of isp.
 *
 */
uint8_t ai_record_cmd_update_isp_info_snapshot(uint8_t action, uint8_t *p_data, uint8_t data_len);

/**
 * ai_record.h
 *
 * \brief  ai record init wifi info, it can get sd and file info.
 *
 */
uint8_t ai_record_cmd_init_wifi_info(void);

/**
 * ai_record.h
 *
 * \brief  ai record record video start.
 * \param  p_name    file name
 * \param  name_len  length of name, the max value \ref AI_RECORD_VIDEO_NAME_MAX_LEN
 */
uint8_t ai_record_cmd_record_video_start(uint8_t *p_name, uint8_t name_len);

/**
 * ai_record.h
 *
 * \brief  ai record record video stop.
 *
 */
uint8_t ai_record_cmd_record_video_stop(void);

/**
 * ai_record.h
 *
 * \brief  ai record record audio start.
 * \param  p_name    file name
 * \param  name_len  length of name, the max value \ref AI_RECORD_VIDEO_NAME_MAX_LEN
 */
uint8_t ai_record_cmd_record_audio_start(uint8_t *p_name, uint8_t name_len);

/**
 * ai_record.h
 *
 * \brief  ai record record audio stop.
 *
 */
uint8_t ai_record_cmd_record_audio_stop(void);

/**
 * ai_record.h
 *
 * \brief  ai record start system upgrade.
 *
 */
uint8_t ai_record_cmd_sys_upgrade_start(uint8_t *p_ota_info, uint8_t length);

void ai_record_cmd_wifi_active(void);

/**
 * ai_record.h
 *
 * \brief  get ai record wifi soc version.
 *
 */
void ai_record_cmd_get_wifi_soc_ver(uint16_t *wifi_id, uint32_t *p_version);

/**
 * ai_record.h
 *
 * \brief  ai record ai voice start.
 * \param format_data it include sample rate, bit width and so on.
 * \param length data length
 *
 */
uint8_t ai_record_cmd_ai_voice_start(uint8_t *format_data, uint16_t length);

/**
 * ai_record.h
 *
 * \brief  ai record ai voice data send.
 * \param voice_data voice data info.
 * \param length data length
 *
 */
uint8_t ai_record_cmd_ai_voice_data_send(uint8_t *voice_data, uint16_t length);

/**
 * ai_record.h
 *
 * \brief  ai record ai voice local complete.
 * \param status ai voice status.
 *
 */
uint8_t ai_record_cmd_ai_voice_local_complete(uint8_t status);

/**
 * ai_record.h
 *
 * \brief  ai record next file status.
 * \param status ture it has other file, false there are not other file.
 *
 */
void ai_record_cmd_get_next_file_resp(uint8_t status);

/**
 * ai_record.h
 *
 * \brief  ai record set wifi STA mode.
 * \param sta mode info.
 * \param length data length
 *
 */
uint8_t ai_record_cmd_set_wifi_sta_mode(uint8_t *data, uint16_t length);

/**
 * ai_record.h
 *
 * \brief  ai record set wifi STA mode response.
 * \param ip info.
 * \param length data length
 *
 */
uint8_t ai_record_cmd_set_wifi_sta_mode_resp(uint8_t *data, uint16_t length);

void ai_record_cmd_report_user_key_event(uint8_t key);

void ai_record_cmd_report_file_cnt(bool is_peer, uint16_t snapshot_cnt, uint16_t video_cnt);

void ai_record_cmd_get_file_cnt(uint16_t *snapshot_cnt, uint16_t *video_cnt);

uint8_t ai_record_cmd_ai_update_wifi_handle(uint8_t *p_data);

void ai_cmd_wifi_atevt_handle(uint16_t at_evt, uint8_t *p_data, uint16_t length);

/**
 * ai_record.h
 *
 * \brief  ai record notify action state.
 * \param action \ref  T_AI_RECORD_STATE.
 * \param reason_state \ref T_AI_RECORD_STATE.
 *
 */
uint8_t ai_record_cmd_notify_action_state(uint8_t action, uint8_t reason_state);

void ai_record_cmd_wifi_switch_real_power_down(bool is_real);

/**
 * ai_record.h
 *
 * \brief  app layer callback dfu status.
 *
 */
void ai_record_dfu_fail_handle(void);

/**
 * ai_record.h
 *
 * \brief  app layer callback dfu status.
 *
 */
void ai_record_dfu_finish_handle(void);

/**
 * ai_record.h
 *
 * \brief  app layer callback system shutdown.
 *
 */
void ai_record_system_shutdown_handle(void);

/**
 * ai_record.h
 *
 * \brief   register ai record callback
 *
 * \param idx ai record callback index \ref T_AI_RECORD_CB_IDX
 * \param func ai record callback \ref AI_RECORD_FUNC_CB
 *
 */
void ai_record_register_cb(T_AI_RECORD_CB_IDX idx, AI_RECORD_FUNC_CB func);
/* @brief  ai record init
*
* @param  void
* @return none
*/
void ai_record_init(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _AI_RECORD_H_ */
