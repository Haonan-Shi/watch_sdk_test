/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "fmna_util.h"
#include "fmna_platform_includes.h"
#include "fmna_sound_platform.h"
#include "fmna_state_machine.h"
#include "app_audio_policy.h"
#include "app_timer.h"
#include "audio.h"

#define FINDMY_VOICE_PROMPT_OFFSET                  0x80

typedef enum
{
    APP_TIMER_FINDMY_SOUND_PLAY,
    APP_TIMER_FINDMY_SOUND_STOP,
} T_APP_FINDMY_TIMER;

static uint8_t findmy_timer_id = 0;
static uint8_t timer_idx_findmy_sound_play = 0;
static uint8_t timer_idx_findmy_sound_stop = 0;
static T_FMNA_SOUND_STATE findmy_sound_state = FMNA_SOUND_STOP;

void fmna_sound_platform_start(void)
{
    APP_PRINT_INFO0("fmna_sound_platform_start: Sound starting...");
    app_audio_tone_type_play(TONE_FINDMY_SOUND, false, false);//play sound
    findmy_sound_state = FMNA_SOUND_PLAY;
    app_start_timer(&timer_idx_findmy_sound_stop, "findmy_stop",
                    findmy_timer_id, APP_TIMER_FINDMY_SOUND_STOP, 0, false,
                    10 * 1000);
}

void fmna_sound_platform_stop(void)
{
    app_audio_tone_type_cancel(TONE_FINDMY_SOUND, false);
    findmy_sound_state = FMNA_SOUND_STOP;
    app_stop_timer(&timer_idx_findmy_sound_play);
    app_stop_timer(&timer_idx_findmy_sound_stop);
    fmna_state_machine_dispatch_event(FMNA_SM_EVENT_SOUND_COMPLETE);
}

static void fmna_rfc_audio_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case AUDIO_EVENT_RINGTONE_STOPPED:
    case AUDIO_EVENT_VOICE_PROMPT_STOPPED:
        {
            uint8_t tone_stopped_idx;

            if (event_type == AUDIO_EVENT_VOICE_PROMPT_STOPPED)
            {
                tone_stopped_idx = param->voice_prompt_stopped.index + FINDMY_VOICE_PROMPT_OFFSET;
            }
            else
            {
                tone_stopped_idx = param->ringtone_stopped.index;
            }

            if ((tone_stopped_idx == app_audio_tone_cfg.tone_find_my) &&
                (findmy_sound_state == FMNA_SOUND_PLAY))
            {
                app_start_timer(&timer_idx_findmy_sound_play, "findmy_sound",
                                findmy_timer_id, APP_TIMER_FINDMY_SOUND_PLAY, 0, false,
                                1000);
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("fmna_rfc_audio_cback: event_type 0x%04x", event_type);
    }
}

void fmna_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("fmna_timeout_cb: timer_evt %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case APP_TIMER_FINDMY_SOUND_PLAY:
        {
            app_stop_timer(&timer_idx_findmy_sound_play);
            app_audio_tone_type_play(TONE_FINDMY_SOUND, false, false);
        }
        break;

    case APP_TIMER_FINDMY_SOUND_STOP:
        {
            fmna_sound_platform_stop();
        }
        break;

    default:
        break;
    }
}

void fmna_sound_platform_init(void)
{
    audio_mgr_cback_register(fmna_rfc_audio_cback);
    app_timer_reg_cb(fmna_timeout_cb, &findmy_timer_id);
}
#endif
