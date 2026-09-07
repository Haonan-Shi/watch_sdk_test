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
#define OEM_CFG_ADDR                    0x70002000
#define OEM_CFG_SIZE                    0x00001400  //5K Bytes
#define BOOT_PATCH_BANK0_ADDR           0x70004000
#define BOOT_PATCH_BANK0_SIZE           0x00003000  //12K Bytes
#define BOOT_PATCH_BANK1_ADDR           0x70007000
#define BOOT_PATCH_BANK1_SIZE           0x00003000  //12K Bytes
#define PLATFORM_EXT_ADDR               0x7000a000
#define PLATFORM_EXT_SIZE               0x00008000  //32K Bytes
#define LOWERSTACK_EXT_ADDR             0x70012000
#define LOWERSTACK_EXT_SIZE             0x0000D000  //52K Bytes
#define UPPERSTACK_ADDR                 0x7001F000
#define UPPERSTACK_SIZE                 0x0003F000  //252K Bytes
#define OTA_BANK0_ADDR                  0x7005E000
#define OTA_BANK0_SIZE                  0x001AC000  //1712K Bytes
#define OTA_BANK1_ADDR                  0x7020A000
#define OTA_BANK1_SIZE                  0x001AC000  //1712K Bytes
#define VP_DATA_ADDR                    0x703B6000
#define VP_DATA_SIZE                    0x00032000  //200K Bytes
#define FTL_ADDR                        0x703E8000
#define FTL_SIZE                        0x00008000  //32K Bytes
#define BKP_DATA1_ADDR                  0x00000000
#define BKP_DATA1_SIZE                  0x00000000  //0K Bytes
#define BKP_DATA2_ADDR                  0x00000000
#define BKP_DATA2_SIZE                  0x00000000  //0K Bytes
#define OTA_TMP_ADDR                    0x00000000
#define OTA_TMP_SIZE                    0x00000000  //0K Bytes

/* ========== OTA Bank0 Flash Layout Configuration ========== */
#define BANK0_OTA_HDR_ADDR              0x7005E000
#define BANK0_OTA_HDR_SIZE              0x00000400  //1K Bytes
#define BANK0_STACK_PATCH_ADDR          0x7005F000
#define BANK0_STACK_PATCH_SIZE          0x00026000  //152K Bytes
#define BANK0_SYS_PATCH_ADDR            0x70085000
#define BANK0_SYS_PATCH_SIZE            0x0001A000  //104K Bytes
#define BANK0_APP_ADDR                  0x7009F000
#define BANK0_APP_SIZE                  0x000DB000  //876K Bytes
#define BANK0_DSP_SYS_ADDR              0x7017A000
#define BANK0_DSP_SYS_SIZE              0x00020000  //128K Bytes
#define BANK0_DSP_APP_ADDR              0x7019A000
#define BANK0_DSP_APP_SIZE              0x00060000  //384K Bytes
#define BANK0_DSP_CFG_ADDR              0x701FA000
#define BANK0_DSP_CFG_SIZE              0x0000A000  //40K Bytes
#define BANK0_APP_CFG_ADDR              0x70204000
#define BANK0_APP_CFG_SIZE              0x00002000  //8K Bytes
#define BANK0_EXT_IMG0_ADDR             0x00000000
#define BANK0_EXT_IMG0_SIZE             0x00000000  //0K Bytes
#define BANK0_EXT_IMG1_ADDR             0x00000000
#define BANK0_EXT_IMG1_SIZE             0x00000000  //0K Bytes
#define BANK0_EXT_IMG2_ADDR             0x00000000
#define BANK0_EXT_IMG2_SIZE             0x00000000  //0K Bytes
#define BANK0_EXT_IMG3_ADDR             0x00000000
#define BANK0_EXT_IMG3_SIZE             0x00000000  //0K Bytes

/* ========== OTA Bank1 Flash Layout Configuration ========== */
#define BANK1_OTA_HDR_ADDR              0x7020A000
#define BANK1_OTA_HDR_SIZE              0x00000400  //1K Bytes
#define BANK1_STACK_PATCH_ADDR          0x7020B000
#define BANK1_STACK_PATCH_SIZE          0x00026000  //152K Bytes
#define BANK1_SYS_PATCH_ADDR            0x70231000
#define BANK1_SYS_PATCH_SIZE            0x0001A000  //104K Bytes
#define BANK1_APP_ADDR                  0x7024B000
#define BANK1_APP_SIZE                  0x000DB000  //876K Bytes
#define BANK1_DSP_SYS_ADDR              0x70326000
#define BANK1_DSP_SYS_SIZE              0x00020000  //128K Bytes
#define BANK1_DSP_APP_ADDR              0x70346000
#define BANK1_DSP_APP_SIZE              0x00060000  //384K Bytes
#define BANK1_DSP_CFG_ADDR              0x703A6000
#define BANK1_DSP_CFG_SIZE              0x0000A000  //40K Bytes
#define BANK1_APP_CFG_ADDR              0x703B0000
#define BANK1_APP_CFG_SIZE              0x00002000  //8K Bytes
#define BANK1_EXT_IMG0_ADDR             0x00000000
#define BANK1_EXT_IMG0_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG1_ADDR             0x00000000
#define BANK1_EXT_IMG1_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG2_ADDR             0x00000000
#define BANK1_EXT_IMG2_SIZE             0x00000000  //0K Bytes
#define BANK1_EXT_IMG3_ADDR             0x00000000
#define BANK1_EXT_IMG3_SIZE             0x00000000  //0K Bytes




#define APP_DEFINED_SECTION_ADDR        0x703F0000
#define APP_DEFINED_SECTION_SIZE        0x0000F000  //60K Bytes
#define USER_DATA1_ADDR                 0x703FF000
#define USER_DATA1_SIZE                 0x00A50000  //10560K Bytes
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


#endif /* _FLASH_MAP_H_ */
