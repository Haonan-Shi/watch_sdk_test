/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_FLASH_DUMP_SUPPORT
// log2flash
// rssi2flash
#include "app_cfg.h"
#include "trace.h"
#include "app_flash_dump.h"
#include "app_report.h"
#include "app_cmd.h"
#include "app_main.h"
#include "string.h"
#include "stdlib.h"
#include "patch_header_check.h"
#include "fmc_api.h"
#include "bt_types.h"

#if F_APP_LOG2FLASH_SUPPORT
#include "task_log_api.h"
#include "log_api.h"
#endif

#if F_APP_CORE_DUMP_SUPPORT
#include "crash_handle_ext.h"
#include "mem_config.h"
#endif

typedef struct t_dumpflash_db
{
    uint32_t    start_addr;
    uint32_t    dump_size;
    uint32_t    block_addr;
    uint32_t    block_size;
    uint32_t    block_used;
    uint16_t    max_transmit_size;
    uint8_t     cmd_path;
    uint8_t     app_idx;
    uint8_t     *block_data;
} T_DUMPFLASH_DB;

typedef struct
{
    uint32_t flash_data_start_addr_tmp;
    uint32_t flash_data_start_addr;
    uint32_t flash_data_size;
    uint8_t flash_data_type;
} T_FLASH_DATA;

T_FLASH_DATA flash_data;
T_DUMPFLASH_DB dump_flash_db;

#if F_APP_LOG2FLASH_SUPPORT
typedef struct
{
    uint32_t flash_start_addr;
    uint32_t flash_size;
    uint8_t  timestamp[6];
} T_LOG2FLASH_MANAGER;

T_LOG2FLASH_MANAGER log2flash_mgr;
#endif

#if F_APP_CORE_DUMP_SUPPORT
typedef struct
{
    uint32_t flash_start_addr;
    uint32_t flash_size;
} T_CORE_DUMP_MANAGER;

T_CORE_DUMP_MANAGER core_dump_mgr;
#endif

#if F_APP_CORE_DUMP_SUPPORT
void app_core_dump_init(uint32_t offset, uint32_t flash_size)
{
    uint32_t start_addr;

    if ((offset & 0xFFF) || (offset < CORE_DUMP_ADDR_OFFSET))
    {
        offset = CORE_DUMP_ADDR_OFFSET;
    }

    if ((flash_size & 0xFFF) || (flash_size > CORE_DUMP_SIZE))
    {
        flash_size = CORE_DUMP_SIZE;
    }

    if (flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0) == get_active_ota_bank_addr())
    {
        start_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);
    }
    else
    {
        start_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);
    }

#if (CONFIG_SOC_SERIES_RTL8773D == 1)
    uint32_t DATA_RAM_SIZE = 0x20000;
    uint32_t dump_size = SHARE_MEMORY_HEAP_SIZE + SHARE_MEMORY_GLOBAL_SIZE + SHARE_MEMORY_TEXT_SIZE +
                         DATA_RAM_SIZE + COMMON_SRAM1_CFG_SIZE;
#else
    uint32_t dump_size = DATA_RAM_SIZE + CODE_RAM_SIZE + PLATFORM_DTCM0_SIZE + DSP_SHM_TOTAl_SIZE;
#endif
    dump_size += 4 * 1024;
    DEBUG_PRINT_INFO("app_core_dump_init start addr: 0x%x, offset: 0x%x, current_size 0x%x dump_size 0x%x",
                     start_addr, offset, flash_size, dump_size);
    core_dump_mgr.flash_start_addr = start_addr + offset;
    core_dump_mgr.flash_size = flash_size;

    hal_crash_dump_set(core_dump_mgr.flash_start_addr, core_dump_mgr.flash_size);
}
#endif

static bool app_cmd_dump_flash_init(uint32_t start_addr, uint32_t dump_size)
{
    if ((dump_size & 0xFFF) || (start_addr < dump_flash_db.start_addr)
        || (start_addr + dump_size > dump_flash_db.start_addr + dump_flash_db.dump_size))
    {
        APP_PRINT_ERROR4("app_cmd_dump_flash_init failed: flash addr 0x%x, size 0x%x <==> dump addr 0x%x, size 0x%x",
                         dump_flash_db.start_addr, dump_flash_db.dump_size, start_addr, dump_size);
        return false;
    }

    DEBUG_PRINT_INFO("app_cmd_dump_flash_init: flash addr 0x%x, size 0x%x <==> dump addr 0x%x, size 0x%x",
                     dump_flash_db.start_addr, dump_flash_db.dump_size, start_addr, dump_size);
    // start from stack patch addr
    dump_flash_db.start_addr = start_addr;
    dump_flash_db.dump_size = dump_size;
    dump_flash_db.block_addr = dump_flash_db.start_addr;
    dump_flash_db.block_size = FLASH_BLOCK_SIZE;

    if (dump_flash_db.block_data == NULL)
    {
        dump_flash_db.block_data = malloc(dump_flash_db.block_size);
        if (dump_flash_db.block_data == NULL)
        {
            APP_PRINT_ERROR0("app_cmd_dump_flash_init malloc block_data failed!!");
            return false;
        }
    }

    return true;
}

static void app_dump_flash_cmd_ack_handle(uint8_t cmd_path, uint8_t app_idx)
{
    uint32_t dump_size;
    uint32_t remain_size;
    uint8_t *data;
    uint8_t crc_result[2];
    extern T_DUMPFLASH_DB dump_flash_db;

    APP_PRINT_INFO2("app_dump_flash_cmd_ack_handle: cmd_path %d bud_role %d", cmd_path,
                    app_cfg_nv.bud_role);
    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SECONDARY)
    {
        return;
    }

    if (!dump_flash_db.block_used)
    {
        if (!fmc_flash_nor_read(dump_flash_db.block_addr, dump_flash_db.block_data,
                                dump_flash_db.block_size))
        {
            APP_PRINT_ERROR0("app_dump_flash_cmd_ack_handle read flash failed!!");
            return;
        }
    }

    dump_size = dump_flash_db.max_transmit_size > 512 ? 512 : dump_flash_db.max_transmit_size;
    remain_size = dump_flash_db.block_size - dump_flash_db.block_used;
    if (remain_size == 0)
    {
        return;
    }

    if (dump_size > remain_size)
    {
        dump_size = remain_size;
    }

    data = malloc(dump_size + 5);
    if (data != NULL)
    {
        data[0] = 0x01; //Frame_type
        LE_UINT32_TO_ARRAY(&data[1], dump_size);
        memcpy(&data[5], dump_flash_db.block_data + dump_flash_db.block_used, dump_size);
        app_report_event(cmd_path, EVENT_DUMP_FLASH, app_idx, data, dump_size + 5);

        dump_flash_db.block_used += dump_size;
        if (dump_flash_db.block_used == dump_flash_db.block_size)
        {
            uint32_t i;
            uint16_t crc16_checksum = 0;
            uint16_t *data = (uint16_t *)dump_flash_db.block_data;

            for (i = 0; i < dump_flash_db.block_size / 2; ++i)
            {
                crc16_checksum = crc16_checksum ^ (*data);
                ++data;
            }

            crc_result[0] = (uint8_t)crc16_checksum;
            crc_result[1] = (uint8_t)(crc16_checksum >> 8);
            app_report_event(cmd_path, EVENT_FLASH_BUFFER_CHECK, app_idx, crc_result, 2);
        }

        free(data);
    }

    return;
}

void app_flash_dump_handle_event(uint16_t event_id, uint8_t cmd_path, uint8_t app_idx)
{
    if (event_id == EVENT_DUMP_FLASH)
    {
        app_dump_flash_cmd_ack_handle(cmd_path, app_idx);
    }
    else if (event_id == EVENT_REPORT_DUMP_STATE)
    {
        if (dump_flash_db.block_data)
        {
            free(dump_flash_db.block_data);
            dump_flash_db.block_data = NULL;
        }
    }
}

void app_flash_dump_handle_cmd(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                               uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE1("app_flash_dump_handle_cmd: cmd_id 0x%04x", cmd_id);

    switch (cmd_id)
    {
#if F_APP_LOG2FLASH_SUPPORT
    case CMD_LOG2FLASH_STATE_SET:
        {
            uint8_t state = cmd_ptr[2];
            // TODO:
            // app_cmd_show_timestamp(&rssi_flash_header.timestamp, 6);

            if (state == 0x00)
            {
                DEBUG_PRINT_INFO("app_cmd_general_cmd_handle: log2flash state %d, date %d-%d-%d, time %d-%d-%d",
                                 state, log2flash_mgr.timestamp[0] + 1970, log2flash_mgr.timestamp[1] + 1,
                                 log2flash_mgr.timestamp[2],
                                 log2flash_mgr.timestamp[3], log2flash_mgr.timestamp[4], log2flash_mgr.timestamp[5]);
                enable_log_to_flash(false);
                memset(log2flash_mgr.timestamp, 0, sizeof(log2flash_mgr.timestamp));
            }
            else if (state == 0x01)
            {
                // log to flash init
                uint64_t mask[LEVEL_NUM];
                memcpy(log2flash_mgr.timestamp, &cmd_ptr[3], 6);

                /*
                // only enable MODULE_DEBUG
                // TODO: for TEST
                memset(mask, 0, sizeof(mask));
                log_module_trace_init(mask);
                log_module_trace_set(MODULE_DEBUG, LEVEL_ERROR, true);
                log_module_trace_set(MODULE_DEBUG, LEVEL_WARN, true);
                log_module_trace_set(MODULE_DEBUG, LEVEL_INFO, true);
                log_module_trace_set(MODULE_DEBUG, LEVEL_TRACE, true);
                log_enable_trace_string(false);  // disable trace string
                // enable too much log module may cause missing of logs
                */

                enable_log_to_flash(true);
                DEBUG_PRINT_INFO("app_cmd_general_cmd_handle: log2flash state %d, date %d-%d-%d, time %d-%d-%d",
                                 state, log2flash_mgr.timestamp[0] + 1970, log2flash_mgr.timestamp[1] + 1,
                                 log2flash_mgr.timestamp[2],
                                 log2flash_mgr.timestamp[3], log2flash_mgr.timestamp[4], log2flash_mgr.timestamp[5]);
            }
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
#endif
    case CMD_DUMP_FLASH:
        {
            uint32_t start_addr;
            uint32_t dump_size;
            uint8_t data[6];

            LE_ARRAY_TO_UINT32(start_addr, &cmd_ptr[2]);
            LE_ARRAY_TO_UINT32(dump_size, &cmd_ptr[6]);
            data[0] = 0x00;
            data[1] = app_cfg_const.bud_side;

            if (app_cmd_dump_flash_init(start_addr, dump_size))
            {
                if (cmd_path == CMD_PATH_SPP)
                {
                    dump_flash_db.max_transmit_size = app_db.br_link[app_idx].vendor_spp.frame_size - 20;
                }
                else if (cmd_path == CMD_PATH_GATT_OVER_BREDR)
                {
                    dump_flash_db.max_transmit_size = app_db.br_link[app_idx].mtu_size - 20;
                }
                else if (cmd_path == CMD_PATH_LE)
                {
                    dump_flash_db.max_transmit_size = app_db.le_link[app_idx].mtu_size - 20;
                }
            }
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

            APP_PRINT_INFO3("app_cmd_general_cmd_handle: dump_flash start_addr 0x%x, dump_size 0x%x, max_transmit_size %d",
                            dump_flash_db.start_addr, dump_flash_db.dump_size, dump_flash_db.max_transmit_size);
            LE_UINT32_TO_ARRAY(&data[2], dump_flash_db.dump_size);
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            app_report_event(cmd_path, EVENT_DUMP_FLASH, app_idx, data, 6);
        }
        break;

    case CMD_FLASH_CHECK_RESULT:
        {
            uint8_t check_result = cmd_ptr[2];

            if (check_result > 0x01)
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            if (check_result)
            {
                dump_flash_db.block_addr += dump_flash_db.block_size;
                dump_flash_db.block_used = 0;
                if (dump_flash_db.block_addr == dump_flash_db.start_addr + dump_flash_db.dump_size)
                {
                    uint8_t state = 1;

                    app_report_event(cmd_path, EVENT_REPORT_DUMP_STATE, app_idx, &state, 1);
                    break;
                }
            }

            APP_PRINT_INFO2("app_cmd_general_cmd_handle: dump_flash block_addr 0x%x, check_result 0x%x",
                            dump_flash_db.block_addr, check_result);
            app_dump_flash_cmd_ack_handle(cmd_path, app_idx);
        }
        break;

    case CMD_FLASH_STATE_GET:
        {
            uint8_t info_type = cmd_ptr[2];
            uint8_t data[9];
            // uint32_t flash_start_addr = 0;
            // uint32_t flash_size = 0;

            if (dump_flash_db.block_data != NULL)
            {
                free(dump_flash_db.block_data);
            }
            memset(&dump_flash_db, 0, sizeof(T_DUMPFLASH_DB));

            data[0] = info_type;
            if (info_type == 0x00)
            {
                dump_flash_db.start_addr = flash_partition_addr_get(PARTITION_FLASH_FTL);
                dump_flash_db.dump_size = flash_partition_size_get(PARTITION_FLASH_FTL) & 0x00FFFFFF;
            }
#if F_APP_LOG2FLASH_SUPPORT
            else if (info_type == 0x01)
            {
                // need to disable log2flash when dump data from flash
                DEBUG_PRINT_INFO("app_cmd_general_cmd_handle: log2flash, date %d-%d-%d, time %d-%d-%d",
                                 log2flash_mgr.timestamp[0] + 1970, log2flash_mgr.timestamp[1] + 1, log2flash_mgr.timestamp[2],
                                 log2flash_mgr.timestamp[3], log2flash_mgr.timestamp[4], log2flash_mgr.timestamp[5]);
                enable_log_to_flash(false); // Note: disable log2flash before dump log from flash
                memset(log2flash_mgr.timestamp, 0, sizeof(log2flash_mgr.timestamp));
                dump_flash_db.start_addr = log2flash_mgr.flash_start_addr;
                dump_flash_db.dump_size = log2flash_mgr.flash_size;
            }
#endif
#if F_APP_CORE_DUMP_SUPPORT
            else if (info_type == 0x03) // dump core from flash
            {
                dump_flash_db.start_addr = core_dump_mgr.flash_start_addr;
                dump_flash_db.dump_size = core_dump_mgr.flash_size;
            }
#endif
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                break;
            }

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            LE_UINT32_TO_ARRAY(&data[1], dump_flash_db.start_addr);
            LE_UINT32_TO_ARRAY(&data[5], dump_flash_db.dump_size);
            app_report_event(cmd_path, EVENT_FLASH_STATE_REPORT, app_idx, data, 9);
        }
        break;
    case CMD_FEATURE_STATE_GET:
        {
            uint8_t info_type = cmd_ptr[2];
            uint8_t data[10];
            uint8_t data_size = 0;

            data[0] = info_type;
#if F_APP_LOG2FLASH_SUPPORT
            if (info_type == 0x00)  // log2flash
            {
                LE_UINT8_TO_ARRAY(&data[1], log_to_flash_status_get());
                data_size = 1;
            }
            else
#endif
            {
                APP_PRINT_INFO1("app_cmd_general_cmd_handle: CMD_FEATURE_STATE_GET, info_type %d", info_type);
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            if (ack_pkt[2] == CMD_SET_STATUS_COMPLETE)
            {
                app_report_event(cmd_path, EVENT_FEATURE_STATE_GET_RESULT, app_idx, data, data_size + 1);
            }
        }
        break;
    default:
        break;
    }
}
#if F_APP_LOG2FLASH_SUPPORT
void app_log2flash_init(uint32_t offset, uint32_t flash_size)
{
    uint32_t start_addr;

    if ((offset & 0xFFF) || (offset < LOG2FLASH_ADDR_OFFSET))
    {
        offset = LOG2FLASH_ADDR_OFFSET;
    }

    if ((flash_size & 0xFFF) || (flash_size > LOG2FLASH_SIZE))
    {
        flash_size = LOG2FLASH_SIZE;
    }

    if (flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0) == get_active_ota_bank_addr())
    {
        start_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);
    }
    else
    {
        start_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);
    }

    // start from stack patch addr
    log2flash_mgr.flash_start_addr = start_addr + offset;
    log2flash_mgr.flash_size = flash_size;

    fmc_flash_nor_set_bp_lv(log2flash_mgr.flash_start_addr, 0);
#if (CONFIG_SOC_SERIES_RTL8773E == 1)
    log_task_init(log2flash_mgr.flash_start_addr, log2flash_mgr.flash_size, 4000, OS_MEM_TYPE_DATA,
                  false);
#else
    log_task_init(log2flash_mgr.flash_start_addr, log2flash_mgr.flash_size, 4000, OS_MEM_TYPE_DATA,
                  true);
#endif
    enable_log_to_flash(false);
}
#endif
#endif
