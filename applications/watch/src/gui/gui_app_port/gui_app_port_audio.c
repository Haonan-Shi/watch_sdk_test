/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <gui_message.h>
#include <watch_adapt.h>
#include <trace.h>
#include "app_task.h"
#include "app_mmi.h"
#include "app_main.h"

static gui_audio_ctrl_t gui_audio;
static gui_control_board_t gui_control_board;

static void gui_audio_music_load(void *p)
{

}
static void gui_audio_music_play(void *p)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_AV_PLAY_PAUSE;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_music_stop(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_AV_STOP;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_music_backward(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_AV_FWD;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_music_forward(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_AV_BWD;
    app_send_msg_to_apptask(&play_msg);
}
static bool gui_audio_music_completion_status(void) { return false; }
static float gui_audio_music_length(void) { return 0.0f; }
static float gui_audio_music_current_time(void) { return 0.0f; }


static void gui_audio_record_start(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_RECORD_START;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_record_stop(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_RECORD_STOP;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_record_play(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_RECORD_PLAY_START;
    app_send_msg_to_apptask(&play_msg);
}
static void gui_audio_record_pause(void)
{
    T_IO_MSG play_msg;
    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_MMI;
    play_msg.u.param = MMI_RECORD_PLAY_STOP;
    app_send_msg_to_apptask(&play_msg);
}

static bool gui_audio_record_completion_status(void) { return false; }

static void gui_control_board_send_bt_status(bool status)
{
    if (status)
    {
        app_bt_policy_event_handle(EVENT_BT_STARTUP, NULL);
    }
    else
    {
        app_bt_policy_event_handle(EVENT_BT_IDLE, NULL);
    }
}

static void gui_control_board_local_play_status(bool status)
{

    T_IO_MSG set_mode_msg;
    set_mode_msg.type = IO_MSG_TYPE_WRISTBNAD;
    set_mode_msg.subtype = IO_MSG_SET_PLAY_MODE;
    if (status)
    {
        set_mode_msg.u.param = MODE_APP_PLAYBACK;
    }
    else
    {
        set_mode_msg.u.param = MODE_NONE;
    }

    app_send_msg_to_apptask(&set_mode_msg);
}

static void gui_control_board_phone_status(bool status)
{
    T_IO_MSG set_mode_msg;
    set_mode_msg.type = IO_MSG_TYPE_WRISTBNAD;
    set_mode_msg.subtype = IO_MSG_SET_PLAY_MODE;
    if (status)
    {
        set_mode_msg.u.param = MODE_APP_A2DP_SNK;
    }
    else
    {
        set_mode_msg.u.param = MODE_NONE;
    }
    app_send_msg_to_apptask(&set_mode_msg);
}

static void gui_control_board_earphone_status(bool status)
{
    T_IO_MSG set_mode_msg;
    set_mode_msg.type = IO_MSG_TYPE_WRISTBNAD;
    set_mode_msg.subtype = IO_MSG_SET_PLAY_MODE;
    if (status)
    {
        set_mode_msg.u.param = MODE_APP_A2DP_SRC;
    }
    else
    {
        set_mode_msg.u.param = MODE_NONE;
    }
    app_send_msg_to_apptask(&set_mode_msg);
}

static void gui_audio_init()
{
    // music
    gui_audio.music_load = gui_audio_music_load;
    gui_audio.music_play = gui_audio_music_play;
    gui_audio.music_stop = gui_audio_music_stop;
    gui_audio.music_backward = gui_audio_music_backward;
    gui_audio.music_forward = gui_audio_music_forward;
    gui_audio.music_completion_status = gui_audio_music_completion_status;
    gui_audio.music_length = gui_audio_music_length;
    gui_audio.music_current_time = gui_audio_music_current_time;

    // record
    gui_audio.record_start = gui_audio_record_start;
    gui_audio.record_stop = gui_audio_record_stop;
    gui_audio.record_play = gui_audio_record_play;
    gui_audio.record_pause = gui_audio_record_pause;
    gui_audio.record_completion_status = gui_audio_record_completion_status;
    gui_audio_ctrl_register(&gui_audio);
}

static void gui_control_board_init()
{
    gui_control_board.send_bt_status = gui_control_board_send_bt_status;
    gui_control_board.send_local_play_status = gui_control_board_local_play_status;
    gui_control_board.send_phone_status = gui_control_board_phone_status;
    gui_control_board.send_earphone_status = gui_control_board_earphone_status;
    gui_control_board_info_register(&gui_control_board);
}

void gui_app_port_audio_init(void)
{
    gui_audio_init();
    gui_control_board_init();
    gui_set_keep_active_time(60000);
}