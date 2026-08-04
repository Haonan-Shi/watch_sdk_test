/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include <string.h>
#include <stdlib.h>
#include "app_cfg.h"
#include "bt_types.h"
#include "dfu_transport.h"
#include "dfu_api.h"
#include "dfu_service.h"
#include "dfu_common.h"
#include "fmc_api.h"
#include "flash_map.h"
#include "os_mem.h"
#include "os_timer.h"
#include "gap_conn_le.h"
#include "ble_conn.h"
#include "ota_ext.h"
#include "os_mem.h"
#include "os_timer.h"
#include "remote.h"
#include "trace.h"
#include "os_sched.h"
#if CONFIG_DFU_NORMAL_OTA
#include "dfu_main.h"
#include "fmc_api_ext.h"
#endif
#include "app_main.h"
#include "storage.h"
/** @defgroup  APP_OTA_SERVICE APP OTA handle
    * @brief APP OTA Service to implement OTA feature
    * @{
    */

/*if enable transfer encrypted air data packet by aes. disable: 0*/
#define ENABLE_OTA_AES                  0

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup APP_OTA_Exported_Variables APP OTA Exported Variables
    * @brief
    * @{
    */
extern bool normal_ota_mode_check;

static OTA_FUNCTION_STRUCT ota_struct;
static uint8_t temp_image_total_num = 0;

bool dfu_active_reset_pending = false;
bool dfu_active_reset_to_ota_mode = false;
bool dfu_switch_to_ota_mode_pending = false;

T_OTA_INFO ota_info;
/** End of APP_OTA_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Private Functions
 *============================================================================*/
/** @defgroup APP_OTA_Exported_Functions APP OTA service Exported Functions
    * @brief
    * @{
    */
/**
    * @brief    get current active bank
    * @param    void
    * @return   0x01:bank0, 0x02:bank1, 0x00:not support bank switch
    */
static uint8_t ota_service_get_active_bank(void)
{
#if (CONFIG_APP_NANDBOOT == 1)
    return NOT_SUPPORT_BANK_SWITCH;
#else
    uint8_t active_bank;
    uint32_t ota_bank0_addr;

    if (is_ota_support_bank_switch())
    {
        ota_bank0_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);

        if (ota_bank0_addr == get_active_ota_bank_addr())
        {
            active_bank = IMAGE_LOCATION_BANK0;
        }
        else
        {
            active_bank = IMAGE_LOCATION_BANK1;
        }
    }
    else
    {
        active_bank = NOT_SUPPORT_BANK_SWITCH;
    }

    return active_bank;
#endif
}

#if defined CONFIG_SOC_SERIES_RTL87X3G
/**
    * @brief    get current active bootpatch bank
    * @param    void
    * @return   0x01:bank0, 0x02:bank1
    */
static uint8_t ota_service_get_bootpatch_active_bank(void)
{
    uint8_t active_bank = NOT_SUPPORT_BANK_SWITCH;

#if 0
    uint32_t boot_patch0_addr;

    //boot_patch0_addr = flash_partition_addr_get(PARTITION_FLASH_BOOT_PATCH0);
    boot_patch0_addr = BOOT_PATCH_BANK0_ADDR;

    if (boot_patch0_addr == get_header_addr_by_img_id(IMG_BOOTPATCH))
    {
        active_bank = IMAGE_LOCATION_BANK0;
    }
    else
    {
        active_bank = IMAGE_LOCATION_BANK1;
    }

#else
    uint32_t boot_patch_bank_num = dfu_get_bootpatch_active_bank_num();
    if (boot_patch_bank_num == 0)
    {
        active_bank = IMAGE_LOCATION_BANK0;
    }
    else if (boot_patch_bank_num == 1)
    {
        active_bank = IMAGE_LOCATION_BANK1;
    }
#endif

    return active_bank;
}
#endif


/**
    * @brief    Wrapper function to send notification to peer
    * @note
    * @param    conn_id     ID to identify the connection
    * @param    opcode      Notification on the specified opcode
    * @param    len         Notification data length
    * @param    data        Additional notification data
    * @return   void
    */
static void dfu_service_prepare_send_notify(uint8_t conn_id, uint8_t opcode, uint16_t len,
                                            uint8_t *data)
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
    uint16_t cid;
    uint8_t cid_num;
    uint16_t conn_handle = le_get_conn_handle(conn_id);
    gap_chann_get_cid(conn_handle, 1, &cid, &cid_num);

    le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size, conn_id);
    p_buffer = malloc(mtu_size);

    if (p_buffer == NULL)
    {
        return;
    }

    if (len < mtu_size - 2)
    {
        p_buffer[0] = DFU_OPCODE_NOTIF;
        p_buffer[1] = opcode;
        memcpy(&p_buffer[2], data, len);

        dfu_service_send_notification(conn_handle, cid, p_buffer, len + 2);
        free(p_buffer);

        return;
    }

    while (remain_size)
    {
        if (remain_size == len)
        {
            p_buffer[0] = DFU_OPCODE_NOTIF;
            p_buffer[1] = opcode;
            memcpy(&p_buffer[2], p_data, mtu_size - 2);
            dfu_service_send_notification(conn_handle, cid, p_buffer, mtu_size);

            p_data += (mtu_size - 2);
            remain_size -= (mtu_size - 2);
            continue;
        }

        send_len = (remain_size > mtu_size) ? mtu_size : remain_size;
        memcpy(p_buffer, p_data, send_len);
        dfu_service_send_notification(conn_handle, cid, p_buffer, send_len);

        p_data += send_len;
        remain_size -= send_len;
    }

    free(p_buffer);
}

/**
    * @brief    check section size before ota
    * @param    void
    * @return   true: check success; false: check fail
    */
static bool dfu_check_section_size(void)
{
    uint32_t section_size = get_temp_ota_bank_img_size_by_img_id((IMG_ID)ota_struct.image_id);

    if (ota_struct.image_total_length > section_size)
    {
        DFU_PRINT_ERROR2("dfu_check_section_size: total size 0x%x, section size 0x%x",
                         ota_struct.image_total_length, section_size);
        return false;
    }

    return true;
}

/**
    * @brief  Reset local variables
    * @return void
    */
static void dfu_clear_local(T_OTA_CLEAR_LOCAL_CAUSE cause)
{
    APP_PRINT_TRACE1("app_ota_clear_local cause: %d", cause);
    temp_image_total_num = 0;
    ota_struct.image_total_length = 0;
    ota_struct.image_id = 0;
    ota_struct.cur_offset = 0;
    ota_struct.next_subimage_offset = 0;
    ota_struct.ota_flag.value = 0;
    ota_struct.ota_temp_buf_used_size = 0;
    if (cause)
    {
        ota_struct.test.value = 0;
    }

    memset(ota_struct.bd_addr, 0, sizeof(ota_struct.bd_addr));
    if (ota_struct.force_temp_mode)
    {
        //force_enable_ota_temp(false);
        ota_struct.force_temp_mode = 0;
    }

    if (ota_struct.p_ota_temp_buf_head != NULL)
    {
        free(ota_struct.p_ota_temp_buf_head);
        ota_struct.p_ota_temp_buf_head = NULL;
        ota_struct.buffer_size = 0;
    }

    if (ota_struct.bp_level)
    {
        fmc_flash_nor_set_bp_lv(flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0),
                                ota_struct.bp_level);
        ota_struct.bp_level = 0;
    }
    if (ota_info.ota_mode == SPP_OTA_MODE)
    {
        bt_sniff_mode_enable(ota_struct.bd_addr, 784, 816, 0, 0);
    }
}

/**
    * @brief  timeout callback for ota
    * @param  timer_id  timer id
    * @param  timer_chann  time channel
    * @return void
    */
static void dfu_timeout_cb(void *p_xtimer)
{
    uint32_t timer_id = 0;

    os_timer_id_get(&p_xtimer, &timer_id);
    DFU_PRINT_TRACE1("dfu_timeout_cb: timer id 0x%02X", timer_id);

    switch (timer_id)
    {
    case TIMER_ID_DFU_IMAGE_TRANS:
        {
            os_timer_stop(&ota_struct.timer_handle_ota_transfer);
            dfu_clear_local(OTA_IMAGE_TRANS_TIMEOUT);
        }
        break;
    default:
        break;
    }
}

static void ota_transfer_timer_start(void)
{
    if (ota_struct.timer_handle_ota_transfer == NULL)
    {
        os_timer_create(&ota_struct.timer_handle_ota_transfer, "ota_image_transfer",
                        TIMER_ID_DFU_IMAGE_TRANS, 30 * 1000, false, dfu_timeout_cb);
        os_timer_start(&ota_struct.timer_handle_ota_transfer);
    }
    else
    {
        os_timer_restart(&ota_struct.timer_handle_ota_transfer, 30 * 1000);
    }
}

static bool dfu_get_enc_setting(void)
{
    if (ENABLE_OTA_AES)
    {
        return true;
    }
    else
    {
        return false;
    }

}

/**
    * @brief    Handle the start dfu control point
    * @param    p_data     data to be written
    * @return   handle result 0x01:success other:fail
    */
static uint8_t dfu_cp_start_dfu_handle(uint8_t *p_data) //0x01
{
    uint8_t results = DFU_ARV_SUCCESS;

    if (p_data == NULL)
    {
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
        return results;
    }

    if (dfu_get_enc_setting())
    {
        DFU_PRINT_TRACE1("start dfu before decrypted: %b", TRACE_BINARY(16, p_data));
        aes256_decrypt_16byte(p_data);
        DFU_PRINT_TRACE1("start dfu after decrypted: %b", TRACE_BINARY(16, p_data));
    }
    T_IMG_CTRL_HEADER_FORMAT *start_dfu_para = (T_IMG_CTRL_HEADER_FORMAT *)p_data;
#if CONFIG_DFU_COMPRESS_OTA
    if (isCompressed(start_dfu_para))
    {
        T_COMPRESS_IMG_CTRL_HEADER_FORMAT *com_start_dfu_para = (T_COMPRESS_IMG_CTRL_HEADER_FORMAT *)p_data;
        ota_struct.force_temp_mode = *(uint8_t *)((T_COMPRESS_IMG_CTRL_HEADER_FORMAT *)p_data + 1);
        ota_struct.image_total_length = com_start_dfu_para->payload_len + sizeof(
                                            T_COMPRESS_IMG_HEADER_FORMAT);
        ota_struct.image_id = com_start_dfu_para->image_id;
        DFU_PRINT_TRACE1("ble_dfu_cp_start_dfu_handle: force_temp_mode=0x%x", ota_struct.force_temp_mode);
        DFU_PRINT_TRACE4("ble_dfu_cp_start_dfu_handle: ic_type=0x%x, CtrlFlag=0x%x, img_id=0x%x, Total Length=0x%x",
                         com_start_dfu_para->ic_type,
                         com_start_dfu_para->ctrl_flag,
                         com_start_dfu_para->image_id,
                         ota_struct.image_total_length
                        );
    }
    else
    {
        ota_struct.force_temp_mode = *(uint8_t *)((T_IMG_CTRL_HEADER_FORMAT *)p_data + 1);
        ota_struct.image_total_length = start_dfu_para->payload_len + DEFAULT_HEADER_SIZE;
        ota_struct.image_id = start_dfu_para->image_id;
        DFU_PRINT_TRACE1("ble_dfu_cp_start_dfu_handle: force_temp_mode=0x%x", ota_struct.force_temp_mode);
        DFU_PRINT_TRACE4("ble_dfu_cp_start_dfu_handle: ic_type=0x%x, CtrlFlag=0x%x, img_id=0x%x ,CRC16=0x%x, Total Length=0x%x",
                         start_dfu_para->ic_type,
                         start_dfu_para->ctrl_flag,
                         start_dfu_para->image_id,
                         ota_struct.image_total_length
                        );
    }

#else
    ota_struct.force_temp_mode = *(uint8_t *)((T_IMG_CTRL_HEADER_FORMAT *)p_data + 1);
    ota_struct.image_total_length = start_dfu_para->payload_len + DEFAULT_HEADER_SIZE;
    ota_struct.image_id = start_dfu_para->image_id;
    DFU_PRINT_TRACE1("dfu_cp_start_dfu_handle: force_temp_mode=0x%x", ota_struct.force_temp_mode);
    DFU_PRINT_TRACE4("dfu_cp_start_dfu_handle: ic_type=0x%x, CtrlFlag=0x%x, img_id=0x%x, Total Length=0x%x",
                     start_dfu_para->ic_type,
                     start_dfu_para->ctrl_flag,
                     start_dfu_para->image_id,
                     ota_struct.image_total_length
                    );
#endif

    if (dfu_check_section_size() == false)
    {
        DFU_PRINT_ERROR0("dfu_cp_start_dfu_handle: Image is oversize");
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
        return results;
    }

    if (ota_struct.force_temp_mode)
    {
        //force_enable_ota_temp(true);
    }

    if ((!is_ota_support_bank_switch() && ota_struct.image_id == IMG_OTA)
        || ota_struct.image_id < IMG_DFU_FIRST
        || (ota_struct.image_id >= IMG_DFU_MAX) &&
        ((USER_IMG_ID)ota_struct.image_id < IMG_USER_DATA_FIRST))
    {
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
        return results;
    }

    if (is_ota_support_bank_switch())
    {
        ota_struct.next_subimage_offset = 0;
    }
    else
    {
        if (ota_struct.image_id == IMG_BOOTPATCH
            || ((USER_IMG_ID)ota_struct.image_id >= IMG_USER_DATA_FIRST &&
                (USER_IMG_ID)ota_struct.image_id < IMG_USER_DATA_MAX))
        {
            /*bootpatch always dual bank*/
            ota_struct.next_subimage_offset = 0;
        }
        else
        {
            /*restore next image offset*/
#if (CONFIG_APP_NANDBOOT == 1)
            //align up to FLASH_NAND_PAGE_SIZE
            ota_struct.next_subimage_offset = (ota_struct.tmp_next_subimage_offset +
                                               FLASH_NAND_BLOCK_ALIGN_MASK) &
                                              (~FLASH_NAND_BLOCK_ALIGN_MASK);
#else
            ota_struct.next_subimage_offset = ota_struct.tmp_next_subimage_offset;
#endif
        }
    }
    ota_struct.ota_flag.is_ota_process = true;
    ota_struct.ota_temp_buf_used_size = 0;
    ota_struct.cur_offset = 0;
#if CONFIG_DFU_NORMAL_OTA
    if (is_normal_ota_mode)
    {
        uint32_t restart_ms = (ota_struct.image_total_length + 0x19000 - 1) / 0x19000 *
                              NORMAL_OTA_TIMEOUT_TOTAL * 1000;
        os_timer_restart(&normal_ota_total_timer_handle, restart_ms);
        DFU_PRINT_INFO1("dfu_cp_start_dfu_handle: restart total timer %d ms", restart_ms);
    }
#endif

    fmc_flash_nor_set_bp_lv(flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0), 0);

    return results;
}

/**
    * @brief    Valid the image
    * @param    p_date     point of input data
    * @return   valid result
    */
static uint8_t dfu_cp_valid_handle(uint8_t *p_data) //0x03
{
    uint8_t results = DFU_ARV_SUCCESS;
    uint16_t image_id;

    if (!ota_struct.ota_flag.skip_flag
        && ota_struct.test.t_skip_fail)
    {
        ota_struct.test.t_skip_fail = 0;
        results = DFU_ARV_FAIL_CRC_ERROR;
        return results;
    }

    ota_struct.ota_flag.skip_flag = 0;

    if (p_data == NULL)
    {
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
        return results;
    }

    LE_ARRAY_TO_UINT16(image_id, p_data);
    LE_ARRAY_TO_UINT8(ota_struct.is_last_image, p_data + 2);
    DFU_PRINT_TRACE3("==>dfu_cp_valid_handle: img_id=0x%x, ota_struct.image_id=0x%x, is_last_image=%d",
                     image_id, ota_struct.image_id, ota_struct.is_last_image);

    if (image_id == ota_struct.image_id)
    {
        if (!dfu_checksum((IMG_ID)image_id, ota_struct.next_subimage_offset))
        {
            results = DFU_ARV_FAIL_CRC_ERROR;
        }
        else
        {

            //user data and boot patch image needn't record into temp info
            if (ota_struct.image_id >= IMG_DFU_FIRST && ota_struct.image_id < IMG_DFU_MAX)
            {
                bool bank_switch = is_ota_support_bank_switch();
                if (!bank_switch && ota_struct.image_id != IMG_BOOTPATCH)
                {

#if (CONFIG_APP_NANDBOOT == 1)
                    //align up to FLASH_NAND_BLOCK_SIZE
                    uint32_t image_size = (ota_struct.image_total_length + FLASH_NAND_BLOCK_ALIGN_MASK) &
                                          (~FLASH_NAND_BLOCK_ALIGN_MASK);

                    uint32_t src_flash_addr = get_active_ota_bank_img_nand_addr_by_img_id((IMG_ID)image_id);
                    uint32_t dest_flash_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_TMP);
                    uint32_t version = 0;
                    uint16_t test_image_id = 0;
                    fmc_flash_nand_read(src_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), (uint8_t *)&version,
                                        4);
                    fmc_flash_nand_read(src_flash_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                                  ctrl_header) + offsetof(T_IMG_CTRL_HEADER_FORMAT, image_id), (uint8_t *)&test_image_id, 2);
                    DFU_PRINT_TRACE3("dfu_cp_valid_handle version 0x%x addr 0x%x img_id 0x%x", version,
                                     src_flash_addr + offsetof(T_IMG_HEADER_FORMAT, ctrl_header), test_image_id);
                    fmc_flash_nand_read(dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), (uint8_t *)&version,
                                        4);
                    fmc_flash_nand_read(dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                                   ctrl_header) + offsetof(T_IMG_CTRL_HEADER_FORMAT, image_id), (uint8_t *)&test_image_id, 2);
                    DFU_PRINT_TRACE3("dfu_cp_valid_handle version temp 0x%x addr 0x%x img_id 0x%x", version,
                                     dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), test_image_id);

                    //TODO: remapping not done
                    fmc_flash_nand_remapping(dest_flash_addr, src_flash_addr, image_size);

                    fmc_flash_nand_read(src_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), (uint8_t *)&version,
                                        4);
                    fmc_flash_nand_read(src_flash_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                                  ctrl_header) + offsetof(T_IMG_CTRL_HEADER_FORMAT, image_id), (uint8_t *)&test_image_id, 2);
                    DFU_PRINT_TRACE3("dfu_cp_valid_handle version 0x%x addr 0x%x img_id 0x%x", version,
                                     src_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), test_image_id);
                    fmc_flash_nand_read(dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), (uint8_t *)&version,
                                        4);
                    fmc_flash_nand_read(dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                                   ctrl_header) + offsetof(T_IMG_CTRL_HEADER_FORMAT, image_id), (uint8_t *)&test_image_id, 2);
                    DFU_PRINT_TRACE3("dfu_cp_valid_handle version temp 0x%x addr 0x%x img_id 0x%x", version,
                                     dest_flash_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver), test_image_id);
#else
                    ota_struct.tmp_next_subimage_offset += ota_struct.cur_offset;
                    ota_struct.next_subimage_offset = ota_struct.tmp_next_subimage_offset;

                    ota_struct.temp_image_info[temp_image_total_num].image_offset =
                        ota_struct.next_subimage_offset - ota_struct.image_total_length;
#endif

                    DFU_PRINT_TRACE3("==>dfu_cp_valid_handle:temp info idx %d,  image_id 0x%x, image_offset 0x%x in temp bank",
                                     temp_image_total_num, ota_struct.image_id,
                                     ota_struct.temp_image_info[temp_image_total_num].image_offset);
                    temp_image_total_num ++;
                }

                ota_struct.cur_offset = 0;
                /* set updated images valid */
                SET_VALID_BITMAP(image_id);

                DFU_PRINT_TRACE1("==>dfu_cp_valid_handle: valid_bitmap= 0x%x", valid_bitmap);
            }

            if (ota_struct.image_id >= IMG_USER_DATA_FIRST && ota_struct.image_id < IMG_USER_DATA_MAX)
            {
                SET_USER_DATA_VALID_BITMAP(image_id);
            }

            DFU_PRINT_TRACE1("==>dfu_cp_valid_handle: checksum Success! temp bank next_subimage_offset= 0x%x",
                             ota_struct.next_subimage_offset);
        }
    }
    else
    {
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
    }

    return results;
}

#if defined CONFIG_SOC_SERIES_RTL87X3G
static void dfu_bootpatch_always_bank_switch(void)
{
    if (ota_struct.test.t_stress_test && GET_VALID_BITMAP(IMG_BOOTPATCH))
    {
        DFU_PRINT_INFO0("<==dfu_bootpatch_always_bank_switch");
#if (CONFIG_APP_NANDBOOT == 1)
        uint32_t image_addr = dfu_get_bootpatch_flash_addr(true);
        DFU_FLASH_ERASE_MODE mode = DFU_FLASH_ERASE_BLOCK;
#else
        uint32_t image_addr = get_active_boot_patch_addr();
        DFU_FLASH_ERASE_MODE mode = DFU_FLASH_ERASE_SECTOR;
#endif
        dfu_common_flash_erase(image_addr, mode);
    }
}
#endif

static void dfu_set_ota_temp_image_ready(void)
{
    uint32_t base_addr = 0;
    T_IMG_HEADER_FORMAT *p_header = NULL;
    base_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_TMP);

    DFU_PRINT_INFO2("<==dfu_set_ota_temp_image_ready: ota_temp_addr =0x%x, temp_image_total_num %d",
                    base_addr, temp_image_total_num);

    for (int i = temp_image_total_num - 1; i >= 0; i--)
    {
        p_header = (T_IMG_HEADER_FORMAT *)(base_addr + ota_struct.temp_image_info[i].image_offset);
#if CONFIG_DFU_COMPRESS_OTA
        if (isCompressed((uint8_t *)(base_addr + ota_struct.temp_image_info[i].image_offset)))
        {
            T_COMPRESS_IMG_HEADER_FORMAT *p_compressed_header = (T_COMPRESS_IMG_HEADER_FORMAT *)(
                                                                    base_addr + ota_struct.temp_image_info[i].image_offset);
            dfu_set_compressed_ready(p_compressed_header);
        }
        else
        {
            if (p_header)
            {
                dfu_set_ready(p_header);
            }
        }
#else
        if (p_header)
        {
            dfu_set_ready(p_header);
        }
#endif
    }
}

static void dfu_cp_active_reset_handle(bool is_enter_dfu_mode)
{
    T_IMG_HEADER_FORMAT *p_header = NULL;
    uint16_t img_id = IMG_DFU_MAX;

    if (!is_ota_support_bank_switch())
    {
        dfu_set_ota_temp_image_ready();
#if defined CONFIG_SOC_SERIES_RTL87X3G
        img_id = IMG_BOOTPATCH;
        if (GET_VALID_BITMAP(img_id))
        {
            p_header = (T_IMG_HEADER_FORMAT *)get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
#if CONFIG_DFU_COMPRESS_OTA
            if (isCompressed((uint8_t *)(get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id))))
            {
                T_COMPRESS_IMG_HEADER_FORMAT *p_compressed_header = (T_COMPRESS_IMG_HEADER_FORMAT *)
                                                                    get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
                dfu_set_compressed_ready(p_compressed_header);
            }
            else
            {
                if (p_header)
                {
                    dfu_set_ready(p_header);
                }
            }
#else
            if (p_header)
            {
                dfu_set_ready(p_header);
            }
#endif
        }
#endif
    }
    else
    {
        /* support bank switch */
        img_id = IMG_DFU_MAX - 1;
        for (; img_id >= IMG_DFU_FIRST; img_id--)
        {
            if (GET_VALID_BITMAP(img_id))
            {
                p_header = (T_IMG_HEADER_FORMAT *)get_temp_ota_bank_addr_by_img_id((IMG_ID)img_id);
#if CONFIG_DFU_COMPRESS_OTA
                if (isCompressed((uint8_t *)(get_temp_ota_bank_addr_by_img_id((IMG_ID)img_id))))
                {
                    T_COMPRESS_IMG_HEADER_FORMAT *p_compressed_header = (T_COMPRESS_IMG_HEADER_FORMAT *)
                                                                        get_temp_ota_bank_addr_by_img_id((IMG_ID)img_id);
                    dfu_set_compressed_ready(p_compressed_header);
                }
                else
                {
                    if (p_header)
                    {
                        dfu_set_ready(p_header);
                    }
                }
#else
                if (p_header)
                {
                    dfu_set_ready(p_header);
                }
#endif
            }
        }
    }

    /*set user data image ready*/
    img_id = IMG_USER_DATA_FIRST;
    for (; img_id < IMG_USER_DATA_MAX; img_id++)
    {
        if (GET_USER_DATA_VALID_BITMAP(img_id))
        {
            p_header = (T_IMG_HEADER_FORMAT *)get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
#if CONFIG_DFU_COMPRESS_OTA
            if (isCompressed((uint8_t *)(get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id))))
            {
                T_COMPRESS_IMG_HEADER_FORMAT *p_compressed_header = (T_COMPRESS_IMG_HEADER_FORMAT *)
                                                                    get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
                dfu_set_compressed_ready(p_compressed_header);
            }
            else
            {
                if (p_header)
                {
                    dfu_set_ready(p_header);
                }
            }
#else
            if (p_header)
            {
                dfu_set_ready(p_header);
            }
#endif
        }
    }
    DFU_PRINT_INFO2("==>dfu_cp_active_reset_handle: is_enter_dfu_mode=%d, is_normal_ota_mode=%d",
                    is_enter_dfu_mode, is_normal_ota_mode);
    if (is_enter_dfu_mode && is_normal_ota_mode)
    {
        //need reset to ota mode
        dfu_active_reset_to_ota_mode = true;
    }

#if defined CONFIG_SOC_SERIES_RTL87X3G
    dfu_bootpatch_always_bank_switch();
#endif

//    le_disconnect(conn_id);

    dfu_active_reset_pending = true;

    dfu_clear_local(OTA_SUCCESS_REBOOT);

}



/**
    * @brief    get image info for ota
    * @param    *p_data   point of input data
    * @param    *data   point of output data
    * @return   void
    */
static void dfu_cp_report_img_info_handle(uint8_t *p_data, uint8_t *p_notify_data) //0x06
{
    uint16_t image_id;
    uint32_t dfu_base_addr;

    if ((p_notify_data == NULL) || (p_data == NULL))
    {
        return;
    }

    LE_ARRAY_TO_UINT16(image_id, p_data);
    DFU_PRINT_TRACE2("dfu_cp_report_img_info_handle: received img_id 0x%x, ota_struct.image_id=0x%x",
                     image_id, ota_struct.image_id);

    if (image_id != ota_struct.image_id)
    {
        ota_struct.cur_offset = 0;
        ota_struct.ota_temp_buf_used_size = 0;
        ota_struct.image_id = image_id;
    }

    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && (image_id < IMG_USER_DATA_FIRST)))
    {
        DFU_PRINT_ERROR1("dfu_cp_report_img_info_handle: received img_id 0x%x invalid!", image_id);

        p_notify_data[0] = DFU_ARV_FAIL_INVALID_PARAMETER;
        return;
    }

    dfu_base_addr = get_temp_ota_bank_img_addr_by_img_id((IMG_ID)image_id);
    if ((dfu_base_addr % FLASH_SECTOR_SIZE) == 0)
    {
        ota_struct.buffer_check_offset = 0;
    }
    else
    {
        ota_struct.buffer_check_offset = dfu_base_addr % FLASH_SECTOR_SIZE;
        dfu_common_flash_read(dfu_base_addr - ota_struct.buffer_check_offset,
                              ota_struct.p_ota_temp_buf_head,
                              ota_struct.buffer_check_offset);
    }

    p_notify_data[0] = DFU_ARV_SUCCESS;
    LE_UINT32_TO_ARRAY(&p_notify_data[1], 0);
    LE_UINT32_TO_ARRAY(&p_notify_data[5], ota_struct.cur_offset);
    LE_UINT16_TO_ARRAY(&p_notify_data[9], ota_struct.buffer_check_offset);

    DFU_PRINT_TRACE2("dfu_cp_report_img_info_handle: Notify cur_offset=0x%x, buffer_check_offset=0x%x",
                     ota_struct.cur_offset, ota_struct.buffer_check_offset);
}


static void dfu_cp_conn_para_update_handle(uint8_t *p_data, uint8_t *p_notify_data) //0x07
{
    uint16_t conn_interval_min;
    uint16_t conn_interval_max;
    uint16_t conn_latency;
    uint16_t superv_tout;

    if ((p_notify_data == NULL) || (p_data == NULL))
    {
        return;
    }

    p_notify_data[0] = DFU_ARV_SUCCESS;

    LE_ARRAY_TO_UINT16(conn_interval_min, p_data);
    LE_ARRAY_TO_UINT16(conn_interval_max, p_data + 2);
    LE_ARRAY_TO_UINT16(conn_latency, p_data + 4);
    LE_ARRAY_TO_UINT16(superv_tout, p_data + 6);
    DFU_PRINT_INFO4("==>le_dfu_cp_conn_para_update_handle: conn_min=0x%x, conn_max=0x%x, latcy=0x%x, timeout=0x%x",
                    conn_interval_min, conn_interval_max, conn_latency, superv_tout);


    if (ota_struct.dfu_conn_para_update_in_progress)
    {
        p_notify_data[0] = DFU_ARV_FAIL_OPERATION;
        DFU_PRINT_INFO0("==>le_dfu_cp_conn_para_update_handle: OTA ConnParaUpdInProgress!");
    }
    else
    {
        if (ble_set_prefer_conn_param(0, conn_interval_min, conn_interval_max, conn_latency,
                                      superv_tout) == GAP_CAUSE_SUCCESS)
        {
            /* Connection Parameter Update Request sent successfully, means this procedure is in progress. */
            ota_struct.dfu_conn_para_update_in_progress = true;
            DFU_PRINT_INFO0("==>le_dfu_cp_conn_para_update_handle: updating");
        }
        else
        {
            p_notify_data[0] = DFU_ARV_FAIL_OPERATION;
            return;
        }
    }
}
/**
    * @brief    Handle buffer check enable cmd
    * @param    p_data      point of output data
    * @param    max_buffer_size     set max buffer size
    * @return   void
    */
static void dfu_cp_buffer_check_en_handle(uint8_t *p_notify_data,
                                          uint16_t max_buffer_size)  //0x09
{
    uint8_t size_factor = 0;

    if (p_notify_data == NULL)
    {
        return;
    }

    ota_struct.ota_flag.buffer_check_en = true;

    while (ota_struct.p_ota_temp_buf_head == NULL)
    {
        ota_struct.buffer_size = max_buffer_size >> size_factor;
        ota_struct.p_ota_temp_buf_head = (uint8_t *)malloc(ota_struct.buffer_size);
        size_factor ++;
        if (ota_struct.buffer_size <= 0)
        {
            break;
        }
    }
#if (CONFIG_APP_NANDBOOT == 1)
    if (ota_struct.buffer_size % FLASH_NAND_PAGE_SIZE != 0)
    {
        //invalid buffer size will ota fail
        //configASSERT(0);  //TODO: need assert function
    }
#endif

    if (ota_struct.p_ota_temp_buf_head == NULL
        || ota_struct.buffer_size == 0)
    {
        ota_struct.ota_flag.buffer_check_en = false;
    }
    p_notify_data[0] = ota_struct.ota_flag.buffer_check_en;
    LE_UINT16_TO_ARRAY(&p_notify_data[1], ota_struct.buffer_size);
    //LE_UINT16_TO_ARRAY(&p_notify_data[3], ota_struct.mtu_size);  //ota verion =4, is rsvd val

    DFU_PRINT_TRACE2("dfu_cp_buffer_check_en: buffer_check_en=%d, buffer_size=%d",
                     ota_struct.ota_flag.buffer_check_en, ota_struct.buffer_size);
}

/**
    * @brief    Handle written request on DFU packet
    * @param    p_data     point of input data
    * @param    data      point of output data
    * @return   void
    */
static void dfu_cp_buffer_check_handle(uint8_t *p_data, uint8_t *data)  //0x0a
{
    uint16_t data_size;
    uint16_t crc;

    if ((p_data == NULL) || (ota_struct.ota_flag.is_ota_process == false))
    {
        ota_struct.ota_temp_buf_used_size = 0;
        data[0] = DFU_ARV_FAIL_OPERATION;
        LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
        return;
    }

    LE_ARRAY_TO_UINT16(data_size, p_data);
    LE_ARRAY_TO_UINT16(crc, p_data + 2);

    DFU_PRINT_TRACE4("===>dfu_cp_buffer_check_handle: data_size=%d, crc=0x%x, ota_temp_buf_used_size=%d, buffer_check_offset=%d",
                     data_size, crc, ota_struct.ota_temp_buf_used_size, ota_struct.buffer_check_offset);

    if (data_size != ota_struct.ota_temp_buf_used_size)
    {
        ota_struct.ota_temp_buf_used_size = 0;
        data[0] = DFU_ARV_FAIL_DATA_LENGTH_ERROR;
        LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
        return;
    }

    if (dfu_check_bufcrc(ota_struct.p_ota_temp_buf_head + ota_struct.buffer_check_offset,
                         ota_struct.ota_temp_buf_used_size, crc))
    {
        ota_struct.ota_temp_buf_used_size = 0;
        ota_struct.buffer_check_offset = 0;
        data[0] = DFU_ARV_FAIL_CRC_ERROR;
        LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
        DFU_PRINT_ERROR0("dfu_cp_buffer_check_handle: buffer check fail!");
    }
    else
    {
        DFU_PRINT_TRACE1("dfu_cp_buffer_check_handle: write flash, offset=0x%x", ota_struct.cur_offset);
        if (dfu_get_enc_setting())
        {
            uint16_t offset = 0;
            while ((ota_struct.ota_temp_buf_used_size - offset) >= 16)
            {
                aes256_decrypt_16byte(ota_struct.p_ota_temp_buf_head + offset);
                offset += 16;
            }
        }

#if (CONFIG_APP_NANDBOOT == 1)
        uint32_t flash_write_result = 0;
        uint32_t flash_write_length = ota_struct.ota_temp_buf_used_size + ota_struct.buffer_check_offset;
        if (flash_write_length % FLASH_NAND_PAGE_SIZE != 0)
        {
            if (ota_struct.buffer_size % FLASH_NAND_PAGE_SIZE == 0)
            {
                memset(ota_struct.p_ota_temp_buf_head + flash_write_length, 0xFF,
                       ota_struct.buffer_size - flash_write_length);
                flash_write_length = ota_struct.buffer_size;
            }
            else
            {
                flash_write_result = __LINE__;
            }
        }
        if (!flash_write_result)
        {
            flash_write_result = dfu_write_data_to_flash(ota_struct.image_id,
                                                         ota_struct.cur_offset,
                                                         ota_struct.next_subimage_offset,
                                                         flash_write_length,
                                                         ota_struct.p_ota_temp_buf_head);
        }
#else
        uint32_t flash_write_result = dfu_write_data_to_flash(ota_struct.image_id,
                                                              ota_struct.cur_offset,
                                                              ota_struct.next_subimage_offset,
                                                              ota_struct.ota_temp_buf_used_size + ota_struct.buffer_check_offset,
                                                              ota_struct.p_ota_temp_buf_head);
#endif

        DFU_PRINT_TRACE1("dfu_cp_buffer_check_handle: write ret %d", flash_write_result);

        if (flash_write_result == 0)
        {
            ota_struct.cur_offset += ota_struct.ota_temp_buf_used_size;
            ota_struct.ota_temp_buf_used_size = 0;
            ota_struct.buffer_check_offset = 0;
            data[0] = DFU_ARV_SUCCESS;
            LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
        }
        else
        {
            uint32_t erase_time = 0;
            uint32_t resend_offset = (ota_struct.next_subimage_offset + ota_struct.cur_offset) /
                                     FLASH_SECTOR_SIZE * FLASH_SECTOR_SIZE;

            if (resend_offset < ota_struct.next_subimage_offset)
            {
                ota_struct.ota_temp_buf_used_size = 0;
                data[0] = DFU_ARV_FAIL_FLASH_ERASE_ERROR;
                LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
                return;
            }

            ota_struct.cur_offset = resend_offset - ota_struct.next_subimage_offset;
            DFU_PRINT_TRACE3("dfu_cp_buffer_check_handle: write fail! cur_offset=0x%x, resend_offset=0x%x, next_subimage_offset=0x%x",
                             ota_struct.cur_offset, resend_offset, ota_struct.next_subimage_offset);

            while (erase_time < 3)
            {
                if (dfu_flash_erase_retry((IMG_ID)ota_struct.image_id, resend_offset) == true)
                {
                    ota_struct.ota_temp_buf_used_size = 0;
                    data[0] = DFU_ARV_FAIL_FLASH_WRITE_ERROR;
                    LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
                    break;
                }
                else
                {
                    erase_time++;
                }
            }
            if (erase_time >= 3)
            {
                ota_struct.ota_temp_buf_used_size = 0;
                data[0] = DFU_ARV_FAIL_FLASH_ERASE_ERROR;
                LE_UINT32_TO_ARRAY(&data[1], ota_struct.cur_offset);
                DFU_PRINT_ERROR0("dfu_cp_buffer_check_handle: erase fail more than three times!");
            }
        }
    }
}

/**
    * @brief    copy image to the other bank
    * @param    p_data    point of input data
    * @return   1: success  other: fail
    */
static uint8_t dfu_cp_copy_img_handle(uint8_t *p_data) //0x0c
{
    uint32_t source_base_addr;
    uint32_t offset = 0;
    uint8_t ret = DFU_ARV_SUCCESS;
    uint16_t img_id;
    uint8_t *buffer_addr = ota_struct.p_ota_temp_buf_head;
    uint32_t buffer_size = MAX_BUFFER_SIZE;
    uint8_t *p_copy_buffer = NULL;
    uint8_t bp_lv;
    uint8_t size_factor = 0;
    uint32_t remain_size;

    ota_struct.ota_flag.skip_flag = 1;

    if (ota_struct.test.t_copy_fail)
    {
        ota_struct.test.t_copy_fail = 0;
        ret = DFU_ARV_FAIL_OPERATION;
        return ret;
    }

    if (p_data == NULL)
    {
        ret = DFU_ARV_FAIL_OPERATION;
        return ret;
    }

    LE_ARRAY_TO_UINT16(img_id, p_data);

    if ((img_id < IMG_DSPSYSTEM) || (img_id >= IMG_DFU_MAX))
    {
        ret = DFU_ARV_FAIL_INVALID_PARAMETER;
        return ret;
    }

    source_base_addr = get_header_addr_by_img_id((IMG_ID)img_id);

    if ((source_base_addr % FLASH_SECTOR_SIZE) || (source_base_addr == 0))
    {
        ret = DFU_ARV_FAIL_OPERATION;
        return ret;
    }

#if CONFIG_DFU_COMPRESS_OTA
    if (isCompressed((uint32_t *)source_base_addr))
    {
        T_COMPRESS_IMG_HEADER_FORMAT *compress_ctrl_header = (T_COMPRESS_IMG_HEADER_FORMAT *)
                                                             source_base_addr;
        T_COMPRESS_IMG_CTRL_HEADER_FORMAT compress_crtl_header;
        dfu_common_flash_read((uint32_t)&compress_ctrl_header->ctrl_header,
                              (uint8_t *)&compress_crtl_header,
                              sizeof(compress_crtl_header));
        if (compress_ctrl_header->ctrl_header.image_id != img_id)
        {
            ret = DFU_ARV_FAIL_OPERATION;
            return ret;
        }

        remain_size = compress_ctrl_header->ctrl_header.payload_len + DEFAULT_HEADER_SIZE;
    }
    else
    {
        T_IMG_HEADER_FORMAT *p_data_header = (T_IMG_HEADER_FORMAT *)source_base_addr;
        T_IMG_CTRL_HEADER_FORMAT ctrl_header;
        dfu_common_flash_read((uint32_t)&p_data_header->ctrl_header, (uint8_t *)&ctrl_header,
                              sizeof(ctrl_header));
        if (ctrl_header.image_id != img_id)
        {
            ret = DFU_ARV_FAIL_OPERATION;
            return ret;
        }

        remain_size = ctrl_header.payload_len + DEFAULT_HEADER_SIZE;
    }

#else
    T_IMG_HEADER_FORMAT *p_data_header = (T_IMG_HEADER_FORMAT *)source_base_addr;
    T_IMG_CTRL_HEADER_FORMAT ctrl_header;
    dfu_common_flash_read((uint32_t)&p_data_header->ctrl_header, (uint8_t *)&ctrl_header,
                          sizeof(ctrl_header));
    if (ctrl_header.image_id != img_id)
    {
        ret = DFU_ARV_FAIL_OPERATION;
        return ret;
    }

    remain_size = ctrl_header.payload_len + DEFAULT_HEADER_SIZE;
#endif

    if (ota_struct.p_ota_temp_buf_head != NULL)
    {
        buffer_size = ota_struct.buffer_size;
    }
    else
    {
        while (p_copy_buffer == NULL && buffer_size)
        {
            buffer_size >>= size_factor;
            p_copy_buffer = (uint8_t *)malloc(buffer_size);
            size_factor ++;
        }

        buffer_addr = p_copy_buffer;
    }

    if (buffer_addr == NULL)
    {
        ret = DFU_ARV_FAIL_OPERATION;
        return ret;
    }

    fmc_flash_nor_get_bp_lv(source_base_addr, &bp_lv);  //use dest addr is more actually

    uint32_t copy_len;
    while (remain_size > 0)
    {
        copy_len = (remain_size > buffer_size) ? buffer_size : remain_size;

        dfu_common_flash_read(source_base_addr + offset, buffer_addr, copy_len);
        if (dfu_write_data_to_flash(img_id, offset, 0, copy_len, buffer_addr) != 0) //check grace
        {
            ret = DFU_ARV_FAIL_FLASH_WRITE_ERROR;
            return ret;
        }
        remain_size -= copy_len;
        offset += copy_len;
    }

    if (dfu_checksum((IMG_ID)img_id, 0) == false)
    {
        ret = DFU_ARV_FAIL_CRC_ERROR;
    }

    if (p_copy_buffer != NULL)
    {
        free(p_copy_buffer);
    }

    fmc_flash_nor_set_bp_lv(flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0), bp_lv);

    DFU_PRINT_TRACE2("dfu_cp_copy_img_handle: ret=%d image_id=0x%x", ret, img_id);

    return ret;
}

/**
    * @brief    compare sha256 or crc value with the image in dest addr
    * @param    p_dest    address of the image
    * @param    p_data    sha256 or crc value receive from controler
    * @param    img_id    image id
    * @return   true: same  false: different
    */
static bool dfu_cmp_checksum(uint8_t *p_dest, uint8_t *p_data, IMG_ID img_id)
{
    uint8_t buffer[SHA256_LEN];
    uint32_t offset = SHA256_OFFSET;
    uint32_t len = SHA256_LEN;

    if (img_id >= IMG_DFU_MAX || img_id < IMG_DFU_FIRST)
    {
        return false;
    }

    dfu_common_flash_read((uint32_t)(p_dest + offset), buffer, len);

    return (memcmp(p_data, buffer, len) == 0);
}

/**
    * @brief    To check if the image in device is same with image needed to update
    * @param    p_data    point of input data
    * @param    data    point of output data
    * @return   void
    */
static void dfu_cp_check_sha256_handle(uint8_t *p_data, uint8_t *p_notify_data) //0x0f
{
    T_IMG_HEADER_FORMAT *img_addr[2];
    uint16_t num = *(uint16_t *)p_data;
    uint8_t buffer[DFU_NOTIFY_LEN_CHECK_SHA256] = {0};

    if (p_notify_data == NULL)
    {
        return;
    }

    SHA256_CHECK *p_src = (SHA256_CHECK *)(p_data + 2);
    DFU_PRINT_TRACE1("===>dfu_cp_check_sha256_handle: img_id=0x%x", p_src->img_id);

    if (p_src->img_id >= IMG_DFU_MAX || p_src->img_id < IMG_DFU_FIRST)
    {
        return;
    }

    for (uint16_t i = 0; i < num; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        *(uint16_t *)buffer = p_src->img_id;

        img_addr[0] = (T_IMG_HEADER_FORMAT *)get_temp_ota_bank_img_addr_by_img_id(p_src->img_id);
        img_addr[1] = (T_IMG_HEADER_FORMAT *)get_active_ota_bank_img_addr_by_img_id(p_src->img_id);

        for (uint8_t j = 0; j < 2; j++)
        {
            if (img_addr[j] && img_addr[j]->ctrl_header.image_id == p_src->img_id
                && dfu_cmp_checksum((uint8_t *)img_addr[j], p_src->sha256, p_src->img_id))
            {
                buffer[2] += (j + 1);
            }
        }
        DFU_PRINT_TRACE2("<===dfu_cp_check_sha256_handle: img_id=0x%x, check resut=%x",
                         *(uint16_t *)buffer, buffer[2]);

        memcpy(p_notify_data, buffer, sizeof(buffer));
        p_notify_data += sizeof(buffer);
        p_src++;
    }
}

/**
    * @brief    Handle written request on DFU packet
    * @param    p_data     data to be written
    * @param    length     Length of value to be written
    * @return   handle result  0x01:success other:fail
    */
static uint8_t dfu_packet_handle(uint8_t *p_data, uint16_t length)
{
    uint8_t results = DFU_ARV_SUCCESS;

    DFU_PRINT_TRACE4("===>dfu_packet_handle: length=%d, nCurOffSet=%d, buffer_check_offset=%d, nImageTotalLength=%d",
                     length, ota_struct.cur_offset, ota_struct.buffer_check_offset, ota_struct.image_total_length);

    ota_struct.ota_flag.skip_flag = 1;

    if ((p_data == NULL) || (ota_struct.ota_flag.is_ota_process == false))
    {
        results = DFU_ARV_FAIL_OPERATION;
        return results;
    }

    if (ota_struct.cur_offset + length + ota_struct.ota_temp_buf_used_size >
        ota_struct.image_total_length)
    {
        results = DFU_ARV_FAIL_INVALID_PARAMETER;
    }
    else
    {
        if (!ota_struct.ota_flag.buffer_check_en)
        {
            if (dfu_get_enc_setting())
            {
                uint16_t offset = 0;
                while ((length - offset) >= 16)
                {
                    aes256_decrypt_16byte(p_data + offset);
                    offset += 16;
                }
            }

            if (dfu_write_data_to_flash(ota_struct.image_id,
                                        ota_struct.cur_offset,
                                        ota_struct.next_subimage_offset,
                                        length,
                                        p_data) != 0)
            {
                results = DFU_ARV_FAIL_OPERATION;
            }
            else
            {
                ota_struct.cur_offset += length;
            }
        }
        else
        {
            DFU_PRINT_TRACE2("===>dfu_packet_handle: bufcheck Enable! ota_temp_buf_used_size=0x%x, buffer_size=0x%x",
                             ota_struct.ota_temp_buf_used_size, ota_struct.buffer_size);
            if (ota_struct.ota_temp_buf_used_size + ota_struct.buffer_check_offset + length <=
                ota_struct.buffer_size)
            {
                memcpy(ota_struct.p_ota_temp_buf_head + ota_struct.buffer_check_offset +
                       ota_struct.ota_temp_buf_used_size, p_data, length);
                ota_struct.ota_temp_buf_used_size += length;
            }
            else
            {
                results = DFU_ARV_FAIL_OPERATION;
            }
        }
    }

    return results;
}

/*============================================================================*
 *                              Public Functions
 *============================================================================*/
/**
    * @brief        Used to get device information
    * @param        ota_info       ble ota mode/spp ota mode and param
    * @param[out]   p_deviceinfo
    * @return       void
    */
void dfu_get_device_info(T_OTA_INFO ota_info, DEVICE_INFO *p_deviceinfo)
{
    if (p_deviceinfo == NULL)
    {
        return;
    }

    memset(p_deviceinfo, 0, sizeof(DEVICE_INFO));
    p_deviceinfo->ic_type = dfu_get_ic_type();
    if (ota_info.ota_mode == BLE_OTA_MODE)
    {
        p_deviceinfo->spec_ver = BLE_OTA_VERSION;
    }
    else if (ota_info.ota_mode == SPP_OTA_MODE)
    {
        p_deviceinfo->spec_ver = SPP_OTA_VERSION;
    }
    p_deviceinfo->mode.buffercheck_en = 1; //ota_struct.test.t_buffercheck_disable ? 0 : 1;
    p_deviceinfo->mode.aes_en = dfu_get_enc_setting();
    p_deviceinfo->mode.aes_mode = 1;
    p_deviceinfo->mode.support_multiimage = 1;
    p_deviceinfo->mode.is_support_normal_ota = CONFIG_DFU_NORMAL_OTA;  //means support normal ota
    p_deviceinfo->mode.is_nand_flash_ota = dfu_check_nand_boot_flag();
#if CONFIG_DFU_COMPRESS_OTA
    p_deviceinfo->mode.compressed_mode = 1;
#else
    p_deviceinfo->mode.compressed_mode = 0;
#endif
    p_deviceinfo->mode.image_ver_8bytes = 1;
    p_deviceinfo->status.b2b_status = SINGLE_DEFAULT;
    p_deviceinfo->status.fc_bud_role = REMOTE_SESSION_ROLE_SINGLE;
#if (CONFIG_APP_NANDBOOT == 1)
    p_deviceinfo->ota_temp_size = 0; //always transport all images and reset once
#else
    p_deviceinfo->ota_temp_size = flash_partition_size_get(PARTITION_FLASH_OTA_TMP) /
                                  FLASH_SECTOR_SIZE; //uint 4KB
#endif
    p_deviceinfo->active_banknum = ota_service_get_active_bank();
#if defined CONFIG_SOC_SERIES_RTL87X3G
    p_deviceinfo->active_bootpatch_banknum = ota_service_get_bootpatch_active_bank();
    p_deviceinfo->image_ctrl_header_offset = 196;
    p_deviceinfo->compressed_image_ctrl_header_offset = 0;
#endif
    if (ota_info.ota_mode == BLE_OTA_MODE)
    {
        le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &p_deviceinfo->mtu_size, ota_info.conn_id);
    }
    else if (ota_info.ota_mode == SPP_OTA_MODE)
    {
        p_deviceinfo->mtu_size = SPP_MTU_SIZE;
    }
    ota_struct.mtu_size = p_deviceinfo->mtu_size;


    ota_struct.ota_flag.is_devinfo = 1;
    ota_struct.ota_flag.is_support_mutil_image = p_deviceinfo->mode.support_multiimage;

    fmc_flash_nor_get_bp_lv(flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0),
                            &ota_struct.bp_level);

#if defined CONFIG_SOC_SERIES_RTL87X3G
    DFU_PRINT_INFO1("<===dfu_get_device_info: active_bootpatch_banknum=%d(0:single bank, 1: dual bank & bank0, 2: dual bank & bank1)",
                    p_deviceinfo->active_bootpatch_banknum);
#endif
    DFU_PRINT_INFO1("<===dfu_get_device_info:  active_banknum=%d(0:single bank, 1: dual bank & bank0, 2: dual bank & bank1)",
                    p_deviceinfo->active_banknum);
    DFU_PRINT_INFO4("<===dfu_get_device_info: aes_en=%d, bufcheck_en=%d, is_nand_flash_ota=%d, is_support_normal_ota=%d)",
                    p_deviceinfo->mode.aes_en, p_deviceinfo->mode.buffercheck_en,
                    p_deviceinfo->mode.is_nand_flash_ota, p_deviceinfo->mode.is_support_normal_ota);
#if CONFIG_DFU_COMPRESS_OTA
    DFU_PRINT_INFO1("<===dfu_get_device_info: compressed_mode=%d",
                    p_deviceinfo->mode.compressed_mode);
#endif
    DFU_PRINT_INFO3("<===dfu_get_device_info: ota_temp_size=%d, mtu_size=%d, bp_lv=%d",
                    p_deviceinfo->ota_temp_size, p_deviceinfo->mtu_size, ota_struct.bp_level);
    DFU_PRINT_INFO2("<===dfu_get_device_info: b2b_status=%d, fc_bud_role=%d",
                    p_deviceinfo->status.b2b_status, p_deviceinfo->status.fc_bud_role);
}

/**
    * @brief    Used to get image version
    * @param    *p_data   point of image version
    * @param    bank   0:active bank    1:inactive bank
    * @return   void
    */
void dfu_get_img_version(uint8_t *p_data, uint8_t bank)
{
    uint8_t num = 0;
    uint8_t buffer[10] = {0};
    //cppcheck-suppress ctuPointerArith
    uint8_t *p_temp = p_data + 1;
    uint32_t addr = 0;
    uint32_t size;

    for (IMG_ID img_id = IMG_DFU_FIRST; img_id < IMG_DFU_MAX; img_id++)
    {
        if (bank == ACTIVE_BANK)
        {
            addr = get_active_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
        }
        else
        {
            addr = get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
        }

        if (img_id == IMG_OTA)
        {
            size = OTA_HEADER_SIZE;
        }
        else if (img_id == PRE_IMG_VP)
        {
            const T_STORAGE_PARTITION_INFO info = storage_partition_get(VP_PARTITION_NAME);
            size = info.size;
        }
        else if (img_id == IMG_BOOTPATCH)
        {
            size = BOOTPATCH_DEFAULT_SIZE;
        }
        else
        {
            size = get_bank_size_by_img_id(img_id);
        }

        if (size == 0 || addr == 0 || size == 0xffffffff)
        {
            continue;
        }

        *(uint16_t *)buffer = img_id;
        *(uint32_t *)&buffer[2] = 0xFFFFFFFF;
        *(uint32_t *)&buffer[6] = 0xFFFFFFFF;

        if (img_id == IMG_OTA)
        {
            dfu_common_flash_read(addr + offsetof(T_IMG_HEADER_FORMAT, git_ver),
                                  (uint8_t *)&buffer[2], 8);
        }
        else if (img_id == IMG_BOOTPATCH || img_id >= IMG_BANK_FIRST)
        {
            uint16_t image_id;
            dfu_common_flash_read(addr + offsetof(T_IMG_HEADER_FORMAT, ctrl_header) +
                                  offsetof(T_IMG_CTRL_HEADER_FORMAT, image_id),
                                  (uint8_t *)&image_id, 2);
            if (image_id == img_id)
            {
                dfu_common_flash_read(addr + offsetof(T_IMG_HEADER_FORMAT, git_ver),
                                      (uint8_t *)&buffer[2], 8);
            }
        }

        num ++;
        memcpy(p_temp, buffer, sizeof(buffer));
        p_temp += sizeof(buffer);
    }

    for (USER_IMG_ID user_data_image_id = IMG_USER_DATA_FIRST; user_data_image_id < IMG_USER_DATA_MAX;
         user_data_image_id++)
    {
        dfu_get_user_data_info(user_data_image_id, &addr, true);
        dfu_get_user_data_info(user_data_image_id, &size, false);

        if (size == 0 || addr == 0 || size == 0xffffffff)
        {
            continue;
        }

        *(uint16_t *)buffer = user_data_image_id;
        dfu_common_flash_read(addr + offsetof(T_IMG_HEADER_FORMAT, git_ver),
                              (uint8_t *)&buffer[2], 8);

        num ++;
        memcpy(p_temp, buffer, sizeof(buffer));
        p_temp += sizeof(buffer);
    }

    *p_data = num;
    DFU_PRINT_INFO2("<===dfu_get_img_version, bank_num:%x img_num:%x", bank, num);
}

/**
    * @brief    Used to get image section size
    * @param    p_data  point of the section size
    * @return   void
    */
void dfu_get_section_size(uint8_t *p_data)
{
    uint8_t num = 0;
    uint8_t buffer[6] = {0};
    //cppcheck-suppress ctuPointerArith
    uint8_t *p_temp = p_data + 1;
    uint32_t size;
    T_IMG_HEADER_FORMAT *ota_header = (T_IMG_HEADER_FORMAT *)get_active_ota_bank_addr();

    for (IMG_ID img_id = IMG_DFU_FIRST; img_id < IMAGE_MAX; img_id++)
    {
        size = get_bank_size_by_img_id(img_id);

        if (size == 0 || size == 0xffffffff)
        {
            continue;
        }

        *(uint16_t *)buffer = img_id;
        *(uint32_t *)&buffer[2] = size;

        num ++;
        memcpy(p_temp, buffer, sizeof(buffer));
        p_temp += sizeof(buffer);
    }

    for (USER_IMG_ID user_data_image_id = IMG_USER_DATA_FIRST; user_data_image_id < IMG_USER_DATA_MAX;
         user_data_image_id++)
    {
        dfu_get_user_data_info(user_data_image_id, &size, false);

        if (size == 0 || size == 0xffffffff)
        {
            continue;
        }

        *(uint16_t *)buffer = user_data_image_id;
        *(uint32_t *)&buffer[2] = size;

        num ++;
        memcpy(p_temp, buffer, sizeof(buffer));
        p_temp += sizeof(buffer);
    }

    *p_data = num;

    DFU_PRINT_INFO1("<===dfu_get_section_size, img_num:%x in active bank", num);
}

void dfu_notify_conn_para_update_req(uint8_t conn_id, uint8_t error_code)
{
    if (ota_struct.dfu_conn_para_update_in_progress == true)
    {
        uint8_t notif_data[DFU_NOTIFY_LEN_CONN_PARA_UPDATE_REQ] = {0};
        ota_struct.dfu_conn_para_update_in_progress = false;

        notif_data[0] = error_code;
        DFU_PRINT_INFO1("<==dfu_notify_conn_para_update_req error_code=0x%x", error_code);
        if (error_code == DFU_ARV_SUCCESS)
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            LE_UINT16_TO_ARRAY(notif_data + 1, conn_interval);
            LE_UINT16_TO_ARRAY(notif_data + 3, conn_slave_latency);
            LE_UINT16_TO_ARRAY(notif_data + 5, conn_supervision_timeout);
        }
        dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_CONN_PARA_UPDATE_REQ,
                                        DFU_NOTIFY_LEN_CONN_PARA_UPDATE_REQ,
                                        notif_data);
    }
}


/**
    * @brief    Handle written request on DFU control point characteristic
    * @param    conn_id     ID to identify the connection
    * @param    length      Length of value to be written
    * @param    p_value     Value to be written
    * @return   T_APP_RESULT
    * @retval   Handle result of this request
    */
T_APP_RESULT dfu_service_handle_cp_req(uint8_t conn_id, uint16_t length, uint8_t *p_value)
{
    T_APP_RESULT cause = APP_RESULT_INVALID_PDU;
    uint8_t results = DFU_ARV_SUCCESS;
    uint8_t opcode = *p_value;
    uint8_t *p = p_value + 1;
    bool error_flag = false;

    DFU_PRINT_INFO2("===>dfu_service_handle_cp_req: opcode=0x%x, length=%d", opcode, length);

    if (opcode > DFU_OPCODE_MIN && opcode <= DFU_OPCODE_REPORT_IMAGE_NUM
        && ota_struct.ota_flag.is_ota_process)
    {
        ota_transfer_timer_start();
    }

    switch (opcode)
    {
    case DFU_OPCODE_START_DFU:
        {
            if (length == DFU_LENGTH_CP_START_DFU)   /* 4 bytes is pending for encrypt */
            {
                /* DFU_PRINT_INFO1("===>dfu_service_handle_cp_req: start dfu data %b", TRACE_BINARY(length,
                                p_value)); */
                cause = APP_RESULT_SUCCESS;
                results = dfu_cp_start_dfu_handle(p);
                dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_START_DFU, sizeof(results), &results);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_RECEIVE_FW_IMAGE_INFO:
        {
            if (length == DFU_LENGTH_CP_RECEIVE_FW_IMAGE_INFO)
            {
                uint16_t image_id;

                LE_ARRAY_TO_UINT16(image_id, p);
                cause = APP_RESULT_SUCCESS;
                if (image_id == ota_struct.image_id)
                {
                    LE_ARRAY_TO_UINT32(ota_struct.cur_offset, p + 2);
                    DFU_PRINT_TRACE2("===>dfu_service_handle_cp_req: image_id=0x%x, cur_offset=%d",
                                     image_id, ota_struct.cur_offset);
                }
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_VALID_FW:
        {
            if (length == DFU_LENGTH_CP_VALID_FW)
            {
                cause = APP_RESULT_SUCCESS;
                results = dfu_cp_valid_handle(p);
                dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_VALID_FW, sizeof(results), &results);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_ACTIVE_IMAGE_RESET:
        {
            if (length == DFU_LENGTH_CP_ACTIVE_IMAGE_RESET)
            {
                bool is_enter_dfu_mode = *p;
                cause = APP_RESULT_SUCCESS;
                dfu_cp_active_reset_handle(is_enter_dfu_mode);
                le_disconnect(conn_id);
                ota_struct.ota_flag.is_ota_process = false;
                os_timer_stop(&ota_struct.timer_handle_ota_transfer);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_SYSTEM_RESET:
        {
            cause = APP_RESULT_SUCCESS;
            dfu_clear_local(OTA_RESET_CMD);
            le_disconnect(conn_id);
        }
        break;
    case DFU_OPCODE_REPORT_TARGET_INFO:
        {
            if (length == DFU_LENGTH_CP_REPORT_TARGET_INFO)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_TARGET_INFO] = {0};

                cause = APP_RESULT_SUCCESS;
                dfu_cp_report_img_info_handle(p, notif_data);
                dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_REPORT_TARGET_INFO,
                                                DFU_NOTIFY_LEN_TARGET_INFO,
                                                notif_data);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_CONN_PARA_UPDATE_REQ:
        {
            if (length == DFU_LENGTH_CP_CONN_PARA_UPDATE_REQ)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_CONN_PARA_UPDATE_REQ] = {0};

                cause = APP_RESULT_SUCCESS;
                dfu_cp_conn_para_update_handle(p, notif_data);
                if (notif_data[0] != DFU_ARV_SUCCESS)
                {
                    dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_CONN_PARA_UPDATE_REQ,
                                                    DFU_NOTIFY_LEN_CONN_PARA_UPDATE_REQ,
                                                    notif_data);
                }
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_BUFFER_CHECK_EN:
        {
            if (length  == DFU_LENGTH_CP_BUFFER_CHECK_EN)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_BUFFER_CHECK_EN] = {0};

                cause = APP_RESULT_SUCCESS;
                dfu_cp_buffer_check_en_handle(notif_data, MAX_BUFFER_SIZE);
                dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_BUFFER_CHECK_EN,
                                                DFU_NOTIFY_LEN_BUFFER_CHECK_EN,
                                                notif_data);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_REPORT_BUFFER_CRC:
        {
            if (length  == DFU_LENGTH_CP_REPORT_BUFFER_CRC)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_BUFFER_CHECK] = {0};

                cause = APP_RESULT_SUCCESS;
                dfu_cp_buffer_check_handle(p, notif_data);
                dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_REPORT_BUFFER_CRC,
                                                DFU_NOTIFY_LEN_BUFFER_CHECK,
                                                notif_data);
            }
            else
            {
                error_flag = true;
            }
        }
        break;
    case DFU_OPCODE_COPY_IMG:
        {
            cause = APP_RESULT_SUCCESS;
            results = dfu_cp_copy_img_handle(p);
            dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_COPY_IMG, sizeof(results), &results);
        }
        break;
    case DFU_OPCODE_GET_IMAGE_VER:
        {
            uint8_t notif_data[DFU_NOTIFY_LEN_IMG_INFO] = {0};

            cause = APP_RESULT_SUCCESS;
            notif_data[0] = *p; //bank_num, 0: active, 1: inactive
            dfu_get_img_version(&notif_data[1], notif_data[0]);
            dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_GET_IMAGE_VER,
                                            2 + 10 * notif_data[1]/*DFU_NOTIFY_LEN_IMG_INFO*/, notif_data);
        }
        break;
    case DFU_OPCODE_GET_SECTION_SIZE:
        {
            uint8_t notif_data[DFU_NOTIFY_LEN_SECTION_SIZE] = {0};

            cause = APP_RESULT_SUCCESS;
            dfu_get_section_size(notif_data);
            dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_GET_SECTION_SIZE,
                                            1 + 6 * notif_data[0]/*DFU_NOTIFY_LEN_SECTION_SIZE*/, notif_data);
        }
        break;
    case DFU_OPCODE_CHECK_SHA256:
        {
            uint8_t notif_data[DFU_NOTIFY_LEN_CHECK_SHA256] = {0};
            uint16_t num = *(uint16_t *)p;
            DFU_PRINT_TRACE1("dfu_service_handle_cp_req: CHECK SHA2556 num=%d", num);

            cause = APP_RESULT_SUCCESS;
            dfu_cp_check_sha256_handle(p, notif_data);
            dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_CHECK_SHA256, DFU_NOTIFY_LEN_CHECK_SHA256,
                                            notif_data); //check grace, send all img sha256 or not?
        }
        break;
    case DFU_OPCODE_GET_RELEASE_VER:
        {
            uint8_t notif_data[DFU_NOTIFY_LEN_RELEASE_VER] = {0};
            uint32_t p_imgheader = get_header_addr_by_img_id(IMG_MCUCONFIG);

            cause = APP_RESULT_SUCCESS;
            memcpy(notif_data, (uint8_t *)(p_imgheader + RELEASE_VER_OFFSET), DFU_NOTIFY_LEN_RELEASE_VER);
            dfu_service_prepare_send_notify(conn_id, DFU_OPCODE_GET_RELEASE_VER, DFU_NOTIFY_LEN_RELEASE_VER,
                                            notif_data);
        }
        break;
    case DFU_OPCODE_TEST_EN:
        {
            cause = APP_RESULT_SUCCESS;
            memcpy(&ota_struct.test.value, p, sizeof(ota_struct.test.value));
        }
        break;
    case DFU_OPCODE_REPORT_IMAGE_NUM:
        {
            uint16_t image_id;
            uint8_t cur_num;
            uint8_t total_num;

            LE_ARRAY_TO_UINT16(image_id, p);
            cur_num = *(p + 2);
            total_num = *(p + 3);
            DFU_PRINT_TRACE3("dfu_service_handle_cp_req: REPORT IMAGE NUM image_id = 0x%x, cur_num = %d, total_num = %d",
                             image_id, cur_num, total_num);

            cause = APP_RESULT_SUCCESS;
        }
        break;
    default:
        DFU_PRINT_ERROR1("dfu_service_handle_cp_req, opcode not expected", opcode);
        break;
    }

    if (error_flag)
    {
        DFU_PRINT_ERROR0("dfu_service_handle_cp_req: invalid length");
    }

    return cause;
}

/**
    * @brief    Handle written request on DFU packet characteristic
    * @param    conn_id     ID to identify the connection
    * @param    length      Length of value to be written
    * @param    p_value     Value to be written
    * @return   T_APP_RESULT
    * @retval   Handle result of this request
    */
T_APP_RESULT dfu_service_handle_packet(uint8_t conn_id, uint16_t length, uint8_t *p_value)
{
    uint8_t result;

    result = dfu_packet_handle(p_value, length);

    if (result == DFU_ARV_FAIL_INVALID_PARAMETER)
    {
        return APP_RESULT_INVALID_PDU;
    }
    else if (result == DFU_ARV_FAIL_OPERATION)
    {
        return APP_RESULT_APP_ERR;
    }

    return APP_RESULT_SUCCESS;
}

/**
    * @brief  handle the active reset event ack
    * @param  event_id  event id of ack
    * @param  status  the status of the ack
    * @return void
    */
void app_ota_cmd_ack_handle(uint16_t event_id, uint8_t status)
{
    APP_PRINT_TRACE2("app_ota_cmd_ack_handle: event_id: %x, status: %d", event_id, status);
    if (status == CMD_SET_STATUS_COMPLETE)
    {
        ota_struct.ota_flag.is_ota_process = false;
        os_timer_stop(&ota_struct.timer_handle_ota_transfer);
        app_bt_policy_event_handle(EVENT_BT_IDLE, NULL);
    }
}

/**
    * @brief  The main function to handle all the spp ota command
    * @param  length length of command id and data
    * @param  p_value data addr
    * @param  app_idx received rx command device index
    * @return void
    */
void app_ota_cmd_handle(uint8_t path, uint16_t length, uint8_t *p_value, uint8_t app_idx)
{
    uint8_t ack_pkt[3];
    uint16_t cmd_id = *(uint16_t *)p_value;
    uint8_t *p;
    uint8_t results = DFU_ARV_SUCCESS;

    bool ack_flag = false;

    ack_pkt[0] = p_value[0];
    ack_pkt[1] = p_value[1];
    ack_pkt[2] = CMD_SET_STATUS_COMPLETE;

    if (length < 2)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
        APP_PRINT_ERROR0("app_ota_cmd_handle: error length");
        return;
    }

    length = length - 2;
    p = p_value + 2;

    APP_PRINT_TRACE2("===>app_ota_cmd_handle, cmd_id:%x, length:%x\n", cmd_id, length);

    if (ota_struct.ota_flag.is_ota_process)
    {
        ota_transfer_timer_start();
    }

    switch (cmd_id)
    {
    case CMD_OTA_DEV_INFO:
        {
            if (length == OTA_LENGTH_OTA_GET_INFO)
            {
                ota_info.ota_mode = SPP_OTA_MODE;
                ota_info.erase_size = SPP_DUF_ERASE_SIZE;
                memcpy(ota_struct.bd_addr, app_db.br_link[app_idx].bd_addr, sizeof(ota_struct.bd_addr));
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                dfu_get_device_info(ota_info, &device_info);
                app_report_event(path, EVENT_OTA_DEV_INFO, app_idx, (uint8_t *)&device_info, sizeof(device_info));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_IMG_VER:
        {
            if (length == OTA_LENGTH_OTA_GET_IMG_VER)
            {
                uint8_t data[IMG_INFO_LEN] = {0};
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                data[0] = *p;
                dfu_get_img_version(&data[1], data[0]);
                app_report_event(path, EVENT_OTA_GET_IMG_VER, app_idx, data,
                                 IMG_INFO_LEN);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_START:
        {
            if (length == OTA_LENGTH_START_OTA)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                results  = dfu_cp_start_dfu_handle(p);
                bt_sniff_mode_disable(ota_struct.bd_addr);
                ota_info.ota_mode = SPP_OTA_MODE;
                ota_info.erase_size = SPP_DUF_ERASE_SIZE;
                app_report_event(path, EVENT_OTA_START, app_idx, &results, sizeof(results));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_PACKET:
        {
            results = dfu_packet_handle(p, length);
//            if (results == DFU_ARV_FAIL_INVALID_PARAMETER)
//            {
//                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
//            }
//            else if (results == DFU_ARV_FAIL_OPERATION)
//            {
//                ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;
//            }
//            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_OTA_VALID:
        {
            if (length == OTA_LENGTH_VALID_FW)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                results = dfu_cp_valid_handle(p);
                app_report_event(path, EVENT_OTA_VALID, app_idx, &results, sizeof(results));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_RESET:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            dfu_clear_local(OTA_RESET_CMD);
        }
        break;
    case CMD_OTA_ACTIVE_RESET:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            bool is_enter_dfu_mode = *p;
            dfu_cp_active_reset_handle(is_enter_dfu_mode);
            app_report_event(path, EVENT_OTA_ACTIVE_ACK, app_idx, &results, 1);
            /*reboot after receiving the event ack*/
        }
        break;
    case CMD_OTA_BUFFER_CHECK_ENABLE:
        {
            if (length  == OTA_LENGTH_BUFFER_CHECK_EN)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_BUFFER_CHECK_EN] = {0};

                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                dfu_cp_buffer_check_en_handle(notif_data, SPP_MAX_BUFFER_SIZE);
                app_report_event(path, EVENT_OTA_BUFFER_CHECK_ENABLE, app_idx, notif_data, sizeof(notif_data));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_BUFFER_CHECK:
        {
            if (length  == OTA_LENGTH_BUFFER_CRC)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_BUFFER_CHECK] = {0};

                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                dfu_cp_buffer_check_handle(p, notif_data);
                app_report_event(path, EVENT_OTA_BUFFER_CHECK, app_idx, notif_data, sizeof(notif_data));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_IMG_INFO:
        {
            if (length == OTA_LENGTH_IMAGE_INFO)
            {
                uint8_t notif_data[DFU_NOTIFY_LEN_TARGET_INFO] = {0};
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                dfu_cp_report_img_info_handle(p, notif_data);
                app_report_event(path, EVENT_OTA_IMG_INFO, app_idx, notif_data, sizeof(notif_data));
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_SECTION_SIZE:
        {
            if (length == OTA_LENGTH_SECTION_SIZE)
            {
                app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
                dfu_get_section_size(section_size);
                app_report_event(path, EVENT_OTA_SECTION_SIZE, app_idx, (uint8_t *)section_size,
                                 SECTION_SIZE_LEN);
            }
            else
            {
                ack_flag = true;
            }
        }
        break;
    case CMD_OTA_PROTOCOL_TYPE:
        {
            uint8_t data[SPEC_VER_LEN] = {0};

            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            *(uint16_t *)data = SPP_PROTOCOL_INFO;
            data[2] = SPP_OTA_VERSION;
            app_report_event(path, EVENT_OTA_PROTOCOL_TYPE, app_idx, (uint8_t *)&data, SPEC_VER_LEN);
        }
        break;
    case CMD_OTA_TEST_EN:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            memcpy(&ota_struct.test.value, p, sizeof(ota_struct.test.value));
        }
        break;
    case CMD_OTA_REPORT_IMAGE_NUM:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            uint16_t image_id;
            uint8_t cur_num;
            uint8_t total_num;

            LE_ARRAY_TO_UINT16(image_id, p);
            cur_num = *(p + 2);
            total_num = *(p + 3);
            DFU_PRINT_TRACE3("dfu_service_handle_cp_req: REPORT IMAGE NUM image_id = 0x%x, cur_num = %d, total_num = %d",
                             image_id, cur_num, total_num);
        }
        break;
    case CMD_ENTER_NORMAL_OTA:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            dfu_switch_to_ota_mode_pending = true;
            os_delay(1000);
            app_bt_policy_event_handle(EVENT_BT_IDLE, NULL);//enter normal OTA after disconnection
        }
        break;
    case CMD_NORMAL_OTA_CHECK:
        {
            app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
            results = normal_ota_mode_check;
            ota_info.ota_mode = SPP_OTA_MODE;
            ota_info.erase_size = SPP_DUF_ERASE_SIZE;
            app_report_event(path, EVENT_NORMAL_OTA_CHECK, app_idx, &results, sizeof(results));
        }
        break;
    default:
        ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
        break;
    }

    if (ack_flag == true)
    {
        APP_PRINT_TRACE0("app_ota_cmd_handle: invalid length");
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        app_report_event(path, EVENT_ACK, app_idx, ack_pkt, 3);
    }
}

/** End of APP_OTA_Exported_Functions
    * @}
    */

/** @} */ /* End of group APP_OTA_SERVICE */
