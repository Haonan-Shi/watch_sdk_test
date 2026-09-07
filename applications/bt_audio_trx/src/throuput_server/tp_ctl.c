/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "tp_service.h"
#include "os_sched.h"
#include "tp_ctl.h"
#include "gap_conn_le.h"
#include "os_timer.h"
#include "stdlib.h"

T_SERVER_ID tp_svc_id;

void *update_para_len_phy_timer_handle = NULL;

uint16_t g_conn_handle;
uint16_t g_cid;

uint64_t tx_begin_time;
uint64_t tx_end_time;
uint64_t tx_data_count;
uint32_t tx_data_rate;

uint64_t rx_begin_time;
uint64_t rx_end_time;
uint64_t rx_data_count;
uint32_t rx_data_rate;

//test mode : 1 - tx, 2 - rx, 3 - trx
uint8_t test_mode = 0;
//update parameter result : 1 - reject, 2 - accept with no change, 3 - accept with new params
uint8_t para_mode = 0;
//set data length result : 1 - not support, 2 - accept with no change, 3 - accept with new length
uint8_t leng_mode = 0;
//updata 2m PHY result : 1 - 1m, 2 - 2m;
uint8_t phy_mode = 0;

#define TP_TEST_OPCODE_MTU_SIZE_REQ             0x01
#define TP_TEST_OPCODE_MTU_SIZE_RSP             0x02

#define TP_TEST_OPCODE_CONFIG_TX_PARAM_REQ      0x11
#define TP_TEST_OPCODE_CONFIG_RX_PARAM_REQ      0x12
#define TP_TEST_OPCODE_CONFIG_TRX_PARAM_REQ     0x13

#define TP_TEST_OPCODE_CONFIG_PARAM_RSP         0x21
#define TP_TEST_OPCODE_TEST_COMPLETE_IND        0x22

#define TP_TEST_OPCODE_TEST_REPORT_REQ          0x31
#define TP_TEST_OPCODE_TEST_REPORT_RSP          0x32

//Parameters in TX, RX, TRX config req packet
uint16_t pkt_len;
uint16_t pkt_time;
uint16_t int_min;
uint16_t int_max;
uint8_t len_ext;
uint8_t phy_2m;

uint16_t init_val = 0;

enum tx_state
{
    S_TX_READY,
    S_TX_SEND_CONFIG_RSP,
    S_TX_SEND_NOTIFY,
    S_TX_SEND_TEST_END
};

enum rx_state
{
    S_RX_READY,
    S_RX_RECV_WRITE_CMD,
    S_RX_RECV_TEST_END
};

uint8_t tx_state_machine = S_TX_READY;
uint8_t rx_state_machine = S_RX_READY;


void tp_recv_test_complete(uint16_t conn_handle, uint16_t cid)
{
    rx_state_machine = S_RX_RECV_TEST_END;
    rx_end_time = os_sys_time_get();
}

void tp_recv_dt_rx(uint16_t conn_handle, uint16_t cid, uint8_t *p_value, uint16_t length)
{
    if (rx_state_machine == S_RX_READY)
    {
        rx_state_machine = S_RX_RECV_WRITE_CMD;
        rx_begin_time = os_sys_time_get();
    }
    if (rx_state_machine == S_RX_RECV_WRITE_CMD)
    {
        rx_data_count += length;
    }
}

static void tp_send_mtu_size(uint16_t conn_handle, uint16_t cid)
{
    uint8_t rsp[3] = {0};
    T_GAP_CHANN_INFO chann_info;
    if (gap_chann_get_info(conn_handle, cid, &chann_info))
    {
        APP_PRINT_INFO1("tp_send_mtu_size %d", chann_info.mtu_size);
        rsp[0] = TP_TEST_OPCODE_MTU_SIZE_RSP;
        memcpy(&rsp[1], &chann_info.mtu_size, 2);

        vendor_tp_service_indication(conn_handle, cid, tp_svc_id, rsp, sizeof(rsp));
    }
}

void tp_send_test_complete(uint16_t conn_handle, uint16_t cid)
{
    uint8_t rsp[1] = {TP_TEST_OPCODE_TEST_COMPLETE_IND};
    APP_PRINT_INFO0("tp_send_test_complete");
    vendor_tp_service_indication(conn_handle, cid, tp_svc_id, rsp, sizeof(rsp));
}

static uint64_t os_time_get_elapsed(uint64_t begin, uint64_t end)
{
    if (end >= begin)
    {
        return end - begin;
    }
    else
    {
        return ((uint64_t)(0xFFFFFFFFFFFFFFFF) - begin + end);
    }
}

void tp_send_notification(uint16_t conn_handle, uint16_t cid, uint16_t credit)
{
    uint8_t *p_value = malloc(1024);
    uint64_t cur_time = os_sys_time_get();
    uint64_t elapsed_time = os_time_get_elapsed(tx_begin_time, cur_time);
    APP_PRINT_INFO2("tp_send_notification elapsed_time %d ms pkt_time %d 000ms",
                    (uint32_t)elapsed_time & 0xFFFFFFFF, pkt_time);

    if (p_value == NULL)
    {
        APP_PRINT_ERROR0("tp_send_notification error p_value is NULL");
        return;
    }

    if (elapsed_time < pkt_time * 1000 && rx_state_machine != S_RX_RECV_TEST_END)
    {
        while (credit)
        {
            memset(p_value, init_val, pkt_len);
            if (vendor_tp_service_notification(conn_handle, cid, tp_svc_id, p_value, pkt_len))
            {
                tx_data_count += pkt_len;
                init_val++;
                credit--;
            }
            else
            {
                break;
            }
        }
    }
    else
    {
        tx_end_time = os_sys_time_get();
        tx_state_machine = S_TX_SEND_TEST_END;
        tp_send_test_complete(conn_handle, cid);
    }

    if (p_value != NULL)
    {
        free(p_value);
        return;
    }

}

void tp_update_complete(uint16_t conn_handle, uint16_t cid, uint16_t credit)
{
    APP_PRINT_INFO2("tp_update_complete tx_state_machine %d, test_mode %d", tx_state_machine,
                    test_mode);
    if (tx_state_machine == S_TX_SEND_CONFIG_RSP)
    {
        if (test_mode == TEST_MODE_TX || test_mode == TEST_MODE_TRX)
        {
            tx_begin_time = os_sys_time_get();
            tx_state_machine = S_TX_SEND_NOTIFY;
            tp_send_notification(conn_handle, cid, credit);
        }
    }
    if (tx_state_machine == S_TX_SEND_NOTIFY)
    {
        tp_send_notification(conn_handle, cid, credit);
    }
    if (tx_state_machine == S_TX_SEND_TEST_END)
    {
    }
}

static void tp_cal_rst(void)
{
    if (test_mode == TEST_MODE_TX)
    {
        uint64_t elapsed_time = os_time_get_elapsed(tx_begin_time, tx_end_time);
        tx_data_rate = tx_data_count * 1000 / elapsed_time;
    }
    if (test_mode == TEST_MODE_RX)
    {
        uint64_t elapsed_time = os_time_get_elapsed(rx_begin_time, rx_end_time);
        rx_data_rate = rx_data_count * 1000 / elapsed_time;
    }
    if (test_mode == TEST_MODE_TRX)
    {
        uint64_t begin_time = tx_begin_time > rx_begin_time ? tx_begin_time : rx_begin_time;
        uint64_t end_time = tx_end_time > rx_end_time ? rx_end_time : tx_end_time;
        uint64_t elapsed_time = os_time_get_elapsed(begin_time, end_time);
        tx_data_rate = tx_data_count * 1000 / elapsed_time;
        rx_data_rate = rx_data_count * 1000 / elapsed_time;
    }
    tp_data_init();
}

void tp_data_init(void)
{
    //reset global params;
    tx_begin_time = 0;
    tx_end_time = 0;
    tx_data_count = 0;

    rx_begin_time = 0;
    rx_end_time = 0;
    rx_data_count = 0;

    tx_state_machine = S_TX_READY;
    rx_state_machine = S_RX_READY;
}

static void tp_send_test_report(uint16_t conn_handle, uint16_t cid)
{
    tp_cal_rst();

    uint8_t rsp[9];
    rsp[0] = TP_TEST_OPCODE_TEST_REPORT_RSP;
    memcpy(rsp + 1, &tx_data_rate, 4);
    memcpy(rsp + 5, &rx_data_rate, 4);

//    data_uart_print("tx rate (bytes/s) %d\r\n", tx_data_rate);
//    data_uart_print("rx rate (bytes/s) %d\r\n", rx_data_rate);

    APP_PRINT_INFO2("tp_send_test_report tx rate %d(bytes/s) rx rate %d(bytes/s)",
                    tx_data_rate, rx_data_rate);

    vendor_tp_service_indication(conn_handle, cid, tp_svc_id, rsp, sizeof(rsp));
}


static void tp_send_config_rsp(uint16_t conn_handle, uint16_t cid, bool para_accp, uint8_t len_res,
                               uint8_t phy_res)
{
    APP_PRINT_INFO4("tp_send_config_rsp conn_handle %d, para_accp %d, len_res %d, phy_res %d",
                    conn_handle, para_accp, len_res, phy_res);

    uint8_t rsp[5] = {TP_TEST_OPCODE_CONFIG_PARAM_RSP, 0x00, 0x0C, 0x00, 0x03};
    rsp[0] = TP_TEST_OPCODE_CONFIG_PARAM_RSP;

    uint8_t interval[2];
    uint8_t conn_id;
    T_GAP_CHANN_INFO p_info;

    gap_chann_get_info(conn_handle, cid, &p_info);

    if ((p_info.chann_type == GAP_CHANN_TYPE_LE_ATT) ||
        (p_info.chann_type == GAP_CHANN_TYPE_LE_ECFC))
    {
        le_get_conn_id_by_handle(conn_handle, &conn_id);
        le_get_conn_param(GAP_PARAM_CONN_INTERVAL, interval, conn_id);
        rsp[2] = interval[0];
        rsp[3] = interval[1];
        rsp[4] = len_res | ((phy_res - 1) << 1);

        if (para_accp == false || len_res != len_ext || phy_res != phy_2m)
        {
            test_mode = TEST_MODE_INVALID;
            rsp[1] = 1;
        }
        else
        {
            rsp[1] = 0;
        }
    }
    else
    {

    }

    tx_state_machine = S_TX_SEND_CONFIG_RSP;
    vendor_tp_service_indication(conn_handle, cid, tp_svc_id, rsp, sizeof(rsp));
}

void tp_update_conn_para_callback(uint16_t conn_handle, uint16_t cid)
{
    bool para_accp = false;
    uint8_t len_res = 0;
    uint8_t phy_res = 0;

    os_timer_stop(&update_para_len_phy_timer_handle);

    if (para_mode == UPDATE_PARAM_RESULT_REJECT)
    {
        para_accp = false;
    }
    else
    {
        uint8_t interval[2];
        uint8_t conn_id;
        T_GAP_CHANN_INFO p_info;

        gap_chann_get_info(conn_handle, cid, &p_info);

        if ((p_info.chann_type == GAP_CHANN_TYPE_LE_ATT) ||
            (p_info.chann_type == GAP_CHANN_TYPE_LE_ECFC))
        {
            le_get_conn_id_by_handle(conn_handle, &conn_id);
            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, interval, conn_id);
            uint16_t conn_interval = interval[1] << 8 | interval[0];
            if (conn_interval >= int_min && conn_interval <= int_max)
            {
                para_accp = true;
            }
            else
            {
                para_accp = false;
            }
        }
    }

    if (leng_mode == UPDATE_DATA_LEN_RESULT_NOT_SUPPORT)
    {
        len_res = UPDATE_DATA_LEN_RESULT_INVALID;
    }
    else
    {
        len_res = len_ext;
    }

    if (phy_mode == UPDATE_PHY_RESULT_1M)
    {
        phy_res = UPDATE_PHY_RESULT_1M;
    }
    else
    {
        phy_res = UPDATE_PHY_RESULT_2M;
    }

    APP_PRINT_INFO1("tp_update_conn_para_callback para_accp %d", para_accp);
    tp_send_config_rsp(conn_handle, cid, para_accp, len_res, phy_res);
}

void tp_update_para_len_phy_timeout_handler(void *p_handle)
{
    APP_PRINT_INFO0("tp_update_para_len_phy_timeout_handler");
    tp_update_conn_para_callback(g_conn_handle, g_cid);
}

static void tp_recv_params_config(uint8_t opcode, uint16_t conn_handle, uint16_t cid,
                                  uint8_t *p_value,
                                  uint16_t length)
{
    pkt_len = p_value[2] << 8 | p_value[1];
    pkt_time = p_value[4] << 8 | p_value[3];
    int_min = p_value[6] << 8 | p_value[5];
    int_max = p_value[8] << 8 | p_value[7];
    len_ext = p_value[9] & 0x01;
    phy_2m = ((p_value[9] & 0x02) >> 1) + 1;

    APP_PRINT_INFO7("tp_recv_params_config opcode %d, pkt_len %d, pkt_time %d, int_min %d, int_max %d, len_ext %d, phy_2m %d",
                    opcode, pkt_len, pkt_time, int_min, int_max, len_ext, phy_2m);
#if 0
    data_uart_print("opcode %d, pkt_len %d, pkt_time %d, int_min %d, int_max %d, len_ext %d, phy_2m %d\r\n",
                    opcode, pkt_len, pkt_time, int_min, int_max, len_ext, phy_2m);
#endif

    tx_data_rate = 0;
    rx_data_rate = 0;

    uint8_t conn_id;
    T_GAP_CHANN_INFO p_info;

    gap_chann_get_info(conn_handle, cid, &p_info);

    g_conn_handle = conn_handle;
    g_cid = cid;

    if ((p_info.chann_type == GAP_CHANN_TYPE_LE_ATT) ||
        (p_info.chann_type == GAP_CHANN_TYPE_LE_ECFC))
    {
        le_get_conn_id_by_handle(conn_handle, &conn_id);
        le_update_conn_param(conn_id, int_min, int_max, 0, 500, 2 * (int_min - 1), 2 * (int_max - 1));

        uint16_t tx_octs = len_ext ? 251 : 27;
        le_set_data_len(conn_id, tx_octs, 0x0848);

        uint8_t phy = phy_2m == 2 ? GAP_PHYS_PREFER_2M_BIT : GAP_PHYS_PREFER_1M_BIT;
        le_set_phy(conn_id, 0, phy, phy, GAP_PHYS_OPTIONS_CODED_PREFER_NO);

        if (NULL == update_para_len_phy_timer_handle)
        {
            os_timer_create(&update_para_len_phy_timer_handle, "update_para_len_phy_timer",
                            0, 5000, false, tp_update_para_len_phy_timeout_handler);
        }
        os_timer_start(&update_para_len_phy_timer_handle);
    }
    else
    {
        tp_send_config_rsp(conn_handle, cid, 0, 0, 0);
    }

}

void tp_recv_ct_rx(uint16_t conn_handle, uint16_t cid, uint8_t *p_value, uint16_t length)
{
    uint8_t opcode = p_value[0];
    APP_PRINT_INFO1("tp_recv_ct_rx opcode 0x%x", opcode);
    switch (opcode)
    {
    case TP_TEST_OPCODE_MTU_SIZE_REQ:
        tp_send_mtu_size(conn_handle, cid);
        break;

    case TP_TEST_OPCODE_TEST_COMPLETE_IND:
        tp_recv_test_complete(conn_handle, cid);
        break;

    case TP_TEST_OPCODE_TEST_REPORT_REQ:
        tp_send_test_report(conn_handle, cid);
        break;

    case TP_TEST_OPCODE_CONFIG_TX_PARAM_REQ:
        test_mode = TEST_MODE_TX;
        tp_recv_params_config(opcode, conn_handle, cid, p_value, length);
        break;

    case TP_TEST_OPCODE_CONFIG_RX_PARAM_REQ:
        test_mode = TEST_MODE_RX;
        tp_recv_params_config(opcode, conn_handle, cid, p_value, length);
        break;

    case TP_TEST_OPCODE_CONFIG_TRX_PARAM_REQ:
        test_mode = TEST_MODE_TRX;
        tp_recv_params_config(opcode, conn_handle, cid, p_value, length);
        break;

    default:
        APP_PRINT_WARN1("tp_recv_ct_rx invalid opcode=%#x", opcode);
        break;
    }
}
