/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "os_mem.h"
#include "os_msg.h"
#include "os_task.h"
#include "os_timer.h"
#include "trace.h"
#include "test_mode.h"
#include "auto_k_rf.h"
#include "os_sched.h"
#include "gap_br.h"
#include "gap_bond_le.h"
#include "gap_msg.h"
#include "gap_le.h"
#include "gap.h"
#include "cfg_item_api.h"

#define HCI_CMD_PKT     0x01
#define HCI_ACL_PKT     0x02
#define HCI_SCO_PKT     0x03
#define HCI_EVT_PKT     0x04
#define HCI_RESET_OPCODE                                    0x0C03
#define HCI_SET_EVENT_FILTER_OPCODE                         0x0C05

#define HCI_VENDOR_SINGLE_TONE_CONT_TX_OPCODE               0xFC78
#define HCI_VENDOR_AUTO_K_RF_OPCODE                         0xFCEB


static void *mp_hci_test_task_handle;
static uint8_t xtal_cap[2] = {0xFF, 0xFF};

void clear_xtal_cap_value(void)
{
    xtal_cap[0] = 0xFF;
    xtal_cap[1] = 0xFF;
}

void get_xtal_cap_value(uint8_t *cap_val)
{
    cap_val[0] = xtal_cap[0];
    cap_val[1] = xtal_cap[1];
}

void hci_cmd_vendor_auto_k_rf(uint8_t rx_channel, int8_t freq_drift_upperbound,
                              int8_t freq_drift_lowerbound, int8_t measure_offset)
{
    clear_xtal_cap_value();
    uint16_t opcode = HCI_VENDOR_AUTO_K_RF_OPCODE;
    uint8_t param_len = 6;

    uint8_t cmd_len = param_len + 4;
    uint8_t *p_buf = os_mem_alloc(OS_MEM_TYPE_DATA, cmd_len);
    uint8_t *p = p_buf;

    if (NULL == p)
    {
        return;
    }

    LE_UINT8_TO_STREAM(p, HCI_CMD_PKT);
    LE_UINT16_TO_STREAM(p, opcode);
    LE_UINT8_TO_STREAM(p, param_len);

    LE_UINT8_TO_STREAM(p, 0x06);
    LE_UINT8_TO_STREAM(p, 0x00);
    LE_UINT8_TO_STREAM(p, rx_channel);
    LE_UINT8_TO_STREAM(p, freq_drift_upperbound);
    LE_UINT8_TO_STREAM(p, freq_drift_lowerbound);
    LE_UINT8_TO_STREAM(p, measure_offset);

    hci_if_write(p_buf, cmd_len);
}

void auto_k_rf_hci_evt_data_ind(bool status, uint8_t *p_buf, uint32_t len)
{
    uint16_t opcode = (p_buf[5] << 8) | p_buf[4];
    if (opcode == HCI_VENDOR_AUTO_K_RF_OPCODE)
    {
        xtal_cap[0] = p_buf[6];
        xtal_cap[1] = p_buf[9];
        APP_PRINT_INFO2("auto k rf status = 0x%x, cap_value = 0x%x", xtal_cap[0], xtal_cap[1]);

        if (xtal_cap[0] == 0)
        {
            bool update_ret = cfg_update_xtal(xtal_cap[1]);
            APP_PRINT_INFO1("cfg_update_xtal result = %d", update_ret);
        }
    }
    return;
}

static bool mp_test_handle_hci_cb(T_SINGLE_TONE_EVT evt, bool status, uint8_t *p_buf, uint32_t len)
{
    APP_PRINT_INFO2("mp_test_handle_hci_cb: evt %d, status(bool) %d", evt, status);

    for (uint8_t i = 0; i < len; i++)
    {
        APP_PRINT_INFO2("hci event [%d] = 0x%x", i, p_buf[i]);
    }

    bool result = true;
    switch (evt)
    {
    case SINGLE_TONE_EVT_OPENED:
        {
        }
        break;

    case SINGLE_TONE_EVT_CLOSED:
        break;

    case SINGLE_TONE_EVT_DATA_IND:
        {
            auto_k_rf_hci_evt_data_ind(status, p_buf, len);
            hci_if_confirm(p_buf);
        }
        break;

    case SINGLE_TONE_EVT_DATA_XMIT:
        {
            os_mem_free(p_buf);
        }
        break;

    case SINGLE_TONE_EVT_ERROR:
        break;

    default:
        break;
    }

    return result;
}


void mp_hci_test_task(void *p_param)
{
    os_delay(100);
    hci_if_open(mp_test_handle_hci_cb);

    while (1)
    {
        os_delay(1000);
    }
}

void mp_hci_task_init(void)
{
    APP_PRINT_INFO0("mp_hci_task_init");
    os_task_create(&mp_hci_test_task_handle, "mp_hci_task", mp_hci_test_task, 0, 256, 1);
}
