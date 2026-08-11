/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _MALLEUS_H_
#define _MALLEUS_H_

#include "trace.h"
#include "remote.h"
#include "audio_track.h"
#include "malleus_secp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    SPATIAL_AUDIO_CLOSE,
    SPATIAL_AUDIO_OPEN,
    SPATIAL_AUDIO_NO_CHANGE,
} T_SPATIAL_AUDIO_MODE;

typedef struct t_malleus_pos
{
    int16_t horz;
    int16_t vert;
    uint16_t factor;
} T_MALLEUS_POS;

typedef void (*P_REPORT_EVENT_CB)(uint8_t, uint8_t, uint8_t *, uint16_t);
typedef void (*P_ASYNC_CB)(uint8_t, uint8_t *, uint16_t);
typedef void (*P_SYNC_CB)(uint8_t, uint8_t *, uint16_t);
typedef bool (*P_B2B_CB)(void);
typedef void (*P_TONE_CB)(uint8_t);
typedef void (*P_BIS_SYNC_CB)(uint8_t);
typedef bool (*P_BSRC_START_CB)(void);
typedef void (*P_SPATIAL_STATUS_CB)(uint8_t);


#define MALLEUS_CMD_EFFECT_SET         (0x06)
#define MALLEUS_CMD_TUNE_START         (0x0D)
#define MALLEUS_CMD_TUNE_DATA_REQ      (0x0E)
#define MALLEUS_CMD_REC_SECP_DATA      (0x0F)
#define MALLEUS_CMD_GET_SECP_NUM       (0x10)
#define MALLEUS_CMD_SET_KEY_VALUE      (0x11)
#define MALLEUS_CMD_GET_KEY_VALUE      (0x12)
#define MALLEUS_CMD_RESTART            (0x13)
#define MALLEUS_CMD_RESTART_ACK        (0x14)
#define MALLEUS_CMD_GET_INFO           (0x15)

#define MALLEUS_MUSIC_MASK             ((uint8_t)(1 << 0))
#define MALLEUS_GAMING_MASK            ((uint8_t)(1 << 1))
#define MALLEUS_MOVIE_MASK             ((uint8_t)(1 << 2))
#define MALLEUS_PARTY_MASK             ((uint8_t)(1 << 3))

#define MALLEUS_FULL_CYCLE             (MALLEUS_MUSIC_MASK | MALLEUS_GAMING_MASK | MALLEUS_MOVIE_MASK | MALLEUS_PARTY_MASK)

#define MALLEUS_VER_INFO_LEN           (6)
#define MALLEUS_KEY_VAL_LEN            (16)
#define MALLEUS_KEY_VAL_OCTETS         (MALLEUS_KEY_VAL_LEN*2)

#define malleus_effect_mode_sel \
    (malleus_get_gaming_mode()!=0) ? malleus_get_gaming_type() : malleus_get_normal_type()

/*single mode effect  is auto selected if rws disconn */
typedef enum
{
    MALLEUS_MUSIC_MODE                 = 0x00,
    MALLEUS_GAMING_MODE                = 0x01,
    MALLEUS_MOVIE_MODE                 = 0x02,
    MALLEUS_PARTY_MODE                 = 0x03,
    MALLEUS_SINGLE_MODE                = 0x04,
} T_MALLEUS_MODE;

enum
{
    // Don't change the order
    MALLEUS_POS_Z_UP_X_RIGHT           = 0x00,
    MALLEUS_POS_Z_UP_X_DOWN            = 0x01,
    MALLEUS_POS_Z_UP_X_LEFT            = 0x02,
    MALLEUS_POS_Z_UP_X_UP              = 0x03,
    MALLEUS_POS_Z_DOWN_X_RIGHT         = 0x04,
    MALLEUS_POS_Z_DOWN_X_DOWN          = 0x05,
    MALLEUS_POS_Z_DOWN_X_LEFT          = 0x06,
    MALLEUS_POS_Z_DOWN_X_UP            = 0x07,
};

typedef enum
{
    MALLEUS_EFFECT_CUR_NON             = 0x00,
    MALLEUS_EFFECT_CUR_AUDIO           = 0x01,
} T_MALLEUS_EFFECT_CUR;

typedef enum
{
    MALLEUS_SYNC_MSG_NULL              = 0x00,

    MALLEUS_SYNC_MSG_EFFECT_SET        = 0x01,
    MALLEUS_SYNC_MSG_EFFECT_TYPE       = 0x02,
    MALLEUS_SYNC_MSG_TUNE_START        = 0x03,
    MALLEUS_SYNC_MSG_SECP_DATA         = 0x04,
    MALLEUS_SYNC_MSG_KEY_VAL           = 0x05,
    MALLEUS_SYNC_MSG_ALL_KEY_VAL       = 0x06,
    MALLEUS_SYNC_MSG_POS_DATA          = 0x07,

    MALLEUS_SYNC_MSG_MAX
} T_MALLEUS_SYNC_MSG;

typedef enum
{
    MALLEUS_CH_L_R_DIV2                = 0x00,
    MALLEUS_CH_L                       = 0x01,
    MALLEUS_CH_R                       = 0x02,
    MALLEUS_CH_LINE_IN                 = 0x03,
} T_MALLEUS_CH;

enum
{
    MALLEUS_CMD_STATUS_COMP            = 0x00,
    MALLEUS_CMD_STATUS_DISALLOW        = 0x01,
    MALLEUS_CMD_STATUS_UNKNOW          = 0x02,
    MALLEUS_CMD_STATUS_PARAM_ERR       = 0x03,
};

typedef struct
{
    uint8_t *normal_type;
    uint8_t *gaming_type;
    uint8_t *gaming_mode;

    uint8_t *factory_addr;
    uint8_t *role;

    bool    spatial_audio_en;
    uint8_t spatial_status;
    uint8_t spatial_pos;

    uint8_t *key_val;
    uint16_t key_flash_offset;
    uint8_t key_flash_len;

    uint8_t *ver_info;

    P_REPORT_EVENT_CB report_event;
    P_ASYNC_CB relay_async;
    P_SYNC_CB relay_sync;
    P_B2B_CB b2b_conn;
    P_TONE_CB play_tone;
    P_BIS_SYNC_CB bis_sync_handle;
    P_BSRC_START_CB bsrc_start;
    P_SPATIAL_STATUS_CB spatial_status_set;
} T_MALLEUS;

extern T_MALLEUS malleus;

/**
 * @brief  audio effect cfg reset.
 *
 * @param  normal_type nomal effect type,if not exist, write zero
 * @param  gaming_type gaming effect type,if not exist, write zero
 * @return number of cycle.
*/
void malleus_cfg_rst(uint8_t normal_type, uint8_t gaming_type);

/**
 * @brief  config secp left right channel.
 *
 * @param  channel left right audio channel.
 * @return void
*/
void malleus_set_channel(T_MALLEUS_CH channel);

/**
 * @brief  parse relay param.
 *
 * @param  msg_type parse msg type.
 * @param  buf parse data.
 * @param  len parse len.
 * @param  status relay status.
 * @return void
*/
void malleus_relay_parse(uint8_t msg_type, uint8_t *buf, uint16_t len,
                         T_REMOTE_RELAY_STATUS status);

/**
 * @brief  load relay param.
 *
 * @param  msg_type parse msg type.
 * @param  data point to load buffer.
 * @param  len point to load len.
 * @return void
*/
void malleus_relay_load(uint8_t msg_type, uint8_t **data, uint16_t *len);

/**
 * @brief  ralay audio effect param.
 *
 * @param  void
 * @return void
*/
void malleus_relay_param(void);

/**
 * @brief  trig audio effect.
 *
 * @param  void
 * @return void
*/
void malleus_effect_trig(void);

/**
 * @brief  audio effect switch to next index in cycle.
 *
 * @param  void
 * @return void
*/
void malleus_effect_switch(void);

/**
 * @brief  bypass effect or not.
 *
 * @param  parameter true is bypass.
 * @return void
*/
void malleus_set_bypass(bool enable);

/**
 * @brief  set spatial fetch data flag.
 *
 * @param  flag  fetch data flag.
 * @return void
*/
void malleus_spatial_set_flag(bool flag);

/**
 * @brief  pos remap for spatial audio.
 *
 * @param  accs  gaccs.
 * @param  gyros gyros.
 * @return void
*/
void malleus_spatial_data_map(int16_t accs[3], int16_t gyros[3]);

/**
 * @brief  malleus cmd handle.
 *
 * @param  cmd_ptr  point to cmd data
 * @param  cmd_len  data len
 * @param  cmd_path transfer path
 * @param  app_idx transfer index
 * @return ack index
*/
uint8_t malleus_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx);

/**
 * @brief  malleus link release.
 *
 * @param  instance  @ref audio effect instance
 * @return void
*/
void malleus_release(T_AUDIO_EFFECT_INSTANCE *instance);

/**
 * @brief  malleus link create.
 *
 * @param  track_handle  @ref audio track handle
 * @return instance  @ref audio effect instance
*/
T_AUDIO_EFFECT_INSTANCE malleus_create(T_AUDIO_TRACK_HANDLE handle);

/**
 * @brief  malleus init.
 *
 * @param  normal_cycle normal mode cycle.
 * @param  gaming_cycle gaming mode cycle.
 * @return void
*/
void malleus_init(uint8_t normal_cycle, uint8_t gaming_cycle);

void malleus_effect_set(void);

/** End of APP_AUDIO_EFFECT
* @}
*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
