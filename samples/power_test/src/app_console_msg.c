/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "bt_types.h"
#include "trace.h"
#include "os_mem.h"
#include "app_msg.h"
#include "app_console_msg.h"
#include "power_test.h"
#include "pm.h"
#include "gap_br.h"
#include "power_test_dvfs.h"
#include "bt_bond.h"
#include "power_test_app_hfp_ag.h"
#include "power_test_rf_power.h"
#include "power_test_flash.h"

extern void power_test_set_mode(uint16_t action, uint8_t *buf);

void app_console_handle_msg(T_IO_MSG console_msg)
{
    uint16_t  subtype;
    uint16_t  id;
    uint16_t  action;
    uint8_t  *p;

    p       = console_msg.u.buf;
    subtype = console_msg.subtype;
    switch (subtype)
    {
    case IO_MSG_CONSOLE_STRING_RX:
        LE_STREAM_TO_UINT16(id, p);

        if (id == POWER_STATE_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_mode(action, console_msg.u.buf);
        }
#ifndef CONFIG_SOC_SERIES_RTL8773E
        else if (id == DVFS_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_dvfs(action, console_msg.u.buf);
        }
#endif
        else if (id == CPU_FREQ_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_cpu_freq(action, console_msg.u.buf);
        }
        else if (id == DSP1_FREQ_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_dsp1_freq(action, console_msg.u.buf);
        }
        else if (id == PLATFORM_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_flash_action((T_POWER_TEST_FLASH_CMD)action, console_msg.u.buf);
        }
#ifdef CONFIG_SOC_SERIES_RTL8773D
        else if (id == MCLK2_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_mclk2(action, console_msg.u.buf);
        }
#endif
        else if (id == CLOCK_32K_ID)
        {
            LE_STREAM_TO_UINT16(action, p);
            power_test_set_32k(action, console_msg.u.buf);
        }
        else if (id == TX_POWER_ID)
        {
            uint8_t     br_1M, edr_2M, edr_3M, le_1M, le_2M;
            LE_STREAM_TO_UINT8(br_1M, p);
            LE_STREAM_TO_UINT8(edr_2M, p);
            LE_STREAM_TO_UINT8(edr_3M, p);
            LE_STREAM_TO_UINT8(le_1M, p);
            LE_STREAM_TO_UINT8(le_2M, p);
            power_test_set_tx_power(br_1M, edr_2M, edr_3M, le_1M, le_2M, console_msg.u.buf);
        }
        else if (id == CONT_TX_ID)
        {
            uint8_t tx_power, packet_type;
            LE_STREAM_TO_UINT8(tx_power, p);
            LE_STREAM_TO_UINT8(packet_type, p);
            power_test_cont_tx(tx_power, packet_type, console_msg.u.buf);
        }
        else if (id == PACKET_RX_ID)
        {
            uint8_t packet_type;
            LE_STREAM_TO_UINT8(packet_type, p);
            power_test_packet_rx(packet_type, console_msg.u.buf);
        }
        else if (id == GAP_LEGACY_ID)
        {
            LE_STREAM_TO_UINT8(action, p);
            DBG_DIRECT("app_console_handle_msg action %d", action);
            switch (action)
            {
            case POWER_TEST_CMD_INQUIRY_SCAN_PARAM_SET:
                {
                    uint8_t  type;
                    uint16_t interval;
                    uint16_t window;

                    LE_STREAM_TO_UINT8(type, p);
                    LE_STREAM_TO_UINT16(interval, p);
                    LE_STREAM_TO_UINT16(window, p);
                    gap_br_cfg_inquiry_scan_param(type, interval, window);
                }
                break;

            case POWER_TEST_CMD_PAGE_SCAN_PARAM_SET:
                {
                    uint8_t  type;
                    uint16_t interval;
                    uint16_t window;
                    uint16_t page_timeout;

                    LE_STREAM_TO_UINT8(type, p);
                    LE_STREAM_TO_UINT16(interval, p);
                    LE_STREAM_TO_UINT16(window, p);
                    LE_STREAM_TO_UINT16(page_timeout, p);
                    gap_br_set_param(GAP_BR_PARAM_PAGE_TIMEOUT, sizeof(uint16_t), &page_timeout);
                    gap_br_cfg_page_scan_param(type, interval, window);
                }
                break;

            case POWER_TEST_CMD_RADIO_MODE_SET:
                {
                    uint8_t  radio_mode;

                    LE_STREAM_TO_UINT8(radio_mode, p);
                    gap_br_set_radio_mode(radio_mode, false, 0);
                }
                break;

            case POWER_TEST_CMD_SNIFF_ENTER:
                {
                    uint16_t min_interval;
                    uint16_t max_interval;
                    uint16_t sniff_attempt;
                    uint16_t sniff_timeout;

                    LE_STREAM_TO_UINT16(min_interval, p);
                    LE_STREAM_TO_UINT16(max_interval, p);
                    LE_STREAM_TO_UINT16(sniff_attempt, p);
                    LE_STREAM_TO_UINT16(sniff_timeout, p);
                    gap_br_enter_sniff_mode(p,
                                            min_interval,
                                            max_interval,
                                            sniff_attempt,
                                            sniff_timeout);
                }
                break;

            case POWER_TEST_CMD_SNIFF_EXIT:
                gap_br_exit_sniff_mode(p);
                break;

            case POWER_TEST_CMD_LINK_DEAULT_POLICY_SET:
                {
                    uint8_t  link_policy;

                    LE_STREAM_TO_UINT8(link_policy, p);
                    gap_br_cfg_default_link_policy(link_policy);
                }
                break;

            case POWER_TEST_CMD_LINK_POLICY_SET:
                {
                    uint8_t  link_policy;

                    LE_STREAM_TO_UINT8(link_policy, p);
                    gap_br_cfg_acl_link_policy(p, link_policy);
                }
                break;

            case POWER_TEST_CMD_INQUIRY_START:
                {
                    uint8_t inquiry_timeout;

                    LE_STREAM_TO_UINT8(inquiry_timeout, p);
                    gap_br_start_inquiry(false, inquiry_timeout);
                }
                break;

            case POWER_TEST_CMD_INQUIRY_STOP:
                gap_br_stop_inquiry();
                break;

            case POWER_TEST_CMD_PAGE_START:
                {
                    T_GAP_UUID_DATA uuid_data;

                    uuid_data.uuid_16 = UUID_HANDSFREE;
                    gap_br_start_sdp_discov(p, GAP_UUID16, uuid_data);
                }
                break;

            case POWER_TEST_CMD_PAGE_STOP:
                gap_br_stop_sdp_discov(p);
                break;

            case POWER_TEST_CMD_HFP_AG_CONN:
                app_power_test_connect_hfp_ag(p, true);
                break;

            case POWER_TEST_CMD_HFP_AG_DISCON:
                app_power_test_disconnect_hfp_ag(p, true);
                break;

            case POWER_TEST_CMD_LEGACY_DISCONNECT:
                gap_br_send_acl_disconn_req(p);
                break;

            case POWER_TEST_CMD_REMOVE_BOND:
                bt_bond_delete(p);
                break;

            default:
                break;
            }
            free(console_msg.u.buf);
        }
        else if (id == GAP_LE_ID)
        {
            LE_STREAM_TO_UINT8(action, p);
            power_handle_le_cmd(action);
        }
        break;
    default:
        break;
    }
}

