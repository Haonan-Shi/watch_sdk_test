/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _FLASH_MAP_H_
#define _FLASH_MAP_H_

#define IMG_HDR_SIZE                    0x00000400  //Changable, default 4K, modify in eFuse if needed

#define CFG_FILE_PAYLOAD_LEN            0x00001000  //Fixed


/* ========== High Level Flash Layout Configuration ========== */
#define UPPERSTACK_ADDR                 0x80100000
#define UPPERSTACK_SIZE                 0x0003F000  //252K Bytes
#define OEM_CFG_ADDR                    0x80140000
#define OEM_CFG_SIZE                    0x00001400  //5K Bytes
#define OTA_BANK0_ADDR                  0x80180000
#define OTA_BANK0_SIZE                  0x00480000  //4608K Bytes
#define OTA_BANK1_ADDR                  0x00000000
#define OTA_BANK1_SIZE                  0x00000000  //0K Bytes
#define OTA_TMP_ADDR                    0x80600000
#define OTA_TMP_SIZE                    0x00200000  //2048K Bytes
#define APP_DEFINED_SECTION_ADDR        0x80800000
#define APP_DEFINED_SECTION_SIZE        0x00000000  //0K Bytes
#define USER_DATA1_ADDR                 0x80800000
#define USER_DATA1_SIZE                 0x01400000  //20480K Bytes
#define USER_DATA1_WITH_HEADER          1
#define USER_DATA2_ADDR                 0x00000000
#define USER_DATA2_SIZE                 0x00000000  //0K Bytes
#define USER_DATA2_WITH_HEADER          0
#define USER_DATA3_ADDR                 0x00000000
#define USER_DATA3_SIZE                 0x00000000  //0K Bytes
#define USER_DATA3_WITH_HEADER          0
#define USER_DATA4_ADDR                 0x00000000
#define USER_DATA4_SIZE                 0x00000000  //0K Bytes
#define USER_DATA4_WITH_HEADER          0
#define USER_DATA5_ADDR                 0x00000000
#define USER_DATA5_SIZE                 0x00000000  //0K Bytes
#define USER_DATA5_WITH_HEADER          0
#define USER_DATA6_ADDR                 0x00000000
#define USER_DATA6_SIZE                 0x00000000  //0K Bytes
#define USER_DATA6_WITH_HEADER          0
#define USER_DATA7_ADDR                 0x00000000
#define USER_DATA7_SIZE                 0x00000000  //0K Bytes
#define USER_DATA7_WITH_HEADER          0
#define USER_DATA8_ADDR                 0x00000000
#define USER_DATA8_SIZE                 0x00000000  //0K Bytes
#define USER_DATA8_WITH_HEADER          0
#define FTL_ADDR                        0x81C00000
#define FTL_SIZE                        0x06100000  //99328K Bytes

/* ========== OTA Bank0 Flash Layout Configuration ========== */
#define BANK0_OTA_HDR_ADDR              0x80180000
#define BANK0_OTA_HDR_SIZE              0x00000400  //1K Bytes
#define BANK0_STACK_PATCH_ADDR          0x801C0000
#define BANK0_STACK_PATCH_SIZE          0x00032000  //200K Bytes
#define BANK0_SYS_PATCH_ADDR            0x80200000
#define BANK0_SYS_PATCH_SIZE            0x00022000  //136K Bytes
#define BANK0_APP_ADDR                  0x80240000
#define BANK0_APP_SIZE                  0x00200000  //2048K Bytes
#define BANK0_DSP_SYS_ADDR              0x80440000
#define BANK0_DSP_SYS_SIZE              0x0009E000  //632K Bytes
#define BANK0_DSP_APP_ADDR              0x80500000
#define BANK0_DSP_APP_SIZE              0x00040000  //256K Bytes
#define BANK0_DSP_CFG_ADDR              0x80540000
#define BANK0_DSP_CFG_SIZE              0x0000A000  //40K Bytes
#define BANK0_APP_CFG_ADDR              0x80580000
#define BANK0_APP_CFG_SIZE              0x00002000  //8K Bytes
#define BANK0_EXT_IMG0_ADDR             0x805C0000
#define BANK0_EXT_IMG0_SIZE             0x00032000  //200K Bytes
#define BANK0_EXT_IMG1_ADDR             0x00000000
#define BANK0_EXT_IMG1_SIZE             0x00000000  //0K Bytes
#define BANK0_EXT_IMG2_ADDR             0x00000000
#define BANK0_EXT_IMG2_SIZE             0x00000000  //0K Bytes
#define BANK0_EXT_IMG3_ADDR             0x00000000
#define BANK0_EXT_IMG3_SIZE             0x00000000  //0K Bytes

/* ========== OTA Bank1 Flash Layout Configuration ========== */
#define BANK1_OTA_HDR_ADDR              0x00000000
#define BANK1_OTA_HDR_SIZE              0x00000000  //0K Bytes
#define BANK1_STACK_PATCH_ADDR          0x00000000
#define BANK1_STACK_PATCH_SIZE          0x00000000  //0K Bytes
#define BANK1_SYS_PATCH_ADDR            0x00000000
#define BANK1_SYS_PATCH_SIZE            0x00000000  //0K Bytes
#define BANK1_APP_ADDR                  0x00000000
#define BANK1_APP_SIZE                  0x00000000  //0K Bytes
#define BANK1_DSP_SYS_ADDR              0x00000000
#define BANK1_DSP_SYS_SIZE              0x00000000  //0K Bytes
#define BANK1_DSP_APP_ADDR              0x00000000
#define BANK1_DSP_APP_SIZE              0x00000000  //0K Bytes
#define BANK1_DSP_CFG_ADDR              0x00000000
#define BANK1_DSP_CFG_SIZE              0x00000000  //0K Bytes
#define BANK1_APP_CFG_ADDR              0x00000000
#define BANK1_APP_CFG_SIZE              0x00000000  //0K Bytes
#define BANK1_EXT_IMG0_ADDR             0x00000000
#define BANK1_EXT_IMG0_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG1_ADDR             0x00000000
#define BANK1_EXT_IMG1_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG2_ADDR             0x00000000
#define BANK1_EXT_IMG2_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG3_ADDR             0x00000000
#define BANK1_EXT_IMG3_SIZE             0x00000000  //0K Bytes

/* ========== High Level Psram Layout Configuration ========== */
#define PSRAM_RSV_ADDR                  0x22000000
#define PSRAM_RSV_SIZE                  0x00002000  //8K Bytes
#define PSRAM_UPPERSTACK_ADDR           0x22002000
#define PSRAM_UPPERSTACK_SIZE           0x0003F000  //252K Bytes
#define PSRAM_OEM_CFG_ADDR              0x22041000
#define PSRAM_OEM_CFG_SIZE              0x00001400  //5K Bytes
#define PSRAM_OTA_BANK0_ADDR            0x22042400
#define PSRAM_OTA_BANK0_SIZE            0x00254400  //2385K Bytes
#define PSRAM_APP_DEFINED_SECTION_ADDR  0x00000000
#define PSRAM_APP_DEFINED_SECTION_SIZE  0x00000000  //0K Bytes

/* ========== OTA Bank Psram Layout Configuration ========== */
#define PSRAM_BANK0_OTA_HDR_ADDR        0x22042400
#define PSRAM_BANK0_OTA_HDR_SIZE        0x00000400  //1K Bytes
#define PSRAM_BANK0_STACK_PATCH_ADDR    0x22042800
#define PSRAM_BANK0_STACK_PATCH_SIZE    0x00032000  //200K Bytes
#define PSRAM_BANK0_SYS_PATCH_ADDR      0x22074800
#define PSRAM_BANK0_SYS_PATCH_SIZE      0x00022000  //136K Bytes
#define PSRAM_BANK0_APP_ADDR            0x22096800
#define PSRAM_BANK0_APP_SIZE            0x00200000  //2048K Bytes
#define PSRAM_BANK0_DSP_SYS_ADDR        0x00000000
#define PSRAM_BANK0_DSP_SYS_SIZE        0x00000000  //0K Bytes
#define PSRAM_BANK0_DSP_APP_ADDR        0x00000000
#define PSRAM_BANK0_DSP_APP_SIZE        0x00000000  //0K Bytes
#define PSRAM_BANK0_DSP_CFG_ADDR        0x00000000
#define PSRAM_BANK0_DSP_CFG_SIZE        0x00000000  //0K Bytes
#define PSRAM_BANK0_APP_CFG_ADDR        0x00000000
#define PSRAM_BANK0_APP_CFG_SIZE        0x00000000  //0K Bytes
#define PSRAM_BANK0_EXT_IMG0_ADDR       0x00000000
#define PSRAM_BANK0_EXT_IMG0_SIZE       0x00000000  //0K Bytes
#define PSRAM_BANK0_EXT_IMG1_ADDR       0x00000000
#define PSRAM_BANK0_EXT_IMG1_SIZE       0x00000000  //0K Bytes
#define PSRAM_BANK0_EXT_IMG2_ADDR       0x00000000
#define PSRAM_BANK0_EXT_IMG2_SIZE       0x00000000  //0K Bytes
#define PSRAM_BANK0_EXT_IMG3_ADDR       0x00000000
#define PSRAM_BANK0_EXT_IMG3_SIZE       0x00000000  //0K Bytes

#endif /* _FLASH_MAP_H_ */
