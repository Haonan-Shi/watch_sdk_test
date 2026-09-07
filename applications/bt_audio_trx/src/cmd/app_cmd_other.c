/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "console.h"
#include "ancs.h"
#include "ams.h"
#include "app_adc.h"
#include "app_cmd.h"
#include "app_cmd_ancs.h"

#include "app_cmd_other.h"
#include "app_dlps.h"
#include "app_main.h"
#include "app_mmi.h"
#include "app_vendor.h"


void app_cmd_other_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path,
                              uint8_t app_idx, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    switch (cmd_id)
    {
    case CMD_ASSIGN_BUFFER_SIZE:
        {
            app_db.external_mcu_mtu = (cmd_ptr[4] | (cmd_ptr[5] << 8));
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_TONE_GEN:
        {
            ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_STRING_MODE:
        {
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
#if F_APP_CONSOLE_SUPPORT
            console_set_mode(CONSOLE_MODE_STRING);
#endif
        }
        break;

    case CMD_SET_AND_READ_DLPS:
        {

            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);

            if (cmd_ptr[2] == SET_DLPS_DISABLE)
            {
                app_cmd_mgr.dlps.status = 0x00;
                app_dlps_disable(APP_DLPS_ENTER_CHECK_CMD);
            }
            else if (cmd_ptr[2] == SET_DLPS_ENABLE)
            {
                app_cmd_mgr.dlps.status = 0x01;
                app_dlps_enable(APP_DLPS_ENTER_CHECK_CMD);
            }

            app_report_event(cmd_path, EVENT_REPORT_DLPS_STATUS, app_idx, &app_cmd_mgr.dlps.status, 1);
        }
        break;

#if F_APP_BT_ANCS_CLIENT_SUPPORT
    case CMD_ANCS_REGISTER ... CMD_ANCS_PERFORM_NOTIFICATION_ACTION:
        ancs_handle_cmd(app_idx, (T_CMD_PATH)cmd_path, cmd_ptr, ack_pkt);
        break;
#endif

#if F_APP_BLE_AMS_CLIENT_SUPPORT
    case CMD_AMS_REGISTER ... CMD_AMS_READ_ENTITY_ATTR:
        ams_handle_cmd(app_idx, (T_CMD_PATH)cmd_path, cmd_ptr, ack_pkt);
        break;
#endif

    case CMD_LED_TEST:
        {

        }
        break;

    case CMD_SWITCH_TO_HCI_DOWNLOAD_MODE:
        {
            //if uart tx shares the same pin with 3pin gpio, set uart tx pin when receive cmd

            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
            app_start_timer_switch_to_hci_mode(app_idx);
        }
        break;

#if F_APP_ADC_SUPPORT
    case CMD_GET_PAD_VOLTAGE:
        {
            uint8_t adc_pin = cmd_ptr[2];

            app_adc_set_cmd_info(cmd_path, app_idx);
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);

            if (!app_adc_enable_read_adc_io(adc_pin))
            {
                uint8_t evt_param[2] = {0xFF, 0xFF};

                app_report_event(cmd_path, EVENT_REPORT_PAD_VOLTAGE, app_idx, evt_param, sizeof(evt_param));
                APP_PRINT_TRACE0("CMD_GET_PAD_VOLTAGE register ADC mgr fail");
            }
        }
        break;
#endif

    case CMD_RF_XTAK_K:
        {
            uint8_t report_status = 0;
            uint8_t rf_channel = cmd_ptr[2];
            uint8_t freq_upperbound = cmd_ptr[3];
            uint8_t freq_lowerbound = cmd_ptr[4];
            uint8_t measure_offset = cmd_ptr[5];

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            report_status = 0;
            app_report_event(cmd_path, EVENT_RF_XTAL_K, app_idx, &report_status, 1);

            app_dlps_disable(APP_DLPS_ENTER_CHECK_RF_XTAL);

            /* start rf xtal auto K flow */
#if F_APP_CLI_BINARY_MP_SUPPORT
            //enter mp test mode
            mp_test_handle_cmd(app_idx, cmd_path, 0, 0, NULL, 1);
#endif

            app_vendor_rf_xtak_k(rf_channel, freq_upperbound, freq_lowerbound, measure_offset);
        }
        break;

    case CMD_RF_XTAL_K_GET_RESULT:
        {
            uint8_t xtal_k_result[3] = {0};

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            app_dlps_disable(APP_DLPS_ENTER_CHECK_RF_XTAL);

            /* start to get xtal cap value */
#if F_APP_CLI_BINARY_MP_SUPPORT
            //enter mp test mode
            mp_test_handle_cmd(app_idx, cmd_path, 0, 0, NULL, 1);
#endif

            app_vendor_get_xtak_k_result();
        }
        break;

    case CMD_MEMORY_DUMP:
        {
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
            app_mmi_handle_action(MMI_MEMORY_DUMP);
        }
        break;

    case CMD_MP_TEST:
        {

        }
        break;

    default:
        break;
    }
}
