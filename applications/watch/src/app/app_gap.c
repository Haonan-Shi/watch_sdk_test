/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "gap.h"
#include "gap_br.h"
#include "app_gap.h"
#include "app_main.h"
#include "app_cfg.h"
#include "remote.h"
#include "btm.h"
#include "app_linkback.h"
#include "bt_bond.h"
#include "app_bt_policy_int.h"
#include "module_font.h"
#include "app_ble_adv.h"
#include "event_bus.h"

const uint8_t null_addr[6] = {0};

typedef struct
{
    uint8_t bd_addr[6];
    uint8_t nam_len;
    uint16_t device_name[25];  // UTF-16 encoded device name (50 bytes / 2)
    uint32_t cod;
} T_SEARCH_RESULT;

T_SEARCH_RESULT       search_result_temp;
static T_SEARCH_STATUS search_status = SEARCH_STOP;


static void app_gap_common_callback(uint8_t cb_type, void *p_cb_data)
{
    T_GAP_CB_DATA cb_data;
    memcpy(&cb_data, p_cb_data, sizeof(T_GAP_CB_DATA));
    APP_PRINT_INFO1("app_gap_common_callback: cb_type = %d", cb_type);
    switch (cb_type)
    {
    case GAP_MSG_WRITE_AIRPLAN_MODE:
        APP_PRINT_INFO1("app_gap_common_callback: GAP_MSG_WRITE_AIRPLAN_MODE cause 0x%04x",
                        cb_data.p_gap_write_airplan_mode_rsp->cause);
        break;

    case GAP_MSG_READ_AIRPLAN_MODE:
        APP_PRINT_INFO2("app_gap_common_callback: GAP_MSG_READ_AIRPLAN_MODE cause 0x%04x, mode %d",
                        cb_data.p_gap_read_airplan_mode_rsp->cause,
                        cb_data.p_gap_read_airplan_mode_rsp->mode);
        break;

    case GAP_MSG_VENDOR_CMD_CMPL_EVENT:
        break;

    case GAP_MSG_SET_LOCAL_BD_ADDR:
        {
            APP_PRINT_INFO1("app_gap_common_callback: GAP_MSG_SET_LOCAL_BD_ADDR: cause 0x%04x",
                            cb_data.p_gap_set_bd_addr_rsp->cause);
        }
        break;

    default:
        break;
    }
    return;
}
static void app_gap_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    APP_PRINT_INFO1("app_gap_bt_cback: event_type:0x%x", event_type);
    switch (event_type)
    {
    case BT_EVENT_READY:
        {
            memcpy(app_db.factory_addr, param->ready.bd_addr, 6);
            APP_PRINT_INFO1("app_gap_bt_cback: bt_ready, bd_addr %b",
                            TRACE_BDADDR(param->ready.bd_addr));
            if (!memcmp(app_cfg_nv.bud_local_addr, null_addr, 6))
            {
                memcpy(app_cfg_nv.bud_local_addr, app_db.factory_addr, 6);
                remote_local_addr_set(app_cfg_nv.bud_local_addr);
            }
            gap_set_bd_addr(app_cfg_nv.bud_local_addr);

            if (!app_db.bt_is_ready)
            {
                app_db.bt_is_ready = true;
                if (app_db.ble_is_ready && app_db.bt_is_ready)
                {
                    app_ble_common_adv_set_param();

                    app_ble_common_adv_start(0);
                }
            }
            app_bt_policy_event_handle(EVENT_BT_STARTUP, NULL);
        }
        break;

    case BT_EVENT_DEVICE_MODE_RSP:
        {

        }
        break;

    case BT_EVENT_LINK_USER_CONFIRMATION_REQ:
        {
            gap_br_user_cfm_req_cfm(param->link_user_confirmation_req.bd_addr, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case BT_EVENT_INQUIRY_RSP:
        {
            APP_PRINT_INFO1("BT_EVENT_INQUIRY_RSP response cause = 0x%x", param->inquiry_rsp.cause);
            if (param->inquiry_rsp.cause == 0)
            {
                app_bt_bond_free_temp_cache_device();
                set_search_status(SEARCH_START);
                //todo: update gui
                //gui_update_by_event(GUI_EVENT_BUDS_SEARCH, NULL, false);

            }
        }
        break;

    case BT_EVENT_INQUIRY_RESULT:
        {
            APP_PRINT_INFO0("BT_EVENT_INQUIRY_RESULT");
            APP_PRINT_INFO6("<-- inquiry device info: addr =[%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
                            param->inquiry_result.bd_addr[5], param->inquiry_result.bd_addr[4],
                            param->inquiry_result.bd_addr[3], param->inquiry_result.bd_addr[2],
                            param->inquiry_result.bd_addr[1], param->inquiry_result.bd_addr[0]);
            APP_PRINT_INFO1("<-- inquiry device info: name = %s", TRACE_STRING(param->inquiry_result.name));
            APP_PRINT_INFO1("<-- inquiry device info: name len = %d",
                            strlen((const char *)param->inquiry_result.name));
            APP_PRINT_INFO1("<-- inquiry device info: rssi = %d", param->inquiry_result.rssi);
            APP_PRINT_INFO1("<-- inquiry device info: class of device = 0x%x", param->inquiry_result.cod);


            if ((param->inquiry_result.rssi >= -60) && \
                (((param->acl_conn_ind.cod & 0x1F00) >> 8) == 0x04))
            {
                uint8_t *device_name = (uint8_t *)param->inquiry_result.name;
                uint8_t length = strlen((const char *)param->inquiry_result.name);
                if (length > 50)
                {
                    length = 50;
                }
                memcpy(search_result_temp.bd_addr, param->inquiry_result.bd_addr, 6);
                search_result_temp.nam_len = watch_utf8_to_unicode(device_name, length,
                                                                   search_result_temp.device_name, 25);
                search_result_temp.cod = param->acl_conn_ind.cod;
                //for RTL87x3EP gui
                app_bt_bond_add_temp_cache_device(search_result_temp.bd_addr, search_result_temp.cod,
                                                  search_result_temp.device_name,
                                                  search_result_temp.nam_len);

                // Notify GUI of inquiry result
                APP_PRINT_INFO1("app_gap: publishing bt/inquiry_result, name_len=%d", search_result_temp.nam_len);
                event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_INQUIRY_RESULT, &search_result_temp,
                                  sizeof(T_SEARCH_RESULT));
            }
        }
        break;

    case BT_EVENT_INQUIRY_CMPL:
        {
            APP_PRINT_INFO0("BT_EVENT_INQUIRY_CMPL");
            //for RTL87x3EP gui
            set_search_status(SEARCH_FINISH);

            // Notify GUI that inquiry is complete
            event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_INQUIRY_CMPL, NULL, 0);
        }
        break;

    case BT_EVENT_INQUIRY_CANCEL_RSP:
        {
            APP_PRINT_INFO0("BT_EVENT_INQUIRY_CANCEL_RSP");
        }
        break;

    default:
        break;
    }
}

void app_gap_init(void)
{
    gap_register_app_cb(app_gap_common_callback);
    bt_mgr_cback_register(app_gap_bt_cback);
}

void set_search_status(T_SEARCH_STATUS status)
{
    if (status <= SEARCH_FINISH)
    {
        search_status = status;
    }
}
T_SEARCH_STATUS get_search_status(void)
{
    return search_status;
}

void app_bt_inquiry_start(void)
{
    if (app_db.bt_state == STATE_LINKBACK)
    {
        linkback_stop();
    }
    set_search_status(SEARCH_START);
    app_bt_bond_free_temp_cache_device();
    T_GAP_CAUSE cause = gap_br_start_inquiry(false, 8);
    APP_PRINT_INFO1("app_bt_inquiry_start: cause = 0x%x", cause);
}

void app_bt_inquiry_stop(void)
{
    set_search_status(SEARCH_STOP);
    gap_br_stop_inquiry();
}
