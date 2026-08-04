/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "ppg_trans_handle.h"
#include "trace.h"
#include "ppg_flash_handle.h"
#include "wristband_private_service.h"
#include "os_timer.h"
#include "string.h"
#include "time.h"
#include "ppg_rtl87x5.h"
#include "os_queue.h"
#include "communicate_parse_health.h"
#include "os_mem.h"
#include "hub_clock.h"
#include "app_report.h"
#include "hub_task.h"
#include "module_global_data.h"


#define OFFSET_TEMP (7) //SATRT + EVENT len
static uint32_t report_remain_len = 0;
static uint32_t report_total_len = 0;
static uint8_t report_idx = 0xff;
static uint8_t reprot_seq = 0x00;
static uint8_t buff[207];
static T_REPORT_STATE report_state = T_STATE_IDLE;
static uint8_t resend_count = 0;
T_PPG_SEND_DATA ppg_data_send_flag = STOP_FLAG;
T_OS_QUEUE ppg_data_queue;
extern T_SERVER_ID         wristband_gatt_srv_id;
void *report_timer_handle;

void report_timeout_handler(void *p_xtimer)
{
    uint32_t timer_id = 0;

    os_timer_id_get(&p_xtimer, &timer_id);

    APP_PRINT_ERROR1("report_timeout_handler, TimerID(%u)", timer_id);

    switch (timer_id)
    {
    case T_REPORT_TO:
        resend_count++;
        if (resend_count > 3)
        {
            memset(buff, 0, sizeof(buff));
            report_state = T_STATE_IDLE;
            resend_count = 0;
            report_remain_len = 0;
            report_idx = 0xff;
            APP_PRINT_ERROR0("report_timeout_handler, abort");
        }
        else
        {
            ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_GET_STORED_DATA, sizeof(buff), buff);
            os_timer_start(&report_timer_handle);
        }
        break;
    }
}

void app_ppg_trans_report_flash_data(void)
{
    if (report_remain_len == 0)
    {
        return;
    }
    APP_PRINT_TRACE3("Cheat reprt idx %d, seq %d, len %d", report_idx, reprot_seq, report_total_len);
    memset(buff, 0, sizeof(buff));
    buff[0] = report_idx;
    buff[2] = reprot_seq;
    buff[3] = (uint8_t)(report_total_len >> 24);
    buff[4] = (uint8_t)(report_total_len >> 16);
    buff[5] = (uint8_t)(report_total_len >> 8);
    buff[6] = (uint8_t)(report_total_len);

    if (report_remain_len > 200)
    {
        buff[1] = 0x00;
        reprot_seq++;
        ppg_flash_get_data(report_idx, 200, &buff[7]);
        ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_GET_STORED_DATA, sizeof(buff), buff);
        report_remain_len -= 200;
    }
    else
    {
        buff[1] = 0x01;
        ppg_flash_get_data(report_idx, report_remain_len, &buff[7]);
        ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_GET_STORED_DATA, sizeof(buff), buff);
        report_remain_len = 0;
        report_idx = 0xff;
    }
}

void resolve_hrs_setting_command(uint8_t key, const uint8_t *pValue, uint16_t length)
{
    switch (key)
    {
    case KEY_PPG_TRANS_GET_STORED_DATA_NUM:
        {
            if (report_state != T_STATE_IDLE)
            {
                APP_PRINT_TRACE1("Cheat INVALID %d", report_state);
                return;
            }
            uint8_t num = ppg_flash_get_stored_data_num();
            uint8_t data[61];
            data[0] = num;
            uint8_t *ptr = &data[1];
            for (uint8_t i = 0; i < num; i++)
            {
                FLASH_SAVE_IDX temp = ppg_flash_get_flash_saved_time(i);

                *ptr = temp.year - 70; //tool side is start from 1970 our is from 1900
                ptr++;

                *ptr = temp.mon;
                ptr++;

                *ptr = temp.day;
                ptr++;

                *ptr = temp.hr;
                ptr++;

                *ptr = temp.min;
                ptr++;

                *ptr = temp.sec;
                ptr++;
            }
            uint8_t len = (num * 6 + 1);
            APP_PRINT_TRACE1("Cheat reply %b", TRACE_BINARY(len, data));
            ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_GET_STORED_DATA_NUM, len, data);
        }
        break;

    case KEY_PPG_TRANS_EARASE_ALL_DATA:
        {
            {
                uint8_t num = 0;
                if (report_state == T_STATE_IDLE)
                {
                    num = 1;
                    ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_EARASE_ALL_DATA, 1, &num);

                    T_IO_MSG hrs_msg;
                    hrs_msg.type = HUB_MSG_PPG_TRANS;
                    hrs_msg.subtype = PPG_TRANS_FLASH_ERASE;

                    send_msg_to_hub_task(&hrs_msg, __LINE__);

                }
                else
                {
                    ppg_send_data(0, EVENT_TRANS_FLASH_DATA_CHECK, KEY_PPG_TRANS_EARASE_ALL_DATA, 1, &num);
                }
            }
        }
        break;

    case KEY_PPG_TRANS_GET_STORED_DATA:
        {
            APP_PRINT_TRACE1("Cheat get all data %b", TRACE_BINARY(length, pValue));
            //get from ftl
            if (length == 0)
            {
                return;
            }
            if (pValue[0] == 0x00)//start
            {
                if (report_state != T_STATE_IDLE)
                {
                    APP_PRINT_TRACE1("Cheat INVALID %d", report_state);
                    return;
                }

                if (pValue[1] < ppg_flash_get_stored_data_num())
                {
                    FLASH_SAVE_IDX temp = ppg_flash_read_data_init(pValue[1]);

                    if (temp.idx >= 10)
                    {
                        APP_PRINT_TRACE0("Cheat INVALID");
                        return;
                    }
                    memset(buff, 0, sizeof(buff));
                    report_state = T_STATE_BUSY;
                    reprot_seq = 0;
                    report_remain_len = temp.len;
                    report_total_len = temp.len;
                    report_idx = temp.idx;
                    app_ppg_trans_report_flash_data();
                    resend_count = 0;
                    os_timer_start(&report_timer_handle);
                }
            }
            else if (pValue[0] == 0x01) //next
            {
                if (report_state == T_STATE_IDLE)
                {
                    return;
                }

                if (pValue[1] == buff[2])
                {
                    os_timer_stop(&report_timer_handle);
                    if (pValue[2] == 0)
                    {
                        app_ppg_trans_report_flash_data();
                        resend_count = 0;
                        os_timer_start(&report_timer_handle);
                    }
                    else if (pValue[2] == 1)
                    {
                        report_state = T_STATE_IDLE;
                    }
                }
            }
        }
        break;

    case KEY_PPG_TRANS_SET_WATCH_TIME:
        {
            time_union_t time;

            time.data = 0;
            time.time.year = pValue[0] + 70;
            time.time.month = pValue[1];
            time.time.day = pValue[2];
            time.time.hours = pValue[3];
            time.time.minute = pValue[4];
            time.time.seconds = pValue[5];

            uint32_t old_sec = RtkWristbandSys.SecondCountRTC;
            set_wristband_clock(time);
            uint32_t new_sec = RtkWristbandSys.SecondCountRTC;
            system_clock_init(RtkWristbandSys.SecondCountRTC);
            APP_PRINT_INFO3("SET TIME, Date: %d-%d-%d", RtkWristbandSys.Global_Time.tm_year,
                            RtkWristbandSys.Global_Time.tm_mon, RtkWristbandSys.Global_Time.tm_mday);
            APP_PRINT_INFO3("SET TIME, Time: %d-%d-%d", RtkWristbandSys.Global_Time.tm_hour,
                            RtkWristbandSys.Global_Time.tm_min, RtkWristbandSys.Global_Time.tm_sec);
        }
        break;

    case KEY_PPG_TRANS_START_PPG_01:
        {
            hrs_power_start_01();
        }
        break;

    case KEY_PPG_TRANS_TRANS_PPG_DATA:
        {
            ppg_data_send_flag = START_FLAG;
            os_queue_init(&ppg_data_queue);
            APP_PRINT_INFO0("PPG_TRANS_on_time_trans_data_start");
        }
        break;


    case KEY_PPG_TRANS_TRANS_PPG_DATA_STOP:
        {
            ppg_data_send_flag = STOP_FLAG;

            T_PPG_DATA_HEADER *da_pkt;
            da_pkt = ppg_data_peek(0);
            if (da_pkt != NULL)
            {
                ppg_data_flush(ppg_data_queue.count);
            }
            APP_PRINT_INFO0("PPG_TRANS_on_time_trans_data_stop");
        }
        break;

    default:
        break;
    }
}


void ppg_trans_event_handler(T_IO_MSG msg)
{
    APP_PRINT_INFO1("ppg_trans_event_handler msg.subtype 0x%4x", msg.subtype);
    switch (msg.subtype)
    {
    case PPG_TRANS_FLASH_ERASE:
        {
            ppg_flash_erase_data();
            break;
        }

    default:
        {
            APP_PRINT_INFO2("file = %s, line = %d", TRACE_STRING(__FILE__), __LINE__);
            break;
        }
    }
}


void *ppg_data_peek(int offset)
{
    void *p_ppg_data_dspkt = os_queue_peek(&ppg_data_queue, offset);;
    return p_ppg_data_dspkt;
}

uint8_t ppg_data_flush(uint16_t cnt)
{
    T_PPG_DATA_HEADER *p_ppg_data_head = NULL;
    if (cnt > ppg_data_queue.count)
    {
        cnt = ppg_data_queue.count;
    }

    for (uint16_t i = 0; i < cnt; i++)
    {
        p_ppg_data_head = (T_PPG_DATA_HEADER *)os_queue_out(&ppg_data_queue);

        if (p_ppg_data_head)
        {
            os_mem_free(p_ppg_data_head);
        }
    }

    return 0;
}

bool ppg_data_in(uint8_t *packet_pt, uint16_t packet_length, uint16_t frame_num)
{
    T_PPG_DATA_HEADER ppg_head;
    T_PPG_DATA_HEADER *ppg_data_packet = NULL;

    ppg_head.p_next = NULL;
    ppg_head.payload_length = packet_length;
    ppg_head.frame_num = frame_num;

    ppg_data_packet = (T_PPG_DATA_HEADER *)os_mem_alloc(RAM_TYPE_DATA_ON,
                                                        sizeof(T_PPG_DATA_HEADER) + packet_length);
    if (ppg_data_packet == NULL)
    {
        APP_PRINT_ERROR0("ppg data queue buf malloc fail!");
        return false;
    }

    memcpy(ppg_data_packet, &ppg_head, sizeof(T_PPG_DATA_HEADER));
    memcpy((uint8_t *)ppg_data_packet + sizeof(T_PPG_DATA_HEADER), packet_pt, packet_length);
    os_queue_in(&ppg_data_queue, ppg_data_packet);

    return true;
}

void ppg_send_data(uint8_t conn_id, uint16_t event_id, uint8_t cmd_type,
                   uint16_t len, uint8_t *data)
{
    uint8_t *p_buffer = NULL;
    uint16_t mtu_size;
    uint16_t remain_size = len;
    uint8_t *p_data = data;
    uint16_t send_len;
    uint8_t credit;


    if ((data == NULL) || (len == 0))
    {
        return;
    }

    le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size, conn_id);
    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &credit);
    APP_PRINT_TRACE2("app_wristband_service_prepare_send_notify mtu_size:%d, len:0x%x", mtu_size, len);

    uint8_t mtu_size_send = mtu_size - 3;
    p_buffer = os_mem_alloc(RAM_TYPE_DATA_ON, mtu_size_send);
    if (p_buffer == NULL)
    {
        return;
    }
    memset(p_buffer, 0, mtu_size_send);

    while (remain_size)
    {
        send_len = (remain_size > (mtu_size_send - OFFSET_TEMP)) ? (mtu_size_send - OFFSET_TEMP) :
                   remain_size;
        p_buffer[0] = 0xab;
        p_buffer[1] = 0xba;
        p_buffer[2] = 0x05;
        p_buffer[3] = 0x00;
        p_buffer[4] = cmd_type;
        p_buffer[5] = (uint8_t)(send_len >> 8);
        p_buffer[6] = (uint8_t) send_len;

        memcpy(&p_buffer[OFFSET_TEMP], p_data, send_len);

        if (!ppg_data_in(p_buffer, send_len + OFFSET_TEMP, data[8]))
        {
            break;
        }

        p_data += send_len;
        remain_size -= send_len;
    }
    os_mem_free(p_buffer);

    while (ppg_data_queue.count && credit)
    {
        T_PPG_DATA_HEADER *da_pkt = NULL;
        da_pkt = ppg_data_peek(0);
        if (da_pkt == NULL)
        {
            break;
        }
        uint16_t cid;
        uint8_t cid_num;
        uint16_t conn_handle = le_get_conn_handle(RtkWristbandSys.wristband_conn_id);
        gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
        gatt_svc_send_data(conn_handle, cid, wristband_gatt_srv_id, GATT_SRV_BWPS_RX_INDEX,
                           da_pkt->p_data, da_pkt->payload_length, GATT_PDU_TYPE_NOTIFICATION);
        ppg_data_flush(1);
        credit--;
    }

    if (ppg_data_queue.count && (credit == 0))
    {
        APP_PRINT_INFO0("PPG send data credits not enough");
    }
}


void ppg_trans_init(void)
{
    APP_PRINT_INFO0("app_ppg_trans_init");
    os_timer_create(&report_timer_handle, "reportTimer", T_REPORT_TO,
                    600, false, report_timeout_handler);
}
