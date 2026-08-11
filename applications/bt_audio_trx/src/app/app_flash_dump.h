/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FLASH_DUMP_H_
#define _APP_FLASH_DUMP_H_


#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
  *                           Header Files
  *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include "app_cmd.h"
#include "app_main.h"

#define CORE_DUMP_ADDR_OFFSET       (flash_cur_bank_img_header_addr_get(FLASH_IMG_STACKPATCH) - get_active_ota_bank_addr()) //start from syspatch
#define CORE_DUMP_SIZE              (0xA9000) // Sample: 676K, need to >= dump_size caculated in app_core_dump_init
#define LOG2FLASH_ADDR_OFFSET       (CORE_DUMP_ADDR_OFFSET + CORE_DUMP_SIZE)
#define LOG2FLASH_SIZE              (0x96000) // Sample: 600K, Or (flash_partition_size_get(PARTITION_FLASH_OTA_BANK_1) - LOG2FLASH_ADDR_OFFSET)

#define FLASH_BLOCK_SIZE            0x1000   // 4K

#define TRANS_DATA_INFO             0x00
#define CONTINUE_TRANS_DATA         0x01
#define END_TRANS_DATA              0x02
#define SUPPORT_IMAGE_TYPE_INFO     0x03

void app_log2flash_init(uint32_t offset, uint32_t flash_size);
void app_flash_dump_handle_event(uint16_t event_id, uint8_t cmd_path, uint8_t app_idx);
void app_flash_dump_handle_cmd(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                               uint16_t cmd_len, uint8_t *ack_pkt);
void app_core_dump_init(uint32_t offset, uint32_t flash_size);


#ifdef __cplusplus
}
#endif

#endif //_APP_FLASH_DUMP_H_
