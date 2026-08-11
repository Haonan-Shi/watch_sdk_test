/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_SD_CARD_PLAY
#include "trace.h"
#include "string.h"
#include "app_src_playback.h"
#include "app_src_play.h"
#include "app_fs_if.h"
#include "app_mp3_decode.h"
#include "app_pcm_decode.h"
#include "app_link_util_cs.h"
#include "app_cfg.h"
#include "app_dsp_cfg.h"
#include "app_main.h"
#include "app_cmd.h"
#include "app_timer.h"
#include "btm.h"
#include "audio.h"
#include "audio_type.h"
#include "bt_bond.h"
#include "audio_track.h"
#include "app_lea_ini_audio_data.h"
#include "hw_tim.h"
#include "ring_buffer.h"
#include "audio_pipe.h"
#include "app_src_play_a2dp.h"
#include "fmc_api.h"
#include "app_audio_policy.h"
#include "wdg.h"
#include "aon_wdg_ext.h"
#include "rtl876x_wdg.h"
#include "app_dlps.h"

/* dlps related */
static uint16_t s_sd_pd_bitmap = 0;
static uint16_t s_sd_dlps_bitmap = 0;

static uint8_t cur_audio_file_name[FF_MAX_LFN];
static uint8_t cur_audio_file_name_len;
//-128-0
static const uint16_t pcm_gain_table[] =
{
    0x8001, 0xeb00, 0xec80, 0xee00, 0xef80, 0xf100, 0xf280, 0xf400,
    0xf580, 0xf700, 0xf880, 0xfa00, 0xfb80, 0xfd00, 0xfe80, 0x0000
};
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
/* pcm drain ring buffer buffers decoded PCM data from audio pipe (PLAY_ROUTE_PCM) */
#define PCM_DRAIN_RBUF_SIZE             1024 * 20
#define PCM_PLAYBACK_START_THRESHOLD    (10 * 1024)  /* pre-fill before allowing USB consumer to read */
bool app_read_start = false;
bool app_is_first_time_play = false;

struct
{
    T_RING_BUFFER           ring_buf;
    uint8_t                 *buf;
} pcm_drain_rb;
static T_HW_TIMER_HANDLE uac_refill_hw_timer = NULL;
static void uac_refill_hw_cb(T_HW_TIMER_HANDLE timer)
{
    app_src_play_send_msg(SD_PIPE_PCM_FILL_TIMER, NULL);
}
#endif
/* Format for headerless raw PCM, supplied by the host before a play command.
 * Only used when the file has no parsable header (raw .pcm). */
static struct
{
    uint32_t sample_rate;
    uint8_t  chann_num;
    uint8_t  bit_width;
    bool     valid;
} g_pcm_fmt_cfg;

/* source codec of the file currently being played */
typedef enum
{
    SD_SRC_CODEC_MP3,
    SD_SRC_CODEC_PCM,
} T_SD_SRC_CODEC;

/* pipe related */
typedef enum
{
    SD_PLAY_STATE_NONE,
    SD_PLAY_STATE_FILLING,
    SD_PLAY_STATE_FILLED,
} T_SD_PIPE_STATE;

/* timer related */
typedef enum
{
    APP_TIMER_SD_LOCAL_PUT_DATA,
    APP_TIMER_SD_DELAY_STOP_LOCAL,
    APP_TIMER_SD_DELAY_STOP_PIPE,
} T_APP_SRC_PLAYBACK_TIMER;

static uint8_t timer_idx_sd_local_put_data = 0;
static uint8_t timer_idx_sd_delay_stop_local = 0;
static uint8_t timer_idx_sd_delay_stop_pipe = 0;
static uint8_t app_src_play_sd_timer_id = 0;

/* le audio related */
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
#define LC3_RBUF_SIZE                  1024 * 3 + 256
static struct
{
    bool                    timer_started;
    T_HW_TIMER_HANDLE       timer_handle;
    uint32_t                timer_duration;
    uint16_t                lc3_tx_seq;
    uint8_t                 *p_lea_send_buf;
    uint16_t                pkt_len;
    T_RING_BUFFER           ring_buf;
    uint8_t                 target_threshold;
    uint8_t                 pre_fill_num;
} lea_tx_mgr =
{
    .timer_handle = NULL,
    .timer_duration = 10000,
    .timer_started = false,
    .lc3_tx_seq = 0,
    .p_lea_send_buf = NULL,
    .pkt_len = 0,
    .pre_fill_num = 0,
};
#endif

/* sd card manager */
static struct
{
    T_FILE_HANDLE               *fs_handle; \
    uint8_t                     src_codec;      /* T_SD_SRC_CODEC */
    T_PCM_FRAME_INFO            pcm_info;       /* valid when src_codec == SD_SRC_CODEC_PCM */
    uint16_t                    frame_length;   /* frame length: before decode */
    uint8_t                     frame_data[1024 * 7];
#if F_APP_SD_CARD_LOCALPLAY
    struct
    {
        T_AUDIO_TRACK_HANDLE    handle;
        T_SD_PLAY_STATE         play_state;
        T_PLAYBACK_DATA         play_monitor;
        uint8_t                 volume;
    } local_play;
#endif
    struct
    {
        T_AUDIO_PIPE_HANDLE     handle;
        T_SD_PLAY_STATE         play_state;
        uint8_t                 volume;
        T_SD_PIPE_STATE         fill_state;
        uint8_t                 fill_seq;
        void                    *p_drain_buf;
#if F_APP_DBG_DUMP_PCM_TO_FILE
        T_FILE_HANDLE           *pcm_dump_file;   /* != NULL when saving decoded PCM to file */
#endif
    } pipe_play;
} sd =
{
    .fs_handle = NULL,
#if F_APP_SD_CARD_LOCALPLAY
    .local_play =
    {
        .play_state = SD_PLAY_STATE_IDLE,
        .handle = NULL,
        .volume = 10,
    },
#endif
    .pipe_play =
    {
        .handle = NULL,
        .volume = 10,
        .play_state = SD_PLAY_STATE_IDLE,
        .fill_state = SD_PLAY_STATE_NONE,
        .p_drain_buf = NULL,
#if F_APP_DBG_DUMP_PCM_TO_FILE
        .pcm_dump_file = NULL,
#endif
    }
};

static struct
{
    uint8_t             *sd_file_name;
    uint16_t            sd_length;
} sd_file_info =
{
    .sd_file_name = NULL,
    .sd_length = 0,
};

static bool src_play_check_and_fill_data(void);
static void app_src_play_sd_pipe_stop(void);
void app_src_play_sd_local_file_close(void);

static int app_src_play_mp3_fs_read(void *handle, uint8_t *buf, uint32_t len)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_read(hdl, buf, len);
}

static int app_src_play_mp3_fs_seek(void *handle, uint32_t offset)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_seek(hdl, offset);
}

static uint32_t app_src_play_mp3_fs_tell(void *handle)
{
    T_FILE_HANDLE *hdl = (T_FILE_HANDLE *)handle;
    return app_fs_tell(hdl);
}

/* case-insensitive compare of a 4-char file extension (e.g. ".pcm") */
static bool app_src_play_ext_is(const uint8_t *p, const char *ext4)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        uint8_t a = p[i];
        uint8_t b = (uint8_t)ext4[i];
        if (a >= 'A' && a <= 'Z') { a += 32; }
        if (b >= 'A' && b <= 'Z') { b += 32; }
        if (a != b) { return false; }
    }
    return true;
}

static uint8_t app_src_play_sd_detect_codec(const uint8_t *name, uint16_t len)
{
    if (len >= 4)
    {
        const uint8_t *ext = name + (len - 4);
        if (app_src_play_ext_is(ext, ".pcm") || app_src_play_ext_is(ext, ".wav"))
        {
            return SD_SRC_CODEC_PCM;
        }
    }
    return SD_SRC_CODEC_MP3;
}

static bool app_src_playback_list_files_cb(const char *name, uint32_t size, void *context)
{
    (void)context;

    size_t full_len = strlen(name);

    if (full_len > (FF_MAX_LFN - 1))
    {
        full_len = FF_MAX_LFN - 1;
    }

    uint8_t rpt[FF_MAX_LFN];

    rpt[0] = (uint8_t)full_len;
    memcpy(&rpt[1], name, full_len);

    app_report_event(CMD_PATH_UART, EVENT_AUDIO_FILE_READ_RESULT, 0, rpt, full_len + 1);

    return true;
}

void app_src_playback_report_local_mp3_files(void)
{
    app_fs_if_list_files(".mp3", app_src_playback_list_files_cb, NULL);
}

/* LE Audio Related*/
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
static void app_src_play_pipe_sd_lea_timer_stop(void)
{
    lea_tx_mgr.timer_started = false;
    hw_timer_stop(lea_tx_mgr.timer_handle);
}

static void app_src_play_pipe_sd_lea_timer_start(void)
{
    lea_tx_mgr.timer_started = true;
    hw_timer_start(lea_tx_mgr.timer_handle);
    hw_timer_restart(lea_tx_mgr.timer_handle, lea_tx_mgr.timer_duration);
}

void app_src_play_sd_pipe_send_lea_data(void)
{
    T_AUDIO_FORMAT_INFO format_info;
    T_RING_BUFFER *p_rbuf = &(lea_tx_mgr.ring_buf);
    uint16_t len = lea_tx_mgr.pkt_len;
    uint8_t *buf = lea_tx_mgr.p_lea_send_buf;

    if (!p_rbuf || !len || !buf)
    {
        return;
    }

    T_PLAY_ROUTE play_route = app_src_play_get_play_route();

    if ((play_route != PLAY_ROUTE_BIS) &&
        (play_route != PLAY_ROUTE_CIS))
    {
        return;
    }

    if (!ring_buffer_read(p_rbuf, len, buf))
    {
        APP_PRINT_WARN2("lea_tx_timer_callback: ringbuf len %d want %d send fake data",
                        ring_buffer_get_data_count(p_rbuf), len);
        memset(buf, 0, len);
    }

    uint32_t ts = lea_tx_mgr.lc3_tx_seq * lea_tx_mgr.timer_duration;
    app_lea_iso_data_send(buf, len, true, ts, lea_tx_mgr.lc3_tx_seq);
    lea_tx_mgr.lc3_tx_seq++;

    src_play_check_and_fill_data();
}

static void sd_pipe_lea_tx_timer_callback(T_HW_TIMER_HANDLE handle)
{
    app_src_play_send_msg(SD_PIPE_LC3_TIMER, NULL);
}

static void app_src_play_sd_lc3_timer_init(void)
{
    lea_tx_mgr.timer_handle = hw_timer_create("lc3_tx", lea_tx_mgr.timer_duration, true,
                                              sd_pipe_lea_tx_timer_callback);

    hw_timer_lpm_set(lea_tx_mgr.timer_handle, false);
    if (lea_tx_mgr.timer_handle == NULL)
    {
        APP_PRINT_ERROR0("app_src_play_sd_lc3_timer_init: create fail");
    }
}

void app_src_play_sd_handle_lea_stop(void)
{
    lea_tx_mgr.lc3_tx_seq = 0;
}
#endif

#if F_APP_SD_CARD_LOCALPLAY
static void app_src_play_sd_local_delay_stop(uint16_t time_ms)
{
    uint16_t delay_ms = time_ms;
    APP_PRINT_TRACE1("app_src_play_sd_local_delay_stop, time_ms:%d", time_ms);
    delay_ms = time_ms + 50;
    app_start_timer(&timer_idx_sd_delay_stop_local, "src_playback delay stop",
                    app_src_play_sd_timer_id,
                    APP_TIMER_SD_DELAY_STOP_LOCAL, 0, false, delay_ms);
}

static void app_src_playback_put_data_start_timer(uint16_t time_ms)
{
    app_start_timer(&timer_idx_sd_local_put_data, "src_playback_put_data",
                    app_src_play_sd_timer_id, APP_TIMER_SD_LOCAL_PUT_DATA, 0, false,
                    time_ms);
}

static void app_src_playback_put_data(uint8_t pkt_num)
{
    int res = 0;
    uint8_t frame_cnt = 0;
    uint16_t time_ms = sd.local_play.play_monitor.put_data_time_ms;
    static uint16_t s_seq_num = 0;

    APP_PRINT_TRACE1("app_src_playback_put_data: pkt_num %d", pkt_num);
    while (frame_cnt < pkt_num)
    {
        // This maybe AUDIO_EVENT_TRACK_BUFFER_HIGH event
        if (sd.local_play.play_monitor.buffer_state == PLAYBACK_BUF_HIGH)
        {
            time_ms = sd.local_play.play_monitor.put_data_time_ms * 2;
            break;
        }

        T_MP3_FRAME_INFO info;
        res = mp3_parser_get_next_frame(sd.fs_handle, sd.frame_data, sizeof(sd.frame_data), &info);
        if (res <= 0)
        {
            APP_PRINT_ERROR1("app_src_playback_put_data: ERROR, RES 0x%x", res);
            break;
        }
        sd.frame_length = (uint16_t)res;
        // uint16_t frame_content_len = sd.frame_length - 4;
        // uint8_t  *frame_content = &sd.frame_data[4];
        uint16_t frame_content_len = sd.frame_length;
        uint8_t  *frame_content = &sd.frame_data[0];
        uint16_t written_len;
        s_seq_num++;
        if (audio_track_write(sd.local_play.handle,
                              0,
                              s_seq_num,
                              AUDIO_STREAM_STATUS_CORRECT,
                              1,
                              frame_content,
                              frame_content_len,
                              &written_len) == false)
        {
            break;
        }
        frame_cnt++;
    }
    sd.local_play.play_monitor.buffer_state = PLAYBACK_BUF_NORMAL;

    if (res == 0) // EOF
    {
        app_stop_timer(&timer_idx_sd_local_put_data);
        app_src_play_sd_local_delay_stop(sd.local_play.play_monitor.delay_stop_ms);
    }
    else if (sd.local_play.play_state == SD_PLAY_STATE_PLAY)
    {
        app_src_playback_put_data_start_timer(time_ms);
    }
}

static void app_src_playback_put_data_stop_timer(uint8_t pkt_num)
{
    APP_PRINT_TRACE0("app_src_playback_put_data_stop_timer");
    app_stop_timer(&timer_idx_sd_local_put_data);

    if (sd.local_play.play_state != SD_PLAY_STATE_PLAY)
    {
        return;
    }
    app_src_playback_put_data(pkt_num);
}

static void app_src_playback_buffer_low_handle(void)
{
    if (sd.local_play.play_state != SD_PLAY_STATE_PLAY)
    {
        return;
    }
    app_src_playback_put_data(sd.local_play.play_monitor.frame_num + 2);
    sd.local_play.play_monitor.buffer_state = PLAYBACK_BUF_LOW;
}

static void app_src_playback_buffer_high_handle(void)
{
//    app_stop_timer(&timer_idx_sd_local_put_data);
    sd.local_play.play_monitor.buffer_state = PLAYBACK_BUF_HIGH;
}

static void app_src_playback_track_cback(T_AUDIO_EVENT event_type, void *event_buf,
                                         uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    if (param->track_state_changed.handle != sd.local_play.handle)
    {
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            APP_PRINT_INFO1("AUDIO_EVENT_TRACK_STATE_CHANGED: %d", param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_RELEASED:
                {
                    sd.local_play.handle = NULL;
                }
                break;

            case AUDIO_TRACK_STATE_STARTED:
                {
                    app_src_playback_put_data(sd.local_play.play_monitor.preq_pkts);
                }
                break;

            default:
                break;
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_LOW:
        {
            if (sd.local_play.play_state == SD_PLAY_STATE_PLAY)
            {
                app_src_playback_buffer_low_handle();
            }
        }
        break;

    case AUDIO_EVENT_TRACK_BUFFER_HIGH:
        {
            if (sd.local_play.play_state == SD_PLAY_STATE_PLAY)
            {
                app_src_playback_buffer_high_handle();
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("app_src_playback_track_cback: event_type 0x%04x", event_type);
    }
}

// Input file format and get set info
static uint8_t app_src_play_sd_set_local_play_info(T_FILE_FORMAT_INFO   *file_format,
                                                   T_LOCALPLAY_SET_INFO *set_info)
{
    uint32_t sample_rate = 0;
    uint16_t sample_counts = 1024; /* default */
    uint16_t frame_duration = 20; /* default */
    uint16_t frame_size = 512;
    uint8_t channel_mode = 0;
    uint8_t media_buf_pkt_max;

    if (file_format->format_info.type == AUDIO_FORMAT_TYPE_MP3)
    {
        sample_rate = file_format->format_info.attr.mp3.sample_rate;
    }
    frame_duration = file_format->frame_duration;
    sample_counts = file_format->sample_counts;
    frame_size = file_format->frame_size;

    if (frame_duration > 100)
    {
        frame_duration = 50;
    }
    if (frame_size > 2048)
    {
        frame_size = 1024;
    }

    media_buf_pkt_max = PLAYBACK_POOL_SIZE / frame_size;
    uint8_t frm_num = STREAM_BUF_SIZE / 2 / frame_size;
    if (media_buf_pkt_max > 20)
    {
        set_info->latency     = 4 * frame_duration;
        set_info->lower_level   = 1 * frame_duration;
        set_info->upper_level = 15 * frame_duration;
        sd.local_play.play_monitor.frame_num = 4;
    }
    else if (media_buf_pkt_max > 10)
    {
        set_info->latency      = 3 * frame_duration;
        set_info->lower_level   = 1 * frame_duration;
        set_info->upper_level = 9 * frame_duration;
        sd.local_play.play_monitor.frame_num = (frm_num > 4) ? 4 : 3;
    }
    else
    {
        set_info->latency      = 2 * frame_duration;
        set_info->lower_level   = 1 * frame_duration;
        set_info->upper_level = (media_buf_pkt_max - 2) * frame_duration;
        sd.local_play.play_monitor.frame_num = (frm_num > 4) ? 4 : 3;
    }
    set_info->play_duration = sd.local_play.play_monitor.frame_num * frame_duration;
    set_info->preq_pkts = sample_rate * (set_info->latency  + set_info->lower_level) / 1000 /
                          sample_counts + 2;//3;

    APP_PRINT_TRACE4("app_src_play_sd_set_local_play_info: MP3,"
                     " type %d, frame_duration %d, sample_counts %d, frame_size %d",
                     file_format->format_info.type, file_format->frame_duration,
                     file_format->sample_counts, file_format->frame_size);

    APP_PRINT_WARN6("app_src_play_sd_set_local_play_info: put_data_time_ms %d, preq_pkts %d " \
                    "media_buf_pkt_max %d, s_frame_num %d, size %d, frame_duration %d",
                    set_info->play_duration, set_info->preq_pkts,
                    media_buf_pkt_max, sd.local_play.play_monitor.frame_num, frame_size, frame_duration);
    return 0;
}

static void app_src_play_sd_local_start(T_FILE_FORMAT_INFO *file_format)
{
    if (sd.local_play.handle != NULL)
    {
        audio_track_release(sd.local_play.handle);
        sd.local_play.handle = NULL;
    }

    T_LOCALPLAY_SET_INFO set_play_info;

    app_src_play_sd_set_local_play_info(file_format, &set_play_info);
    sd.local_play.play_monitor.put_data_time_ms = set_play_info.play_duration;
    sd.local_play.play_monitor.preq_pkts = set_play_info.preq_pkts;

    uint32_t device = app_db.playback_device;

    sd.local_play.handle = audio_track_create(AUDIO_STREAM_TYPE_PLAYBACK, //stream_type
                                              AUDIO_STREAM_MODE_NORMAL, // mode
                                              AUDIO_STREAM_USAGE_SNOOP, // usage
                                              file_format->format_info, //format_info
                                              sd.local_play.volume, //volume
                                              0,
                                              device, // device
                                              NULL,
                                              NULL);

    if (sd.local_play.handle != NULL)
    {
        audio_track_latency_set(sd.local_play.handle, set_play_info.latency, true);
        audio_track_threshold_set(sd.local_play.handle, set_play_info.upper_level,
                                  set_play_info.lower_level);
        sd.local_play.play_monitor.delay_stop_ms = set_play_info.latency;
    }
    sd.local_play.play_state = SD_PLAY_STATE_PLAY;
    sd.local_play.play_monitor.buffer_state = PLAYBACK_BUF_NORMAL;
    audio_track_start(sd.local_play.handle);
}

static void app_src_play_sd_local_stop(void)
{
    uint8_t res = PLAYBACK_SUCCESS;

    app_stop_timer(&timer_idx_sd_local_put_data);
    sd.local_play.play_state = SD_PLAY_STATE_IDLE;
    if (sd.local_play.handle != NULL)
    {
        audio_track_release(sd.local_play.handle);
    }

    app_src_play_sd_local_file_close();
}
#endif

static void app_src_play_sd_pipe_delay_stop(uint16_t time_ms)
{
    uint16_t delay_ms = time_ms;
    APP_PRINT_TRACE1("app_src_play_sd_pipe_delay_stop, time_ms:%d", time_ms);
    app_start_timer(&timer_idx_sd_delay_stop_pipe, "sd_pipe_delay_stop",
                    app_src_play_sd_timer_id,
                    APP_TIMER_SD_DELAY_STOP_PIPE, 0, false, delay_ms);
}

static void app_src_play_sd_pipe_fill_data(void)
{
    int res = 0;
    if (sd.src_codec == SD_SRC_CODEC_PCM)
    {
        res = pcm_parser_get_next_frame(sd.fs_handle, sd.frame_data, sizeof(sd.frame_data),
                                        &sd.pcm_info);
        APP_PRINT_INFO0("app_src_play_sd_pipe_fill_data get data");
    }
    else
    {
        T_MP3_FRAME_INFO info;
        res = mp3_parser_get_next_frame(sd.fs_handle, sd.frame_data, sizeof(sd.frame_data), &info);
    }
    if (res <= 0)
    {
        if (res == 0)
        {
            if (sd.pipe_play.play_state == SD_PLAY_STATE_PLAY)
            {
                app_src_play_sd_pipe_delay_stop(50);
                sd.pipe_play.play_state = SD_PLAY_STATE_STOPPING;
            }
        }
        else
        {
            APP_PRINT_ERROR1("app_src_play_sd_pipe_fill_data: ERROR, RES 0x%x", res);
        }
        return;
    }
    sd.frame_length = res;
    // uint16_t frame_content_len = sd.frame_length - 4;
    // uint8_t  *frame_content = &sd.frame_data[4];
    uint16_t frame_content_len = sd.frame_length;
    uint8_t  *frame_content = &sd.frame_data[0];

    bool fill_result = audio_pipe_fill(sd.pipe_play.handle,
                                       0,
                                       sd.pipe_play.fill_seq,
                                       AUDIO_STREAM_STATUS_CORRECT,
                                       1,
                                       frame_content,
                                       frame_content_len);
    APP_PRINT_INFO2("app_src_play_sd_pipe_fill_data: seq %d, len %d", sd.pipe_play.fill_seq,
                    frame_content_len);
    if (fill_result)
    {
        sd.pipe_play.fill_seq++;
        sd.pipe_play.fill_state = SD_PLAY_STATE_FILLING;
    }
}

void app_src_play_sd_pipe_fill_data_for_app(void)
{
    app_src_play_sd_pipe_fill_data();
}

bool app_src_play_sd_pipe_is_playing(void)
{
    return sd.pipe_play.play_state == SD_PLAY_STATE_PLAY;
}

static void src_play_pre_fill_data(void)
{
    T_PLAY_ROUTE play_route = app_src_play_get_play_route();

    if (play_route == PLAY_ROUTE_A2DP)
    {
        // TODO: Support A2DP TX
    }
    else if (play_route == PLAY_ROUTE_HFP_AG)
    {
        // TODO: Support HFP TX
    }
    else if (play_route == PLAY_ROUTE_PCM)
    {
        app_src_play_sd_pipe_fill_data();
    }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
    else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
    {
        if (lea_tx_mgr.pre_fill_num > 0)
        {
            app_src_play_sd_pipe_fill_data();
            lea_tx_mgr.pre_fill_num--;
        }
    }
#endif
}

static bool src_play_check_and_fill_data(void)
{
    bool ret = false;
    if (sd.pipe_play.fill_state == SD_PLAY_STATE_FILLED)
    {
        T_PLAY_ROUTE play_route = app_src_play_get_play_route();

        if (play_route == PLAY_ROUTE_A2DP)
        {
            // TODO: Support A2DP TX
            return false;
        }
        else if (play_route == PLAY_ROUTE_HFP_AG)
        {
            // TODO: Support HFP TX
            return false;
        }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
        else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
        {
            uint32_t used_size = ring_buffer_get_data_count(&lea_tx_mgr.ring_buf);
            uint8_t  left_frame_num = used_size / lea_tx_mgr.pkt_len;
            APP_PRINT_INFO2("src_play_check_and_fill_data: used_size %d left_frame_num %d",
                            used_size, left_frame_num);
            if (left_frame_num <= lea_tx_mgr.target_threshold)
            {
                app_src_play_sd_pipe_fill_data();
                ret = true;
            }
        }
#endif
    }

    return ret;
}

static void app_src_play_sd_pipe_start_next_action(uint8_t next_action)
{
    // TODO: restart
}

bool app_src_play_pcm_stop()
{
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
    APP_PRINT_INFO0("emmy app_src_play_pcm_stop");
    app_read_start = false;
#endif
    return true;
}

static void app_src_play_sd_pipe_handle_release(void)
{
    APP_PRINT_INFO0("app_src_play_sd_pipe_handle_release");
    sd.pipe_play.play_state = SD_PLAY_STATE_IDLE;

    T_PLAY_ROUTE play_route = app_src_play_get_play_route();
    sd.pipe_play.handle = NULL;
    if (sd.pipe_play.p_drain_buf)
    {
        free(sd.pipe_play.p_drain_buf);
        sd.pipe_play.p_drain_buf = NULL;
    }
    if (play_route == PLAY_ROUTE_PCM)
    {
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
        app_read_start = false;
        hw_timer_delete(uac_refill_hw_timer);
        uac_refill_hw_timer = NULL;
        ring_buffer_clear(&pcm_drain_rb.ring_buf);
#endif
    }
    else if (play_route == PLAY_ROUTE_A2DP)
    {
        // TODO: Support A2DP TX
        return;
    }
    else if (play_route == PLAY_ROUTE_HFP_AG)
    {
        // TODO: Support HFP TX
        return;
    }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
    else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
    {
        if (lea_tx_mgr.p_lea_send_buf)
        {
            free(lea_tx_mgr.p_lea_send_buf);
            lea_tx_mgr.p_lea_send_buf = NULL;
        }
        app_src_play_pipe_sd_lea_timer_stop();
        ring_buffer_clear(&lea_tx_mgr.ring_buf);
        lea_tx_mgr.lc3_tx_seq = 0;
    }
#endif
#if F_APP_DBG_DUMP_PCM_TO_FILE
    /* Close PCM dump file. */
    if (sd.pipe_play.pcm_dump_file != NULL)
    {
        app_fs_close_file(sd.pipe_play.pcm_dump_file);
        sd.pipe_play.pcm_dump_file = NULL;
    }
#endif
}
static void app_src_play_sd_pipe_handle_data_ind(void)
{
    uint16_t len = 0;
    uint32_t timestamp = 0;
    uint16_t seq = 0;
    T_AUDIO_STREAM_STATUS status;
    uint8_t  frame_num = 0;

    bool ret = audio_pipe_drain(sd.pipe_play.handle,
                                &timestamp,
                                &seq,
                                &status,
                                &frame_num,
                                sd.pipe_play.p_drain_buf,
                                &len);
    if (!ret)
    {
        APP_PRINT_ERROR0("pipe_handle_data_ind: drain failed");
        return;
    }

    T_PLAY_ROUTE play_route = app_src_play_get_play_route();

    if (play_route == PLAY_ROUTE_A2DP)
    {
        // TODO: Support A2DP TX
        return;
    }
    else if (play_route == PLAY_ROUTE_HFP_AG)
    {
        // TODO: Support HFP TX
        return;
    }
    else if (play_route == PLAY_ROUTE_PCM)
    {
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
        /* Write decoded PCM data to drain ring buffer. */
        if (pcm_drain_rb.buf)
        {
            uint32_t remain_size = ring_buffer_get_remaining_space(&pcm_drain_rb.ring_buf);
            if (remain_size < len)
            {
                APP_PRINT_ERROR0("pcm_drain_rb full!!");
            }
            else
            {
                ring_buffer_write(&pcm_drain_rb.ring_buf, sd.pipe_play.p_drain_buf, len);
                if (app_is_first_time_play)
                {
                    /* Wait until ring buffer accumulates enough data before
                     * allowing the USB consumer to start reading. This prevents
                     * early underflow during playback startup. */
                    if (ring_buffer_get_data_count(&pcm_drain_rb.ring_buf) >= PCM_PLAYBACK_START_THRESHOLD)
                    {
                        app_read_start = true;
                        app_is_first_time_play = false;
                    }
                }
            }
        }
#endif
        /* Write decoded PCM data to dump file (debug). */
#if F_APP_DBG_DUMP_PCM_TO_FILE
        if (sd.pipe_play.pcm_dump_file != NULL)
        {
            APP_PRINT_INFO2("pcm_dump: len=%d data(%b)", len,
                            TRACE_BINARY(10, sd.pipe_play.p_drain_buf));
            ssize_t written = app_fs_write(sd.pipe_play.pcm_dump_file,
                                           sd.pipe_play.p_drain_buf, len);
            if (written < 0)
            {
                APP_PRINT_ERROR1("pipe_handle_data_ind: pcm dump write failed %d", written);
            }
        }
        if (app_read_start)
        {
            ring_buffer_read(&pcm_drain_rb.ring_buf, len, sd.pipe_play.p_drain_buf);
        }
#endif

        APP_PRINT_INFO1("app_src_play_sd_pipe_handle_data_ind: drain len %d", len);
    }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
    else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
    {
        uint32_t remain_size = ring_buffer_get_remaining_space(&lea_tx_mgr.ring_buf);
        if (remain_size < len)
        {
            APP_PRINT_ERROR0("BUF_FULL!!");
            return;
        }

        ring_buffer_write(&(lea_tx_mgr.ring_buf), sd.pipe_play.p_drain_buf, len);
    }
#endif
}


static bool app_src_play_sd_pipe_cback(T_AUDIO_PIPE_HANDLE handle, T_AUDIO_PIPE_EVENT event,
                                       uint32_t param)
{
    APP_PRINT_INFO1("app_src_play_sd_pipe_cback: event 0x%x", event);

    switch (event)
    {
    case AUDIO_PIPE_EVENT_CREATED:
        {
            uint32_t snk_buf_size = 0;
            if (sd.src_codec == SD_SRC_CODEC_PCM)
            {
                snk_buf_size = 1024 * 7;
            }
            else
            {
                snk_buf_size = 1024 * 5;
            }

            APP_PRINT_TRACE1("app_src_play_sd_pipe_cback: snk_buf_size:0x%x", snk_buf_size);
            app_audio_pipe_chann_set(handle);
            audio_pipe_start(handle);
            if (sd.pipe_play.p_drain_buf == NULL)
            {
                sd.pipe_play.p_drain_buf = malloc(snk_buf_size);
            }
            sd.pipe_play.play_state = SD_PLAY_STATE_PLAY;
#if F_APP_DBG_DUMP_PCM_TO_FILE
            /* Open PCM dump file for debug saves decoded PCM data from the drain. */
            if (sd.pipe_play.pcm_dump_file == NULL)
            {
                sd.pipe_play.pcm_dump_file = app_fs_open_file_with_timestamp("playback.pcm",
                                                                             FS_O_CREATE | FS_O_WRITE);
                if (sd.pipe_play.pcm_dump_file == NULL)
                {
                    APP_PRINT_WARN0("app_src_play_sd_pipe_cback: failed to open pcm dump file");
                }
            }
#endif
        }
        break;

    case AUDIO_PIPE_EVENT_STARTED:
        {
            src_play_pre_fill_data();
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
            app_is_first_time_play = true;

            if (uac_refill_hw_timer == NULL)
            {
                if (sd.src_codec == SD_SRC_CODEC_PCM)
                {
                    uac_refill_hw_timer = hw_timer_create("uac_refill", 5000, true, uac_refill_hw_cb);

                }
                else
                {
                    uac_refill_hw_timer = hw_timer_create("uac_refill", 16000, true, uac_refill_hw_cb);

                }
            }
            hw_timer_start(uac_refill_hw_timer);
#endif

            T_PLAY_ROUTE play_route = app_src_play_get_play_route();
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
            if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
            {
                if (!lea_tx_mgr.timer_started)
                {
                    app_src_play_pipe_sd_lea_timer_start();
                }
            }
#endif
        }
        break;

    case AUDIO_PIPE_EVENT_DATA_IND:
        {
            app_src_play_sd_pipe_handle_data_ind();
        }
        break;

    case AUDIO_PIPE_EVENT_DATA_FILLED:
        {
            sd.pipe_play.fill_state = SD_PLAY_STATE_FILLED;
            // src_play_check_and_fill_data();
            src_play_pre_fill_data();
        }
        break;

    case AUDIO_PIPE_EVENT_RELEASED:
        {
            app_src_play_sd_pipe_handle_release();
        }
        break;

    default:
        break;
    }
    return true;
}

void app_src_play_sd_pipe_start(T_FILE_FORMAT_INFO *file_format)
{
    T_AUDIO_FORMAT_INFO src_info = file_format->format_info;
    T_AUDIO_FORMAT_INFO snk_info;
    T_PLAY_ROUTE play_route = app_src_play_get_play_route();

    if (play_route == PLAY_ROUTE_A2DP)
    {
        // TODO: Support A2DP TX
        return;
    }
    else if (play_route == PLAY_ROUTE_HFP_AG)
    {
        // TODO: Support HFP TX
        return;
    }
    else if (play_route == PLAY_ROUTE_PCM)
    {
        if (sd.src_codec == SD_SRC_CODEC_PCM)
        {
            //source pcm test
            snk_info.type = AUDIO_FORMAT_TYPE_PCM;
            snk_info.frame_num = 1;
            snk_info.attr.pcm.sample_rate = src_info.attr.pcm.sample_rate;//192000
            snk_info.attr.pcm.chann_num = src_info.attr.pcm.chann_num;//2;
            snk_info.attr.pcm.bit_width = src_info.attr.pcm.bit_width;//24;
            snk_info.attr.pcm.frame_length = ((5000 * snk_info.attr.pcm.sample_rate) / 1000000) *
                                             (snk_info.attr.pcm.bit_width / 8);
        }
        else
        {
            //source mp3 test
            snk_info.type = AUDIO_FORMAT_TYPE_PCM;
            snk_info.frame_num = 1;
            snk_info.attr.pcm.sample_rate = src_info.attr.mp3.sample_rate;
            snk_info.attr.pcm.bit_width = 16;
            if (src_info.attr.mp3.chann_mode == AUDIO_MP3_CHANNEL_MODE_MONO)
            {
                snk_info.attr.pcm.chann_num = 1;
            }
            else
            {
                snk_info.attr.pcm.chann_num = 2;
            }
            snk_info.attr.pcm.frame_length = ((file_format->frame_duration * 1000 *
                                               snk_info.attr.pcm.sample_rate) / 1000000) *
                                             (snk_info.attr.pcm.bit_width / 8);
            if (snk_info.attr.pcm.chann_num == 1)
            {
                snk_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
            }
            else if (snk_info.attr.pcm.chann_num == 2)
            {
                snk_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_FL | AUDIO_CHANNEL_LOCATION_FR;
            }
        }
    }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
    else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
    {
        if (!app_lea_get_data_format(LEA_CODEC_DIR_ENCODE, &snk_info))
        {
            APP_PRINT_ERROR0("app_src_play_sd_pipe_start: lc3 format info does not exist");
            return;
        }

        uint8_t chnl_cnt;
        if (snk_info.attr.lc3.chann_location == AUDIO_LOCATION_MONO)
        {
            chnl_cnt = 1;
        }
        else
        {
            chnl_cnt = __builtin_popcount(snk_info.attr.lc3.chann_location);
        }

        lea_tx_mgr.pkt_len = snk_info.attr.lc3.frame_length * chnl_cnt;
        lea_tx_mgr.p_lea_send_buf = calloc(1, lea_tx_mgr.pkt_len);
        if (lea_tx_mgr.p_lea_send_buf == NULL)
        {
            APP_PRINT_ERROR0("app_src_play_sd_pipe_start: p_lea_send_buf malloc fail");
            return;
        }
        lea_tx_mgr.target_threshold = 5; // frame_cnt
        lea_tx_mgr.pre_fill_num = 4;
        APP_PRINT_INFO2("app_src_play_sd_pipe_start: pkt_len %d, target_threshold %d",
                        lea_tx_mgr.pkt_len, lea_tx_mgr.target_threshold);
        if (snk_info.attr.lc3.frame_duration == AUDIO_LC3_FRAME_DURATION_10_MS)
        {
            lea_tx_mgr.timer_duration = 10000;
        }
        else
        {
            lea_tx_mgr.timer_duration = 7500;
        }
    }
#endif
    src_info.frame_num = 1;
    snk_info.frame_num = 1;
    if (sd.pipe_play.handle == NULL)
    {
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
        APP_PRINT_INFO1("app_src_play_sd_pipe_start: vol %d", sd.pipe_play.volume);
        sd.pipe_play.handle = audio_pipe_create(AUDIO_STREAM_MODE_DIRECT,
                                                src_info, snk_info,
                                                pcm_gain_table[sd.pipe_play.volume],
                                                app_src_play_sd_pipe_cback);
#else
        sd.pipe_play.handle = audio_pipe_create(AUDIO_STREAM_MODE_NORMAL,
                                                src_info, snk_info,
                                                pcm_gain_table[sd.pipe_play.volume],
                                                app_src_play_sd_pipe_cback);
#endif
    }
}

static bool app_src_play_sd_get_file_format(T_FILE_FORMAT_INFO *file_format)
{
    T_MP3_FRAME_INFO frame_info;
    /* Save position so we can seek back the first frame must be
     * re-read by the data pump later. */
    uint32_t first_frame_pos = app_fs_tell(sd.fs_handle);
    int len = mp3_parser_get_next_frame(sd.fs_handle, sd.frame_data, sizeof(sd.frame_data),
                                        &frame_info);

    if (len <= 0)
    {
        APP_PRINT_INFO1("app_src_play_sd_get_file_format: read first frame failed, ret=%d", len);
        return false;
    }

    /* Decide where the data pump should start:
     * - Xing frame skip it (it's metadata, not audio), stay at frame 2.
     * - Normal CBR seek back to first frame so the data pump plays it. */
    if (frame_info.is_xing_frame)
    {
        APP_PRINT_INFO0("app_src_play_sd_get_file_format: Xing detected, skip first frame");
    }
    else
    {
        app_fs_seek(sd.fs_handle, first_frame_pos);
    }

    file_format->format_info.type = AUDIO_FORMAT_TYPE_MP3;
    file_format->format_info.frame_num = 1;
    file_format->format_info.attr.mp3.sample_rate = frame_info.sampling_frequency;
    file_format->format_info.attr.mp3.bitrate = frame_info.bit_rate;
    file_format->format_info.attr.mp3.version = frame_info.version;
    file_format->format_info.attr.mp3.layer = frame_info.layer;

    switch (frame_info.channel_mode)
    {
    case MP3_MODE_STEREO:
        file_format->format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_STEREO;
        file_format->format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_FL |
                                                           AUDIO_CHANNEL_LOCATION_FR;
        break;
    case MP3_MODE_JOINT_STEREO:
        file_format->format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_JOINT_STEREO;
        file_format->format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_FL |
                                                           AUDIO_CHANNEL_LOCATION_FR;
        break;
    case MP3_MODE_DUAL_CHANNEL:
        file_format->format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_DUAL;
        file_format->format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_FL |
                                                           AUDIO_CHANNEL_LOCATION_FR;
        break;
    case MP3_MODE_MONO:
        file_format->format_info.attr.mp3.chann_mode = AUDIO_MP3_CHANNEL_MODE_MONO;
        file_format->format_info.attr.mp3.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        break;

    default:
        break;
    }

    file_format->frame_size = frame_info.frame_size;
    file_format->frame_duration = frame_info.frame_duration;
    file_format->sample_counts = frame_info.sample_counts;


    APP_PRINT_INFO7("MP3 Format: ver=%d, layer=%d, Rate=%d Hz, Bitrate=%d bps, ChMode=%d, dura=%d, fsize=%d",
                    frame_info.version,
                    frame_info.layer,
                    frame_info.sampling_frequency,
                    frame_info.bit_rate,
                    frame_info.channel_mode,
                    frame_info.frame_duration,
                    frame_info.frame_size
                   );

    return true;
}

static bool app_src_play_sd_get_pcm_format(T_FILE_FORMAT_INFO *file_format)
{
    memset(&sd.pcm_info, 0, sizeof(sd.pcm_info));

    /* For headerless raw PCM, pre-fill the host-supplied format. The parser keeps
     * these values when no WAV header is found, and overrides them for a real WAV. */
    if (g_pcm_fmt_cfg.valid)
    {
        sd.pcm_info.sample_rate = g_pcm_fmt_cfg.sample_rate;
        sd.pcm_info.chann_num   = g_pcm_fmt_cfg.chann_num;
        sd.pcm_info.bit_width   = g_pcm_fmt_cfg.bit_width;
    }

    if (app_pcm_file_init(sd.fs_handle, &sd.pcm_info) != 0)
    {
        APP_PRINT_ERROR0("app_src_play_sd_get_pcm_format: pcm init failed");
        return false;
    }

    T_PCM_FRAME_INFO *info = &sd.pcm_info;

    file_format->format_info.type = AUDIO_FORMAT_TYPE_PCM;
    file_format->format_info.frame_num = 1;
    file_format->format_info.attr.pcm.sample_rate  = info->sample_rate;
    file_format->format_info.attr.pcm.chann_num    = info->chann_num;
    file_format->format_info.attr.pcm.bit_width    = info->bit_width;
    file_format->format_info.attr.pcm.frame_length = info->frame_size / info->chann_num;
    // file_format->format_info.attr.pcm.frame_length = ((5000 *info->sample_rate) /1000000) *(info->bit_width/8);

    if (info->chann_num >= 2)
    {
        file_format->format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_FL |
                                                           AUDIO_CHANNEL_LOCATION_FR;
    }
    else
    {
        file_format->format_info.attr.pcm.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
    }

    file_format->frame_size     = info->frame_size;
    file_format->frame_duration = info->frame_duration;
    file_format->sample_counts  = info->sample_counts;

    APP_PRINT_INFO5("PCM Format: rate=%d Hz, ch=%d, bits=%d, dura=%d, fsize=%d",
                    info->sample_rate, info->chann_num, info->bit_width,
                    info->frame_duration, info->frame_size);
    return true;
}

void app_src_play_sd_local_file_close(void)
{
    if (sd.fs_handle != NULL)
    {
        app_fs_close_file(sd.fs_handle);
        sd.fs_handle = NULL;
    }
}

static void app_src_play_sd_sw_timer_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_INFO2("app_src_play_sd_sw_timer_cb: timer_evt 0x%02x, param 0x%x", timer_evt,
                    param);

    switch (timer_evt)
    {
#if F_APP_SD_CARD_LOCALPLAY
    case APP_TIMER_SD_LOCAL_PUT_DATA:
        {
            app_src_playback_put_data_stop_timer(sd.local_play.play_monitor.frame_num);
        }
        break;

    case APP_TIMER_SD_DELAY_STOP_LOCAL:
        {
            app_stop_timer(&timer_idx_sd_delay_stop_local);
            app_src_play_sd_local_stop();
        }
        break;
#endif

    case APP_TIMER_SD_DELAY_STOP_PIPE:
        {
            T_PLAY_ROUTE play_route = app_src_play_get_play_route();
            if (play_route == PLAY_ROUTE_PCM)
            {
                app_stop_timer(&timer_idx_sd_delay_stop_pipe);
                app_src_play_sd_pipe_stop();
            }
#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
            else if (play_route == PLAY_ROUTE_BIS || play_route == PLAY_ROUTE_CIS)
            {
                uint32_t used_size = ring_buffer_get_data_count(&lea_tx_mgr.ring_buf);

                if (used_size < lea_tx_mgr.pkt_len)
                {
                    app_stop_timer(&timer_idx_sd_delay_stop_pipe);
                    app_src_play_sd_pipe_stop();
                }
                else
                {
                    app_src_play_sd_pipe_delay_stop(50);
                }
            }
#endif
        }

    default:
        break;
    }
}

// route in start
bool app_src_play_sd_start(uint8_t play_route)
{
    uint8_t err_code = 0;
    T_FILE_FORMAT_INFO file_format_info;

    if (sd.fs_handle != NULL)
    {
        err_code = 1;
        goto ERR;
    }

    sd.fs_handle = app_fs_open_file(cur_audio_file_name, FS_O_READ);
    if (sd.fs_handle == NULL)
    {
        err_code = 2;
        goto ERR;
    }


    sd.src_codec = app_src_play_sd_detect_codec(cur_audio_file_name, cur_audio_file_name_len);

    if (sd.src_codec == SD_SRC_CODEC_PCM)
    {
        // parse WAV header (or fall back to raw PCM) and get format
        if (!app_src_play_sd_get_pcm_format(&file_format_info))
        {
            err_code = 2;
            goto ERR;
        }
    }
    else
    {
        if (app_mp3_file_init(sd.fs_handle) != 0)
        {
            err_code = 3;
            goto ERR;
        }

        // read file header to get format
        if (!app_src_play_sd_get_file_format(&file_format_info))
        {
            err_code = 2;
            goto ERR;
        }
    }

    app_dlps_disable(APP_DLPS_ENTER_CHECK_SD_CARD);
    if (play_route == PLAY_ROUTE_A2DP || play_route == PLAY_ROUTE_HFP_AG
#if BAP_BROADCAST_SOURCE
        || play_route == PLAY_ROUTE_BIS
#endif
#if BAP_UNICAST_CLIENT
        || play_route == PLAY_ROUTE_CIS
#endif
        || play_route == PLAY_ROUTE_PCM
       )
    {
        app_src_play_sd_pipe_start(&file_format_info);
    }
#if F_APP_SD_CARD_LOCALPLAY
    else if (play_route == PLAY_ROUTE_LOCAL)
    {
        app_src_play_sd_local_start(&file_format_info);
    }
#endif
    else
    {
        err_code = 3;
        goto ERR;
    }

    return true;

ERR:
    app_src_play_sd_local_file_close();
    app_dlps_enable(APP_DLPS_ENTER_CHECK_SD_CARD);
    APP_PRINT_ERROR1("app_src_play_sd_start: err_code %d", -err_code);
    return false;
}

static void app_src_play_sd_pipe_stop(void)
{
    if (sd.pipe_play.handle != NULL)
    {
        audio_pipe_release(sd.pipe_play.handle);
    }
#if F_APP_DBG_DUMP_PCM_TO_FILE
    /* Close PCM dump file if release callback didn't fire. */
    if (sd.pipe_play.pcm_dump_file != NULL)
    {
        app_fs_close_file(sd.pipe_play.pcm_dump_file);
        sd.pipe_play.pcm_dump_file = NULL;
    }
#endif
    app_src_play_sd_local_file_close();
}

void app_src_play_sd_stop(uint8_t play_route)
{
    if (play_route == PLAY_ROUTE_A2DP || play_route == PLAY_ROUTE_HFP_AG
#if BAP_BROADCAST_SOURCE
        || play_route == PLAY_ROUTE_BIS
#endif
#if BAP_UNICAST_CLIENT
        || play_route == PLAY_ROUTE_CIS
#endif
        || play_route == PLAY_ROUTE_PCM
       )
    {
        app_src_play_sd_pipe_stop();
    }
#if F_APP_SD_CARD_LOCALPLAY
    else if (play_route == PLAY_ROUTE_LOCAL)
    {
        app_src_play_sd_local_stop();
    }
#endif
    app_dlps_enable(APP_DLPS_ENTER_CHECK_SD_CARD);
}

// TODO: Support all route
void app_src_playback_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                     uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE4("app_src_playback_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u, play_state %d",
                     cmd_id, cmd_len, cmd_path, sd.local_play.play_state);

    switch (cmd_id)
    {
    case CMD_SRC_PLAY_SET_SRC_FILE_INDEX:
        {
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
            uint8_t file_len = cmd_ptr[2];
            uint8_t *file_name = &cmd_ptr[3];
            cur_audio_file_name_len = file_len >= sizeof(cur_audio_file_name) ?
                                      sizeof(cur_audio_file_name) - 1 : file_len;
            memcpy(cur_audio_file_name, file_name, cur_audio_file_name_len);
            cur_audio_file_name[cur_audio_file_name_len] = '\0';
        }
        break;

    case CMD_SRC_PLAY_SET_PCM_FORMAT:
        {
            /* payload: sample_rate (LE32) | chann_num (u8) | bit_width (u8) */
            if (cmd_len < 8)
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
                app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
                break;
            }

            g_pcm_fmt_cfg.sample_rate = (uint32_t)cmd_ptr[2] | ((uint32_t)cmd_ptr[3] << 8) |
                                        ((uint32_t)cmd_ptr[4] << 16) | ((uint32_t)cmd_ptr[5] << 24);
            g_pcm_fmt_cfg.chann_num = cmd_ptr[6];
            g_pcm_fmt_cfg.bit_width = cmd_ptr[7];
            g_pcm_fmt_cfg.valid     = true;

            APP_PRINT_INFO3("CMD_SRC_PLAY_SET_PCM_FORMAT: rate=%d ch=%d bits=%d",
                            g_pcm_fmt_cfg.sample_rate, g_pcm_fmt_cfg.chann_num,
                            g_pcm_fmt_cfg.bit_width);

            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    default:
        break;
    }
}

bool app_pipe_pcm_volume_set(uint8_t volume)
{
    uint8_t max_volume = app_dsp_cfg_vol.playback_volume_max;
    uint8_t min_volume = app_dsp_cfg_vol.playback_volume_min;
    APP_PRINT_TRACE3("app_pipe_pcm_volume_set vol:%d, %d, %d", volume, max_volume, min_volume);

    bool ret = false;
    if (volume > max_volume)
    {
        volume = max_volume;
    }
    if (sd.pipe_play.handle != NULL)
    {
        ret = audio_pipe_gain_set(sd.pipe_play.handle, pcm_gain_table[volume], pcm_gain_table[volume]);
    }
    sd.pipe_play.volume = volume;

    return ret;
}

typedef enum t_bt_volume_type
{
    HFP_VOL      = 0x00,
    LOCAL_VOL    = 0x01,
    A2DP_SRC_VOL = 0x02,
    RINGTONE_VOL = 0x03,
    VP_VOL       = 0x04,
    TTS_VOL      = 0x05,
    RECORD_VOL   = 0x06,
    PIPE_PCM_VOL = 0x07,
} T_BT_VOLUME_TYPE;

static bool app_audio_set_volume(uint8_t volume, uint8_t type)
{
    bool ret = false;
    switch (type)
    {
    case PIPE_PCM_VOL:
        {
            ret = app_pipe_pcm_volume_set(volume);
        }
        break;
    }
    return ret;
}

void handle_volume_cmd(uint8_t app_idx, T_CMD_PATH cmd_path, uint8_t *cmd_ptr,
                       uint16_t cmd_len, uint8_t *ack_pkt)
{
#define VOLUME_MAX (15)

    uint8_t volume = cmd_ptr[2];
    uint8_t type = cmd_ptr[3];
    bool ret = false;
    if (volume > VOLUME_MAX)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        return;
    }
    else
    {
        ret = app_audio_set_volume(volume, type);
        if (ret)
        {
            ack_pkt[2] = CMD_SET_STATUS_COMPLETE;
        }
        else
        {
            ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;

        }
        app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
    }
}


void app_src_playback_init(void)
{
    app_mp3_decode_init(app_src_play_mp3_fs_read, app_src_play_mp3_fs_seek, app_src_play_mp3_fs_tell);
    app_pcm_decode_init(app_src_play_mp3_fs_read, app_src_play_mp3_fs_seek, app_src_play_mp3_fs_tell);
    app_timer_reg_cb(app_src_play_sd_sw_timer_cb, &app_src_play_sd_timer_id);

#if F_APP_SD_CARD_LOCALPLAY
    audio_mgr_cback_register(app_src_playback_track_cback);
#endif

#if (BAP_BROADCAST_SOURCE || BAP_UNICAST_CLIENT)
    uint8_t *p_buf = NULL;
    p_buf = (uint8_t *)calloc(1, LC3_RBUF_SIZE);
    ring_buffer_init(&lea_tx_mgr.ring_buf, p_buf, LC3_RBUF_SIZE);
    app_src_play_sd_lc3_timer_init();
#endif
#if F_APP_DBG_DUMP_PCM_TO_RINGBUF
    /* Initialize PCM drain ring buffer. */
    pcm_drain_rb.buf = (uint8_t *)calloc(1, PCM_DRAIN_RBUF_SIZE);
    if (pcm_drain_rb.buf)
    {
        ring_buffer_init(&pcm_drain_rb.ring_buf, pcm_drain_rb.buf, PCM_DRAIN_RBUF_SIZE);
    }
    else
    {
        APP_PRINT_ERROR0("pcm_drain_rb: calloc failed");
    }
#endif
}

#endif
