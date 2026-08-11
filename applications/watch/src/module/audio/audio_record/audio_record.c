/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "string.h"
#include "audio.h"
#include "app_cfg.h"
#include "audio_record.h"
#include "app_main.h"
#include "audio_track.h"
#include "trace.h"
#include "app_report.h"
#include "app_timer.h"
#include "stdint.h"
#include "audio_resource.h"
#include "zephyr/fs/fs.h"
#include "fmc_api.h"
#include "flash_map.h"
#include "platform_utils.h"
#include "app_fs_if.h"
#include "record_playlist.h"
#include "nrec.h"
#include <stdio.h>

#define RECORD_PLAY_LEVEL_HIGH      240  //high level  ms
#define RECORD_PLAY_LEVEL_LOW       100  //low  level  ms

#define RECORD_SECTION_SIZE         1024
#define RECORD_TO_FLASH_ADDR        (OTA_TMP_ADDR+0x1000)
#define RECORD_TO_FLASH_SIZE        (300*1024)
#define RECORD_FILE_PATH_PREFIX_LEN (sizeof(RECORD_FILE_PATH) - 1U)

typedef enum
{
    AUDIO_RECORD_TIMER_RECORD_WRITE        = 0x00,
    AUDIO_RECORD_TIMER_RECORD_UPDATE_UI    = 0x01,
} T_AUDIO_RECORD_TIMER;

typedef struct
{
    T_AUDIO_TRACK_HANDLE record_handle;
    T_AUDIO_TRACK_HANDLE play_handle;
    T_AUDIO_RECORD_FORMAT format;
    uint32_t play_ofs;
    uint32_t record_ofs;
    float total_time;
    float current_time;
    T_AUDIO_RECORD_MODE record_mode;
    T_AUDIO_EFFECT_INSTANCE record_nrec_instance;
} AUDIO_RECORD;


/*============================================================================*
  *                                        Variables
  *============================================================================*/
static AUDIO_RECORD mic_record =
{
    .record_handle = NULL,
    .play_handle = NULL,
    .format = MIC_PCM_16KHZ,
    .total_time = 0.0f,
    .current_time = 0.0f,
    .record_mode = AUDIO_RECORD_SAVE_FS,
};

static uint8_t audio_record_timer_id = 0;
static uint8_t timer_idx_record_write = 0;
static uint8_t timer_idx_record_update_time = 0;
static uint8_t read_data[RECORD_SECTION_SIZE];
static bool record_play_level_high = false;
static bool playlist_initialized = false;
static struct fs_file_t record_dat;
static struct fs_file_t play_dat;
static char record_file_path[RECORD_MAX_NAME_LEN];

static void audio_voice_gen_format_info(T_AUDIO_FORMAT_INFO *p_format_info,
                                        T_AUDIO_RECORD_FORMAT audio_format);

static int32_t record_nrec_attach(void)
{
    T_AUDIO_EFFECT_INSTANCE nrec_instance;

    if (mic_record.record_handle == NULL)
    {
        return -1;
    }

    nrec_instance = nrec_create(NREC_CONTENT_TYPE_RECORD, NREC_MODE_HIGH_SOUND_QUALITY, 0);
    if (nrec_instance == NULL)
    {
        return -2;
    }

    nrec_enable(nrec_instance);
    audio_track_effect_attach(mic_record.record_handle, nrec_instance);
    mic_record.record_nrec_instance = nrec_instance;

    return 0;
}

static int32_t record_nrec_detach(void)
{
    if (mic_record.record_handle == NULL)
    {
        return -1;
    }

    if (mic_record.record_nrec_instance == NULL)
    {
        return -2;
    }

    audio_track_effect_detach(mic_record.record_handle, mic_record.record_nrec_instance);
    nrec_disable(mic_record.record_nrec_instance);
    nrec_release(mic_record.record_nrec_instance);

    mic_record.record_nrec_instance = NULL;
    return 0;
}

#if 0
static void audio_dump_record_data(const char *title, uint8_t *record_data_buf, uint32_t data_len)
{
    const uint32_t bat_num = 8;
    uint32_t times = data_len / bat_num;
    uint32_t residue = data_len % bat_num;
    uint8_t *residue_buf = record_data_buf + times * bat_num;

    APP_PRINT_TRACE3("ama_dump_record_data: data_len %d, times %d, residue %d", data_len,
                     times, residue);
    APP_PRINT_TRACE2("ama_dump_record_data: record_data_buf is 0x%08x, residue_buf is 0x%08x\r\n",
                     (uint32_t)record_data_buf,
                     (uint32_t)residue_buf);

    APP_PRINT_TRACE1("@@@@@@@@@@@@@@@@@@@@@%s@@@@@@@@@@@@@@@@@@@@@@@@@@@", TRACE_STRING(title));

    for (int32_t i = 0; i < times; i++)
    {
        APP_PRINT_TRACE8("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                         record_data_buf[i * bat_num], record_data_buf[i * bat_num + 1], record_data_buf[i * bat_num + 2],
                         record_data_buf[i * bat_num + 3],
                         record_data_buf[i * bat_num + 4], record_data_buf[i * bat_num + 5],
                         record_data_buf[i * bat_num + 6],
                         record_data_buf[i * bat_num + 7]);
    }

    switch (residue)
    {
    case 1:
        APP_PRINT_TRACE1("ama_dump_record_data: 0x%02x\r\n", residue_buf[0]);
        break;
    case 2:
        APP_PRINT_TRACE2("ama_dump_record_data: 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1]);
        break;
    case 3:
        APP_PRINT_TRACE3("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0], residue_buf[1],
                         residue_buf[2]);
        break;
    case 4:
        APP_PRINT_TRACE4("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n", residue_buf[0],
                         residue_buf[1], residue_buf[2], residue_buf[3]);
        break;
    case 5:
        APP_PRINT_TRACE5("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                         residue_buf[0], residue_buf[1], residue_buf[2], residue_buf[3], residue_buf[4]);
        break;
    case 6:
        APP_PRINT_TRACE6("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                         residue_buf[0], residue_buf[1], residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5]);
        break;
    case 7:
        APP_PRINT_TRACE7("ama_dump_record_data: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x\r\n",
                         residue_buf[0], residue_buf[1], residue_buf[2], residue_buf[3], residue_buf[4], residue_buf[5],
                         residue_buf[6]);
        break;

    default:
        break;
    }

    APP_PRINT_TRACE0("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}
#else

static void audio_dump_record_data(const char *title, uint8_t *record_data_buf, uint32_t data_len)
{
}
#endif

static float audio_record_calc_pcm_time_sec(uint32_t size, const T_AUDIO_FORMAT_INFO *info)
{
    if (info == NULL || info->type != AUDIO_FORMAT_TYPE_PCM ||
        info->attr.pcm.sample_rate == 0 || info->attr.pcm.bit_width == 0 ||
        info->attr.pcm.chann_num == 0)
    {
        return 0.0f;
    }

    return (float)size / (float)(info->attr.pcm.sample_rate *
                                 (info->attr.pcm.bit_width / 8) *
                                 info->attr.pcm.chann_num);
}

static float audio_record_calc_file_duration_sec(uint32_t file_size)
{
    T_AUDIO_FORMAT_INFO format_info = {};

    if (file_size == 0)
    {
        return 0.0f;
    }

    audio_voice_gen_format_info(&format_info, mic_record.format);
    return audio_record_calc_pcm_time_sec(file_size, &format_info);
}

static uint32_t audio_record_time_sec_to_ms(float time_sec)
{
    return (time_sec <= 0.0f) ? 0U : (uint32_t)(time_sec * 1000.0f);
}

static bool audio_record_calc_track_time_sec(T_AUDIO_TRACK_HANDLE handle,
                                             uint32_t size,
                                             float *time_sec)
{
    T_AUDIO_FORMAT_INFO info = {};

    if (handle == NULL || time_sec == NULL)
    {
        return false;
    }

    if (audio_track_format_info_get(handle, &info) == false)
    {
        return false;
    }

    *time_sec = audio_record_calc_pcm_time_sec(size, &info);
    return true;
}

static const char *audio_record_get_file_name_from_path(void)
{
    if (strncmp(record_file_path, RECORD_FILE_PATH, RECORD_FILE_PATH_PREFIX_LEN) != 0)
    {
        return record_file_path;
    }

    return &record_file_path[RECORD_FILE_PATH_PREFIX_LEN];
}

static bool audio_record_update_record_time(void)
{
    float curr_time = 0.0f;

    if (mic_record.record_handle == NULL)
    {
        return false;
    }

    if (!audio_record_calc_track_time_sec(mic_record.record_handle, mic_record.record_ofs, &curr_time))
    {
        return false;
    }

    mic_record.current_time = curr_time;
    mic_record.total_time = mic_record.current_time;
    return true;
}

static bool audio_record_update_play_time(void)
{
    float curr_time = 0.0f;

    if (!audio_record_calc_track_time_sec(mic_record.play_handle, mic_record.play_ofs, &curr_time))
    {
        return false;
    }

    mic_record.current_time = curr_time;
    return true;
}

static void audio_record_refresh_time(bool refresh_play_time)
{
    if (audio_record_is_recording())
    {
        audio_record_update_record_time();
    }
    else if (refresh_play_time && audio_record_is_playing())
    {
        audio_record_update_play_time();
    }
}

static bool audio_record_save_data(uint8_t *buf, uint16_t length)
{
    bool ret = true;
    switch (mic_record.record_mode)
    {
    case AUDIO_RECORD_SAVE_FS:
        {
            ssize_t actual_write_len;
            actual_write_len = fs_write(&record_dat, buf, length);
            mic_record.record_ofs += length;
            if (actual_write_len != length)
            {
                ret = false;
            }
        }
        break;
    case AUDIO_RECORD_SAVE_FLASH:
        {
            if ((length < RECORD_SECTION_SIZE) || (mic_record.record_ofs >= RECORD_TO_FLASH_SIZE))
            {
                APP_PRINT_ERROR0("audio_record_read_cb: required_len not 1024 or count is larger than setting");
                ret = false;
                break;
            }
            if (mic_record.record_ofs % FMC_SEC_SECTION_LEN == 0)
            {
                fmc_flash_nor_erase(RECORD_TO_FLASH_ADDR + mic_record.record_ofs,
                                    FMC_FLASH_NOR_ERASE_SECTOR);
            }
            fmc_flash_nor_write(RECORD_TO_FLASH_ADDR + mic_record.record_ofs, buf, length);
            mic_record.record_ofs += length;
        }
        break;
    case AUDIO_RECORD_LOOP_BACK:
        {
            audio_record_play_data_write(buf, length);
        }
        break;
    case AUDIO_RECORD_UART_REPORT:
        {
            app_report_event(CMD_PATH_UART, EVENT_RECORDING_DATA, 0, buf, length);
        }
        break;
    default:
        break;
    }

    return ret;
}

static bool audio_record_load_data(uint8_t *buf, uint16_t length)
{
    bool ret = true;
    switch (mic_record.record_mode)
    {
    case AUDIO_RECORD_SAVE_FS:
        {
            ssize_t actual_read_len = 0;
            fs_seek(&play_dat, mic_record.play_ofs, FS_SEEK_SET);
            actual_read_len = fs_read(&play_dat, buf, length);
            mic_record.play_ofs += length;
            if (actual_read_len <= 0) //zephyr fs_read will return 0, if read end of file.
            {
                ret = false;
                APP_PRINT_ERROR1("audio_record_load_data: read record data fail res = %d", actual_read_len);
            }
        }
        break;
    case AUDIO_RECORD_SAVE_FLASH:
        {
            if (mic_record.play_ofs < mic_record.record_ofs)
            {
                fmc_flash_nor_read(RECORD_TO_FLASH_ADDR + mic_record.play_ofs, buf, length);
                mic_record.play_ofs += length;
            }
            else
            {
                ret = false;
            }
        }
        break;
    default:
        break;
    }

    return ret;
}

static bool audio_record_read_cb(T_AUDIO_TRACK_HANDLE  handle,
                                 uint32_t             *timestamp,
                                 uint16_t             *seq_num,
                                 T_AUDIO_STREAM_STATUS *status,
                                 uint8_t              *frame_num,
                                 void                 *buf,
                                 uint16_t              required_len,
                                 uint16_t             *actual_len)
{
    APP_PRINT_TRACE4("audio_record_read_cb: buf 0x%08x, required_len %d seq_num %d frame_num %d", buf,
                     required_len, *seq_num, *frame_num);

    {
        audio_dump_record_data("audio_record_read_cb", buf, required_len);
        if (mic_record.record_handle != NULL && handle == mic_record.record_handle)
        {
            audio_record_save_data(buf, required_len);
            APP_PRINT_INFO1("record data write: required_len %d", required_len);
        }

    }

    *actual_len = required_len;
    return true;
}

static void audio_voice_gen_format_info(T_AUDIO_FORMAT_INFO *p_format_info,
                                        T_AUDIO_RECORD_FORMAT audio_format)
{
    APP_PRINT_TRACE1("audio_voice_gen_format_info: audio_format %d", audio_format);

    p_format_info->frame_num = 1;
    switch (audio_format)
    {
#if 0 //opus encode by MCU in 8773G
    case OPUS_16KHZ_16KBPS_CBR_0_20MS:
        p_format_info->type = AUDIO_FORMAT_TYPE_OPUS;
        p_format_info->attr.opus.sample_rate = 16 * 1000;
        p_format_info->attr.opus.chann_num = 1;
        p_format_info->attr.opus.cbr = 1;
        p_format_info->attr.opus.cvbr = 0;
        p_format_info->attr.opus.mode = 3;
        p_format_info->attr.opus.complexity = 3;
        p_format_info->attr.opus.frame_duration = AUDIO_OPUS_FRAME_DURATION_20_MS;
        p_format_info->attr.opus.entropy = 0;
        p_format_info->attr.opus.bitrate = 16 * 1000;
        p_format_info->attr.opus.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;
#endif
    case MIC_PCM_16KHZ:
        p_format_info->type = AUDIO_FORMAT_TYPE_PCM;
        p_format_info->attr.pcm.sample_rate = 16 * 1000;//16 * 1000;
        p_format_info->attr.pcm.bit_width = 16;
        p_format_info->attr.pcm.chann_num = 1;
        p_format_info->attr.pcm.frame_length = 1024;
        p_format_info->attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;

    default:
        APP_PRINT_ERROR0("gen_format_info: only support XIAOAI_OPUS_16KHZ_16KBPS_CBR_0_20MS");
        break;
    }
}

/**
 * @brief Stops an active recording session.
 */
void audio_record_stop_recording(void)
{
    T_AUDIO_TRACK_HANDLE record_handle;

    if (mic_record.record_handle == NULL)
    {
        APP_PRINT_ERROR0("audio_stop_recording: already stopped!");
        return;
    }

    record_handle = mic_record.record_handle;
    audio_record_update_record_time();

    APP_PRINT_TRACE0("audio_stop_recording");
    mic_record.record_handle = NULL;
    audio_track_release(record_handle);

    mic_record.current_time = 0.0f;
    app_stop_timer(&timer_idx_record_update_time);

    if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
    {
        const char *record_file_name = audio_record_get_file_name_from_path();

        /* Close the recording file BEFORE updating the playlist. Keeping it open
         * both leaves the last data unflushed and holds a FatFS FIL slot; the
         * playlist update then has to open header.bin + name.bin, and with the
         * slab already near capacity fatfs_open() returns -ENOMEM, so the header
         * bin is never written (file count stays stale -> list shows no file and
         * the UI hangs on "Saving..."). record_playlist_update() only uses the
         * file name, not the handle, so closing first is safe. */
        fs_close(&record_dat);
        record_playlist_update(&record_dat, record_file_name, strlen(record_file_name));
    }
}

/**
 * @brief Starts a prepared recording session.
 */
void audio_record_start_recording(void)
{
    if (mic_record.record_handle == NULL)
    {
        APP_PRINT_ERROR0("audio_start_recording: handle is NULL");
        return;
    }

    APP_PRINT_INFO0("audio_start_recording");
    audio_track_start(mic_record.record_handle);
}

/**
 * @brief Indicates whether a recording session is active.
 *
 * @retval true Recording is active.
 * @retval false Recording is inactive.
 */
bool audio_record_is_recording(void)
{
    return mic_record.record_handle != NULL;
}

/**
 * @brief Indicates whether a playback session is active.
 *
 * @retval true Playback is active.
 * @retval false Playback is inactive.
 */
bool audio_record_is_playing(void)
{
    return mic_record.play_handle != NULL;
}

static void audio_record_write_timer_start(void)
{
    app_start_timer(&timer_idx_record_write, "record_write",
                    audio_record_timer_id, AUDIO_RECORD_TIMER_RECORD_WRITE, 0, false,
                    30);
}
static void audio_record_update_ui_timer_start(void)
{
    app_start_timer(&timer_idx_record_update_time, "record_update_ui",
                    audio_record_timer_id, AUDIO_RECORD_TIMER_RECORD_UPDATE_UI, 0, true,
                    60);
}

static void audio_record_play_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != mic_record.play_handle)
    {
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            APP_PRINT_INFO1("audio_record_play_cback: track_state_changed.state %d",
                            param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                if ((mic_record.record_mode == AUDIO_RECORD_SAVE_FS) ||
                    (mic_record.record_mode == AUDIO_RECORD_SAVE_FLASH))
                {
                    audio_record_write_timer_start();
                }
                break;

            case AUDIO_TRACK_STATE_STOPPED:
                break;

            default:
                break;
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_LOW:
        {
            record_play_level_high = false;
            //for future use, report buffer level high event
            //app_report_event(CMD_PATH_UART, EVENT_RECORD_PLAY_LEVEL_HIGH, 0, NULL, 0);
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_HIGH:
        {
            record_play_level_high = true;
            //for future use, report buffer level low event
            //app_report_event(CMD_PATH_UART, EVENT_RECORD_PLAY_LEVEL_LOW, 0, NULL, 0);
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("audio_record_play_cback: event_type 0x%04x", event_type);
    }
}


static void audio_record_record_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != mic_record.record_handle)
    {
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            APP_PRINT_INFO1("audio_record_record_cback: track_state_changed.state %d",
                            param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                record_nrec_attach();
                break;

            case AUDIO_TRACK_STATE_STOPPED:
                record_nrec_detach();
                break;

            default:
                break;
            }
        }
        break;
    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("audio_record_record_cback: event_type 0x%04x", event_type);
    }
}

/**
 * @brief Prepares a new recording session.
 *
 * @param[in] record_mode Recording mode.
 */
void audio_record_init_recorder(T_AUDIO_RECORD_MODE record_mode)
{
    APP_PRINT_INFO0("audio_record_init_recorder");
    if (audio_record_is_recording())
    {
        APP_PRINT_ERROR0("audio_record_init_recorder: already recording");
        return;
    }

    T_AUDIO_FORMAT_INFO format_info = {};
    audio_voice_gen_format_info(&format_info, mic_record.format);
    mic_record.total_time = 0.0f;
    mic_record.current_time = 0.0f;
    mic_record.play_ofs = 0;
    mic_record.record_ofs = 0;
    mic_record.record_mode = record_mode;

    mic_record.record_handle = audio_track_create(AUDIO_STREAM_TYPE_RECORD,
                                                  AUDIO_STREAM_MODE_NORMAL,
                                                  AUDIO_STREAM_USAGE_LOCAL,
                                                  format_info,
                                                  0,
                                                  15,
                                                  AUDIO_DEVICE_IN_MIC,
                                                  NULL,
                                                  audio_record_read_cb);

    if (mic_record.record_handle == NULL)
    {
        APP_PRINT_ERROR0("audio_record_init_recorder: handle is NULL");
        return;
    }

    if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
    {
        int path_len;

        path_len = snprintf(record_file_path,
                            sizeof(record_file_path),
                            "%s",
                            RECORD_FILE_PATH);
        if (path_len <= 0 || (size_t)path_len >= sizeof(record_file_path))
        {
            APP_PRINT_ERROR0("audio_record_init_recorder: init path failed");
            audio_track_release(mic_record.record_handle);
            mic_record.record_handle = NULL;
            return;
        }

        record_playlist_generate_new_file_name(&record_file_path[path_len],
                                               sizeof(record_file_path) - (size_t)path_len);
        record_file_path[RECORD_MAX_NAME_LEN - 1] = '\0';
        APP_PRINT_TRACE1("audio_record_init_recorder: new record file name %s",
                         TRACE_STRING(record_file_path));

        fs_file_t_init(&record_dat);
        int res = fs_open(&record_dat, record_file_path, FS_O_CREATE | FS_O_WRITE);
        if (res < 0)
        {
            APP_PRINT_ERROR1("audio_record_init_recorder: open file failed: %d", res);
            audio_track_release(mic_record.record_handle);
            mic_record.record_handle = NULL;
            return;
        }
        APP_PRINT_INFO1("audio_record_init_recorder: recording to %s", TRACE_STRING(record_file_path));
    }
}

/**
 * @brief Stops an active playback session.
 */
void audio_record_stop_playing(void)
{
    APP_PRINT_INFO0("audio_record_stop_playing");
    if (mic_record.play_handle)
    {
        audio_track_release(mic_record.play_handle);
        mic_record.play_handle = NULL;
    }

    if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
    {
        fs_close(&play_dat);
    }

    mic_record.play_ofs = 0;
    mic_record.current_time = 0.0f;
    mic_record.total_time = 0.0f;
    // app_stop_timer(&timer_idx_record_update_time);
    app_stop_timer(&timer_idx_record_write);
}

/**
 * @brief Writes PCM data into the active playback track.
 *
 * @param[in] buf PCM data buffer.
 * @param[in] len Buffer length in bytes.
 */
void audio_record_play_data_write(uint8_t *buf, uint16_t len)
{
    static uint16_t  seq_num = 0;
    uint16_t written_len;
    seq_num++;
    audio_track_write(mic_record.play_handle, 0xFFFFFFFF,
                      seq_num,
                      AUDIO_STREAM_STATUS_CORRECT,
                      1,
                      buf,
                      len,
                      &written_len);
    APP_PRINT_INFO2("play data write require len = %d, actual len = %d", len, written_len);
}

static void audio_record_play_start(void)
{
    uint16_t written_len;
    static uint16_t  seq_num = 0;
    size_t size = 0;

    if (record_play_level_high)
    {
        APP_PRINT_INFO0("audio_record_play_start: buffer level high");
        audio_record_write_timer_start();
        return;
    }

    APP_PRINT_INFO0("audio_record_play_start");
    audio_record_update_play_time();

    if (audio_record_load_data(read_data, sizeof(read_data)))
    {
        seq_num++;
        audio_track_write(mic_record.play_handle, 0xFFFFFFFF,
                          seq_num,
                          AUDIO_STREAM_STATUS_CORRECT,
                          1,
                          read_data,
                          sizeof(read_data),
                          &written_len);
        audio_record_write_timer_start();
    }
    else
    {
        audio_track_release(mic_record.play_handle);
        mic_record.play_handle = NULL;
        mic_record.play_ofs = 0;
        if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
        {
            fs_close(&play_dat);
        }
        app_stop_timer(&timer_idx_record_update_time);
    }
}

static void audio_record_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("audio_record_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case AUDIO_RECORD_TIMER_RECORD_WRITE:
        {
            app_stop_timer(&timer_idx_record_write);
            audio_record_play_start();
        }
        break;

    default:
        break;
    }
}

/**
 * @brief Prepares a playback session for a recording source.
 *
 * @param[in] record_mode Playback source mode.
 * @param[in] play_path Optional file path. When NULL in file-system mode,
 *            the latest recording is used.
 *
 * @retval true Playback initialization succeeded.
 * @retval false Playback initialization failed.
 */
bool audio_record_init_player(T_AUDIO_RECORD_MODE record_mode, const char *play_path)
{
    T_AUDIO_FORMAT_INFO format_info = {};
    size_t play_file_size = 0;

    audio_voice_gen_format_info(&format_info, mic_record.format);
    mic_record.record_mode = record_mode;

    if (mic_record.play_handle != NULL)
    {
        APP_PRINT_ERROR0("audio_record_init_player: already playing");
        return false;
    }

    mic_record.play_handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK,
                                                AUDIO_STREAM_MODE_NORMAL,
                                                AUDIO_STREAM_USAGE_LOCAL,
                                                format_info,
                                                15,
                                                0,
                                                AUDIO_DEVICE_OUT_SPK,
                                                NULL,
                                                NULL);

    if (mic_record.play_handle == NULL)
    {
        APP_PRINT_ERROR0("audio_record_init_player: handle is NULL");
        return false;
    }

    audio_track_threshold_set(mic_record.play_handle, RECORD_PLAY_LEVEL_HIGH, RECORD_PLAY_LEVEL_LOW);
    record_play_level_high = false;
    mic_record.play_ofs = 0;
    mic_record.current_time = 0.0f;
    mic_record.total_time = 0.0f;

    if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
    {
        if (play_path == NULL)
        {
            record_playlist_load_latest_file_name(record_file_path, sizeof(record_file_path));
        }
        else
        {
            if (!record_playlist_ensure_file_path((char *)play_path))
            {
                audio_track_release(mic_record.play_handle);
                mic_record.play_handle = NULL;
                return false;
            }
            strncpy(record_file_path, play_path, RECORD_MAX_NAME_LEN - 1);
            record_file_path[RECORD_MAX_NAME_LEN - 1] = '\0';
        }

        fs_file_t_init(&play_dat);
        int res = fs_open(&play_dat, record_file_path, FS_O_READ);
        if (res < 0)
        {
            APP_PRINT_ERROR1("audio_record_init_player: open file failed: %d", res);
            audio_track_release(mic_record.play_handle);
            mic_record.play_handle = NULL;
            return false;
        }

        if (fs_size(&play_dat, &play_file_size) != 0)
        {
            play_file_size = 0;
        }

        mic_record.total_time = audio_record_calc_file_duration_sec((uint32_t)play_file_size);
        APP_PRINT_INFO1("audio_record_init_player: playing %s", TRACE_STRING(record_file_path));
    }

    if (audio_track_start(mic_record.play_handle))
    {
        APP_PRINT_INFO0("audio_record_init_player: playback started");
        return true;
    }
    else
    {
        APP_PRINT_ERROR0("audio_record_init_player: playback start failed");
        if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
        {
            fs_close(&play_dat);
        }
        audio_track_release(mic_record.play_handle);
        mic_record.play_handle = NULL;
        return false;
    }
}

/**
 * @brief Gets the current elapsed time for the active record or playback session.
 *
 * @return Current elapsed time in milliseconds.
 */
uint32_t audio_record_get_current_time_ms(void)
{
    audio_record_refresh_time(true);

    return audio_record_time_sec_to_ms(mic_record.current_time);
}

/**
 * @brief Gets the total duration for the active record or playback session.
 *
 * @return Total duration in milliseconds.
 */
uint32_t audio_record_get_total_time_ms(void)
{
    audio_record_refresh_time(false);

    return audio_record_time_sec_to_ms(mic_record.total_time);
}

/**
 * @brief Converts a recorded file size to a PCM duration.
 *
 * @param[in] file_size File size in bytes.
 *
 * @return Calculated duration in milliseconds.
 */
uint32_t audio_record_get_duration_ms_by_file(uint32_t file_size)
{
    return audio_record_time_sec_to_ms(audio_record_calc_file_duration_sec(file_size));
}

/**
 * @brief Initializes the recording module.
 *
 * This routine should be called during application startup.
 */
void audio_record_init(void)
{
    app_timer_reg_cb(audio_record_timeout_cb, &audio_record_timer_id);
    audio_mgr_cback_register(audio_record_play_cback);
    audio_mgr_cback_register(audio_record_record_cback);
    if (mic_record.record_mode == AUDIO_RECORD_SAVE_FS)
    {
        record_fs_init();
    }
}
