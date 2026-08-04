/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "hrs_gatt_svc.h"
#include "app_ble_service_info.h"

T_SERVER_ID hrs_gatt_srv_id = 0xFF;

static T_APP_RESULT app_hrs_gatt_svc_callback(uint16_t conn_handle, uint16_t cid, uint8_t type,
                                              void *p_data)
{
    if (type == GATT_MSG_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE_IND)
    {
        T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE *p_read = (T_HRS_SERVER_READ_BODY_SENSOR_LOCATION_VALUE
                                                                *)p_data;
        APP_PRINT_INFO1("app_hrs_gatt_svc_callback: read_value_index 0x%x",
                        p_read->read_value_index);
    }
    else if (type == GATT_MSG_HRS_SERVER_WRITE_CP_OPCODE)
    {
        T_HRS_SERVER_WRITE_CP_OPCODE *p_write = (T_HRS_SERVER_WRITE_CP_OPCODE *)p_data;
        APP_PRINT_INFO1("app_hrs_gatt_svc_callback: wrtie cp opcode 0x%x",
                        p_write->opcode);
    }
    else if (type == GATT_MSG_HRS_SERVER_CCCD_UPDATE)
    {
        T_HRS_SERVER_CCCD_UPDATE *p_update = (T_HRS_SERVER_CCCD_UPDATE *)p_data;
        APP_PRINT_INFO1("app_hrs_gatt_svc_callback: notification_enable_index 0x%x",
                        p_update->notification_enable_index);
    }
    return APP_RESULT_SUCCESS;
}

void rtk_ppg_gatt_init(void)
{
    hrs_gatt_srv_id = hrs_reg_srv(app_hrs_gatt_svc_callback);
}

APP_BLE_SERVICE_INFO(rtk_ppg, 1);