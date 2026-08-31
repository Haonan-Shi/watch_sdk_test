/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "rtl876x_pinmux.h"
#include "audio_probe.h"
#include "app_cmd.h"

#include "app_cmd_customized.h"
#include "app_audio_route.h"
#include "app_main.h"

void app_cmd_customized_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path,
                                   uint8_t app_idx, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    switch (cmd_id)
    {
    case CMD_IO_PIN_PULL_HIGH:
        {
            uint8_t report_status = 0;
            uint8_t pin_num = cmd_ptr[2];

            Pad_Config(pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);

            app_report_event(cmd_path, EVENT_IO_PIN_PULL_HIGH, app_idx, &report_status, 1);
        }
        break;

    case CMD_ENTER_BAT_OFF_MODE:
        {
            uint8_t report_status = 0;

            app_report_event(cmd_path, EVENT_ENTER_BAT_OFF_MODE, app_idx, &report_status, 1);
        }
        break;

    case CMD_MIC_MP_VERIFY_BY_HFP:
        {
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            uint8_t specified_mic = cmd_ptr[2];
            uint8_t report_status = 0;
            uint8_t i;
            T_AUDIO_ROUTE_IO_TYPE logical_mic[2] = {AUDIO_ROUTE_IO_VOICE_PRIMARY_IN, AUDIO_ROUTE_IO_VOICE_SECONDARY_IN};
            T_AUDIO_ROUTE_ENTRY mic_entry[2];
            T_APP_BR_LINK *p_link = NULL;

            p_link = app_link_find_br_link(app_db.br_link[app_hfp_get_active_idx()].bd_addr);

            memset(&mic_entry, 0, sizeof(mic_entry));

            APP_PRINT_INFO4("CMD_MIC_MP_VERIFY_BY_HFP specified_mic = %x, pri_sel_ori = %x, pri_sel_new = %x, pri_type_new = %x",
                            specified_mic, app_db.mic_mp_verify_pri_sel_ori, cmd_ptr[3], cmd_ptr[4]);

            for (i = 0; i < 2; i++)
            {
                app_audio_route_entry_get(AUDIO_CATEGORY_VOICE,
                                          AUDIO_DEVICE_OUT_SPK | AUDIO_DEVICE_IN_MIC,
                                          logical_mic[i],
                                          &mic_entry[i]);
            }

            if (specified_mic)
            {
                mic_entry[0].endpoint_attr.mic.id = (T_AUDIO_ROUTE_MIC_ID)cmd_ptr[3];
                mic_entry[0].endpoint_attr.mic.type = (T_AUDIO_ROUTE_MIC_TYPE)cmd_ptr[4];
                mic_entry[1].endpoint_attr.mic.id = AUDIO_ROUTE_EXT_MIC;
            }
            else
            {
                mic_entry[0].endpoint_attr.mic.id = (T_AUDIO_ROUTE_MIC_ID)app_db.mic_mp_verify_pri_sel_ori;
                mic_entry[0].endpoint_attr.mic.type = (T_AUDIO_ROUTE_MIC_TYPE)app_db.mic_mp_verify_pri_type_ori;
                mic_entry[1].endpoint_attr.mic.id = (T_AUDIO_ROUTE_MIC_ID)app_db.mic_mp_verify_sec_sel_ori;
                mic_entry[1].endpoint_attr.mic.type = (T_AUDIO_ROUTE_MIC_TYPE)app_db.mic_mp_verify_sec_type_ori;
            }

            app_audio_route_entry_update(AUDIO_CATEGORY_VOICE,
                                         AUDIO_DEVICE_OUT_SPK | AUDIO_DEVICE_IN_MIC,
                                         p_link->sco.track_handle,
                                         2,
                                         mic_entry);

            app_report_event(cmd_path, EVENT_MIC_MP_VERIFY_BY_HFP, app_idx, &report_status, 1);
        }
        break;

    case CMD_USBH_AUDIO_SET_PARAM:
        {
            uint32_t rate       = (uint32_t)(cmd_ptr[2] | (cmd_ptr[3] << 8) | (cmd_ptr[4] << 16) |
                                             (cmd_ptr[5] << 24));
            uint32_t ch         = (uint32_t)(cmd_ptr[6] | (cmd_ptr[7] << 8) | (cmd_ptr[8] << 16) |
                                             (cmd_ptr[9] << 24));
            uint32_t bits       = (uint32_t)(cmd_ptr[10] | (cmd_ptr[11] << 8) | (cmd_ptr[12] << 16) |
                                             (cmd_ptr[13] << 24));
            uint32_t buf_intrvl = (uint32_t)(cmd_ptr[14] | (cmd_ptr[15] << 8) | (cmd_ptr[16] << 16) |
                                             (cmd_ptr[17] << 24));

            app_usbh_audio_set_param(rate, ch, bits, buf_intrvl);
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;

    case CMD_USBH_AUDIO_CONTROL:
        {
            if (cmd_ptr[2] == 0x01)
            {
                usbh_mgr_start();
            }
            else if (cmd_ptr[2] == 0x00)
            {
                usbh_mgr_stop();
            }
            else
            {
                APP_PRINT_WARN1("CMD_USBH_AUDIO_CONTROL: invalid control value %d", cmd_ptr[2]);
            }
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    default:
        break;
    }
}
