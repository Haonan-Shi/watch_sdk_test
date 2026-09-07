/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_APP_AI_AUTH
#include "ftl.h"
#include "string.h"
#include "trace.h"
#include "bind_handler.h"
#include "app_cfg.h"
#include "app_main.h"
#include "bt_types.h"
#include "app_link_util_cs.h"
#include "bt_bond_le_sync.h"
#include "app_cmd.h"
#include "app_rtk_auth.h"
#include "app_ipc.h"
#include "app_bond.h"
#include "app_io_msg.h"
#include "os_task.h"
#include "os_msg.h"

#define CMD_START_AUTH_LEN                   8
#define CMD_DYNAMIC_AUTH_DATA_EXCHANGE_LEN  24
#define CMD_ENCRYPTED_AUTH_DATA_VERIFY_LEN  32

#define AUTH_VERIFY_VER         1
#define AUTH_PUBLIC_KEY_LEN     48
#define AUTH_SHARE_KEY_LEN      24

typedef enum
{
    AUTH_START_NEW              = 0,
    AUTH_START_FIND_IT          = 1,
    AUTH_START_FORCE_EXCHAGE    = 2,
} T_AUTH_START_STATE;

typedef struct
{
    uint8_t  status;
    uint8_t  ver;
    uint8_t  rsvd[6];
} T_AUTH_START_INFO;

typedef enum
{
    AUTH_KEY_MSG_PUBLIC = 1,
    AUTH_KEY_MSG_SHARE  = 2,
} T_AUTH_KEY_MSG;

static void *g_msg_queue_handle;
static uint8_t g_public_key[AUTH_PUBLIC_KEY_LEN];
static uint8_t g_path;
static uint8_t g_app_idx;

static void app_rtk_auth_key_req(T_AUTH_KEY_MSG auth_key_msg);
static void app_rtk_auth_dynamic_data_exchange_resp(uint8_t path, uint8_t app_idx);
static void app_rtk_auth_data_verity_resp(uint8_t path, uint8_t app_idx, bool verify_ok);

static void app_rtk_auth_start_req(uint8_t path, uint8_t app_idx, uint8_t *data_in, uint8_t len)
{
    g_path = path;
    g_app_idx = app_idx;

    //gen dev public key - start
    reset_bind_key_info();
    app_rtk_auth_key_req(AUTH_KEY_MSG_PUBLIC);

    APP_PRINT_INFO0("app_rtk_auth_start_req");
}

static void app_rtk_auth_start_resp(int ret)
{
    T_AUTH_START_INFO dev_auth_info;

    //gen dev public key - done
    memset(&dev_auth_info, 0, sizeof(T_AUTH_START_INFO));
    if (ret == 0)
    {
        dev_auth_info.status = AUTH_START_NEW;
        dev_auth_info.ver = AUTH_VERIFY_VER;
    }
    app_report_event(g_path, EVENT_START_AUTH, g_app_idx, (uint8_t *)&dev_auth_info,
                     sizeof(dev_auth_info));

    APP_PRINT_INFO1("app_rtk_auth_start_resp: ret %d", ret);
}

static void app_rtk_auth_public_key_exchange_req(uint8_t path, uint8_t app_idx, uint8_t *p_data,
                                                 uint16_t len)
{
    uint8_t *app_public_key;
    int ret;

    g_path = path;
    g_app_idx = app_idx;

    //get public key from app
    app_public_key = p_data + 2;
    ret = set_peer_bind_public_key(app_public_key, AUTH_PUBLIC_KEY_LEN);

    //gen dev share key - start
    app_rtk_auth_key_req(AUTH_KEY_MSG_SHARE);

    APP_PRINT_INFO1("app_rtk_auth_public_key_exchange_req: ret %d", ret);
}

static void app_rtk_auth_public_key_exchange_resp(int ret)
{
    uint8_t piblic_key_resp[AUTH_PUBLIC_KEY_LEN + 2];

    //gen dev share key - done
    //send dev public key to app
    LE_UINT16_TO_ARRAY(piblic_key_resp, AUTH_PUBLIC_KEY_LEN);
    memcpy(piblic_key_resp + 2, g_public_key, AUTH_PUBLIC_KEY_LEN);
    app_report_event(g_path, EVENT_AUTH_KEY_EXCHANGE, g_app_idx, piblic_key_resp,
                     AUTH_PUBLIC_KEY_LEN + 2);

    APP_PRINT_INFO1("app_rtk_auth_public_key_exchange_resp: ret %d", ret);
}

static void app_rtk_auth_dynamic_data_exchange_req(uint8_t path, uint8_t app_idx, uint8_t *p_data,
                                                   uint16_t len)
{
    int ret = 0;
    uint8_t *app_dynamic_data;

    //get dynamic data from app
    app_dynamic_data = p_data;
    ret = set_peer_dynamic_auth_data(app_dynamic_data, len);

    APP_PRINT_INFO1("app_rtk_auth_dynamic_data_exchange_req: ret %d", ret);

    app_rtk_auth_dynamic_data_exchange_resp(path, app_idx);
}

static void app_rtk_auth_dynamic_data_exchange_resp(uint8_t path, uint8_t app_idx)
{
    int ret = 0;
    uint8_t dev_dynamic_data[CMD_DYNAMIC_AUTH_DATA_EXCHANGE_LEN];

    //send dev dynamic data to app
    memset(dev_dynamic_data, 0, sizeof(dev_dynamic_data));
    ret = get_bind_dynamic_auth_data(dev_dynamic_data, sizeof(dev_dynamic_data));
    app_report_event(path, EVENT_DYNAMIC_AUTH_DATA_EXCHANGE, app_idx, dev_dynamic_data,
                     sizeof(dev_dynamic_data));

    APP_PRINT_INFO1("app_rtk_auth_dynamic_data_exchange_resp: ret %d", ret);
}

static void app_rtk_auth_data_verity_req(uint8_t path, uint8_t app_idx, uint8_t *p_data,
                                         uint16_t len)
{
    int ret = 0;
    uint8_t *app_enc_data;
    bool verify_ok = false;

    //get app enc data
    app_enc_data = p_data;
    ret = verify_peer_bind_enc_auth_data(app_enc_data, len);

    APP_PRINT_INFO1("app_rtk_auth_data_verity_req: ret %d", ret);

    verify_ok = (ret == 0 ? true : false);
    app_rtk_auth_data_verity_resp(path, app_idx, verify_ok);
}

static void app_rtk_auth_data_verity_resp(uint8_t path, uint8_t app_idx, bool verify_ok)
{
    int ret = 0;
    uint8_t dev_enc_data[CMD_ENCRYPTED_AUTH_DATA_VERIFY_LEN];

    memset(dev_enc_data, 0, sizeof(dev_enc_data));

    if (verify_ok)
    {
        ret = get_bind_encrypted_auth_data(dev_enc_data, sizeof(dev_enc_data));
    }

    app_report_event(path, EVENT_ENCRYPTED_AUTH_DATA_VERIFY, app_idx, dev_enc_data,
                     sizeof(dev_enc_data));

    APP_PRINT_INFO1("app_rtk_auth_data_verity_resp: ret %d", ret);
}

void app_rtk_auth_cmd_handle(uint8_t path, uint16_t length, uint8_t *p_value, uint8_t app_idx)
{
    uint8_t ack_pkt[3];
    uint16_t cmd_id = *(uint16_t *)p_value;
    uint8_t *p;

    bool ack_flag = false;

    ack_pkt[0] = p_value[0];
    ack_pkt[1] = p_value[1];
    ack_pkt[2] = CMD_SET_STATUS_COMPLETE;

    if (length < 2)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
        APP_PRINT_ERROR0("app_rtk_auth_cmd_handle: error length");
        return;
    }

    length = length - 2;
    p = p_value + 2;

    APP_PRINT_TRACE2("app_rtk_auth_cmd_handle: cmd_id 0x%x, length %d", cmd_id, length);

    switch (cmd_id)
    {
    case CMD_START_AUTH:
        {
            if (length == CMD_START_AUTH_LEN)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                app_rtk_auth_start_req(path, app_idx, p, length);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_AUTH_KEY_EXCHANGE:
        {
            uint16_t key_len;
            LE_ARRAY_TO_UINT16(key_len, p);
            if (length == (key_len + 2))
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                app_rtk_auth_public_key_exchange_req(path, app_idx, p, length);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_DYNAMIC_AUTH_DATA_EXCHANGE:
        {
            if (length == CMD_DYNAMIC_AUTH_DATA_EXCHANGE_LEN)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                app_rtk_auth_dynamic_data_exchange_req(path, app_idx, p, length);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_ENCRYPTED_AUTH_DATA_VERIFY:
        {
            if (length == CMD_ENCRYPTED_AUTH_DATA_VERIFY_LEN)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                app_rtk_auth_data_verity_req(path, app_idx, p, length);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    default:
        ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
        break;
    }

    if (ack_flag == true)
    {
        APP_PRINT_TRACE0("app_rtk_auth_cmd_handle: invalid length");
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
    }
}

/*
    The entire auth process is synchronous,
    so no need to consider the mutual exclusion.
    All global variables can be shared directly
    between app task and auth key task.
*/

void app_rtk_auth_key_ret_handle(T_IO_MSG *io_msg)
{
    T_AUTH_KEY_MSG auth_key_msg = (T_AUTH_KEY_MSG)io_msg->subtype;
    int ret = io_msg->u.param;

    switch (auth_key_msg)
    {
    case AUTH_KEY_MSG_PUBLIC:
        {
            app_rtk_auth_start_resp(ret);
        }
        break;
    case AUTH_KEY_MSG_SHARE:
        {
            app_rtk_auth_public_key_exchange_resp(ret);
        }
        break;
    default:
        break;
    }
}

static void app_rtk_auth_key_ret_ack(T_AUTH_KEY_MSG auth_key_msg, int auth_key_ret)
{
    T_IO_MSG io_msg;

    io_msg.type = IO_MSG_TYPE_AUTH_KEY;
    io_msg.subtype = auth_key_msg;
    io_msg.u.param = auth_key_ret;
    app_io_msg_send(&io_msg);
}

static void app_rtk_auth_key_req(T_AUTH_KEY_MSG auth_key_msg)
{
    os_msg_send(g_msg_queue_handle, &auth_key_msg, 0);
}

static void app_rtk_auth_key_task(void *arg)
{
    T_AUTH_KEY_MSG auth_key_msg;
    int auth_key_ret = 0;

    os_msg_queue_create(&g_msg_queue_handle, "auth_key_queue", 8, sizeof(T_AUTH_KEY_MSG));
    while (true)
    {
        if (os_msg_recv(g_msg_queue_handle, &auth_key_msg, 0xFFFFFFFF) == true)
        {
            switch (auth_key_msg)
            {
            case AUTH_KEY_MSG_PUBLIC:
                auth_key_ret = get_bind_public_key(g_public_key, AUTH_PUBLIC_KEY_LEN);
                app_rtk_auth_key_ret_ack(auth_key_msg, auth_key_ret);
                break;
            case AUTH_KEY_MSG_SHARE:
                auth_key_ret = generate_bind_share_key();
                app_rtk_auth_key_ret_ack(auth_key_msg, auth_key_ret);
                break;
            default:
                break;
            }
        }
    }
}

void app_rtk_auth_init(void)
{
    void *task_handle;

    os_task_create(&task_handle, "auth_key_task", app_rtk_auth_key_task, NULL, 800, 1);
}
#endif
