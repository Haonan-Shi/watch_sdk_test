/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include <string.h>
#include "stdlib.h"
#include "trace.h"
#include "audio.h"
#include "remote.h"
#include "bt_gfps.h"
#include "app_audio_policy.h"
#include "app_bt_policy_api.h"
#include "app_gfps.h"
#include "app_gfps_cfg.h"
#include "app_link_util.h"
#include "app_main.h"
#include "app_sdp.h"
#include "stdlib.h"
#include "app_timer.h"
#include "app_dsp_cfg.h"
#include "app_gfps_msg.h"
#include "app_cfg.h"
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "app_gfps_finder.h"
#include "gfps_find_my_device.h"
#endif

#define GFPS_VOICE_PROMPT_OFFSET   0x80

static uint8_t gfps_msg_timer_id          = 0;
static uint8_t timer_idx_gfps_ring_period = 0;
static uint8_t timer_idx_gfps_ring_stop   = 0;
static uint16_t gfps_msg_ring_timeout     = 0;

/** @brief indicate current ring state*/
static T_GFPS_MSG_RING_STATE gfps_msg_ring_state = GFPS_MSG_RING_STOP;

/**
 * @brief GFPS message stream timer
 * APP_TIMER_GFPS_RING_PERIOD:
 * the interval for ring voice prompt stopped and next ring voice prompt start,
 * the default interval is 2s(GFPS_RING_PERIOD_VALUE)
 * APP_TIMER_GFPS_RING_STOP: ring timeout and shall stop ting
 */
typedef enum
{
    APP_TIMER_GFPS_RING_PERIOD,
    APP_TIMER_GFPS_RING_STOP,
} T_GFPS_MSG_TIMER;

/**
 * @brief app_gfps_msg_reverse_data
 * swap big endian mode or little endian mode
 * @param data
 * @param len
 */
void app_gfps_msg_reverse_data(uint8_t *data, uint16_t len)
{
    uint8_t *header = data;
    uint8_t *tail = data;
    uint8_t temp;

    tail += (len - 1);

    while (header < tail)
    {
        temp = *header;
        *header++ = *tail;
        *tail-- = temp;
    }
}

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
uint8_t app_gfps_msg_get_ring_timeout(void)
{
    return gfps_msg_ring_timeout;
}

void app_gfps_msg_set_ring_timeout(uint8_t ring_timeout)
{
    gfps_msg_ring_timeout = ring_timeout;
}
#endif

T_GFPS_MSG_RING_STATE app_gfps_msg_get_ring_state(void)
{
    return gfps_msg_ring_state;
}

void app_gfps_msg_set_ring_state(T_GFPS_MSG_RING_STATE ring_state)
{
    gfps_msg_ring_state = ring_state;
}

void app_gfps_msg_update_ring_status(uint8_t ring_status)
{
    uint8_t i;

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        if ((app_db.le_link[i].used == true) &&
            (app_db.le_link[i].gfps_link.gfps_conn_id != 0xFF) &&
            (app_db.le_link[i].gfps_link.gfps_msg_cid != 0))
        {
            T_APP_LE_LINK *p_link = &app_db.le_link[i];

            if (p_link != NULL)
            {
                bt_gfps_send_ring_status(app_db.le_link[i].gfps_link.gfps_conn_id,
                                         app_db.le_link[i].gfps_link.gfps_msg_cid,
                                         NULL, 0, &ring_status, 1);
            }

            APP_PRINT_INFO1("app_gfps_msg_update_ring_status: path BLE, ring_status %d", ring_status);
        }
    }
}

static void app_gfps_msg_ring_play(void)
{
    app_audio_tone_play(0x8d, false, false);//TONE_POWER_ON

    if (gfps_msg_ring_timeout)
    {
        APP_PRINT_INFO1("app_gfps_msg_ring_play: start timer to stop ring, timeout  %d",
                        gfps_msg_ring_timeout);

        app_start_timer(&timer_idx_gfps_ring_stop, "gfps_ring_stop",
                        gfps_msg_timer_id, APP_TIMER_GFPS_RING_STOP, 0, false,
                        gfps_msg_ring_timeout * 1000);
    }
}

void app_gfps_msg_ring_stop(void)
{
    gfps_msg_ring_timeout = 0;
    app_audio_tone_cancel(0x8d, false);//TONE_POWER_ON
    app_stop_timer(&timer_idx_gfps_ring_period);
    app_stop_timer(&timer_idx_gfps_ring_stop);
}

/**
 * @brief current ring state is stop and need todo next ring event
 * state indicate current ring state
 * event indicate expected ring event
 * @param event
 */
static void app_gfps_msg_ring_stop_state(uint8_t event)
{
    APP_PRINT_TRACE2("app_gfps_msg_ring_stop_state: state 0x%02x, event 0x%02x,",
                     gfps_msg_ring_state, event);
    switch (event)
    {
    case GFPS_ALL_STOP:
        {
        }
        break;

    case GFPS_RIGHT_RING:
        {

            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_RIGHT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_LEFT_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_LEFT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_ALL_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_ALL_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief current ring state is right ring and need todo next ring event
 * state indicate current ring state
 * event indicate expected ring event
 * @param event
 */
static void app_gfps_msg_right_ring_state(uint8_t event)
{
    APP_PRINT_TRACE2("app_gfps_msg_right_ring_state: state 0x%02x, event 0x%02x",
                     gfps_msg_ring_state, event);
    switch (event)
    {
    case GFPS_ALL_STOP:
        {
            app_gfps_msg_ring_stop();
            gfps_msg_ring_state = GFPS_MSG_RING_STOP;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_RIGHT_RING:
        {

        }
        break;

    case GFPS_LEFT_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_LEFT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);

        }
        break;

    case GFPS_ALL_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_ALL_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief current ring state is left ring and need todo next ring event
 * state indicate current ring state
 * event indicate expected ring event
 * @param event
 */
static void app_gfps_msg_left_ring_state(uint8_t event)
{
    APP_PRINT_TRACE2("app_gfps_msg_left_ring_state: state 0x%02x, event 0x%02x",
                     gfps_msg_ring_state, event);
    switch (event)
    {
    case GFPS_ALL_STOP:
        {
            app_gfps_msg_ring_stop();
            gfps_msg_ring_state = GFPS_MSG_RING_STOP;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_RIGHT_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_RIGHT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_LEFT_RING:
        {

        }
        break;

    case GFPS_ALL_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_ALL_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    default:
        break;
    }
}

/**
 * @brief current ring state is all ring and need todo next ring event
 * state indicate current ring state
 * event indicate expected ring event
 * @param event
 */
static void app_gfps_msg_all_ring_state(uint8_t event)
{
    APP_PRINT_TRACE2("app_gfps_msg_all_ring_state: state 0x%02x, event 0x%02x",
                     gfps_msg_ring_state, event);
    switch (event)
    {
    case GFPS_ALL_STOP:
        {
            app_gfps_msg_ring_stop();
            gfps_msg_ring_state = GFPS_MSG_RING_STOP;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_RIGHT_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_RIGHT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_LEFT_RING:
        {
            app_gfps_msg_ring_play();
            gfps_msg_ring_state = GFPS_MSG_LEFT_RING;
            app_gfps_msg_update_ring_status((uint8_t)gfps_msg_ring_state);
        }
        break;

    case GFPS_ALL_RING:
        {

        }
        break;

    default:
        break;
    }
}

/**
 * @brief app_gfps_msg_handle_ring_event
 *
 * @param event indicate expected ring event
 */
void app_gfps_msg_handle_ring_event(uint8_t event)
{
    switch (gfps_msg_ring_state)
    {
    case GFPS_MSG_RING_STOP:
        app_gfps_msg_ring_stop_state(event);
        break;

    case GFPS_MSG_RIGHT_RING:
        app_gfps_msg_right_ring_state(event);
        break;

    case GFPS_MSG_LEFT_RING:
        app_gfps_msg_left_ring_state(event);
        break;

    case GFPS_MSG_ALL_RING:
        app_gfps_msg_all_ring_state(event);
        break;

    default:
        break;
    }

    APP_PRINT_TRACE2("app_gfps_msg_handle_ring_event: state 0x%02x, event 0x%02x", gfps_msg_ring_state,
                     event);
}

/**
 * @brief Used to handle GFPS message stream
 *
 * @param bd_addr
 * @param server_chann
 * @param p_cmd
 * @param cmd_len
 */
void app_gfps_msg_handle_data_transfer(uint8_t conn_id, uint16_t cid, uint8_t *bd_addr,
                                       uint8_t server_chann,
                                       uint8_t *p_cmd, uint16_t cmd_len)
{
    uint8_t msg_group;
    uint8_t msg_code;
    uint8_t *p;
    uint16_t len;

    p = p_cmd;
    len = cmd_len;

    while (len >= GFPS_HEADER_LEN)
    {
        bool unknow_cmd = false;

        BE_STREAM_TO_UINT8(msg_group, p);
        BE_STREAM_TO_UINT8(msg_code, p);
        APP_PRINT_TRACE3("app_gfps_msg_handle_data_transfer: msg_group 0x%02x, msg_code 0x%02x, len %d",
                         msg_group, msg_code, len);

        if (msg_group == GFPS_DEVICE_INFO_EVENT)
        {
            switch (msg_code)
            {
            case GFPS_ACITVE_COMPONENTS_REQ:
                {
                    uint8_t active_components = GFPS_RIGHT_ACTIVE;

                    active_components = GFPS_RIGHT_ACTIVE;

                    APP_PRINT_INFO1("gfps active_components %d", active_components);

#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
                    if ((conn_id != 0xFF) && app_gfps_cfg.gfps_le_device_support)
                    {
                        bt_gfps_active_components_rsp(conn_id, cid, NULL, 0,
                                                      &active_components,
                                                      GFPS_ACTIVE_COMPONENTS_LEN);
                    }
#endif

                    if (bd_addr != NULL)
                    {
                        bt_gfps_active_components_rsp(0xFF, 0, bd_addr, server_chann,
                                                      &active_components,
                                                      GFPS_ACTIVE_COMPONENTS_LEN);
                    }

                    len -= GFPS_HEADER_LEN;
                    p += GFPS_ADDITIONAL_DATA_LEN;
                }
                break;

            case GFPS_CAPABILITIES:
                {
                    uint8_t capabilities;

                    p += GFPS_ADDITIONAL_DATA_LEN;
                    BE_STREAM_TO_UINT8(capabilities, p);
                    APP_PRINT_TRACE1("app_gfps_msg_handle_data_transfer: capabilities 0x%02x", capabilities);
                    len -= (GFPS_HEADER_LEN + GFPS_SUPPORT_CPBS_LEN);
                }
                break;

            case GFPS_PLATFORM_TYPE:
                {
                    uint8_t platform_type = 0;
                    uint8_t platform_version = 0;
                    p += GFPS_ADDITIONAL_DATA_LEN;
                    BE_STREAM_TO_UINT8(platform_type, p);
                    BE_STREAM_TO_UINT8(platform_version, p);
                    APP_PRINT_TRACE2("app_gfps_msg_handle_data_transfer: platform type 0x%x, platform version 0x%x",
                                     platform_type,
                                     platform_version);
                    len -= (GFPS_HEADER_LEN + GFPS_PLATFORM_TYPE_LEN);
                }
                break;

            case GFPS_FIRMWARE_REVISION:
                {
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
                    if ((conn_id != 0xFF) && app_gfps_cfg.gfps_le_device_support)
                    {
                        bt_gfps_send_firmware_revision(conn_id, cid, NULL, 0,
                                                       app_gfps_cfg.gfps_version,
                                                       sizeof(app_gfps_cfg.gfps_version));
                    }
#endif

                    if (bd_addr != NULL)
                    {
                        bt_gfps_send_firmware_revision(0xFF, 0, bd_addr, server_chann,
                                                       app_gfps_cfg.gfps_version,
                                                       sizeof(app_gfps_cfg.gfps_version));
                    }
                }
                break;

            default:
                {
                    unknow_cmd = true;
                }
                break;
            }
        }
        else if (msg_group == GFPS_DEVICE_ACTION_EVENT)
        {
            switch (msg_code)
            {
            case GFPS_RING:
                {
                    T_GFPS_MSG_RING_STATE pre_state = app_gfps_msg_get_ring_state();
                    uint8_t ring_type;
                    uint16_t ring_len;
                    uint8_t data[GFPS_RING_ACK_LEN];
                    uint8_t *temp = data;

                    BE_STREAM_TO_UINT16(ring_len, p);
                    BE_STREAM_TO_UINT8(ring_type, p);

                    if (ring_type == GFPS_ALL_STOP)
                    {
                        app_gfps_msg_handle_ring_event(ring_type);
                    }
                    else
                    {
                        // The second byte in additional data, if present, represents the timeout in seconds
                        if (ring_len != GFPS_RING_LEN)
                        {
                            BE_STREAM_TO_UINT8(gfps_msg_ring_timeout, p);
                        }
                        app_gfps_msg_handle_ring_event(ring_type);
                    }
                    APP_PRINT_TRACE2("app_gfps_msg_handle_data_transfer: ring_type 0x%02x, gfps_msg_ring_timeout 0x%02x",
                                     ring_type, gfps_msg_ring_timeout);

                    BE_UINT8_TO_STREAM(temp, msg_group);
                    BE_UINT8_TO_STREAM(temp, msg_code);
                    BE_UINT8_TO_STREAM(temp, ring_type);
                    BE_UINT8_TO_STREAM(temp, gfps_msg_ring_timeout);

                    if (pre_state != app_gfps_msg_get_ring_state())
                    {
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
                        if ((conn_id != 0xFF) && app_gfps_cfg.gfps_le_device_support)
                        {
                            bt_gfps_send_ack(conn_id, cid, NULL, 0, data, GFPS_RING_ACK_LEN);
                        }
#endif

                        if (bd_addr != NULL)
                        {
                            bt_gfps_send_ack(0xFF, 0, bd_addr, server_chann, data, GFPS_RING_ACK_LEN);
                        }
                    }
                    else
                    {
#if CONFIG_REALTEK_GFPS_LE_DEVICE_SUPPORT
                        if ((conn_id != 0xFF) && app_gfps_cfg.gfps_le_device_support)
                        {
                            bt_gfps_send_nak(conn_id, cid, NULL, 0, data, GFPS_RING_NAK_LEN, GFPS_NOT_ALLOW);
                        }
#endif

                        if (bd_addr != NULL)
                        {
                            bt_gfps_send_nak(0xFF, 0, bd_addr, server_chann, data, GFPS_RING_NAK_LEN, GFPS_NOT_ALLOW);
                        }
                    }

                    len -= (GFPS_HEADER_LEN + ring_len);
                }
                break;

            default:
                {
                    unknow_cmd = true;
                }
                break;
            }
        }
        else if (msg_group == GFPS_DEVICE_CAPABILITY_SYNC_EVENT)
        {
            uint16_t additional_data_len = 0;
            uint8_t message_nonce[8];
            uint8_t mac[8];
            BE_STREAM_TO_UINT16(additional_data_len, p);
            len -= (GFPS_HEADER_LEN + additional_data_len);
            switch (msg_code)
            {
            case GFPS_REQ_CAPABILITY_UPDATE:
                {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                    if (app_gfps_cfg.gfps_finder_support)
                    {
                        app_gfps_finder_send_eddystone_capability(bd_addr, server_chann);
                    }
#endif
                }
                break;
            }
        }
        else
        {
            unknow_cmd = true;
        }

        if (unknow_cmd == true)
        {
            APP_PRINT_WARN0("app_gfps_msg_handle_data_transfer: unknow cmd");
            break;
        }
    }
}

void app_gfps_msg_le_loop_check_data_complete(uint8_t conn_id, uint16_t cid, uint8_t *data,
                                              uint16_t len)
{
    APP_PRINT_INFO3("app_gfps_msg_le_loop_check_data_complete: conn_id %d, cid 0x%x, data %b",
                    conn_id, cid, TRACE_BINARY(len, data));

    if (data == NULL)
    {
        return;
    }

    bool ret = false, cont = false;
    T_APP_LE_LINK *p_link = NULL;

    p_link = app_find_le_link_by_conn_id(conn_id);

    if (p_link != NULL)
    {
        uint8_t *p_buf;
        if (p_link->gfps_link.p_gfps_cmd == NULL)
        {
            p_buf = malloc(len);
            if (p_buf != NULL)
            {
                p_link->gfps_link.p_gfps_cmd = p_buf;
                memcpy(p_link->gfps_link.p_gfps_cmd, data, len);
                p_link->gfps_link.gfps_cmd_len = len;

                if (len >= GFPS_HEADER_LEN)
                {
                    uint8_t *p_temp;
                    uint16_t temp_len = 0;

                    p_temp = p_link->gfps_link.p_gfps_cmd;
                    p_temp += (GFPS_HEADER_LEN - GFPS_ADDITIONAL_DATA_LEN);
                    BE_STREAM_TO_UINT16(temp_len, p_temp);
                    uint8_t chunck_len = temp_len + GFPS_HEADER_LEN;

                    if (chunck_len == len)
                    {
                        ret = true; //one packet one cmd
                    }
                    else if (chunck_len < len)
                    {
                        //one packet several cmds
                        ret = true;
                        cont = true;
                        p_link->gfps_link.gfps_cmd_len = chunck_len;
                        data += chunck_len;
                        len -= chunck_len;
                    }
                    else
                    {
                        //not whole cmd, do nothing
                    }
                }
            }
        }
        else
        {
            p_buf = malloc(len + p_link->gfps_link.gfps_cmd_len);
            if (p_buf != NULL)
            {
                uint8_t pre_len = p_link->gfps_link.gfps_cmd_len;
                memcpy(p_buf, p_link->gfps_link.p_gfps_cmd, p_link->gfps_link.gfps_cmd_len);
                memcpy(p_buf + p_link->gfps_link.gfps_cmd_len, data, len);
                free(p_link->gfps_link.p_gfps_cmd);
                p_link->gfps_link.p_gfps_cmd = p_buf;
                p_link->gfps_link.gfps_cmd_len += len;

                if (p_link->gfps_link.gfps_cmd_len >= GFPS_HEADER_LEN)
                {
                    uint8_t *p_temp;
                    uint16_t temp_len = 0;

                    p_temp = p_buf;
                    p_temp += (GFPS_HEADER_LEN - GFPS_ADDITIONAL_DATA_LEN);
                    BE_STREAM_TO_UINT16(temp_len, p_temp);

                    uint8_t chunck_len = temp_len + GFPS_HEADER_LEN;

                    if (chunck_len == p_link->gfps_link.gfps_cmd_len)
                    {
                        ret = true;
                    }
                    else if (chunck_len < p_link->gfps_link.gfps_cmd_len)
                    {
                        ret = true;
                        cont = true;
                        p_link->gfps_link.gfps_cmd_len = chunck_len;
                        data += (chunck_len - pre_len);
                        len -= (chunck_len - pre_len);
                    }
                }
            }
            else
            {
                free(p_link->gfps_link.p_gfps_cmd);
                p_link->gfps_link.gfps_cmd_len = 0;
                p_link->gfps_link.p_gfps_cmd = NULL;
            }
        }
        APP_PRINT_TRACE4("app_gfps_msg_le_loop_check_data_complete: len %d, cmd_len %d, ret %d, cont %d\n",
                         len,
                         p_link->gfps_link.gfps_cmd_len, ret, cont);
        if (ret)
        {
            app_gfps_msg_handle_data_transfer(conn_id, cid, NULL, 0,
                                              p_link->gfps_link.p_gfps_cmd, p_link->gfps_link.gfps_cmd_len);

            free(p_link->gfps_link.p_gfps_cmd);
            p_link->gfps_link.gfps_cmd_len = 0;
            p_link->gfps_link.p_gfps_cmd = NULL;
        }

        if (cont)
        {
            app_gfps_msg_le_loop_check_data_complete(conn_id, cid, data, len);
        }
    }

    return;
}

static void app_gfps_msg_rfc_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_REMOTE_DISCONN_CMPL:
        {
            gfps_msg_ring_state = GFPS_MSG_RING_STOP;
            APP_PRINT_INFO1("app_gfps_msg_rfc_bt_cback: gfps_msg_ring_state 0x%02x", gfps_msg_ring_state);
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_gfps_msg_rfc_bt_cback: event_type 0x%04x", event_type);
    }
}

static void app_gfps_msg_rfc_audio_cback(T_AUDIO_EVENT event_type, void *event_buf,
                                         uint16_t buf_len)
{
    bool handle = true;
    T_AUDIO_EVENT_PARAM *param = event_buf;

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    static uint8_t default_volume = 0;
    T_GFPS_FINDER_RING_VOLUME_LEVEL ring_volume_level = GFPS_FINDER_RING_VOLUME_DEFAULT;

    if (app_gfps_cfg.gfps_finder_support)
    {
        ring_volume_level = app_gfps_finder_get_ring_volume_level();
    }
#endif

    switch (event_type)
    {
    case AUDIO_EVENT_RINGTONE_STARTED:
    case AUDIO_EVENT_VOICE_PROMPT_STARTED:
        {
            uint8_t tone_started_idx;

            if (event_type == AUDIO_EVENT_VOICE_PROMPT_STARTED)
            {
                tone_started_idx = param->voice_prompt_started.index + GFPS_VOICE_PROMPT_OFFSET;
            }
            else
            {
                tone_started_idx = param->ringtone_started.index;
            }

            if (tone_started_idx == app_gfps_cfg.tone_gfps_findme)
            {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support)
                {
                    default_volume = voice_prompt_volume_get();

                    if (ring_volume_level == GFPS_FINDER_RING_VOLUME_LOW)
                    {
                        voice_prompt_volume_set(app_dsp_cfg_vol.voice_prompt_volume_min + 2);
                    }
                    else if (ring_volume_level == GFPS_FINDER_RING_VOLUME_HIGH)
                    {
                        voice_prompt_volume_set(app_dsp_cfg_vol.voice_prompt_volume_max);
                    }
                    else if (ring_volume_level == GFPS_FINDER_RING_VOLUME_MEDIUM)
                    {
                        voice_prompt_volume_set((app_dsp_cfg_vol.voice_prompt_volume_max +
                                                 app_dsp_cfg_vol.voice_prompt_volume_min) / 2);
                    }
                }
#endif
            }
        }
        break;

    case AUDIO_EVENT_RINGTONE_STOPPED:
    case AUDIO_EVENT_VOICE_PROMPT_STOPPED:
        {
            uint8_t tone_stopped_idx;

            if (event_type == AUDIO_EVENT_VOICE_PROMPT_STOPPED)
            {
                tone_stopped_idx = param->voice_prompt_stopped.index + GFPS_VOICE_PROMPT_OFFSET;
            }
            else
            {
                tone_stopped_idx = param->ringtone_stopped.index;
            }

            if (tone_stopped_idx == app_gfps_cfg.tone_gfps_findme)
            {
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support)
                {
                    if (ring_volume_level != GFPS_FINDER_RING_VOLUME_DEFAULT)
                    {
                        voice_prompt_volume_set(default_volume);
                    }
                }
#endif
                bool period_fg = false;

                if (gfps_msg_ring_state != GFPS_MSG_RING_STOP)
                {
                    period_fg = true;
                }

                if (period_fg == true)
                {
                    app_start_timer(&timer_idx_gfps_ring_period, "gfps_ring_period",
                                    gfps_msg_timer_id, APP_TIMER_GFPS_RING_PERIOD, 0, false,
                                    GFPS_RING_PERIOD_VALUE * 1000);
                }
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_gfps_msg_rfc_audio_cback: event_type 0x%04x", event_type);
    }
}

static void app_gfps_msg_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE3("app_gfps_msg_timeout_cb: timer_evt %d, param %d, state 0x%02x",
                     timer_evt, param, gfps_msg_ring_state);
    switch (timer_evt)
    {
    case APP_TIMER_GFPS_RING_PERIOD:
        {
            app_stop_timer(&timer_idx_gfps_ring_period);
            app_audio_tone_play(0x8d, false, false);//TONE_POWER_ON
        }
        break;

    case APP_TIMER_GFPS_RING_STOP:
        {
            APP_PRINT_TRACE0("app_gfps_msg_timeout_cb: APP_TIMER_GFPS_RING_STOP");

            app_gfps_msg_handle_ring_event(GFPS_ALL_STOP);

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
            if (app_gfps_cfg.gfps_finder_support)
            {
                uint8_t ring_state = GFPS_FINDER_RING_TIMEOUT_STOP;
                app_gfps_finder_send_ring_rsp(ring_state);
            }
#endif
        }
        break;

    default:
        break;
    }
}

void app_gfps_msg_update_rpa_addr(void)
{
    uint8_t i;
    uint8_t ble_addr[6] = {0};
    app_gfps_get_random_addr(ble_addr);
    app_gfps_msg_reverse_data(ble_addr, 6);

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        if ((app_db.le_link[i].used == true) &&
            (app_db.le_link[i].gfps_link.gfps_conn_id != 0xFF) &&
            (app_db.le_link[i].gfps_link.gfps_msg_cid != 0))
        {
            T_APP_LE_LINK *p_link = &app_db.le_link[i];

            if (p_link != NULL)
            {
                bt_gfps_send_ble_addr(app_db.le_link[i].gfps_link.gfps_conn_id,
                                      app_db.le_link[i].gfps_link.gfps_msg_cid,
                                      NULL, 0, ble_addr);
            }

            APP_PRINT_INFO1("app_gfps_rfc_update_ble_addr: BLE link_id %d", i);
        }
    }
}

void app_gfps_msg_rfc_init(void)
{
    bt_mgr_cback_register(app_gfps_msg_rfc_bt_cback);
    audio_mgr_cback_register(app_gfps_msg_rfc_audio_cback);
    app_timer_reg_cb(app_gfps_msg_timeout_cb, &gfps_msg_timer_id);
}
#endif
