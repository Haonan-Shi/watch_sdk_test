/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include <string.h>
#include <bt_types.h>
#include "trace.h"
#include "app_main.h"
#include "app_cmd.h"
#include "app_mmi.h"
#include "app_cfg.h"
#include "app_timer.h"
#include "os_msg.h"
#include "os_task.h"
#include "rtl876x.h"
#include "app_report.h"
#include "stdlib.h"
#include "patch_header_check.h"
#include "app_playback_update_file.h"
#include "app_fs_if.h"
#include "ff.h"
#include "gap_conn_le.h"
#include "gap.h"
#include "playback_service.h"
#include "audio_playback.h"
#include "ble_conn.h"
#include "playback_playlist.h"
#include "os_sync.h"
#include "module_font.h"
#include "app_module_init.h"

#define PLAYBACK_SD_ACTIVE_TIME_S               60
#define OFFSET_TEMP (0 + 2) //START + EVENT len
/** @defgroup  APP_PLAYBACK_SERVICE APP PLAYBACK handle
    * @brief APP PLAYBACK Service to implement PLAYBACK feature
    * @{
    */

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup APP_PLAYBACK_Exported_Variables APP PLAYBACK Exported Variables
    * @brief
    * @{
    */
static PB_TRANS_FUNCTION_STRUCT pb_trans_struct;

T_FILE_HANDLE *playback_write_fs_handle = NULL;

/** @brief  PLAYBACK timer callback */
typedef enum
{
    APP_TIMER_PB_TRANS_FILE,
    APP_TIMER_PB_TRANS_UP,
    APP_TIMER_PB_TRANS_SD_ACTIVE,
} T_APP_TIMER_PLAYBACK_TRANS;

static uint8_t timer_idx_pb_trans_up = 0;
static uint8_t timer_idx_pb_trans_file = 0;
static uint8_t timer_idx_pb_trans_sd_active = 0;
static uint8_t playback_trans_timer_id = 0;

extern void *flash_mutex_handle;
/** End of APP_PLAYBACK_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Private Functions
 *============================================================================*/
/** @defgroup APP_PLAYBACK_Exported_Functions APP PLAYBACK service Exported Functions
    * @brief
    * @{
    */
static void app_playback_trans_sd_active_start_timer(uint8_t time_s);
static uint8_t app_playback_trans_stop_and_del_file(void);

/**
    * @brief    Wrapper function to send notification to peer
    * @note
    * @param    conn_id     ID to identify the connection (LE only, 0xFF for BR/EDR)
    * @param    event_id    Notification event ID
    * @param    len         Notification data length
    * @param    data        Additional notification data
    * @return   void
    */
static void app_playback_service_prepare_send_notify(uint8_t conn_id, uint16_t event_id,
                                                     uint16_t len, uint8_t *data)
{
    uint8_t *p_buffer = NULL;
    uint16_t mtu_size;
    uint16_t remain_size = len;
    uint8_t *p_data = data;
    uint16_t send_len;

    if ((data == NULL) || (len == 0))
    {
        return;
    }

    // Get MTU based on channel type
    if ((pb_trans_struct.chann_type == GAP_CHANN_TYPE_LE_ATT) ||
        (pb_trans_struct.chann_type == GAP_CHANN_TYPE_LE_ECFC))
    {
        le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size, conn_id);
    }
    else if ((pb_trans_struct.chann_type == GAP_CHANN_TYPE_BREDR_ATT) ||
             (pb_trans_struct.chann_type == GAP_CHANN_TYPE_BREDR_ECFC))
    {
        // BR/EDR GATT uses fixed MTU
        mtu_size = 512;
    }
    else
    {
        // Default MTU for unknown channel type
        mtu_size = BLE_PLAYBACK_MTU_SIZE;
    }

    APP_PRINT_TRACE2("app_playback_service_prepare_send_notify mtu_size:%d, len:0x%x", mtu_size, len);
    p_buffer = malloc(mtu_size);
    if (p_buffer == NULL)
    {
        return;
    }
    memset(p_buffer, 0, mtu_size);

    uint16_t cid = pb_trans_struct.cid;
    uint8_t cid_num;
    uint16_t conn_handle = pb_trans_struct.conn_handle;

    // For BR/EDR, need to get cid from conn_handle
    if ((pb_trans_struct.chann_type == GAP_CHANN_TYPE_BREDR_ATT) ||
        (pb_trans_struct.chann_type == GAP_CHANN_TYPE_BREDR_ECFC))
    {
        gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);
    }

    if (len < mtu_size - OFFSET_TEMP)
    {
        LE_UINT16_TO_ARRAY((uint8_t *)&p_buffer[0], event_id);
        memcpy(&p_buffer[OFFSET_TEMP], data, len);
        playback_service_send_notification(conn_handle, cid, p_buffer, len + OFFSET_TEMP);
        free(p_buffer);
        return;
    }

    while (remain_size)
    {
        if (remain_size == len)
        {
            LE_UINT16_TO_ARRAY((uint8_t *)&p_buffer[0], event_id);
            memcpy(&p_buffer[OFFSET_TEMP], p_data, mtu_size - OFFSET_TEMP);
            playback_service_send_notification(conn_handle, cid, p_buffer, mtu_size);
            p_data += (mtu_size - OFFSET_TEMP);
            remain_size -= (mtu_size - OFFSET_TEMP);
            continue;
        }

        send_len = (remain_size > mtu_size) ? mtu_size : remain_size;
        memcpy(p_buffer, p_data, send_len);
        playback_service_send_notification(conn_handle, cid, p_buffer, send_len);
        p_data += send_len;
        remain_size -= send_len;
    }

    free(p_buffer);
}

static void app_playback_trans_write_result_ack(uint8_t wr_result)
{
    APP_PRINT_TRACE1("app_playback_trans_write_result_ack: wr_result 0x%x", wr_result);

    if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
    {
        app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_REPORT_BUFFER_CHECK, pb_trans_struct.id.spp_idx,
                         &wr_result, sizeof(wr_result));
    }
    else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
    {
        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                 EVENT_PLAYBACK_REPORT_BUFFER_CHECK, sizeof(wr_result), &wr_result);
    }
    if (wr_result != PB_TRANS_RET_SUCCESS)
    {
        app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);
    }
}

static void app_playback_write_data_to_sd(void)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    uint16_t write_res = 0;
    uint32_t wrd_len;

    APP_PRINT_TRACE2("app_playback_write_data_to_sd: write flash, cur_offset: 0x%x, is_trans_pb_process: %d",
                     pb_trans_struct.cur_offset, pb_trans_struct.is_trans_pb_process);

    if (pb_trans_struct.is_trans_pb_process == false)
    {
        results = PB_TRANS_RET_OPERATION_ERROR;
    }
    else
    {
        write_res = app_fs_write(playback_write_fs_handle,
                                 pb_trans_struct.p_temp_buf_head,
                                 pb_trans_struct.temp_buf_used_size);
        if (write_res == pb_trans_struct.temp_buf_used_size)
        {
            pb_trans_struct.cur_offset += pb_trans_struct.temp_buf_used_size;
            pb_trans_struct.temp_buf_used_size = 0;
            pb_trans_struct.buf_is_full = false;
            results = PB_TRANS_RET_BUF_SUCCESS;
        }
        else
        {
            results = PB_TRANS_RET_WRITE_ERROR;
        }
    }

    APP_PRINT_TRACE2("app_playback_write_data_to_sd, write_result: %d, results: 0x%x", write_res,
                     results);
    app_playback_trans_write_result_ack(results);
}

static void app_playback_trans_set_start_status(void)
{
    APP_PRINT_INFO0("transfer started");
    app_db.transfer_status = TRANSFER_STARTED;
    app_mmi_handle_action(MMI_AV_STOP);
    if (g_curr_song != NULL)
    {
        Mp3_FreeHandle(g_curr_song);
        g_curr_song = NULL;
    }
    bt_sniff_mode_disable(pb_trans_struct.bd_addr);
}

static void app_playback_trans_set_stop_status(T_PB_TRANS_TERMINATION_REASON reason)
{
    app_db.transfer_status = TRANSFER_STOPPED;
    bt_sniff_mode_enable(pb_trans_struct.bd_addr, 500, 500, 0, 0);
    if (reason == PB_TRANS_NORMAL)
    {
        if (flash_mutex_handle)
        {
            os_mutex_take(flash_mutex_handle, 0xffffffff);
            playback_sync_playlist();
            os_mutex_give(flash_mutex_handle);
        }
        else
        {
            playback_sync_playlist();
        }
        playback_reset_cur_play_index();
    }
}

/**
    * @brief  Reset local variables
    * @return void
    */
static void app_playback_trans_clear_local(void)
{
    APP_PRINT_TRACE0("app_playback_trans_clear_local");
    pb_trans_struct.file_total_length = 0;
    pb_trans_struct.file_id = 0;
    pb_trans_struct.cur_offset = 0;
    pb_trans_struct.send_cnt = 0;
    pb_trans_struct.file_list_type = PLAYBACK_HEAD_BIN;
    pb_trans_struct.is_trans_pb_process = false;
    pb_trans_struct.local_seq = 0;
    if (pb_trans_struct.p_temp_buf_head != NULL)
    {
        free(pb_trans_struct.p_temp_buf_head);
        pb_trans_struct.p_temp_buf_head = NULL;
    }
    pb_trans_struct.buf_is_full = false;
    pb_trans_struct.temp_buf_used_size = 0;

    app_stop_timer(&timer_idx_pb_trans_up); // ble use
    app_stop_timer(&timer_idx_pb_trans_file);

    if (playback_write_fs_handle != NULL)
    {
        uint16_t res = app_fs_close_file(playback_write_fs_handle);
        if (res != 0)
        {
            res = PB_TRANS_RET_CLOSE_ERROR;
            APP_PRINT_ERROR1("app_playback_trans_clear_local, app_fs_close_file result: %d", res);
        }
        playback_write_fs_handle = NULL;
    }
}

/**
    * @brief  Reset local variables and set stop status
    * @return void
    */
static void app_playback_trans_error_clear_local(void)
{
    app_playback_trans_clear_local();
    app_playback_trans_set_stop_status(PB_TRANS_CANCEL);
}

/**
    * @brief    Used to get device information
    * @param    p_data    point of device info data
    * @return   void
    */
static void app_playback_trans_get_device_info(PLAYBACK_DEVICE_INFO *p_deviceinfo)
{
    if (p_deviceinfo == NULL)
    {
        return;
    }
    app_playback_trans_clear_local();
    memset(p_deviceinfo, 0, sizeof(PLAYBACK_DEVICE_INFO));
    p_deviceinfo->ic_type = IC_TYPE;
    p_deviceinfo->pkt_size          = PLAYBACK_PKT_SIZE;
    p_deviceinfo->buf_check_size    = PLAYBACK_BUF_CHECK_SIZE;
    p_deviceinfo->mode              = 0x00;
    if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
    {
        p_deviceinfo->pkt_size          = PLAYBACK_PKT_SIZE;
        p_deviceinfo->protocol_ver      = PLAYBACK_PROTOCOL_VERSION;
    }
    else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
    {
        p_deviceinfo->pkt_size      = BLE_PLAYBACK_MTU_SIZE;
        p_deviceinfo->protocol_ver  = BLE_PLAYBACK_VERSION;
    }
    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_PRIMARY)
    {
        p_deviceinfo->mode |= PLAYBACK_MODE_COUPLE | PLAYBACK_MODE_COUPLE_PRI;
    }
    else if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SECONDARY)
    {
        p_deviceinfo->mode |= PLAYBACK_MODE_COUPLE | PLAYBACK_MODE_COUPLE_SEC;
    }
    else if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
    {
        p_deviceinfo->mode |= PLAYBACK_MODE_SINGLE;
    }
    p_deviceinfo->song_format_type = PB_FORMAT_TYPE_DEFAULT | PB_FORMAT_TYPE_MP3;
}

/**
    * @brief    Used to send playback file list and play list to host
    * @param    p       point of rx cmd
    * @param    p_data    [out] point of file one packet
    * @return   result  0x01:success others:fail
    */
static uint8_t app_playback_trans_list_data_info(uint8_t *p, uint8_t *p_data)
{
    uint8_t res = PB_TRANS_RET_SUCCESS;
    uint8_t file_type = *p;
    uint16_t file_crc;

    app_playback_trans_clear_local();

    pb_trans_struct.file_list_type = file_type;
    if (PLAYBACK_NAME_BIN == file_type)
    {
        pb_trans_struct.file_total_length = app_fs_get_name_bin_size();
        file_crc = 0x1234;
    }
    else if (PLAYBACK_HEAD_BIN == file_type)
    {
        pb_trans_struct.file_total_length = app_fs_get_header_bin_size();
        file_crc = 0x5678;
    }
    else
    {
        res = PB_TRANS_RET_READ_ERROR;
        return res;
    }

    APP_PRINT_TRACE2("app_playback_trans_list_data_info:crc:0x%x,file_total_length:0x%x", file_crc,
                     pb_trans_struct.file_total_length);
    // send start file_crc and length
    p_data[0] = PLAYBACK_TRANS_START;
    LE_UINT16_TO_ARRAY((uint8_t *)&p_data[1], file_crc);
    LE_UINT32_TO_ARRAY((uint8_t *)&p_data[3], pb_trans_struct.file_total_length);

    if (pb_trans_struct.p_temp_buf_head != NULL)
    {
        free(pb_trans_struct.p_temp_buf_head);
        pb_trans_struct.p_temp_buf_head = NULL;
    }

    pb_trans_struct.p_temp_buf_head = (uint8_t *)malloc(PLAYBACK_PKT_SIZE);
    if (pb_trans_struct.p_temp_buf_head == NULL)
    {
        res = PB_TRANS_RET_OPERATION_ERROR;
    }
    APP_PRINT_TRACE1("app_playback_trans_list_data_info, res:0x%x", res);
    return res;
}

uint8_t app_playback_trans_set_scenario(uint8_t *p_data)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    uint16_t file_length;
    uint16_t trans_scenario;
    T_FILE_FORMAT trans_file_suffix;

    LE_ARRAY_TO_UINT16(file_length, p_data);
    LE_ARRAY_TO_UINT16(trans_scenario, p_data + 2);
    LE_ARRAY_TO_UINT8(trans_file_suffix, p_data + 4);

    APP_PRINT_TRACE3("app_playback_trans_set_scenario,"\
                     "file_length:0x%x, trans_scenario:0x%x, trans_file_suffix:0x%x",
                     file_length, trans_scenario, trans_file_suffix);
    // switch trans_scenario
    return results;
}

/**
    * @brief    get songs info for playback and create file.
    * @param    *p_data   point of input data
    * @return   void
    */
static uint8_t app_playback_trans_get_create_file_handle(uint8_t *p_data)
{
    uint8_t res = PB_TRANS_RET_SUCCESS;

    uint32_t file_total_len;
    uint16_t file_name_len;
    uint8_t len_offset = 0;
    uint32_t free_space;
    uint16_t result = app_fs_free_space(&free_space);
    LE_ARRAY_TO_UINT16(file_name_len, p_data);
    uint8_t *file_name = p_data + 2;
    // total file length
    len_offset = 2 + file_name_len;

    LE_ARRAY_TO_UINT32(file_total_len, (uint8_t *)(p_data + len_offset));
    if (!result)
    {
        if (free_space < file_total_len)
        {
            res = PB_TRANS_RET_WRITE_ERROR;
            APP_PRINT_ERROR2("The sd free space is insufficient: file_total_len:0x%x, free_space:0x%x",
                             file_total_len, free_space);
            return res;
        }
    }

    app_playback_trans_clear_local();

    APP_PRINT_TRACE3("app_playback_trans_get_create_file_handle,"\
                     "file_name_len:0x%x, file_total_len:0x%x,filename:%b",
                     file_name_len, file_total_len, TRACE_BINARY(file_name_len, file_name));
    playback_write_fs_handle = app_fs_open_file(file_name,
                                                FS_O_CREATE | FS_O_WRITE);
    if (playback_write_fs_handle == NULL)
    {
        res = PB_TRANS_RET_CREAT_ERROR;
        APP_PRINT_ERROR1("app_playback_trans_get_create_file_handle creat FAIL,res:0x%x", res);
        return res;
    }
    result = fs_file_add_to_header_name_bin(playback_write_fs_handle, &scan_hdl);
    if (result != 0)
    {
        APP_PRINT_ERROR0("app_playback_trans_valid_handle audio_fs_add_file fail");
    }
//    res = PB_TRANS_RET_SUCCESS;
//    APP_PRINT_TRACE6("end mem, data on %d, data off %d, buf on %d, buf off %d, comm on %d, comm off %d",
//                    os_mem_peek(RAM_TYPE_DATA_ON), os_mem_peek(RAM_TYPE_DATA_OFF),
//                    os_mem_peek(RAM_TYPE_BUFFER_ON), os_mem_peek(RAM_TYPE_BUFFER_OFF),
//                    os_mem_peek(RAM_TYPE_COMMON_ON), os_mem_peek(RAM_TYPE_COMMON_OFF));
    if (pb_trans_struct.p_temp_buf_head != NULL)
    {
        free(pb_trans_struct.p_temp_buf_head);
        pb_trans_struct.p_temp_buf_head = NULL;
    }

    pb_trans_struct.p_temp_buf_head = (uint8_t *)malloc(PLAYBACK_BUF_CHECK_SIZE);
    if (pb_trans_struct.p_temp_buf_head == NULL)
    {
        res = PB_TRANS_RET_OPERATION_ERROR;
    }
    pb_trans_struct.file_total_length = file_total_len;
    pb_trans_struct.cur_offset = 0;
    pb_trans_struct.temp_buf_used_size = 0;
    pb_trans_struct.is_trans_pb_process = true;
    APP_PRINT_TRACE1("app_playback_trans_get_create_file_handle, res:0x%x", res);
    return res;

}

/**
 * @brief  get 16bit data swapped.
 *
 * @param  val          16bit data to be swapped.
 * @return value after being swapped.
*/
static uint16_t swap_16(uint16_t val)
{
    uint16_t result;

    /* Idiom Recognition for REV16 */
    result = ((val & 0xff) << 8) | ((val & 0xff00) >> 8);

    return result;
}

/**
* @brief calculate checksum of lenth of buffer.
*
* @param  offset             offset of the image.
* @param  length             length of data.
* @param  crcValue          ret crc value point.
* @return  0 if buffer checksum calcs successfully, error line number otherwise
*/
static uint32_t app_playback_checkbufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal)
{
    uint16_t checksum16 = 0;
    uint32_t result = 0;
    uint32_t i;
    uint16_t *p16;

    p16 = (uint16_t *)buf;
    for (i = 0; i < length / 2; ++i)
    {
        checksum16 = checksum16 ^ (*p16);
        ++p16;
    }

    checksum16 = swap_16(checksum16);
    if (checksum16 != mCrcVal)
    {
        result =  __LINE__;
        goto L_Return;
    }
    return result;

L_Return:
    APP_PRINT_ERROR3("<====playback_checkbufcrc :checksum16 0x%x, mCrcVal 0x%x result:%d",
                     checksum16, mCrcVal, result);
    return result;
}

/**
    * @brief    Handle written request on packet
    * @param    p_data     data to be written
    * @param    length     Length of value to be written
    * @return   handle result  0x01:success other:fail
    */
static uint8_t app_playback_trans_data_packet_handle(uint8_t *p_data, uint16_t length)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    uint16_t host_sequence;
    uint16_t host_pkt_crc;
    uint32_t file_offset;
    uint16_t pkt_len;
    uint8_t *p_file_data;

    LE_ARRAY_TO_UINT16(host_sequence, (p_data + 0));
    LE_ARRAY_TO_UINT16(host_pkt_crc, (p_data + 2));
    LE_ARRAY_TO_UINT32(file_offset, (p_data + 4));
    LE_ARRAY_TO_UINT16(pkt_len, (p_data + 8));
    p_file_data = p_data + 10;
    APP_PRINT_TRACE5("app_playback_trans_data_packet_handle:local_sequence:0x%x, "
                     "host_sequence 0x%x, host_pkt_crc:0x%x, file_offset=0x%x, pkt_len =%d",
                     pb_trans_struct.local_seq, host_sequence, host_pkt_crc, file_offset, pkt_len);

    if (pb_trans_struct.is_trans_pb_process == false)
    {
        results = PB_TRANS_RET_OPERATION_ERROR;
        goto L_Return;
    }

    if (pb_trans_struct.buf_is_full)
    {
        results = PB_TRANS_RET_BUF_FULL_ERROR;
        goto L_Return;
    }

    if (host_sequence != pb_trans_struct.local_seq)
    {
        results = PB_TRANS_RET_SEQUENCE_ERROR;
        goto L_Return;
    }
    pb_trans_struct.local_seq++;

    APP_PRINT_TRACE4("app_playback_trans_data_packet_handle:length:0x%x, cur_offset:0x%x, "
                     "buf_used_size :0x%x, file_total_length: 0x%x",
                     length, pb_trans_struct.cur_offset,
                     pb_trans_struct.temp_buf_used_size,
                     pb_trans_struct.file_total_length);
    if (file_offset != (pb_trans_struct.cur_offset + pb_trans_struct.temp_buf_used_size))

    {
        results = PB_TRANS_RET_OFFSET_ERROR;
        goto L_Return;
    }

    if (app_playback_checkbufcrc(p_file_data, pkt_len, host_pkt_crc))
    {
        results = PB_TRANS_RET_CRC_ERROR;
        goto L_Return;
    }

    if (pb_trans_struct.cur_offset + pkt_len + pb_trans_struct.temp_buf_used_size >
        pb_trans_struct.file_total_length)
    {
        results = PB_TRANS_RET_LENTH_ERROR;
        goto L_Return;
    }
    else
    {
        if (pb_trans_struct.temp_buf_used_size + pkt_len <= PLAYBACK_BUF_CHECK_SIZE)
        {
            memcpy(pb_trans_struct.p_temp_buf_head + pb_trans_struct.temp_buf_used_size, p_file_data,
                   pkt_len);
            pb_trans_struct.temp_buf_used_size += pkt_len;
        }
        else
        {
            results = PB_TRANS_RET_OPERATION_ERROR;
            goto L_Return;
        }
    }

    if (((pb_trans_struct.temp_buf_used_size >= PLAYBACK_BUF_CHECK_SIZE) ||
         ((pb_trans_struct.temp_buf_used_size + pb_trans_struct.cur_offset) ==
          pb_trans_struct.file_total_length)))
    {
        if (results == PB_TRANS_RET_SUCCESS)
        {
            pb_trans_struct.buf_is_full = true;
            app_playback_write_data_to_sd(); // flush buf to sd
        }
    }
    APP_PRINT_TRACE1("app_playback_trans_data_packet_handle, res:0x%x", results);
    return results;

L_Return:
    APP_PRINT_ERROR1("app_playback_trans_data_packet_handle, results:0x%x", results);
    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_REPORT_BUFFER_CHECK, pb_trans_struct.id.spp_idx,
                     &results, sizeof(results));
    app_playback_trans_stop_and_del_file();
    app_playback_trans_error_clear_local();
    app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

    return results;
}

/**
    * @brief    Valid the image
    * @param    image_id     image id
    * @return   valid result
    */
uint8_t app_playback_trans_valid_handle(uint8_t *p_date)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    uint32_t host_file_len;
    uint16_t host_file_crc;
    uint32_t local_file_len;
//    uint16_t local_file_crc;

    LE_ARRAY_TO_UINT32(host_file_len, p_date);
    LE_ARRAY_TO_UINT16(host_file_crc, p_date + 4);


    local_file_len = app_fs_size(playback_write_fs_handle);
    APP_PRINT_TRACE4("app_playback_trans_valid_handle,"\
                     "host_file_len:0x%x, host_crc:0x%x, file_len:0x%x,results: 0x%x",
                     host_file_len, host_file_crc, local_file_len, results);
    if (host_file_len != local_file_len)
    {
        results = PB_TRANS_RET_LENTH_ERROR;
    }

    if (results == PB_TRANS_RET_SUCCESS)
    {
        fs_file_update_header_bin(playback_write_fs_handle, &scan_hdl);
    }
    results = app_fs_close_file(playback_write_fs_handle);
    if (results != 0)
    {
        results = PB_TRANS_RET_CLOSE_ERROR;
    }
    else
    {
        results = PB_TRANS_RET_SUCCESS;
    }
    playback_write_fs_handle = NULL;
    pb_trans_struct.is_trans_pb_process = false;
    APP_PRINT_TRACE1("app_playback_trans_valid_handle, res:0x%x", results);
    return results;
}

static uint8_t app_playback_trans_stop_and_del_file(void)
{
    uint16_t res = PB_TRANS_RET_SUCCESS;

    if (playback_write_fs_handle != NULL)
    {
        uint8_t *file_name;
        uint16_t name_len = app_fs_get_filename_len(playback_write_fs_handle);
        uint8_t *delete_file = (uint8_t *)malloc(name_len);
        if (delete_file != NULL)
        {
            file_name = app_fs_get_filename(playback_write_fs_handle);
            memcpy(delete_file, file_name, name_len);
        }
        else
        {
            APP_PRINT_ERROR0("app_playback_trans_stop_and_del_file malloc fail");
        }
        res = app_fs_close_file(playback_write_fs_handle);
        playback_write_fs_handle = NULL;
        if (res != 0)
        {
            res = PB_TRANS_RET_CLOSE_ERROR;
        }
        else if (delete_file != NULL)
        {
            res = app_fs_unlink_file(delete_file);
            if (res != 0)
            {
                res = PB_TRANS_RET_DELETE_ERROR;
            }
            else
            {
                res = PB_TRANS_RET_SUCCESS;
            }
        }

        if (delete_file != NULL)
        {
            free(delete_file);
        }
    }
    APP_PRINT_TRACE1("app_playback_trans_stop_and_del_file, res:0x%d", res);
    return res;
}

static uint8_t app_playback_trans_cancel_handle(void)
{
    uint16_t res = PB_TRANS_RET_SUCCESS;
    res = app_playback_trans_stop_and_del_file();
    APP_PRINT_TRACE1("app_playback_trans_cancel_handle res:0x%x", res);
    app_playback_trans_error_clear_local();
    app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);
    return res;
}
/**
    * @brief    Delete file handle
    * @param    p_date     file info
    * @return   valid result
    */
static uint8_t app_playback_trans_delete_handle(uint8_t *p_data)
{
    uint8_t res = PB_TRANS_RET_SUCCESS;
    PB_DELETE_FILE_STRUCT trans_info;
    uint8_t *file_name;

    memcpy((uint8_t *)&trans_info, p_data, sizeof(PB_DELETE_FILE_STRUCT));
    file_name = p_data + sizeof(PB_DELETE_FILE_STRUCT);

    APP_PRINT_TRACE3("app_playback_trans_delete_handle,file_idx:%d, LEN:%d,%b", trans_info.file_idx,
                     trans_info.name_len, TRACE_BINARY(trans_info.name_len, file_name));
    res = fs_mark_delete_file_to_head_bin(file_name, &scan_hdl, trans_info.file_idx);
    if (res != 0)
    {
        res = PB_TRANS_RET_DELETE_ERROR;
    }
    else
    {
        res = app_fs_unlink_file(file_name);
    }
    if (res == 0)
    {
        res = PB_TRANS_RET_SUCCESS;
    }
    APP_PRINT_TRACE1("app_playback_trans_delete_handle, res:0x%d", res);
    return res;
}
static uint8_t app_playback_trans_single_delete_handle(uint8_t *p_data)
{
    uint8_t res = app_playback_trans_delete_handle(p_data);
    return res;
}
/**
    * @brief    exit transfer handle
    * @return   valid result
    */
static void app_playback_trans_exit_trans_handle(void)
{

    APP_PRINT_TRACE0("app_playback_trans_exit_trans_handle");
    app_playback_trans_stop_and_del_file();
    app_playback_trans_error_clear_local();
}
/**
    * @brief    change playlist info
    * @param    event_id   event_id
    * @param    p_date     check file info
    * @return   result
    */
static uint8_t app_playback_trans_list_updata_handle(uint16_t event_id, uint8_t *p_data)
{
    uint8_t res = PB_TRANS_RET_SUCCESS;
    PB_PLAYLIST_UPDATE_STRUCT trans_info;
    uint8_t *file_name = NULL;

    memcpy((uint8_t *)&trans_info, p_data, sizeof(PB_PLAYLIST_UPDATE_STRUCT));
    file_name = (uint8_t *)malloc(trans_info.name_len + 1);

    if (file_name == NULL)
    {
        res = PB_TRANS_RET_OPERATION_ERROR;
        return res;
    }

    memset(file_name, 0, (trans_info.name_len + 1));
    memcpy(file_name, (uint8_t *)(p_data + sizeof(PB_PLAYLIST_UPDATE_STRUCT)), trans_info.name_len);

    APP_PRINT_TRACE4("app_playback_trans_list_updata_handle:pblist:0x%x,fileidx,0x%x,len%d,%b",
                     trans_info.playlist_idx, trans_info.file_idx, trans_info.name_len,
                     TRACE_BINARY(trans_info.name_len, file_name));
    res = fs_set_playlist_info_to_head_bin(file_name, &scan_hdl,
                                           trans_info.playlist_idx,
                                           trans_info.file_idx);

    if (res != 0)
    {
        res = PB_TRANS_RET_OPERATION_ERROR;
    }
    else
    {
        res = PB_TRANS_RET_SUCCESS;
    }
    free(file_name);
    APP_PRINT_TRACE1("app_playback_trans_list_updata_handle, res:0x%x", res);
    return res;
}

static uint8_t app_playback_trans_delete_all_file(void)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    if (fs_unlink_all_files(&scan_hdl) == 0)
    {
        results = PB_TRANS_RET_SUCCESS;
    }
    else
    {
        results = PB_TRANS_RET_DELETE_ERROR;
    }
    APP_PRINT_TRACE1("app_playback_trans_delete_all_file, results:0x%x", results);
    return results;
}

static uint8_t app_playback_trans_delete_all_file_by_format(uint8_t *format)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    T_FILE_FORMAT file_format = *format;
    if (fs_delete_all_files_by_format(file_format, &scan_hdl) == 0)
    {
        results = PB_TRANS_RET_SUCCESS;
    }
    else
    {
        results = PB_TRANS_RET_DELETE_ERROR;
    }
    return results;
    APP_PRINT_TRACE1("app_playback_trans_delete_all_file_by_format, results:0x%x", results);
}

/**
    * @brief    Used to get sd space information
    * @param    *p_sd_space_info   point of sd space info data
    * @return   result: 0 success, others fail
    */
static int app_playback_trans_get_sd_space_info(PLAYBACK_SD_SPACE_INFO *p_sd_space_info)
{
    if (p_sd_space_info == NULL)
    {
        return 0xff;
    }
    memset(p_sd_space_info, 0, sizeof(*p_sd_space_info));
    uint32_t total_space = 0;
    uint32_t free_space = 0;
    int res = app_fs_get_space_info(&total_space, &free_space);
    if (res == 0)
    {
        p_sd_space_info->result = PB_TRANS_RET_SUCCESS;
        p_sd_space_info->sd_total_space = total_space;
        p_sd_space_info->sd_free_space = free_space;
        APP_PRINT_INFO2("get_sd_space_info: totalSpace:0x%x, freeSpace:0x%x", total_space, free_space);
    }
    else
    {
        p_sd_space_info->result = PB_TRANS_RET_READ_ERROR;
    }
    APP_PRINT_TRACE1("app_playback_trans_get_sd_space_info, res:0x%x", res);
    return res;
}

/**
    * @brief    Used to get sd space information
    * @param    *p_sd_space_info   point of sd space info data
    * @return   result: 0 success, others fail
    */
static void app_playback_trans_handle_ble_spp_cmd(T_CMD_ID cmd_id, uint16_t length, uint8_t *p)
{
    uint8_t results = PB_TRANS_RET_SUCCESS;
    bool ack_flag = false;
    uint8_t ack_pkt[3];
    switch (cmd_id)
    {
    case CMD_PLAYBACK_QUERY_INFO:
        {
            if (length == PLAYBACK_LENGTH_GET_INFO)
            {
                PLAYBACK_DEVICE_INFO device_info;
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    memcpy(pb_trans_struct.bd_addr, app_db.br_link[pb_trans_struct.id.spp_idx].bd_addr,
                           sizeof(pb_trans_struct.bd_addr));
                    app_playback_trans_get_device_info(&device_info);
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_QUERY_INFO, pb_trans_struct.id.spp_idx,
                                     (uint8_t *)&device_info,
                                     sizeof(device_info));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    app_playback_trans_get_device_info(&device_info);
                    le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &device_info.pkt_size, pb_trans_struct.id.ble_conn_id);
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, EVENT_PLAYBACK_QUERY_INFO,
                                                             sizeof(device_info),
                                                             (uint8_t *)&device_info);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_GET_LIST_DATA:
        {
            if (length == PLAYBACK_LENGTH_GET_LIST_DATA)
            {
                uint8_t list_info[7] = {0};
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    results = app_playback_trans_list_data_info(p, list_info);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                        app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_GET_LIST_DATA, pb_trans_struct.id.spp_idx, &results,
                                         sizeof(results));
                        break;
                    }
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_GET_LIST_DATA, pb_trans_struct.id.spp_idx, list_info,
                                     sizeof(list_info));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    results = app_playback_trans_list_data_info(p, list_info);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                                 EVENT_PLAYBACK_GET_LIST_DATA, sizeof(results),
                                                                 &results);
                        break;
                    }
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                             EVENT_PLAYBACK_GET_LIST_DATA, sizeof(list_info),
                                                             (uint8_t *)&list_info);
                    app_start_timer(&timer_idx_pb_trans_up, "pb_trans_up",
                                    playback_trans_timer_id, APP_TIMER_PB_TRANS_UP, 0, false,
                                    100);
                }

            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_TRANS_SET_SCENARIO:
        {
            if (length == PLAYBACK_LENGTH_SET_SCENARIO)
            {
                results = app_playback_trans_set_scenario(p);
                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_SET_SCENARIO, pb_trans_struct.id.spp_idx, &results,
                                     sizeof(results));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, EVENT_SET_SCENARIO,
                                                             sizeof(results),
                                                             &results);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_PLAYBACK_TRANS_START:
        {
            uint16_t file_name_len;
            uint8_t cmd_len;
            LE_ARRAY_TO_UINT16(file_name_len, p);
            cmd_len = 2 + file_name_len + 4;
            app_playback_trans_set_start_status();
            if ((length == cmd_len) && (pb_trans_struct.is_usb_plug_in == false))
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    results  = app_playback_trans_get_create_file_handle(p);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                    }
                    else
                    {
                        app_stop_timer(&timer_idx_pb_trans_sd_active);
                    }
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_TRANS_START, pb_trans_struct.id.spp_idx, &results,
                                     sizeof(results));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    uint16_t conn_interval_min = 0xc;
                    uint16_t conn_interval_max = 0xc;
                    uint16_t supervision_timeout = 500;

                    results  = app_playback_trans_get_create_file_handle(p);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                    }
                    else
                    {
                        app_stop_timer(&timer_idx_pb_trans_sd_active);
                    }
                    ble_set_prefer_conn_param(pb_trans_struct.id.ble_conn_id, conn_interval_min, conn_interval_max, 0,
                                              supervision_timeout);
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, EVENT_PLAYBACK_TRANS_START,
                                                             sizeof(results),
                                                             &results);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_TRANS_CONTINUE:
        {
            results = app_playback_trans_data_packet_handle(p, length);
            app_stop_timer(&timer_idx_pb_trans_file);
            app_start_timer(&timer_idx_pb_trans_file, "pb_trans_file",
                            playback_trans_timer_id, APP_TIMER_PB_TRANS_FILE, 0, false,
                            1600);

        }
        break;

    case CMD_PLAYBACK_REPORT_BUFFER_CHECK:
        {
            if (length == PLAYBACK_LENGTH_BUFFER_CHECK_EN)
            {
                //for reserve
                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    //results = app_playback_trans_buffer_check_handle(p);
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_REPORT_BUFFER_CHECK, pb_trans_struct.id.spp_idx,
                                     &results,
                                     sizeof(results));
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_VALID_SONG:
        {
            if (length == PLAYBACK_LENGTH_VALID_SONG)
            {
                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    results  = app_playback_trans_valid_handle(p);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                    }
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_VALID_SONG, pb_trans_struct.id.spp_idx, &results,
                                     sizeof(results));
                    app_stop_timer(&timer_idx_pb_trans_file);
                    app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);
                    app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    results  = app_playback_trans_valid_handle(p);
                    if (results != PB_TRANS_RET_SUCCESS)
                    {
                        app_playback_trans_error_clear_local();
                    }
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, EVENT_PLAYBACK_VALID_SONG,
                                                             sizeof(results),
                                                             &results);
                    app_stop_timer(&timer_idx_pb_trans_file);
                    app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);
                    app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_TRANS_CANCEL:
        {
            if (length == PLAYBACK_LENGTH_TRANS_CANCEL)
            {
                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    results = app_playback_trans_cancel_handle();
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_TRANS_CANCEL, pb_trans_struct.id.spp_idx, &results,
                                     sizeof(results));
                    app_stop_timer(&timer_idx_pb_trans_file);
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    results = app_playback_trans_cancel_handle();
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                             EVENT_PLAYBACK_TRANS_CANCEL, sizeof(results),
                                                             &results);
                    app_stop_timer(&timer_idx_pb_trans_file);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_EXIT_TRANS:
        {
            app_playback_trans_sd_active_start_timer(3);

            if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
            {
                app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                {
                    app_playback_trans_exit_trans_handle();
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_EXIT_TRANS, pb_trans_struct.id.spp_idx, &results,
                                     sizeof(results));
                }
            }
            else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
            {
                if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                {
                    app_playback_trans_exit_trans_handle();
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, EVENT_PLAYBACK_EXIT_TRANS,
                                                             sizeof(results),
                                                             &results);
                }
            }
        }
        break;

    case CMD_PLAYBACK_PERMANENT_DELETE_SONG:
        {
            uint16_t rx_file_len;
            LE_ARRAY_TO_UINT16(rx_file_len, (p + sizeof(PB_DELETE_FILE_STRUCT) - PLAYBACK_LEN_NAME_LENTH));
            app_playback_trans_set_start_status();
            if (length == (rx_file_len + sizeof(PB_DELETE_FILE_STRUCT)))
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_single_delete_handle(p);
                        app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_PERMANENT_DELETE_SONG, pb_trans_struct.id.spp_idx,
                                         &results,
                                         sizeof(results));
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_single_delete_handle(p);
                        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                                 EVENT_PLAYBACK_PERMANENT_DELETE_SONG,
                                                                 sizeof(results), &results);
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_PERMANENT_DELETE_ALL_SONG:
        {
            app_playback_trans_set_start_status();
            if (length == PLAYBACK_LENGTH_DELETE_ALL_SONG)
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_delete_all_file();
                        app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_PERMANENT_DELETE_ALL_SONG, pb_trans_struct.id.spp_idx,
                                         &results,
                                         sizeof(results));
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_delete_all_file();
                        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                                 EVENT_PLAYBACK_PERMANENT_DELETE_ALL_SONG,
                                                                 sizeof(results), &results);
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PERMANENT_DELETE_ALL_FILE_BY_FORMAT:
        {
            app_playback_trans_set_start_status();
            if (length == PLAYBACK_LENGTH_DELETE_ALL_BY_FORMAT)
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_delete_all_file_by_format(p);
                        APP_PRINT_INFO2("app_playback_trans_delete_all_file_by_format, format: %x, result:%x", *p, results);
                        app_report_event(CMD_PATH_SPP, EVENT_PERMANENT_DELETE_ALL_FILE_BY_FORMAT,
                                         pb_trans_struct.id.spp_idx, &results,
                                         sizeof(results));
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_delete_all_file_by_format(p);
                        APP_PRINT_INFO2("app_playback_trans_delete_all_file_by_format, format: %x, result:%x", *p, results);
                        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                                 EVENT_PERMANENT_DELETE_ALL_FILE_BY_FORMAT,
                                                                 sizeof(results), &results);
                        app_playback_trans_set_stop_status(PB_TRANS_NORMAL);
                    }
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_PLAYLIST_ADD_SONG:
    case CMD_PLAYBACK_PLAYLIST_DELETE_SONG:
        {
            uint16_t rx_file_len;
            uint16_t playlist_event_id = 0;
            LE_ARRAY_TO_UINT16(rx_file_len, (p + sizeof(PB_PLAYLIST_UPDATE_STRUCT) - PLAYBACK_LEN_NAME_LENTH));
            if (length == (rx_file_len + sizeof(PB_PLAYLIST_UPDATE_STRUCT)))
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    if (cmd_id == CMD_PLAYBACK_PLAYLIST_ADD_SONG)
                    {
                        playlist_event_id = EVENT_PLAYBACK_PLAYLIST_ADD_SONG;
                    }
                    else if (cmd_id == CMD_PLAYBACK_PLAYLIST_DELETE_SONG)
                    {
                        playlist_event_id = EVENT_PLAYBACK_PLAYLIST_DELETE_SONG;
                    }
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_list_updata_handle(playlist_event_id, p);
                        app_report_event(CMD_PATH_SPP, playlist_event_id, pb_trans_struct.id.spp_idx, &results,
                                         sizeof(results));
                    }
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    if (cmd_id == CMD_PLAYBACK_PLAYLIST_ADD_SONG)
                    {
                        playlist_event_id = EVENT_PLAYBACK_PLAYLIST_ADD_SONG;
                    }
                    else if (cmd_id == CMD_PLAYBACK_PLAYLIST_DELETE_SONG)
                    {
                        playlist_event_id = EVENT_PLAYBACK_PLAYLIST_DELETE_SONG;
                    }
                    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
                    {
                        results = app_playback_trans_list_updata_handle(playlist_event_id, p);
                        app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id, playlist_event_id,
                                                                 sizeof(results),
                                                                 &results);
                    }
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_GET_SD_SPACE_INFO:
        {
            if (length == PLAYBACK_LENGTH_GET_SD_SPACE_INFO)
            {
                PLAYBACK_SD_SPACE_INFO sd_space_info;
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    int result = app_playback_trans_get_sd_space_info(&sd_space_info);
                    if (result != 0)
                    {
                        APP_PRINT_ERROR1("audio_fs_get_space_info fail, error is : %d", result);
                    }
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_GET_SD_SPACE_INFO, pb_trans_struct.id.spp_idx,
                                     (uint8_t *)&sd_space_info,
                                     sizeof(sd_space_info));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    int result = app_playback_trans_get_sd_space_info(&sd_space_info);
                    if (result != 0)
                    {
                        APP_PRINT_ERROR1("audio_fs_get_space_info fail, error is : %d", result);
                    }
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                             EVENT_PLAYBACK_GET_SD_SPACE_INFO,
                                                             sizeof(sd_space_info),
                                                             (uint8_t *)&sd_space_info);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;

    case CMD_PLAYBACK_GET_FLASH_SPACE_INFO:
        {
            if (length == PLAYBACK_LENGTH_GET_FLASH_SPACE_INFO)
            {
                app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);

                if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
                {
                    app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
                    PLAYBACK_FLASH_SPACE_INFO flash_space_info;
                    flash_space_info.result = PB_TRANS_RET_SUCCESS;
                    flash_space_info.flash_total_space = 0;//for test
                    flash_space_info.flash_free_space = 0;
                    APP_PRINT_INFO2("get_flash_space_info: totalSpace:0x%x, freeSpace:0x%x",
                                    flash_space_info.flash_total_space, flash_space_info.flash_free_space);
                    app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_GET_FLASH_SPACE_INFO, pb_trans_struct.id.spp_idx,
                                     (uint8_t *)&flash_space_info,
                                     sizeof(flash_space_info));
                }
                else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
                {
                    PLAYBACK_FLASH_SPACE_INFO flash_space_info;
                    flash_space_info.result = PB_TRANS_RET_SUCCESS;
                    flash_space_info.flash_total_space = 0;//for test;
                    flash_space_info.flash_free_space = 0;
                    app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                             EVENT_PLAYBACK_GET_FLASH_SPACE_INFO,
                                                             sizeof(flash_space_info),
                                                             (uint8_t *)&flash_space_info);
                }
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    default:
        if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
        {
            ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
            app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
        }
        else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
        {
            APP_PRINT_ERROR1("app_playback_trans_handle_ble_spp_cmd, cmd_id not expected = %x", cmd_id);
        }
        break;
    }

    if (ack_flag)
    {
        APP_PRINT_TRACE0("app_playback_trans_handle_ble_spp_cmd: invalid length");
        if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
        {
            ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            app_report_event(CMD_PATH_SPP, EVENT_ACK, pb_trans_struct.id.spp_idx, ack_pkt, 3);
        }
    }
}

/*============================================================================*
 *                              Public Functions
 *============================================================================*/
/**
    * @brief    Used to send playback file list and play list data to host
    * @return   void
    */
void app_playback_trans_list_data_handle(void)
{
    uint16_t fs_res = 0;
    uint16_t send_data_len = 0;
    uint16_t data_pkt_len = 0;
    uint16_t playback_read_frame_size = PLAYBACK_READ_FRAME_SIZE;

    uint8_t *p_data = pb_trans_struct.p_temp_buf_head;
    uint8_t *p_send_data = p_data + PLAYBACK_LIST_HEAD_LEN + PLAYBACK_LIST_READ_LENGTH_LEN;

    if (pb_trans_struct.cur_offset < pb_trans_struct.file_total_length)
    {
        if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
        {
            le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &playback_read_frame_size,
                              pb_trans_struct.id.ble_conn_id);
            playback_read_frame_size -= 10; // Reserve redundancy some bytes
            playback_read_frame_size = playback_read_frame_size & 0xF0;
        }

        //set pkt header
        if ((pb_trans_struct.cur_offset + playback_read_frame_size) <
            pb_trans_struct.file_total_length) //continue
        {
            send_data_len = playback_read_frame_size;
            p_data[0] = PLAYBACK_TRANS_CONTINUE;
            LE_UINT16_TO_ARRAY((uint8_t *)&p_data[1], send_data_len);
        }
        else //end
        {
            send_data_len = pb_trans_struct.file_total_length - pb_trans_struct.cur_offset;
            p_data[0] = PLAYBACK_TRANS_END;
            LE_UINT16_TO_ARRAY((uint8_t *)&p_data[1], send_data_len);
            //may be clear some value
        }
        uint32_t read_len;
        if (PLAYBACK_NAME_BIN == pb_trans_struct.file_list_type)
        {
            fs_res = app_fs_read_name_bin(pb_trans_struct.cur_offset, p_send_data, send_data_len, &read_len);
        }
        else if (PLAYBACK_HEAD_BIN == pb_trans_struct.file_list_type)
        {
            fs_res = app_fs_read_header_bin(pb_trans_struct.cur_offset, p_send_data, send_data_len,
                                            &read_len);
        }
        if (fs_res)
        {
            APP_PRINT_ERROR1("app_playback_trans_list_data_handle read error,fs_res:%d", fs_res);
            app_playback_trans_error_clear_local();
            return;
        }

        data_pkt_len = send_data_len + PLAYBACK_LIST_HEAD_LEN + PLAYBACK_LIST_READ_LENGTH_LEN;

        if (pb_trans_struct.cur_offset != pb_trans_struct.file_total_length)
        {
            if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
            {
                app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_GET_LIST_DATA, pb_trans_struct.id.spp_idx,
                                 p_data, data_pkt_len);
            }
            else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
            {
                app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                         EVENT_PLAYBACK_GET_LIST_DATA,
                                                         data_pkt_len, (uint8_t *)p_data);
            }
        }
        else
        {
            //close file
            APP_PRINT_ERROR0("app_playback_trans_list_data_handle FILE END");
        }
        pb_trans_struct.send_cnt++;
        pb_trans_struct.cur_offset = pb_trans_struct.cur_offset + send_data_len;

        if (pb_trans_struct.cur_offset != pb_trans_struct.file_total_length)
        {
            if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
            {
                app_start_timer(&timer_idx_pb_trans_up, "pb_trans_up",
                                playback_trans_timer_id, APP_TIMER_PB_TRANS_UP, 0, false,
                                100);
            }
        }
        else
        {
            if (pb_trans_struct.p_temp_buf_head != NULL)
            {
                free(pb_trans_struct.p_temp_buf_head);
                pb_trans_struct.p_temp_buf_head = NULL;
            }
        }
        APP_PRINT_TRACE2("app_playback_trans_list_data_handle,file_total_length:%d,cur_offset:%d",
                         pb_trans_struct.file_total_length, pb_trans_struct.cur_offset);
    }
}

/**
    * @brief  Handle all the spp command
    * @param  length length of command id and data
    * @param  p_value data addr
    * @param  app_idx received rx command device index
    * @return void
    */

void app_playback_trans_cmd_handle(uint16_t length, uint8_t *p_value, uint8_t app_idx)
{
    uint8_t ack_pkt[3];
    uint16_t cmd_id = *(uint16_t *)p_value;
    uint8_t *p;

    ack_pkt[0] = p_value[0];
    ack_pkt[1] = p_value[1];
    ack_pkt[2] = CMD_SET_STATUS_COMPLETE;

    if (length < 2)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_report_event(CMD_PATH_SPP, EVENT_ACK, app_idx, ack_pkt, 3);
        APP_PRINT_ERROR0("app_playback_trans_cmd_handle: error length");
        return;
    }

    length = length - 2;
    p = p_value + 2;
    pb_trans_struct.trans_mode = PLAYBACK_TRANS_SPP_MODE;
    pb_trans_struct.id.spp_idx = app_idx;

    APP_PRINT_TRACE2("===>app_playback_trans_cmd_handle, cmd_id:0x%x, length:0x%x\n", cmd_id, length);

    app_playback_trans_handle_ble_spp_cmd(cmd_id, length, p);

}

/**
    * @brief    Handle written request characteristic
    * @param    conn_id     ID to identify the connection (LE only, 0xFF for BR/EDR)
    * @param    conn_handle Connection handle
    * @param    cid         Channel ID
    * @param    chann_type  Channel type (LE or BREDR)
    * @param    length      Length of value to be written
    * @param    p_value     Value to be written
    * @return   T_APP_RESULT
    * @retval   Handle result of this request
    */

T_APP_RESULT app_playback_ble_handle_cp_req(uint8_t conn_id, uint16_t conn_handle, uint16_t cid,
                                            T_GAP_CHANN_TYPE chann_type,
                                            uint16_t length, uint8_t *p_value)
{
    T_APP_RESULT cause = APP_RESULT_INVALID_PDU;
    uint16_t cmd_id = *(uint16_t *)p_value;
    uint8_t *p = p_value + 2;

    if (length < 2)
    {
        APP_PRINT_ERROR0("app_playback_ble_handle_cp_req: error length");
        return cause;
    }

    length = length - 2;
    pb_trans_struct.id.ble_conn_id = conn_id;
    pb_trans_struct.conn_handle = conn_handle;
    pb_trans_struct.cid = cid;
    pb_trans_struct.chann_type = chann_type;
    pb_trans_struct.trans_mode = PLAYBACK_TRANS_BLE_MODE;
    APP_PRINT_TRACE2("===>app_playback_ble_handle_cp_req, cmd_id:0x%x, length:0x%x\n", cmd_id, length);

    app_playback_trans_handle_ble_spp_cmd(cmd_id, length, p);

    return APP_RESULT_SUCCESS;
}

/**
    * @brief  Cancel the transfer to avoid using the SD card at the same time
    */
void app_playback_trans_cancel(void)
{
    pb_trans_struct.is_usb_plug_in = true;
    if (app_db.transfer_status == TRANSFER_STARTED)
    {
        uint8_t results = PB_TRANS_RET_SUCCESS;
        results = app_playback_trans_cancel_handle();
        if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_SPP_MODE)
        {
            app_report_event(CMD_PATH_SPP, EVENT_PLAYBACK_TRANS_CANCEL, pb_trans_struct.id.spp_idx,
                             &results, sizeof(results));
        }
        else if (pb_trans_struct.trans_mode == PLAYBACK_TRANS_BLE_MODE)
        {
            app_playback_service_prepare_send_notify(pb_trans_struct.id.ble_conn_id,
                                                     EVENT_PLAYBACK_TRANS_CANCEL, sizeof(results), &results);
        }
        app_stop_timer(&timer_idx_pb_trans_file);
    }
}

/**
    * @brief  restore usb status
    * @return void
    */
void app_playback_trans_restore(void)
{
    pb_trans_struct.is_usb_plug_in = false;
}

/**
    * @brief  get ota status
    * @return True:is doing ota; False: is not doing ota
    */
bool app_playback_trans_is_busy(void)
{
    return pb_trans_struct.is_trans_pb_process;
}

/////////////////////////////////////////timer/////////////////////////////////////////////////////
/**
    * @brief  timeout callback
    * @param  timer_id  timer id
    * @param  timer_chann  time channel
    * @return void
    */
static void app_playback_trans_timeout_cb(uint8_t timer_evt, uint16_t timer_chann)
{
    APP_PRINT_TRACE1("app_playback_trans_timeout_cb: timer_evt 0x%02x", timer_evt);

    switch (timer_evt)
    {
    case APP_TIMER_PB_TRANS_FILE:
        {
            app_stop_timer(&timer_idx_pb_trans_file);
            app_playback_trans_error_clear_local();
            app_playback_trans_sd_active_start_timer(PLAYBACK_SD_ACTIVE_TIME_S);
        }
        break;

    case APP_TIMER_PB_TRANS_UP:
        {
            app_stop_timer(&timer_idx_pb_trans_up);
            app_playback_trans_list_data_handle();
        }
        break;

    case APP_TIMER_PB_TRANS_SD_ACTIVE:
        {
            app_stop_timer(&timer_idx_pb_trans_sd_active);
            app_fs_disk_power_down_enable(APP_DISK_CHECK_TRANS_FILE);
        }
        break;

    default:
        break;
    }
}

static void app_playback_trans_sd_active_start_timer(uint8_t time_s)
{
    app_stop_timer(&timer_idx_pb_trans_sd_active);
    app_fs_disk_power_down_disable(APP_DISK_CHECK_TRANS_FILE);
    app_fs_disk_power_on();
    app_start_timer(&timer_idx_pb_trans_sd_active, "pb_trans_sd_active",
                    playback_trans_timer_id, APP_TIMER_PB_TRANS_SD_ACTIVE, 0, false,
                    time_s * 1000);
}
/////////////////////////////////////////timer end/////////////////////////////////////////////////////

void app_playback_update_file_init(void)
{
    app_db.transfer_status = TRANSFER_STOPPED;
    pb_trans_struct.is_usb_plug_in = false;
    app_timer_reg_cb(app_playback_trans_timeout_cb, &playback_trans_timer_id);
}
/*============================================================================*
 *                              Module Auto Init
 *============================================================================*/

/**
 * @brief Local-playback module entry registered via APP_MODULE_INIT().
 *
 *        The Playback GATT service itself is registered centrally in
 *        app_ble_service.c, so this entry only handles module-local
 *        initialization:
 *
 *          1) Initialize playback transfer state and timer callback.
 *          2) Register the SDP record (BR/EDR transport only).
 *             The GATT service must already be registered by app_ble_service
 *             before this runs, otherwise the ATT handle range advertised in
 *             the SDP record will be invalid.
 */
static void app_playback_module_init(void)
{
    /* Step 1: business layer init - timer registration, transfer state reset. */
    app_playback_update_file_init();

    /* Step 2: Register Playback GATT service */
    srv_id_local = playback_reg_srv(playback_gatt_svc_callback);

#if CONFIG_PLAYBACK_GATT_OVER_BREDR
    /* Step 3: SDP record registration */
    playback_sdp_register();
#endif
}
APP_MODULE_INIT(app_playback_module_init);

/** End of APP_PLAYBACK_Exported_Functions
    * @}
    */

/** @} */ /* End of group APP_PLAYBACK_SERVICE */
