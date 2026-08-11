/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "app_cmd.h"
#include "app_cfg.h"
#include "app_link_util.h"
#include "trace.h"
#include "ringtone.h"
#include "gap_conn_le.h"
#include "os_mem.h"
#include "dfu_api.h"
#include "dfu_transport.h"
#include "ota_service.h"
#include "dfu_service.h"
#include "transmit_service.h"
#include "app_ble_service.h"
#include "app_main.h"
#include "app_ble_gap.h"
#include "app_ble_service_info.h"
#include "wristband_private_service.h"
#include "bt_gatt_svc.h"
#include "bas_gatt_svc.h"
#include "dis_gatt_svc.h"
#if CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT
#include "ancs_sample.h"
#include "bt_gatt_client.h"
#endif
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "gfps.h"
#include "app_gfps_cfg.h"
#include "app_gfps_finder.h"
#endif
T_SERVER_ID bas_gatt_srv_id = 0xFF;
T_SERVER_ID dis_gatt_srv_id = 0xFF;
T_SERVER_ID transmit_gatt_srv_id = 0xFF;
T_SERVER_ID ota_gatt_srv_id = 0xFF;
T_SERVER_ID dfu_gatt_srv_id = 0xFF;
T_SERVER_ID wristband_gatt_srv_id = 0xFF;

/** @defgroup  PERIPH_SEVER_CALLBACK Profile Server Callback Event Handler
    * @brief Handle profile server callback event
    * @{
    */

void app_gatt_svc_general_cb(uint8_t type, void *p_data)
{
    if (type == GATT_SVC_EVENT_REG_RESULT)
    {
        T_GATT_SVC_REG_RESULT *p_result = (T_GATT_SVC_REG_RESULT *)p_data;

        APP_PRINT_INFO1("GATT_SVC_EVENT_REG_RESULT: result 0x%x", p_result->result);
    }
    else if (type == GATT_SVC_EVENT_REG_AFTER_INIT_RESULT)
    {
        T_GATT_SVC_REG_AFTER_INIT_RESULT *p_result = (T_GATT_SVC_REG_AFTER_INIT_RESULT *)p_data;
        APP_PRINT_INFO2("GATT_SVC_EVENT_REG_AFTER_INIT_RESULT: service_id %d, cause 0x%x",
                        p_result->service_id, p_result->cause);
    }
}

T_APP_RESULT app_bas_gatt_svc_callback(uint16_t conn_handle, uint16_t cid, uint8_t type,
                                       void *p_data)
{
    if (type == GATT_MSG_BAS_SERVER_READ_BATTERY_LEVEL_IND)
    {
        uint8_t battery_level = 90;
        bas_battery_level_read_confirm(conn_handle, cid, bas_gatt_srv_id, battery_level);
    }
    else if (type == GATT_MSG_BAS_SERVER_CCCD_UPDATE)
    {
        T_BAS_SERVER_CCCD_UPDATE *p_update = (T_BAS_SERVER_CCCD_UPDATE *)p_data;
        APP_PRINT_INFO2("app_bas_gatt_svc_callback: char uuid 0x%x, cccd_cfg 0x%x",
                        p_update->char_uuid, p_update->cccd_cfg);
    }
    return APP_RESULT_SUCCESS;
}

T_APP_RESULT app_dis_gatt_svc_callback(uint16_t conn_handle, uint16_t cid, uint8_t type,
                                       void *p_data)
{
    bool ret = true;
    if (type == GATT_MSG_DIS_SERVER_READ_CHAR_IND)
    {
        T_DIS_SERVER_READ_CHAR_IND *p_read = (T_DIS_SERVER_READ_CHAR_IND *)p_data;

        APP_PRINT_INFO3("app_dis_gatt_svc_callback: service id %d, char_uuid 0x%x, offset %d",
                        p_read->service_id, p_read->char_uuid, p_read->offset);

        switch (p_read->char_uuid)
        {
        case GATT_UUID_CHAR_SYSTEM_ID:
            {
                const uint8_t DISSystemID[DIS_SYSTEM_ID_LENGTH] = {0, 1, 2, 0, 0, 3, 4, 5};
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            DIS_SYSTEM_ID_LENGTH, (uint8_t *)DISSystemID);
            }
            break;

        case GATT_UUID_CHAR_MODEL_NUMBER:
            {
                const uint8_t DISModelNumber[] = "Model Nbr 0.9";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISModelNumber), (uint8_t *)DISModelNumber);
            }
            break;

        case GATT_UUID_CHAR_SERIAL_NUMBER:
            {
                const uint8_t DISSerialNumber[] = "RTKBeeSerialNum";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISSerialNumber), (uint8_t *)DISSerialNumber);
            }
            break;

        case GATT_UUID_CHAR_FIRMWARE_REVISION:
            {
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
                if (app_gfps_cfg.gfps_finder_support &&
                    (app_gfps_cfg.gfps_device_type == GFPS_LOCATOR_TRACKER) &&
                    app_gfps_finder_provisoned())
                {
                    APP_PRINT_INFO0("gfps finder do not support read firmware version");
                    return APP_RESULT_REJECT;
                }
                else
#endif
                {
                    ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                                sizeof(app_gfps_cfg.gfps_version), (uint8_t *)app_gfps_cfg.gfps_version);
                }
#else
                const uint8_t DISFirmwareRev[] = "RTKBeeFirmwareRev";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISFirmwareRev), (uint8_t *)DISFirmwareRev);
#endif
            }
            break;

        case GATT_UUID_CHAR_HARDWARE_REVISION:
            {
                const uint8_t DISHardwareRev[] = "RTKBeeHardwareRev";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISHardwareRev), (uint8_t *)DISHardwareRev);
            }
            break;

        case GATT_UUID_CHAR_SOFTWARE_REVISION:
            {
                const uint8_t DISSoftwareRev[] = "RTKBeeSoftwareRev";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISSoftwareRev), (uint8_t *)DISSoftwareRev);
            }
            break;

        case GATT_UUID_CHAR_MANUFACTURER_NAME:
            {
                const uint8_t DISManufacturerName[] = "Realtek BT";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISManufacturerName), (uint8_t *)DISManufacturerName);
            }
            break;

        case GATT_UUID_CHAR_IEEE_CERTIF_DATA_LIST:
            {
                const uint8_t DISIEEEDataList[] = "RTKBeeIEEEDatalist";
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(DISIEEEDataList), (uint8_t *)DISIEEEDataList);
            }
            break;

        case GATT_UUID_CHAR_PNP_ID:
            {
                uint16_t version = 0x01; //VERSION_BUILD;
                uint8_t DISPnpID[DIS_PNP_ID_LENGTH] = {0x01, 0x5D, 0x00, 0x01, 0x00,
                                                       (uint8_t)version, (uint8_t)(version >> 8)
                                                      }; //VID_005D&PID_0001?
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            DIS_PNP_ID_LENGTH, DISPnpID);
            }
            break;

        case GATT_UUID_CHAR_UDI_MEDICAL_DEVICES:
            {
                uint8_t UDIMedicalDevices[] = {0x01, 'R', 'T', 'K', 'U', 'D', 'I', 'L', 'a', 'b', 'e', 'l', 0x00};
                ret = dis_char_read_confirm(conn_handle, cid, p_read->service_id, p_read->char_uuid, p_read->offset,
                                            sizeof(UDIMedicalDevices), UDIMedicalDevices);
            }
            break;

        default:
            break;
        }
    }

    if (ret)
    {
        return APP_RESULT_SUCCESS;
    }
    else
    {
        return APP_RESULT_APP_ERR;
    }
}
static T_APP_RESULT app_transmit_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;

    T_APP_LE_LINK *p_link;

    T_TRANSMIT_SRV_CALLBACK_DATA *p_callback = (T_TRANSMIT_SRV_CALLBACK_DATA *)p_data;
    APP_PRINT_INFO2("app_transmit_gatt_svc_callback: conn_id %d, type %d", p_callback->conn_id,
                    type);
    p_link = app_find_le_link_by_conn_id(p_callback->conn_id);
    if (p_link != NULL)
    {
        if (type == GATT_MSG_TRANSMIT_SERVER_WRITE)
        {
            uint8_t         *p_data;
            uint16_t        data_len;
            uint16_t        total_len;

            p_data = p_callback->msg_data.rx_data.p_value;
            data_len = p_callback->msg_data.rx_data.len;

            if (p_callback->attr_index == TRANSMIT_SVC_RX_DATA_INDEX)
            {
                uint8_t rx_seqn;

                if (p_link->p_embedded_cmd == NULL)
                {
                    uint16_t cmd_len;

                    while (data_len > 5)
                    {
                        if (p_data[0] == CMD_SYNC_BYTE)
                        {
                            rx_seqn = p_data[1];
                            cmd_len = (p_data[2] | (p_data[3] << 8)) + 4; //sync_byte, seqn, length
                            if (data_len >= cmd_len)
                            {
                                app_handle_cmd_set(&p_data[4], (cmd_len - 4), CMD_PATH_LE, rx_seqn, p_link->id);
                                data_len -= cmd_len;
                                p_data += cmd_len;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            data_len--;
                            p_data++;
                        }
                    }

                    if (data_len)
                    {
                        p_link->p_embedded_cmd = malloc(data_len);
                        if (p_link->p_embedded_cmd != NULL)
                        {
                            memcpy(p_link->p_embedded_cmd, p_data, data_len);
                            p_link->embedded_cmd_len = data_len;
                        }
                    }
                }
                else
                {
                    uint8_t *p_temp;
                    uint16_t cmd_len;

                    p_temp = p_link->p_embedded_cmd;
                    total_len = p_link->embedded_cmd_len + data_len;
                    p_link->p_embedded_cmd = malloc(total_len);
                    if (p_link->p_embedded_cmd != NULL)
                    {
                        memcpy(p_link->p_embedded_cmd, p_temp, p_link->embedded_cmd_len);
                        free(p_temp);
                        memcpy(p_link->p_embedded_cmd + p_link->embedded_cmd_len, p_data, data_len);
                        p_link->embedded_cmd_len = total_len;
                        data_len = total_len;
                    }
                    else
                    {
                        p_link->p_embedded_cmd = p_temp;
                        data_len = p_link->embedded_cmd_len;
                    }
                    p_data = p_link->p_embedded_cmd;

                    //ios will auto combine two cmd into one pkt
                    while (data_len > 5)
                    {
                        if (p_data[0] == CMD_SYNC_BYTE)
                        {
                            rx_seqn = p_data[1];
                            cmd_len = (p_data[2] | (p_data[3] << 8)) + 4;
                            if (data_len >= cmd_len)
                            {
                                app_handle_cmd_set(&p_data[4], (cmd_len - 4), CMD_PATH_LE, rx_seqn, p_link->id);
                                data_len -= cmd_len;
                                p_data += cmd_len;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else
                        {
                            data_len--;
                            p_data++;
                        }
                    }

                    if (data_len && p_data != NULL)
                    {
                        p_temp = p_link->p_embedded_cmd;
                        p_link->p_embedded_cmd = malloc(data_len);
                        if (p_link->p_embedded_cmd != NULL)
                        {
                            memcpy(p_link->p_embedded_cmd, p_data, data_len);
                            p_link->embedded_cmd_len = data_len;
                            free(p_temp);
                        }
                    }
                }
            }
            else if (p_callback->attr_index == TRANSMIT_SVC_DEVICE_INFO_INDEX)
            {
                //handle  p_data data_len here
                APP_PRINT_INFO2("app_transmit_srv_cb: TRANSMIT_SVC_DEVICE_INFO_INDEX data_len %d, data %b",
                                data_len, TRACE_BINARY(data_len, p_data));
            }
        }
        else if (type == GATT_MSG_TRANSMIT_SERVER_CCCD_UPDATE)
        {
            if (p_callback->attr_index == TRANSMIT_SVC_TX_DATA_CCCD_INDEX)
            {
                if (p_callback->msg_data.notification_indification_value == TRANSMIT_SVC_TX_DATA_CCCD_ENABLE)
                {
                    p_link->transmit_srv_tx_enable_fg |= TX_ENABLE_CCCD_BIT;
                    APP_PRINT_INFO0("app_transmit_srv_cb: TRANSMIT_SVC_TX_DATA_CCCD_ENABLE");
                }
                else if (p_callback->msg_data.notification_indification_value == TRANSMIT_SVC_TX_DATA_CCCD_DISABLE)
                {
                    p_link->transmit_srv_tx_enable_fg &= ~TX_ENABLE_CCCD_BIT;
                    APP_PRINT_INFO0("app_transmit_srv_cb: TRANSMIT_SVC_TX_DATA_CCCD_DISABLE");
                }
            }
        }
    }

    return app_result;
}

static T_APP_RESULT app_ota_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_OTA_CALLBACK_DATA *p_callback = (T_OTA_CALLBACK_DATA *)p_data;
    switch (type)
    {
    case GATT_MSG_OTA_SERVER_WRITE:
        {
            if (OTA_WRITE_CHAR_VAL == p_callback->msg_data.write.opcode &&
                OTA_VALUE_ENTER == p_callback->msg_data.write.value)
            {
                /*battery level is above 60 percent*/
                APP_PRINT_INFO0("Preparing switch into OTA mode");
                /*prepare to enter OTA mode, before switch action, we should disconnect first.*/
                dfu_switch_to_ota_mode_pending = true;
                le_disconnect(p_callback->conn_id);
            }
        }
        break;
    default:
        break;
    }
    return app_result;
}

static T_APP_RESULT app_dfu_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_DFU_CALLBACK_DATA *p_callback = (T_DFU_CALLBACK_DATA *)p_data;
    switch (type)
    {
    case GATT_MSG_DFU_SERVER_CCCD_UPDATE:
        {
            if (p_callback->msg_data.notification_indification_index == DFU_CP_NOTIFY_ENABLE)
            {
                APP_PRINT_INFO0("dfu notification enable");
            }
            else if (p_callback->msg_data.notification_indification_index ==
                     DFU_CP_NOTIFY_DISABLE)
            {
                APP_PRINT_INFO0("dfu notification disable");
            }
        }
        break;
    case GATT_MSG_DFU_SERVER_WRITE:
        {

        }
        break;
    default:
        break;
    }

    return app_result;
}

static T_APP_RESULT app_wristband_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_WRISTBAND_CALLBACK_DATA *p_wristband_cb_data = (T_WRISTBAND_CALLBACK_DATA *)p_data;
    APP_PRINT_INFO1("app_wristband_gatt_svc_callback:  type %d", type);
    switch (type)
    {

    case GATT_MSG_WRISTBAND_SERVER_CCCD_UPDATE:
        {
            APP_PRINT_INFO0("GATT_MSG_WRISTBAND_SERVER_CCCD_UPDATE");
        }
        break;

    default:

        break;
    }

    return app_result;
}

void app_wristband_gatt_svc_send_callback(T_EXT_SEND_DATA_RESULT result)
{
    if (result.cause == GAP_SUCCESS)
    {
#if CONFIG_CHATGPT
        void chatgpt_port_ble_send_completed_proc(void);
        chatgpt_port_ble_send_completed_proc();//ble send completed handle
#endif
    }
}

/** @} */ /* End of group PERIPH_SEVER_CALLBACK */

void app_ble_service_init(void)
{
    uint32_t total_services = app_ble_service_get_total();

#if CONFIG_GATT_OVER_BREDR
    gatt_svc_init(GATT_SVC_USE_EXT_SERVER, total_services);
#else
    gatt_svc_init(GATT_SVC_USE_NORMAL_SERVER, total_services);
#endif
    bas_gatt_srv_id = bas_reg_srv(app_bas_gatt_svc_callback);
    dis_gatt_srv_id = dis_reg_srv(app_dis_gatt_svc_callback);
    ota_gatt_srv_id = ota_reg_srv(app_ota_gatt_svc_callback);
    dfu_gatt_srv_id = dfu_reg_srv(app_dfu_gatt_svc_callback);
    transmit_gatt_srv_id = transmit_reg_srv(app_transmit_gatt_svc_callback);
    wristband_gatt_srv_id = wristband_reg_service(app_wristband_gatt_svc_callback,
                                                  app_wristband_gatt_svc_send_callback);

    gatt_svc_register_general_cb(app_gatt_svc_general_cb);

#if CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT
    gatt_client_init(GATT_CLIENT_DISCOV_MODE_REG_SVC_BIT |  GATT_CLIENT_DISCOV_MODE_USE_EXT_CLIENT);
    app_ancs_client_init();
#endif
    //ams_init(1);
}

/* Base BLE services registration */
APP_BLE_SERVICE_INFO(bas, 1);    /* Battery Service */
APP_BLE_SERVICE_INFO(dis, 1);    /* Device Information Service */
APP_BLE_SERVICE_INFO(ota, 1);     /* OTA Service */
APP_BLE_SERVICE_INFO(dfu, 1);     /* DFU Service */
APP_BLE_SERVICE_INFO(transmit, 1);
APP_BLE_SERVICE_INFO(wristband, 1);

