/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_SRC_PLAYBACK_H_
#define _APP_SRC_PLAYBACK_H_

#include "rtl876x.h"
#include "audio_type.h"
#include "app_msg.h"
#include "fmc_platform.h"
#include "flash_map.h"
#include "app_report.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PLAYBACK_PLAYLIST_PAUSE_EN  1
#define PLAYBACK_PLAYLIST_NUM   8
#define STREAM_BUF_SIZE             (1024 * 20)
#define PLAYBACK_TRACK_STATE_CLEAR                  0xFF


#define FS_HEADER_COUNT_OFFSET           0
#define FS_HEADER_COUNT_SIZE             sizeof(uint16_t)
#define FS_HEADER_RSV_SIZE               sizeof(uint32_t)
#define FS_HEADER_INFO_START             (FS_HEADER_COUNT_SIZE + FS_HEADER_RSV_SIZE)

typedef enum
{
    SD_PLAY_STATE_IDLE,
    SD_PLAY_STATE_PLAY,
    SD_PLAY_STATE_STOPPING,
} T_SD_PLAY_STATE;

typedef struct
{
    T_AUDIO_FORMAT_INFO format_info;
    uint16_t sample_counts;
    uint16_t frame_duration;
    uint16_t frame_size;
} T_FILE_FORMAT_INFO;

typedef struct
{
    uint16_t latency;
    uint16_t lower_level;
    uint16_t upper_level;
    uint16_t play_duration;
    uint16_t preq_pkts;
    uint8_t frame_size;
} T_LOCALPLAY_SET_INFO;

typedef enum
{
    SD_STOPPED_IDLE_ACTION                        = 0x00,
    SD_STOPPED_SWITCH_BWD_ACTION                  = 0x01,
    SD_STOPPED_SWITCH_FWD_ACTION                  = 0x02,
    SD_STOPPED_FILE_END_TO_NEXT_ACTION            = 0x03,
} T_PLAYBACK_STOPPED_NEXT_ACTION;

typedef enum
{
    SINGLE_FILE = 0x00,
    NEXT_FILE,
    PREVERSE_FILE,
    RANDOM_FILE,
    RETURN_IDX,
} T_PLAYBACK_FILE;

typedef enum
{
    PLAYBACK_BUF_NORMAL         = 0x00,
    PLAYBACK_BUF_LOW            = 0x01,
    PLAYBACK_BUF_HIGH           = 0x02,
} T_PLAYBACK_BUF_STATE;

typedef enum
{
    PLAYBACK_SUCCESS            = 0x00,
    PLAYBACK_READ_ERROR         = 0x01,
    PLAYBACK_IDX_ERROR          = 0x02,
    PLAYBACK_OPEN_FILE_ERROR    = 0x03,
    PLAYBACK_CRC_ERROR          = 0x04,
    PLAYBACK_CLOSE_FILE_ERROR   = 0x05,
    PLAYBACK_PLAYLIST_ERROR     = 0x06,
    PLAYBACK_WRITE_ERROR        = 0x07,
    PLAYBACK_SYS_ERROR          = 0x08,
    PLAYBACK_PIPE_DRAIN_ERROR   = 0x09,
    PLAYBACK_ENC_RX_ERROR       = 0x0A,
    PLAYBACK_DATA_SEND_ERROR    = 0x0B,
    PLAYBACK_PIPE_FILL_ERROR    = 0x0C,
    PLAYBACK_PIPE_CREATE_ERROR  = 0x0D,
    PLAYBACK_CREATE_FILE_ERROR  = 0x0E,

} T_PLAYBACK_ERROR;

typedef struct
{
    uint32_t                    sample_rate;
    uint8_t                     buffer_state;
    uint8_t                     frame_num;
    uint16_t                    put_data_time_ms;
    uint16_t                    preq_pkts;
    uint16_t                    delay_stop_ms;
} T_PLAYBACK_DATA;

void app_src_play_sd_pipe_send_lea_data(void);
void app_src_play_sd_handle_lea_stop(void);
void app_src_playback_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                     uint16_t cmd_len, uint8_t *ack_pkt);
void app_src_playback_report_local_mp3_files(void);
bool app_src_play_sd_start(uint8_t play_route);
void app_src_play_sd_stop(uint8_t play_route);
void app_src_playback_init(void);
void handle_volume_cmd(uint8_t app_idx, T_CMD_PATH cmd_path, uint8_t *cmd_ptr,
                       uint16_t cmd_len, uint8_t *ack_pkt);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_SRC_PLAY_SOURCE_H_ */
