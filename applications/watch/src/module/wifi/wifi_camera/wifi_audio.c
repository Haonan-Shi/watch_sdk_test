/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "string.h"
#include "audio.h"
#include "app_cfg.h"
#include "wifi_audio.h"
#include "app_main.h"
#include "audio_track.h"
#include "trace.h"
#include "app_report.h"
#include "app_timer.h"
#include "stdint.h"
#include "zephyr/fs/fs.h"
#include "fmc_api.h"
#include "flash_map.h"
#include "platform_utils.h"
#include "section.h"
#include "app_cfg.h"
#include "autoconf.h"


/*============================================================================*
 *                              Ring Buffer Instances
 *============================================================================*/



#if (CONFIG_WIFI_AUDIO_RECORDER || CONFIG_WIFI_AUDIO_PLAYER)

/*============================================================================*
 *                              Ring Buffer Structure (Generic)
 *============================================================================*/
typedef struct
{
    uint8_t  *buffer;                          /* Ring buffer data storage pointer */
    uint32_t buffer_size;                      /* Total buffer size */
    uint32_t write_ptr;                        /* Write pointer (offset in bytes) */
    uint32_t read_ptr;                         /* Read pointer (offset in bytes) */
    uint32_t data_size;                        /* Current data size in buffer */
    bool     is_full;                          /* Full flag */
} wifi_audio_ring_buffer_t;

/*============================================================================*
 *                          Forward Declarations
 *============================================================================*/
static void wifi_audio_ring_buffer_init(wifi_audio_ring_buffer_t *buf, uint8_t *storage,
                                        uint32_t size);
static void wifi_audio_ring_buffer_reset(wifi_audio_ring_buffer_t *buf);
static bool wifi_audio_ring_buffer_is_empty(wifi_audio_ring_buffer_t *buf);
static bool wifi_audio_ring_buffer_is_full(wifi_audio_ring_buffer_t *buf);
static uint32_t wifi_audio_ring_buffer_get_free_space(wifi_audio_ring_buffer_t *buf);
static uint32_t wifi_audio_ring_buffer_get_data_size(wifi_audio_ring_buffer_t *buf);
static uint32_t wifi_audio_ring_buffer_write(wifi_audio_ring_buffer_t *buf, const uint8_t *data,
                                             uint32_t len, bool auto_discard);
static uint32_t wifi_audio_ring_buffer_read(wifi_audio_ring_buffer_t *buf, uint8_t *data,
                                            uint32_t len);
static uint32_t wifi_audio_ring_buffer_peek(wifi_audio_ring_buffer_t *buf, uint8_t *data,
                                            uint32_t len);
static uint32_t wifi_audio_ring_buffer_skip(wifi_audio_ring_buffer_t *buf, uint32_t len);

int32_t wifi_audio_get_sync_delay_ms(void);

/*============================================================================*
 *                          Ring Buffer Control Functions
 *============================================================================*/

/**
 * @brief Initialize a generic ring buffer
 * @param buf Pointer to ring buffer instance
 * @param storage Pointer to storage array
 * @param size Size of the buffer in bytes
 * @return void
 */
static void wifi_audio_ring_buffer_init(wifi_audio_ring_buffer_t *buf, uint8_t *storage,
                                        uint32_t size)
{
    if (buf == NULL || storage == NULL || size == 0)
    {
        return;
    }

    memset(buf, 0, sizeof(wifi_audio_ring_buffer_t));
    buf->buffer = storage;
    buf->buffer_size = size;
    buf->write_ptr = 0;
    buf->read_ptr = 0;
    buf->data_size = 0;
    buf->is_full = false;

    APP_PRINT_INFO1("Ring buffer initialized, size: %d bytes", size);
}


/**
 * @brief Reset a generic ring buffer (clear all data)
 * @param buf Pointer to ring buffer instance
 * @return void
 */
static void wifi_audio_ring_buffer_reset(wifi_audio_ring_buffer_t *buf)
{
    if (buf == NULL)
    {
        return;
    }

    buf->write_ptr = 0;
    buf->read_ptr = 0;
    buf->data_size = 0;
    buf->is_full = false;
}



/**
 * @brief Check if a ring buffer is empty
 * @param buf Pointer to ring buffer instance
 * @return true if empty, false otherwise
 */
static bool wifi_audio_ring_buffer_is_empty(wifi_audio_ring_buffer_t *buf)
{
    if (buf == NULL)
    {
        return true;
    }
    return (buf->data_size == 0);
}



/**
 * @brief Check if a ring buffer is full
 * @param buf Pointer to ring buffer instance
 * @return true if full, false otherwise
 */
static bool wifi_audio_ring_buffer_is_full(wifi_audio_ring_buffer_t *buf)
{
    if (buf == NULL)
    {
        return false;
    }
    return buf->is_full;
}

/**
 * @brief Get available space in a ring buffer
 * @param buf Pointer to ring buffer instance
 * @return Number of bytes available for writing
 */
static uint32_t wifi_audio_ring_buffer_get_free_space(wifi_audio_ring_buffer_t *buf)
{
    if (buf == NULL)
    {
        return 0;
    }
    return buf->buffer_size - buf->data_size;
}

/**
 * @brief Get current data size in a ring buffer
 * @param buf Pointer to ring buffer instance
 * @return Number of bytes available for reading
 */
static uint32_t wifi_audio_ring_buffer_get_data_size(wifi_audio_ring_buffer_t *buf)
{
    if (buf == NULL)
    {
        return 0;
    }
    return buf->data_size;
}


/**
 * @brief Write data to a ring buffer (generic)
 * @param buf Pointer to ring buffer instance
 * @param data Pointer to source data
 * @param len Number of bytes to write
 * @param auto_discard Auto discard old data when full
 * @return Number of bytes actually written (0 if buffer is full)
 */
static uint32_t wifi_audio_ring_buffer_write(wifi_audio_ring_buffer_t *buf, const uint8_t *data,
                                             uint32_t len, bool auto_discard)
{
    if (buf == NULL || data == NULL || len == 0)
    {
        return 0;
    }

    /* Check available space */
    uint32_t free_space = wifi_audio_ring_buffer_get_free_space(buf);
    if (free_space == 0)
    {
        if (auto_discard)
        {
            /* Buffer full: discard oldest 25% to make room for new data */
            uint32_t discard_size = buf->buffer_size / 4;
            wifi_audio_ring_buffer_skip(buf, discard_size);
            free_space = wifi_audio_ring_buffer_get_free_space(buf);
            APP_PRINT_WARN1("Ring buffer overflow, cleared %d bytes", discard_size);
        }
        else
        {
            return 0;  /* Buffer full, no auto-discard */
        }
    }

    /* Limit write length to available space */
    uint32_t write_len = (len > free_space) ? free_space : len;
    uint32_t remaining = write_len;

    /* Write data in two parts if wrap around occurs */
    uint32_t first_part = buf->buffer_size - buf->write_ptr;
    if (first_part >= remaining)
    {
        /* No wrap around, write in one go */
        memcpy(&buf->buffer[buf->write_ptr], data, remaining);
        buf->write_ptr = (buf->write_ptr + remaining) % buf->buffer_size;
    }
    else
    {
        /* Wrap around: write in two parts */
        memcpy(&buf->buffer[buf->write_ptr], data, first_part);
        memcpy(&buf->buffer[0], data + first_part, remaining - first_part);
        buf->write_ptr = remaining - first_part;
    }

    /* Update data size and full flag */
    buf->data_size += write_len;
    if (buf->data_size >= buf->buffer_size)
    {
        buf->is_full = true;
    }

    return write_len;
}



/**
 * @brief Read data from a ring buffer (generic)
 * @param buf Pointer to ring buffer instance
 * @param data Pointer to destination buffer
 * @param len Number of bytes to read
 * @return Number of bytes actually read (0 if buffer is empty)
 */
static uint32_t wifi_audio_ring_buffer_read(wifi_audio_ring_buffer_t *buf, uint8_t *data,
                                            uint32_t len)
{
    if (buf == NULL || data == NULL || len == 0)
    {
        return 0;
    }

    /* Check available data */
    uint32_t available = buf->data_size;
    if (available == 0)
    {
        return 0;
    }

    /* Limit read length to available data */
    uint32_t read_len = (len > available) ? available : len;
    uint32_t remaining = read_len;

    /* Read data in two parts if wrap around occurs */
    uint32_t first_part = buf->buffer_size - buf->read_ptr;
    if (first_part >= remaining)
    {
        /* No wrap around, read in one go */
        memcpy(data, &buf->buffer[buf->read_ptr], remaining);
        buf->read_ptr = (buf->read_ptr + remaining) % buf->buffer_size;
    }
    else
    {
        /* Wrap around: read in two parts */
        memcpy(data, &buf->buffer[buf->read_ptr], first_part);
        memcpy(data + first_part, &buf->buffer[0], remaining - first_part);
        buf->read_ptr = remaining - first_part;
    }

    /* Update data size and full flag */
    buf->data_size -= read_len;
    buf->is_full = false;

    return read_len;
}


/**
 * @brief Peek data from ring buffer without removing it (generic)
 * @param buf Pointer to ring buffer instance
 * @param data Pointer to destination buffer
 * @param len Number of bytes to peek
 * @return Number of bytes actually peeked
 */
static uint32_t wifi_audio_ring_buffer_peek(wifi_audio_ring_buffer_t *buf, uint8_t *data,
                                            uint32_t len)
{
    if (buf == NULL || data == NULL || len == 0)
    {
        return 0;
    }

    /* Check available data */
    uint32_t available = buf->data_size;
    if (available == 0)
    {
        return 0;
    }

    /* Limit peek length to available data */
    uint32_t peek_len = (len > available) ? available : len;
    uint32_t remaining = peek_len;
    uint32_t temp_read_ptr = buf->read_ptr;

    /* Peek data in two parts if wrap around occurs */
    uint32_t first_part = buf->buffer_size - temp_read_ptr;
    if (first_part >= remaining)
    {
        /* No wrap around */
        memcpy(data, &buf->buffer[temp_read_ptr], remaining);
    }
    else
    {
        /* Wrap around */
        memcpy(data, &buf->buffer[temp_read_ptr], first_part);
        memcpy(data + first_part, &buf->buffer[0], remaining - first_part);
    }

    return peek_len;
}

/**
 * @brief Skip (discard) specified number of bytes from ring buffer (generic)
 * @param buf Pointer to ring buffer instance
 * @param len Number of bytes to skip
 * @return Number of bytes actually skipped
 */
static uint32_t wifi_audio_ring_buffer_skip(wifi_audio_ring_buffer_t *buf, uint32_t len)
{
    if (buf == NULL || len == 0)
    {
        return 0;
    }

    /* Limit skip length to available data */
    uint32_t available = buf->data_size;
    uint32_t skip_len = (len > available) ? available : len;

    /* Update read pointer */
    buf->read_ptr = (buf->read_ptr + skip_len) % buf->buffer_size;

    /* Update data size and full flag */
    buf->data_size -= skip_len;
    buf->is_full = false;

    return skip_len;
}

#endif /* CONFIG_WIFI_AUDIO_PLAYER || CONFIG_WIFI_AUDIO_RECORDER */



/*============================================================================*
 *                              Audio Player Start
 *============================================================================*/

#if  CONFIG_WIFI_AUDIO_PLAYER

#define RECORD_PLAY_LEVEL_HIGH      240  //high level  ms
#define RECORD_PLAY_LEVEL_LOW        100  //low  level  ms

/* WiFi Audio Ring Buffer Configuration */
#ifndef WIFI_AUDIO_REV_BUF_SIZE
#define WIFI_AUDIO_REV_BUF_SIZE     (10*1024 )  /* 10KB buffer - reduced for lower latency */
#endif

#define WIFI_AUDIO_PLAY_FRAME_SIZE    (512)

typedef enum
{
    WIFI_AUDIO_PLAY_WRITE        = 0x00,
} T_WIFI_AUDIO_TIMER;

typedef enum _T_AUDIO_RECORD_FORMAT
{
    OPUS_16KHZ_16KBPS_CBR_0_20MS,
    MIC_PCM_16KHZ,
    MIC_PCM_8KHZ,

} T_AUDIO_RECORD_FORMAT;

typedef struct
{
    T_AUDIO_TRACK_HANDLE record_handle;
    T_AUDIO_TRACK_HANDLE play_handle;
    T_AUDIO_RECORD_FORMAT format;
    uint32_t play_ofs;
    uint32_t record_ofs;
    float total_time;
    float current_time;
    bool is_record_start;
} AUDIO_RECORD;


/*============================================================================*
  *                                        Variables
  *============================================================================*/
static AUDIO_RECORD wifi_mic_player =
{
    .record_handle = NULL,
    .play_handle = NULL,
    .format = MIC_PCM_8KHZ,
    .total_time = 0.0f,
    .current_time = 0.0f,
    .is_record_start = false,
};

static bool record_play_level_high = false;
static uint16_t g_play_seq_num = 0;
static bool g_play_started = false;
static uint32_t g_audio_rtp_timestamp = 0;  // Last audio RTP timestamp
static uint32_t g_video_rtp_timestamp = 0;  // Last video RTP timestamp for sync reference



/* Playback buffer storage (10KB) */
static DSP_RAM_BSS_SECTION uint8_t audio_play_buf_storage[WIFI_AUDIO_REV_BUF_SIZE] = {0};

/* Playback buffer instance */
static DSP_RAM_BSS_SECTION wifi_audio_ring_buffer_t audio_play_buf = {0};

static uint8_t wifi_audio_player_timer_id = 0;
static uint8_t wifi_audio_play_write_timer = 0;

/**
 * @brief Initialize the WiFi audio playback ring buffer
 * @return void
 */
void wifi_audio_rev_buffer_init(void)
{
    wifi_audio_ring_buffer_init(&audio_play_buf, audio_play_buf_storage, WIFI_AUDIO_REV_BUF_SIZE);
    APP_PRINT_INFO0("Playback buffer initialized");
}
/**
 * @brief Reset the WiFi audio playback ring buffer (clear all data)
 * @return void
 */
void wifi_audio_rev_buffer_reset(void)
{
    wifi_audio_ring_buffer_reset(&audio_play_buf);
}

/**
 * @brief Check if the playback ring buffer is empty
 * @return true if empty, false otherwise
 */
bool wifi_audio_rev_buffer_is_empty(void)
{
    return wifi_audio_ring_buffer_is_empty(&audio_play_buf);
}

/**
 * @brief Check if the playback ring buffer is full
 * @return true if full, false otherwise
 */
bool wifi_audio_rev_buffer_is_full(void)
{
    return wifi_audio_ring_buffer_is_full(&audio_play_buf);
}

/**
 * @brief Get available space in the playback ring buffer
 * @return Number of bytes available for writing
 */
uint32_t wifi_audio_rev_buffer_get_free_space(void)
{
    return wifi_audio_ring_buffer_get_free_space(&audio_play_buf);
}

/**
 * @brief Get current data size in the playback ring buffer
 * @return Number of bytes available for reading
 */
uint32_t wifi_audio_rev_buffer_get_data_size(void)
{
    return wifi_audio_ring_buffer_get_data_size(&audio_play_buf);
}
/**
 * @brief Write data to the playback ring buffer
 * @param data Pointer to source data
 * @param len Number of bytes to write
 * @return Number of bytes actually written (0 if buffer is full)
 */
uint32_t wifi_audio_rev_buffer_write(const uint8_t *data, uint32_t len)
{
    return wifi_audio_ring_buffer_write(&audio_play_buf, data, len, true);
}

/**
 * @brief Read data from the playback ring buffer
 * @param data Pointer to destination buffer
 * @param len Number of bytes to read
 * @return Number of bytes actually read (0 if buffer is empty)
 */
uint32_t wifi_audio_rev_buffer_read(uint8_t *data, uint32_t len)
{
    return wifi_audio_ring_buffer_read(&audio_play_buf, data, len);
}

/**
 * @brief Peek data from playback ring buffer without removing it
 * @param data Pointer to destination buffer
 * @param len Number of bytes to peek
 * @return Number of bytes actually peeked
 */
uint32_t wifi_audio_rev_buffer_peek(uint8_t *data, uint32_t len)
{
    return wifi_audio_ring_buffer_peek(&audio_play_buf, data, len);
}

/**
 * @brief Skip (discard) specified number of bytes from playback ring buffer
 * @param len Number of bytes to skip
 * @return Number of bytes actually skipped
 */
uint32_t wifi_audio_rev_buffer_skip(uint32_t len)
{
    return wifi_audio_ring_buffer_skip(&audio_play_buf, len);
}


static void audio_voice_gen_format_info(T_AUDIO_FORMAT_INFO *p_format_info,
                                        T_AUDIO_RECORD_FORMAT audio_format)
{
    APP_PRINT_TRACE1("audio_voice_gen_format_info: audio_format %d", audio_format);

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
    case MIC_PCM_8KHZ:
        p_format_info->type = AUDIO_FORMAT_TYPE_PCM;
        p_format_info->attr.pcm.sample_rate = 8 * 1000;//16 * 1000;
        p_format_info->attr.pcm.bit_width = 16;
        p_format_info->attr.pcm.chann_num = 1;
        p_format_info->attr.pcm.frame_length = WIFI_AUDIO_PLAY_FRAME_SIZE;
        p_format_info->attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;

    default:
        APP_PRINT_ERROR0("gen_format_info: only support XIAOAI_OPUS_16KHZ_16KBPS_CBR_0_20MS");
        break;
    }
}

/**
    * @brief        This function can stop record data playing.
    * @return       void
    */
void wifi_audio_stop_play(void)
{
    APP_PRINT_INFO0("wifi_audio_stop_play");
    if (wifi_mic_player.play_handle)
    {
        audio_track_stop(wifi_mic_player.play_handle);
        audio_track_release(wifi_mic_player.play_handle);

    }
    g_play_started = false;
    // wifi_mic_player.play_ofs = 0;
    // app_stop_timer(&timer_idx_record_update_time);
    // app_stop_timer(&timer_idx_record_write);
}

#define AUDIO_PLAY_LEVEL_HIGH_THRESHOLD   500 //max=640ms PLAYBACK_POOL_SIZE app_cfg.h
uint16_t wifi_audio_play_get_remain_bytes(void)
{
    uint16_t level_ms = 0;

    if (wifi_mic_player.play_handle == NULL || !g_play_started)
    {
        return 0;
    }

    if (audio_track_buffer_level_get(wifi_mic_player.play_handle, &level_ms))
    {
        APP_PRINT_INFO1("[Yuyin] wifi_audio_play_get_level = %d", level_ms);
        if (level_ms >= AUDIO_PLAY_LEVEL_HIGH_THRESHOLD)
        {
            DBG_DIRECT("[Yuyin] play level high");
            return 0;
        }
        uint16_t left_ms = AUDIO_PLAY_LEVEL_HIGH_THRESHOLD - level_ms;
        uint16_t left_bytes = left_ms * 8/*8K sample rate*/ * 2/*2 byte = 16bit*/ ;
        return left_bytes;
    }

    APP_PRINT_INFO0("[Yuyin] audio_play_get_level 0");
    return 0;
}

void wifi_audio_play_data_write(const uint8_t *buf, uint16_t len)
{
    uint16_t remain_bytes = wifi_audio_play_get_remain_bytes();
    uint16_t written_len = 0;

    if (wifi_mic_player.play_handle == NULL)
    {
        wifi_audio_rev_buffer_write(buf, len);
        return;
    }

    /* Audio-Video Sync: Drop audio that's too far behind */
    int32_t sync_diff = wifi_audio_get_sync_delay_ms();
    if (sync_diff < -200)  // Audio is 200ms behind video
    {
        DBG_DIRECT("[Sync] Drop audio: too old (diff=%dms)", sync_diff);
        return;  // Discard this audio frame to catch up
    }

    /* 1. Try to write to codec if queue is empty AND track is started */
    if (wifi_audio_rev_buffer_is_empty() && g_play_started)
    {
        uint16_t offset = 0;
        while (remain_bytes >= WIFI_AUDIO_PLAY_FRAME_SIZE &&
               (len - written_len) >= WIFI_AUDIO_PLAY_FRAME_SIZE)
        {
            g_play_seq_num++;
            uint16_t actual_written = 0;
            bool ret = audio_track_write(wifi_mic_player.play_handle, 0xFFFFFFFF,
                                         g_play_seq_num,
                                         AUDIO_STREAM_STATUS_CORRECT,
                                         1,
                                         (void *)(buf + offset),
                                         WIFI_AUDIO_PLAY_FRAME_SIZE,
                                         &actual_written);

            APP_PRINT_INFO2("play data write direct: req=%d, act=%d", WIFI_AUDIO_PLAY_FRAME_SIZE,
                            actual_written);

            if (ret == true && actual_written > 0)
            {
                written_len += actual_written;
                offset += actual_written;
                if (remain_bytes >= actual_written)
                {
                    remain_bytes -= actual_written;
                }
                else
                {
                    remain_bytes = 0;
                }
            }
            else
            {
                APP_PRINT_ERROR1("play data write direct fail ret=%d", ret);
                break;
            }
        }

        if (written_len < len)
        {
            /* 2. Write remainder to queue */
            wifi_audio_rev_buffer_write(buf + written_len, len - written_len);
        }
    }
    else
    {
        /* Queue not empty OR not started, write new data to queue first */
        wifi_audio_rev_buffer_write(buf, len);
    }

    /* 3. Drain queue to codec if space available AND started */
    if (!wifi_audio_rev_buffer_is_empty() && g_play_started)
    {
        if (remain_bytes >= WIFI_AUDIO_PLAY_FRAME_SIZE)
        {
            uint8_t temp_buf[WIFI_AUDIO_PLAY_FRAME_SIZE];

            while (remain_bytes >= WIFI_AUDIO_PLAY_FRAME_SIZE &&
                   wifi_audio_rev_buffer_get_data_size() >= WIFI_AUDIO_PLAY_FRAME_SIZE)
            {
                wifi_audio_rev_buffer_read(temp_buf, WIFI_AUDIO_PLAY_FRAME_SIZE);

                g_play_seq_num++;
                uint16_t actual_written = 0;
                bool ret = audio_track_write(wifi_mic_player.play_handle, 0xFFFFFFFF,
                                             g_play_seq_num,
                                             AUDIO_STREAM_STATUS_CORRECT,
                                             1,
                                             temp_buf,
                                             WIFI_AUDIO_PLAY_FRAME_SIZE,
                                             &actual_written);

                APP_PRINT_INFO2("play data drain queue: req=%d, act=%d", WIFI_AUDIO_PLAY_FRAME_SIZE,
                                actual_written);

                if (ret == true && actual_written > 0)
                {
                    if (remain_bytes >= actual_written)
                    {
                        remain_bytes -= actual_written;
                    }
                    else
                    {
                        remain_bytes = 0;
                    }
                }
                else
                {
                    APP_PRINT_ERROR1("play data drain queue fail ret=%d", ret);
                    break;
                }
            }
        }
    }
}

void wifi_audio_play_queue_process(void)
{
    if (wifi_mic_player.play_handle == NULL || !g_play_started)
    {
        return;
    }

    uint16_t remain_bytes = wifi_audio_play_get_remain_bytes();

    if (remain_bytes >= WIFI_AUDIO_PLAY_FRAME_SIZE && !wifi_audio_rev_buffer_is_empty())
    {
        uint8_t temp_buf[WIFI_AUDIO_PLAY_FRAME_SIZE];

        while (remain_bytes >= WIFI_AUDIO_PLAY_FRAME_SIZE &&
               wifi_audio_rev_buffer_get_data_size() >= WIFI_AUDIO_PLAY_FRAME_SIZE)
        {
            wifi_audio_rev_buffer_read(temp_buf, WIFI_AUDIO_PLAY_FRAME_SIZE);

            g_play_seq_num++;
            uint16_t actual_written = 0;
            bool ret = audio_track_write(wifi_mic_player.play_handle, 0xFFFFFFFF,
                                         g_play_seq_num,
                                         AUDIO_STREAM_STATUS_CORRECT,
                                         1,
                                         temp_buf,
                                         WIFI_AUDIO_PLAY_FRAME_SIZE,
                                         &actual_written);

            APP_PRINT_INFO2("play queue process: req=%d, act=%d", WIFI_AUDIO_PLAY_FRAME_SIZE, actual_written);

            if (ret == true && actual_written > 0)
            {
                if (remain_bytes >= actual_written)
                {
                    remain_bytes -= actual_written;
                }
                else
                {
                    remain_bytes = 0;
                }
            }
            else
            {
                APP_PRINT_ERROR1("play queue process fail ret=%d", ret);
                break;
            }
        }
    }
}


static void wifi_audio_play_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != wifi_mic_player.play_handle)
    {
        DBG_DIRECT("[Yuyin] wifi_audio_play_cback handle error");
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            APP_PRINT_INFO1("wifi_audio_play_cback: track_state_changed.state %d",
                            param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                g_play_started = true;
                app_start_timer(&wifi_audio_play_write_timer, "wifi_audio_player_timer",
                                wifi_audio_player_timer_id, WIFI_AUDIO_PLAY_WRITE, 0, true,
                                20);
                break;

            case AUDIO_TRACK_STATE_STOPPED:
                g_play_started = false;
                app_stop_timer(&wifi_audio_play_write_timer);
                wifi_mic_player.play_handle = NULL;
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
        APP_PRINT_TRACE1("wifi_audio_play_cback: event_type 0x%04x", event_type);
    }
}

void audio_player_init(void)
{
    T_AUDIO_FORMAT_INFO format_info = {};
    audio_voice_gen_format_info(&format_info, wifi_mic_player.format);

    if (wifi_mic_player.play_handle != NULL)
    {
        APP_PRINT_ERROR0("audio_record_init_player: already playing");
        return;
    }

    wifi_mic_player.play_handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK,
                                                     AUDIO_STREAM_MODE_NORMAL,
                                                     AUDIO_STREAM_USAGE_LOCAL,
                                                     format_info,
                                                     8,
                                                     0,
                                                     AUDIO_DEVICE_OUT_SPK,
                                                     NULL,
                                                     NULL);

    if (wifi_mic_player.play_handle == NULL)
    {
        APP_PRINT_ERROR0("wifi_audio_player_init: handle is NULL");
    }
    else
    {
        audio_track_threshold_set(wifi_mic_player.play_handle, RECORD_PLAY_LEVEL_HIGH,
                                  RECORD_PLAY_LEVEL_LOW);
        record_play_level_high = false;
    }


    // if (wifi_mic_player.recrod_mode == AUDIO_RECORD_SAVE_FS)
    // {
    //     fs_file_t_init(&play_dat);
    //     int res = fs_open(&play_dat, (const char *)record_file, FS_O_READ);
    //     if (res < 0)
    //     {
    //         APP_PRINT_ERROR0("wifi_audio_player_init: open record file error!");
    //         return;
    //     }
    // }

    //audio_record_update_ui_timer_start();
    //gui_update_by_event(GUI_EVENT_RECORD, &wifi_mic_player.total_time, false);

    if (audio_track_start(wifi_mic_player.play_handle))
    {
        APP_PRINT_ERROR0("wifi_audio_player_init: audio_track_start success");
    }
    else
    {
        APP_PRINT_ERROR0("wifi_audio_player_init: audio_track_start fail");
    }
}

void wifi_audio_play_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("wifi_audio_play_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case WIFI_AUDIO_PLAY_WRITE:
        {
            //app_stop_timer(&wifi_audio_player_timer_id);
            //chatgpt_play_write(p_play_secne);
            wifi_audio_play_queue_process();
        }
        break;
    default:
        break;
    }
}

void wifi_audio_set_video_timestamp(uint32_t ts)
{
    g_video_rtp_timestamp = ts;
}

int32_t wifi_audio_get_sync_delay_ms(void)
{
    // Calculate audio-video time difference
    // Video @ 90kHz, Audio @ 8kHz
    // Convert to common time base (ms)
    if (g_video_rtp_timestamp == 0 || g_audio_rtp_timestamp == 0)
    {
        return 0;  // No sync data yet
    }

    // Convert timestamps to milliseconds
    int32_t video_ms = (int32_t)(g_video_rtp_timestamp / 90);  // 90kHz -> ms
    int32_t audio_ms = (int32_t)(g_audio_rtp_timestamp / 8);   // 8kHz -> ms

    // Positive: audio ahead, Negative: video ahead
    int32_t diff_ms = audio_ms - video_ms;

    DBG_DIRECT("[Sync] V_ts=%u (=%dms), A_ts=%u (=%dms), diff=%dms",
               g_video_rtp_timestamp, video_ms,
               g_audio_rtp_timestamp, audio_ms, diff_ms);

    return diff_ms;
}

void wifi_audio_player_init(void)
{
    wifi_audio_rev_buffer_init();
    g_play_seq_num = 0;
    g_play_started = false;
    g_audio_rtp_timestamp = 0;
    g_video_rtp_timestamp = 0;

    audio_player_init();

    if (wifi_audio_player_timer_id == 0)
    {
        app_timer_reg_cb(wifi_audio_play_timeout_cb, &wifi_audio_player_timer_id);
    }
    audio_mgr_cback_register(wifi_audio_play_cback);
}


void wifi_audio_rtp_depayload(uint8_t *pkt, uint16_t len)
{
    if (len < 12) { return; }

    // Parse RTP header
    uint8_t cc = pkt[0] & 0x0F;           // CSRC count
    uint8_t ext = (pkt[0] & 0x10) >> 4;   // Extension bit

    // Extract RTP timestamp (bytes 4-7, big-endian)
    uint32_t rtp_timestamp = ((uint32_t)pkt[4] << 24) |
                             ((uint32_t)pkt[5] << 16) |
                             ((uint32_t)pkt[6] << 8)  |
                             ((uint32_t)pkt[7]);
    g_audio_rtp_timestamp = rtp_timestamp;

    size_t hdr_len = 12 + cc * 4;         // Base + CSRC

    // Skip extension header if present
    if (ext && len >= hdr_len + 4)
    {
        uint16_t ext_len = (pkt[hdr_len + 2] << 8) | pkt[hdr_len + 3];
        hdr_len += 4 + ext_len * 4;
    }

    if (hdr_len >= len) { return; }  // Invalid header

    const uint8_t *payload = pkt + hdr_len;
    size_t payload_len = len - hdr_len;

    // Convert from big-endian to little-endian (16-bit PCM samples)
    static uint8_t le_buffer[1050];  // Expanded buffer to handle up to 2KB RTP payload

    if (payload_len > sizeof(le_buffer))
    {
        APP_PRINT_WARN2("RTP payload too large: %d > %d, truncating", payload_len, sizeof(le_buffer));
        payload_len = sizeof(le_buffer);
    }

    // Ensure even number of bytes for 16-bit samples
    payload_len &= ~1;

    // Swap byte order for each 16-bit sample
    for (size_t i = 0; i < payload_len; i += 2)
    {
        le_buffer[i] = payload[i + 1];      // Low byte
        le_buffer[i + 1] = payload[i];      // High byte
    }

    void wifi_audio_play_data_write(const uint8_t *buf, uint16_t len);
    wifi_audio_play_data_write(le_buffer, payload_len);
    //wifi_audio_rev_buffer_write();
}

#endif //CONFIG_WIFI_AUDIO_PLAYER

/////////////////////////////////////////////// Audio Record Start /////////////////////////////////////////////////

#if  CONFIG_WIFI_AUDIO_RECORDER
/********************************************************************************************************
* WiFi Audio Record Configuration and Buffer Management
********************************************************************************************************/

/* WiFi Audio Record Buffer Configuration */
#ifndef WIFI_AUDIO_RECORD_BUF_SIZE
#define WIFI_AUDIO_RECORD_BUF_SIZE  (5*1024)    /* 5KB buffer - lower latency for recording */
#endif

#define WIFI_AUDIO_RECORD_FRAME_SIZE  (320)     /* 8kHz * 20ms * 2byte = 320 bytes */

/*============================================================================*
 *                              Record Buffer Instance
 *============================================================================*/
/* Record buffer storage (5KB) */
static DSP_RAM_BSS_SECTION uint8_t audio_record_buf_storage[WIFI_AUDIO_RECORD_BUF_SIZE] = {0};

/* Record buffer instance */
static DSP_RAM_BSS_SECTION wifi_audio_ring_buffer_t audio_record_buf = {0};

static uint8_t wifi_audio_recorder_timer_id = 0;
static uint8_t wifi_audio_record_read_timer = 0;

/*============================================================================*
 *                          Record Buffer Control Functions
 *============================================================================*/

/**
 * @brief Initialize the WiFi audio record ring buffer
 * @return void
 */
void wifi_audio_record_buffer_init(void)
{
    wifi_audio_ring_buffer_init(&audio_record_buf, audio_record_buf_storage,
                                WIFI_AUDIO_RECORD_BUF_SIZE);
    APP_PRINT_INFO0("Record buffer initialized");
}

/**
 * @brief Reset the WiFi audio record ring buffer (clear all data)
 * @return void
 */
void wifi_audio_record_buffer_reset(void)
{
    wifi_audio_ring_buffer_reset(&audio_record_buf);
}

/**
 * @brief Check if the record ring buffer is empty
 * @return true if empty, false otherwise
 */
bool wifi_audio_record_buffer_is_empty(void)
{
    return wifi_audio_ring_buffer_is_empty(&audio_record_buf);
}

/**
 * @brief Get current data size in the record ring buffer
 * @return Number of bytes available for reading
 */
uint32_t wifi_audio_record_buffer_get_data_size(void)
{
    return wifi_audio_ring_buffer_get_data_size(&audio_record_buf);
}

/**
 * @brief Write data to the record ring buffer
 * @param data Pointer to source data
 * @param len Number of bytes to write
 * @return Number of bytes actually written
 * @note When buffer is full, automatically discards oldest 25% to make room for new data
 */
uint32_t wifi_audio_record_buffer_write(const uint8_t *data, uint32_t len)
{
    return wifi_audio_ring_buffer_write(&audio_record_buf, data, len, true);
}

/**
 * @brief Read data from the record ring buffer
 * @param data Pointer to destination buffer
 * @param len Number of bytes to read
 * @return Number of bytes actually read (0 if buffer is empty)
 */
uint32_t wifi_audio_record_buffer_read(uint8_t *data, uint32_t len)
{
    return wifi_audio_ring_buffer_read(&audio_record_buf, data, len);
}

/*============================================================================*
 *                          Voice Recorder APIs
 *============================================================================*/

T_AUDIO_TRACK_HANDLE wifi_record_hdl = NULL;

bool wifi_recorder_read_cb(T_AUDIO_TRACK_HANDLE  handle,
                           uint32_t             *timestamp,
                           uint16_t             *seq_num,
                           T_AUDIO_STREAM_STATUS *status,
                           uint8_t              *frame_num,
                           void                 *buf,
                           uint16_t              required_len,
                           uint16_t             *actual_len)
{
    DBG_DIRECT("wifi_recorder_read_cb: buf 0x%08x, required_len %d seq_num %d frame_num %d",
               buf,
               required_len, *seq_num, *frame_num);
    if (handle == wifi_record_hdl)
    {
        /* Write captured audio data to record buffer */
        //////test start!!!!
        // static uint32_t offset = 0;
        // extern unsigned char audio_test_data[];
        // uint32_t written = wifi_audio_record_buffer_write((uint8_t *)audio_test_data + (offset%(130*1024)), required_len);
        // offset += written;

        //////test end!!!!

        uint32_t written = wifi_audio_record_buffer_write((uint8_t *)buf, required_len);

        if (written < required_len)
        {
            DBG_DIRECT("Record buffer full, written %d/%d bytes", written, required_len);
        }
        else
        {
            DBG_DIRECT("Recorded %d bytes, buffer level: %d bytes",
                       written, wifi_audio_record_buffer_get_data_size());
        }

        /* TODO: Add encoding logic here if needed */
        // uint8_t out[45];
        // int out_len = 0;
        // out[0] = voice_seq++;
        // wifi_opus_encoder(buf, &out[1], &out_len);
        // voice_data_in(out, out_len + 1, 1);
    }

    *actual_len = required_len;
    return true;
}

/**
 * @brief Send recorded audio data from buffer via network
 * @return Number of bytes sent
 */
uint32_t wifi_audio_record_send_data(void)
{
    uint32_t data_size = wifi_audio_record_buffer_get_data_size();

    if (data_size < WIFI_AUDIO_RECORD_FRAME_SIZE)
    {
        /* Not enough data to send a complete frame */
        return 0;
    }

    uint8_t temp_buf[WIFI_AUDIO_RECORD_FRAME_SIZE];
    uint32_t sent_total = 0;

    /* Send all complete frames available */
    while (data_size >= WIFI_AUDIO_RECORD_FRAME_SIZE)
    {
        uint32_t read_len = wifi_audio_record_buffer_read(temp_buf, WIFI_AUDIO_RECORD_FRAME_SIZE);

        if (read_len == WIFI_AUDIO_RECORD_FRAME_SIZE)
        {
            /* TODO: Send via RTP or other protocol */
            // wifi_audio_rtp_payload_send(temp_buf, read_len);

            sent_total += read_len;
            data_size -= read_len;

            APP_PRINT_INFO2("Sent %d bytes, remaining %d bytes in buffer", read_len, data_size);
        }
        else
        {
            APP_PRINT_ERROR1("Record buffer read failed, expected %d bytes", WIFI_AUDIO_RECORD_FRAME_SIZE);
            break;
        }
    }

    return sent_total;
}

void wifi_recorder_start(void)
{
    if (wifi_record_hdl != NULL)
    {
        APP_PRINT_ERROR0("wifi_recorder_start: already recording");
        return;
    }

    /* Initialize record buffer */
    wifi_audio_record_buffer_init();
    wifi_audio_record_buffer_reset();

    T_AUDIO_FORMAT_INFO format_info;
    format_info.type = AUDIO_FORMAT_TYPE_PCM;
    format_info.attr.pcm.sample_rate = 8 * 1000;//16 * 1000;
    format_info.attr.pcm.bit_width = 16;
    format_info.attr.pcm.chann_num = 1;
    format_info.attr.pcm.frame_length = WIFI_AUDIO_RECORD_FRAME_SIZE;
    format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;

    wifi_record_hdl = audio_track_create(AUDIO_STREAM_TYPE_RECORD,
                                         AUDIO_STREAM_MODE_NORMAL,
                                         AUDIO_STREAM_USAGE_LOCAL,
                                         format_info,
                                         0,
                                         15,
                                         AUDIO_DEVICE_IN_MIC,
                                         NULL,
                                         wifi_recorder_read_cb);

    if (wifi_record_hdl == NULL)
    {
        APP_PRINT_ERROR0("wifi_recorder_start: handle is NULL");
        return;
    }

    if (audio_track_start(wifi_record_hdl))
    {
        APP_PRINT_ERROR0("wifi_recorder_start: audio_track_start success");
    }
    else
    {
        APP_PRINT_ERROR0("wifi_recorder_start: audio_track_start fail");
    }
}

void wifi_recorder_stop(void)
{
    if (wifi_record_hdl == NULL)
    {
        APP_PRINT_ERROR0("wifi_recorder_stop: already stopped!");
        return;
    }

    APP_PRINT_TRACE0("wifi_recorder_stop");
    audio_track_release(wifi_record_hdl);
    wifi_record_hdl = NULL;
}

#endif /* CONFIG_WIFI_AUDIO_RECORDER */
/////////////////////////////////////////////// Audio Record End /////////////////////////////////////////////////
