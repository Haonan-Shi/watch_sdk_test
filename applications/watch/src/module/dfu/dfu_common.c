/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include <string.h>
#include <stddef.h>
#include "dfu_api.h"
#include "dfu_common.h"
#include "dfu_transport.h"
#include "flash_map.h"
#include "fmc_api.h"
#if (CONFIG_APP_NANDBOOT == 1)
#include "fmc_api_ext.h"
#endif
#include "rtl876x.h"
#include "aon_wdg_ext.h"
#include "wdg.h"
#include "sha256.h"
#include "trace.h"
#include "os_mem.h"
#include "storage.h"




/** @defgroup  APP_OTA_SERVICE APP OTA handle
    * @brief APP OTA Service to implement OTA feature
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
#define SHA256_LENGTH                   32
#define SHA256_BUFFER_SIZE              512
#define READ_BACK_BUFFER_SIZE           64

#define BTAON_FAST_REBOOT_SW_INFO0      0x2

#if (CONFIG_APP_NANDBOOT == 1)
/*TODO: Temp use last block in ota temp bank to record image valid info.
Because nand flash can't write the same address twice after block erase once.
Thus it can't use not_ready bit represent image valid or not.
*/
#define NAND_FLASH_IMAGE_VALID_INFO_ADDR  (OTA_BANK1_ADDR + OTA_BANK1_SIZE - FLASH_NAND_BLOCK_SIZE)
#endif

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup APP_OTA_Exported_Variables APP OTA Exported Variables
    * @brief
    * @{
*/

/*max img num = 32, BIT0: OTA, BIT1: SecureBoot, ..., */
uint32_t valid_bitmap = 0;
PLATFORM_STATIC_ASSERT((IMG_DFU_MAX - IMG_DFU_FIRST) < (sizeof(valid_bitmap) << 3),
                       valid_bitmap);
uint32_t user_data_valid_bitmap = 0;
PLATFORM_STATIC_ASSERT((IMG_USER_DATA_MAX - IMG_USER_DATA_FIRST) < (sizeof(
                                                                        user_data_valid_bitmap) << 3),
                       user_data_valid_bitmap);

/** End of APP_OTA_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Types
 *============================================================================*/
/* 0x02 (BTAON_FAST_REBOOT_SW_INFO0) */
typedef union _BTAON_FAST_REBOOT_SW_INFO0_TYPE
{
    uint16_t d16;
    struct
    {
        uint16_t ota_valid: 1;                  /* bit[0]: ota valid */
        uint16_t is_rom_code_patch: 1;          /* bit[1]: is rom code patch ? */
        uint16_t is_fw_trig_wdg_to: 1;          /* bit[2]: does fw trigger watchdog timeout ? */
        uint16_t is_airplane_mode: 1;           /* bit[3]: does h5 link reset ? */
        uint16_t is_send_patch_end_evt: 1;      /* bit[4]: does we send patch end event ? */
        uint16_t ota_mode: 1;                   /* bit[5]: ota mode */
        uint16_t is_hci_mode: 1;                /* bit[6]: switch to hci mode? */
        uint16_t is_single_tone_mode: 1;        /* bit[7]: reserved */
        uint16_t is_ft_mode: 1;                 /* bit[8]: ft mode */
        uint16_t ota_transport: 1;              /* bit[9]: ota transport, ble : 0 ; spp : 1,*/
        uint16_t REBOOT_SW_INFO1: 6;            /* bit[15:10] for AON_FAST_REBOOT_SW_INFO1 */
    };
} BTAON_FAST_REBOOT_SW_INFO0_TYPE;

typedef struct
{
    union
    {
        uint64_t version;
        struct
        {
            uint64_t _version_reserve: 32;   //!< reserved
            uint64_t _version_revision: 16; //!< revision version
            uint64_t _version_minor: 8;     //!< minor version
            uint64_t _version_major: 8;     //!< major version
        } ota_sub_version;
        struct
        {
            uint64_t _version_reserve: 32;   //!< reserved
            uint64_t _version_revision: 16; //!< revision version
            uint64_t _version_minor: 8;     //!< minor version
            uint64_t _version_major: 8;     //!< major version
        } app_cfg_sub_version;
        struct
        {
            uint64_t _version_reserve: 32;   //!< reserved
            uint64_t _version_revision: 16; //!< revision version
            uint64_t _version_minor: 8;     //!< minor version
            uint64_t _version_major: 8;     //!< major version
        } app_sub_version;
        struct
        {
            uint64_t _version_reserve: 32;   //!< reserved
            uint64_t _version_revision: 16; //!< revision version
            uint64_t _version_minor: 8;     //!< minor version
            uint64_t _version_major: 8;     //!< major version
        } sub_version;
        struct
        {
            uint64_t _version_reserve: 32;   //!< reserved
            uint64_t _version_revision: 16; //!< revision version
            uint64_t _version_minor: 8;     //!< minor version
            uint64_t _version_major: 8;     //!< major version
        } dsp_sub_version;
    } ver_info;
} T_IMAGE_VERSION_FORMAT;

/** @defgroup APP_OTA_Exported_Functions APP OTA service Exported Functions
    * @brief
    * @{
    */


/*============================================================================*
 *                              Private Functions
 *============================================================================*/
extern uint16_t btaon_fast_read_safe(uint16_t offset);
extern void btaon_fast_write_safe(uint16_t offset, uint16_t data);
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

#if CONFIG_DFU_COMPRESS_OTA
static bool dfu_check_compressed_image_sha256(T_COMPRESS_IMG_HEADER_FORMAT *p_header)
{
    uint8_t sha256sum[SHA256_LENGTH];
    uint8_t sha256img[SHA256_LENGTH];
    uint8_t *buf = (uint8_t *)os_mem_alloc(RAM_TYPE_ITCM1, SHA256_BUFFER_SIZE);;
    uint32_t len;
    uint32_t i;
    uint32_t loop_cnt, remain_size;
    uint32_t pdata = (uint32_t)p_header + sizeof(T_COMPRESS_IMG_HEADER_FORMAT);

    len  = p_header->ctrl_header.payload_len;
    loop_cnt = len / SHA256_BUFFER_SIZE;
    remain_size = len % SHA256_BUFFER_SIZE;
    SHA256_CTX ctx = {0};
    SHA256_Init(&ctx);
    for (i = 0; i < loop_cnt; ++i)
    {
        dfu_common_flash_read(pdata, buf, SHA256_BUFFER_SIZE);
        SHA256_Update(&ctx, buf, SHA256_BUFFER_SIZE);
        pdata += SHA256_BUFFER_SIZE;
    }
    if (remain_size)
    {
        dfu_common_flash_read(pdata, buf, remain_size);
        SHA256_Update(&ctx, buf, remain_size);
    }
    SHA256_Final(&ctx, sha256sum);
    os_mem_free(buf);

    dfu_common_flash_read((uint32_t)p_header->compressed_image_sha256, sha256img, SHA256_LENGTH);

    return (memcmp(sha256img, sha256sum, SHA256_LENGTH) == 0);
}

/**
 * @brief  calculated checksum(CRC16 or SHA256 determined by image) over the image, and compared
 *         with given checksum value.
 * @param  p_header image header info of the given image.
 * @return true if image integrity check pass via checksum compare, false otherwise.
*/
static bool check_compressed_image_chksum(T_COMPRESS_IMG_HEADER_FORMAT *p_header)
{
    return dfu_check_compressed_image_sha256(p_header);
}
#endif
/**
 * @brief      Check image sha256
 * @param[in]  p_header   pointer to dfu check image header
 * @return     Check result
 * @retval     true check pass
 * @retval     false check fail
 */
static bool dfu_check_sha256(T_IMG_HEADER_FORMAT *p_header)
{
    uint8_t sha256sum[SHA256_LENGTH];
    uint8_t sha256img[SHA256_LENGTH];
    uint8_t buf[SHA256_BUFFER_SIZE] = {0};
    uint32_t len;
    uint32_t i;
    uint32_t loop_cnt, remain_size;
    uint32_t pdata = (uint32_t)p_header + offsetof(T_AUTH_HEADER_FORMAT, PubKey);

    uint32_t payload_len;
    dfu_common_flash_read((uint32_t)p_header + offsetof(T_IMG_HEADER_FORMAT, ctrl_header)
                          + offsetof(T_IMG_CTRL_HEADER_FORMAT, payload_len), (uint8_t *)&payload_len, 4);

    len = sizeof(T_IMG_HEADER_FORMAT) - offsetof(T_AUTH_HEADER_FORMAT, PubKey) + payload_len;

    loop_cnt = len / SHA256_BUFFER_SIZE;
    remain_size = len % SHA256_BUFFER_SIZE;
    SHA256_CTX ctx = {0};
    SHA256_Init(&ctx);
    for (i = 0; i < loop_cnt; ++i)
    {
        dfu_common_flash_read(pdata, buf, SHA256_BUFFER_SIZE);
#if (CONFIG_APP_NANDBOOT == 0)
        if (i == 0)
        {
            //because not_ready is included when calculate image sha256
            //offset = 68 is the ctrl header offset after image hash
            uint32_t offset = offsetof(T_IMG_HEADER_FORMAT, ctrl_header) - offsetof(T_AUTH_HEADER_FORMAT,
                                                                                    PubKey);
            ((T_IMG_CTRL_HEADER_FORMAT *)(buf + offset))->ctrl_flag.not_ready = 0;
        }
#endif
        SHA256_Update(&ctx, buf, SHA256_BUFFER_SIZE);
        pdata += SHA256_BUFFER_SIZE;
    }
    if (remain_size)
    {
        dfu_common_flash_read(pdata, buf, remain_size);
#if (CONFIG_APP_NANDBOOT == 0)
        if (loop_cnt == 0)
        {
            uint32_t offset = offsetof(T_IMG_HEADER_FORMAT, ctrl_header) - offsetof(T_AUTH_HEADER_FORMAT,
                                                                                    PubKey);
            ((T_IMG_CTRL_HEADER_FORMAT *)(buf + offset))->ctrl_flag.not_ready = 0;
        }
#endif
        SHA256_Update(&ctx, buf, remain_size);
    }
    SHA256_Final(&ctx, sha256sum);

    dfu_common_flash_read((uint32_t)&p_header->auth.image_hash, sha256img, SHA256_LENGTH);

    return (memcmp(sha256img, sha256sum, SHA256_LENGTH) == 0);
}


/*============================================================================*
 *                              Public Functions
 *============================================================================*/
int isCompressed(const void *structPtr)
{
    const char target[3] = "COM";

    const uint8_t *bytePtr = (const uint8_t *)structPtr;

    return memcmp(bytePtr, target, 3) == 0;
}

#if defined CONFIG_SOC_SERIES_RTL87X3G
uint32_t dfu_get_bootpatch_active_bank_num(void)
{
    extern uint32_t boot_patch_bank_num;
    return boot_patch_bank_num;
}
#endif

/**
 * @brief  Get active bank number
 * @param  None
 * @return Active bank number
 */
T_ACTIVE_BANK_NUM dfu_get_active_ota_bank_num(void)
{
    T_ACTIVE_BANK_NUM bank_num = OTA_BANK_MAX;

#if defined CONFIG_SOC_SERIES_RTL87X3G
    extern uint32_t ota_bank_num;
    DFU_PRINT_INFO1("dfu_get_active_ota_bank_num: ota_bank_num %d", ota_bank_num);

    bank_num = (T_ACTIVE_BANK_NUM)ota_bank_num;
#else
    uint32_t ota_bank0_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);
    uint32_t ota_bank1_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);
    uint32_t ota_header_addr = get_active_ota_bank_addr();

    DFU_PRINT_INFO3("dfu_get_active_ota_bank_num: bank0 0x%x, bank1 0x%x, header 0x%x",
                    ota_bank0_addr, ota_bank1_addr, ota_header_addr);
    if (ota_bank0_addr == ota_header_addr)
    {
        bank_num = OTA_BANK0;
    }
    else if (ota_bank1_addr == ota_header_addr)
    {
        bank_num = OTA_BANK1;
    }
    else
    {
        /* invalid case */
        //bank_num = OTA_BANK_MAX;
    }
#endif

    return bank_num;
}

bool dfu_check_nand_boot_flag(void)
{
#if (CONFIG_APP_NANDBOOT == 1)
    return fmc_flash_is_nand_boot();
#else
    return false;
#endif
}

bool dfu_common_flash_read(uint32_t addr, void *data, uint32_t len)
{
    //DFU_PRINT_INFO3("dfu_common_flash_read: addr 0x%x, data 0x%x, len %d", addr, data, len);
#if (CONFIG_APP_NANDBOOT == 1)
    if (dfu_check_nand_boot_flag())
    {
        if (FMC_IS_SPIC1_ADDR(addr))
        {
            memcpy(data, (uint8_t *)addr, len);
            return true;
        }
        else
        {
            return fmc_flash_nand_read(addr, data, len);
        }
    }
    else
#endif
    {
        // uint32_t s = os_lock();
        bool ret = fmc_flash_nor_read(addr, data, len);
        // os_unlock(s);
        return ret;
    }
}

bool dfu_common_flash_write(uint32_t addr, void *data, uint32_t len)
{
    //DFU_PRINT_INFO3("dfu_common_flash_write: addr 0x%x, data 0x%x, len %d", addr, data, len);

#if (CONFIG_APP_NANDBOOT == 1)
    if (dfu_check_nand_boot_flag())
    {
        if (FMC_IS_SPIC1_ADDR(addr))
        {
            memcpy((uint8_t *)addr, data, len);
            return true;
        }
        else
        {
            return fmc_flash_nand_page_write(addr, data, len);
        }
    }
    else
#endif
    {
        // uint32_t s = os_lock();
        bool ret = fmc_flash_nor_write(addr, data, len);
        // os_unlock(s);
        return ret;
    }
}

bool dfu_common_flash_erase(uint32_t addr, DFU_FLASH_ERASE_MODE mode)
{
    //DFU_PRINT_INFO2("dfu_common_flash_erase: addr 0x%x, mode %d", addr, mode);

#if (CONFIG_APP_NANDBOOT == 1)
    if (dfu_check_nand_boot_flag())
    {
        if (FMC_IS_SPIC1_ADDR(addr))
        {
            return true;
        }
        else
        {
            if (DFU_FLASH_ERASE_BLOCK == mode)
            {
                return fmc_flash_nand_erase_block(addr);
            }
            else
            {
                return false;
            }
        }
    }
    else
#endif
    {
        // uint32_t s = os_lock();
        bool ret = fmc_flash_nor_erase(addr, (FMC_FLASH_NOR_ERASE_MODE)mode);
        // os_unlock(s);
        return ret;
    }
}

/**
 * @brief erase a sector of the flash.
 *
 * @param  signature          signature to identify FW.
 * @param  offset             offset of the image.
 * @return  0 if erase successfully, error line number otherwise
*/
uint32_t dfu_flash_erase_retry(uint16_t signature, uint32_t offset)
{
    uint32_t result = 0;
    uint32_t dfu_base_addr;


    dfu_base_addr = get_temp_ota_bank_img_addr_by_img_id((IMG_ID)signature);
    if (dfu_base_addr == 0)
    {
        result = __LINE__;
        goto L_Return;
    }

#if(CONFIG_APP_NANDBOOT == 1)
    result = dfu_common_flash_erase(dfu_base_addr + offset, DFU_FLASH_ERASE_BLOCK);
#else
    result = dfu_common_flash_erase(dfu_base_addr + offset, DFU_FLASH_ERASE_SECTOR);
#endif

L_Return:
    APP_PRINT_INFO1("<==dfu_flash_erase result:%d \r\n", result);
    return result;
}


/**
 * @brief  Get image size of bank area
 * @param  image_id  image ID
 * @return bank size
 */
uint32_t get_bank_size_by_img_id(IMG_ID image_id)
{
    uint32_t bank_size = 0;

    if (image_id < IMG_BANK_FIRST || image_id >= IMAGE_MAX)
    {
        DFU_PRINT_INFO1("get_bank_size_by_img_id: Invalid image ID %d", image_id);
        return 0;
    }

    T_IMG_HEADER_FORMAT *ota_header = (T_IMG_HEADER_FORMAT *)get_active_ota_bank_addr();
#if (CONFIG_APP_NANDBOOT == 1)
    bank_size = ota_header->image_info[(image_id - IMG_SBL)].image_load_size;
#else
    bank_size = ota_header->image_info[(image_id - IMG_SBL)].image_exe_size;
#endif

    return bank_size;
}

/**
 * @brief  Chip reset for dfu
 * @param  reset_mode  reset mode
 * @param  reason      reset reason
 * @return None
 */
void dfu_fw_reboot(T_WDG_MODE reset_mode, T_RESET_REASON reason)
{
    DBG_DIRECT("dfu_fw_reboot: reset_mode %x, reason 0x%x", reset_mode, reason);
    chip_reset(reset_mode);
}

/**
 * @brief  Check ota mode flag, if image need update
 * @param  None
 * @return Result: true: image need update, false: image don't need update
 */
bool dfu_check_ota_mode_flag(void)
{
    BTAON_FAST_REBOOT_SW_INFO0_TYPE nFastBoot =
        (BTAON_FAST_REBOOT_SW_INFO0_TYPE)btaon_fast_read_safe(BTAON_FAST_REBOOT_SW_INFO0);
    DFU_PRINT_INFO1("dfu_check_ota_mode_flag: ota(%d)", nFastBoot.ota_mode);

    return nFastBoot.ota_mode ? true : false;
}

/**
 * @brief  Set ota mode flag
 * @param  enable ota mode flag
 * @return None
 */
void dfu_set_ota_mode_flag(bool enable)
{
    BTAON_FAST_REBOOT_SW_INFO0_TYPE nFastBoot = {.d16 = btaon_fast_read_safe(BTAON_FAST_REBOOT_SW_INFO0)};

    if (enable)
    {
        nFastBoot.ota_mode = 1;
    }
    else
    {
        nFastBoot.ota_mode = 0;
    }
    btaon_fast_write_safe(BTAON_FAST_REBOOT_SW_INFO0, nFastBoot.d16);
    DFU_PRINT_INFO1("dfu_set_ota_mode_flag ota(%d)", nFastBoot.ota_mode);
}

/**
 * @brief  Check whether OTA transport is SPP.
 * @param  None
 * @return Result: true: OTA transport is SPP, false: OTA transport is BLE
 */
bool dfu_check_ota_transport_spp(void)
{
    BTAON_FAST_REBOOT_SW_INFO0_TYPE nFastBoot =
        (BTAON_FAST_REBOOT_SW_INFO0_TYPE)btaon_fast_read_safe(BTAON_FAST_REBOOT_SW_INFO0);
    DFU_PRINT_INFO1("dfu_check_ota_transport_spp: (%d)", nFastBoot.ota_transport);

    return nFastBoot.ota_transport ? true : false;
}

/**
 * @brief  Set ota transport is SPP
 * @param  is_spp SPP flag
 * @return None
 */
void dfu_set_ota_transport_spp(bool is_spp)
{
    BTAON_FAST_REBOOT_SW_INFO0_TYPE nFastBoot = {.d16 = btaon_fast_read_safe(BTAON_FAST_REBOOT_SW_INFO0)};

    if (is_spp)
    {
        nFastBoot.ota_transport = 1;
    }
    else
    {
        nFastBoot.ota_transport = 0;
    }
    btaon_fast_write_safe(BTAON_FAST_REBOOT_SW_INFO0, nFastBoot.d16);
    DFU_PRINT_INFO1("dfu_set_ota_transport_spp (%d)", nFastBoot.ota_transport);
}

/**
 * @brief  Switch to the OTA mode, if support normal ota app need call it.
 * @param  None
 * @return None
 */
void dfu_switch_to_ota_mode(void)
{
    DFU_PRINT_INFO0("==>dfu_switch_to_ota_mode");
    dfu_set_ota_mode_flag(true);

    if (ota_info.ota_mode == SPP_OTA_MODE)
    {
        dfu_set_ota_transport_spp(true);
    }

    dfu_fw_reboot(RESET_ALL_EXCEPT_AON, DFU_SWITCH_TO_OTA_MODE);
}

/**
 * @brief  Get the ic type of current firmware
 * @param  None
 * @return IC type
 */
uint8_t dfu_get_ic_type(void)
{
    uint8_t ic_type = IC_TYPE;
    uint32_t image_addr = get_active_ota_bank_img_addr_by_img_id(IMG_MCUAPP);

    ic_type = ((T_IMG_HEADER_FORMAT *)image_addr)->ctrl_header.ic_type;
    return ic_type;
}

/**
    * @brief    check if ota header exist
    * @param    header_addr    ota header address
    * @return   true: exist; false: not exist
    */
bool check_ota_header(uint32_t header_addr)
{
    if (header_addr == 0)
    {
        return false;
    }
    T_IMG_CTRL_HEADER_FORMAT ctrl_header;
    uint32_t magic_pattern;
    dfu_common_flash_read(header_addr + offsetof(T_IMG_HEADER_FORMAT, ctrl_header),
                          (uint8_t *)&ctrl_header, sizeof(T_IMG_CTRL_HEADER_FORMAT));
    dfu_common_flash_read(header_addr + offsetof(T_IMG_HEADER_FORMAT, magic_pattern),
                          (uint8_t *)&magic_pattern, 4);

    if (magic_pattern != FLASH_TABLE_MAGIC_PATTERN
        || ctrl_header.image_id != IMG_OTA)
    {
        return false;
    }

    return true;
}

#if defined CONFIG_SOC_SERIES_RTL87X3G
uint32_t dfu_get_bootpatch_flash_addr(bool is_active_bank)
{
    uint32_t active_bootpatch_num =  dfu_get_bootpatch_active_bank_num();

    if (active_bootpatch_num >= 2)
    {
        return 0;
    }

#if (CONFIG_APP_NANDBOOT == 1)
    if (is_active_bank)
    {
        return ((active_bootpatch_num == 0) ? NAND_BOOT_PATCH0_ADDR : NAND_BOOT_PATCH1_ADDR);
    }
    else
    {
        return ((active_bootpatch_num == 0) ? NAND_BOOT_PATCH1_ADDR : NAND_BOOT_PATCH0_ADDR);
    }
#else
    if (is_active_bank)
    {
        return ((active_bootpatch_num == 0) ? BOOT_PATCH0_ADDR : BOOT_PATCH1_ADDR);
    }
    else
    {
        return ((active_bootpatch_num == 0) ? BOOT_PATCH1_ADDR : BOOT_PATCH0_ADDR);
    }
#endif
}
#endif

uint32_t dfu_get_otabank_flash_addr(bool is_active_bank)
{
    T_ACTIVE_BANK_NUM active_otabank_num =  dfu_get_active_ota_bank_num();
    uint32_t ota_bank0_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);
    uint32_t ota_bank1_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);

    if (active_otabank_num >= OTA_BANK_MAX)
    {
        return 0;
    }

    if (is_active_bank)
    {
        return ((active_otabank_num == OTA_BANK0) ? ota_bank0_addr : ota_bank1_addr);
    }
    else
    {
        return ((active_otabank_num == OTA_BANK0) ? ota_bank1_addr : ota_bank0_addr);
    }
}

/**
    * @brief    get inactive bank's image address
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_temp_ota_bank_img_addr_by_img_id(IMG_ID image_id)
{
    uint32_t image_addr = 0;
    uint32_t temp_bank_addr;
    uint32_t ota_bank0_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);

    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && ((USER_IMG_ID)image_id < IMG_USER_DATA_FIRST)))
    {
        return image_addr;
    }

    if ((USER_IMG_ID)image_id < IMG_USER_DATA_MAX && (USER_IMG_ID)image_id >= IMG_USER_DATA_FIRST)
    {
        dfu_get_user_data_info((USER_IMG_ID)image_id, &image_addr, true);
        return image_addr;
    }

    if (image_id == PRE_IMG_VP)
    {
        const T_STORAGE_PARTITION_INFO info = storage_partition_get(VP_PARTITION_NAME);
        return info.address;
    }

#if defined CONFIG_SOC_SERIES_RTL87X3G
#if (CONFIG_APP_NANDBOOT == 1)
    if (image_id == IMG_BOOTPATCH)
    {
        image_addr = dfu_get_bootpatch_flash_addr(false);
        return image_addr;
    }
#else
    if (image_id == IMG_BOOTPATCH)
    {
        if (BOOT_PATCH0_ADDR == get_active_boot_patch_addr())
        {
            image_addr = BOOT_PATCH1_ADDR;
        }
        else
        {
            image_addr = BOOT_PATCH0_ADDR;
        }
        return image_addr;
    }
#endif
#endif

    if (!is_ota_support_bank_switch())
    {
        if (image_id == IMG_OTA)
        {
            return 0;
        }
        else
        {
            image_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_TMP);
        }
    }
    else
    {
#if 0   //TODO: Workaround to get inactive bank map with only bank0 ota header for nand flash
#if (CONFIG_APP_NANDBOOT == 1)
        if (image_id == IMG_OTA)
        {
            image_addr = dfu_get_otabank_flash_addr(false);
        }
        else
        {
            temp_bank_addr = ota_bank0_addr;
            if (!check_ota_header(temp_bank_addr))
            {
                return 0;
            }

            uint32_t image_addr0 = ((T_IMG_HEADER_FORMAT *)temp_bank_addr)->nand_image_info[(image_id -
                                                                                            IMG_BANK_FIRST) ]; //bank0 nand addr
            if (image_addr0 == 0xffffffff)
            {
                image_addr0 = 0x0;
            }
            image_addr = image_addr0 + BANK1_OTA_HDR_ADDR - BANK0_OTA_HDR_ADDR;
            // DFU_PRINT_INFO3("image 0x%x: bank0 nand addr 0x%08x, bank1 nand addr 0x%08x", image_id, image_addr0,
            //                 image_addr);

        }
#endif
#else
#if (CONFIG_APP_NANDBOOT == 1)
        if (ota_bank0_addr == dfu_get_otabank_flash_addr(true))
#else
        if (ota_bank0_addr == get_active_ota_bank_addr())
#endif
        {
            temp_bank_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);
        }
        else
        {
            temp_bank_addr = ota_bank0_addr;
        }

        if (image_id == IMG_OTA)
        {
            image_addr = temp_bank_addr;
        }
        else
        {
            if (!check_ota_header(temp_bank_addr))
            {
                return 0;
            }
#if (CONFIG_APP_NANDBOOT == 1)
            dfu_common_flash_read(temp_bank_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                            image_info) + offsetof(T_IMAGE_INFO, image_exe_base)
                                  + (image_id - IMG_BANK_FIRST) * 8, (uint8_t *)&image_addr, 4);
#else
            image_addr = ((T_IMG_HEADER_FORMAT *)temp_bank_addr)->image_info[(image_id -
                                                                              IMG_BANK_FIRST)].image_load_base;
#endif
        }
#endif
    }

    if (image_addr == 0xffffffff)
    {
        return 0;
    }

    return image_addr;
}

/**
 * @brief  Get template ota bank image size by images id
 * @param  image_id  image ID
 * @return Image size
 */
uint32_t get_temp_ota_bank_img_size_by_img_id(IMG_ID image_id)
{
    uint32_t image_size = 0;
    uint32_t temp_bank_addr;
    uint32_t ota_bank0_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0);

    /*not support image id*/
    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && ((USER_IMG_ID)image_id < IMG_USER_DATA_FIRST)))
    {
        //invalid image id
        return image_size;
    }

    if ((USER_IMG_ID)image_id >= IMG_USER_DATA_FIRST && (USER_IMG_ID)image_id < IMG_USER_DATA_MAX)
    {
        dfu_get_user_data_info((USER_IMG_ID)image_id, &image_size, false);
        return image_size;
    }

#if defined CONFIG_SOC_SERIES_RTL87X3G
    if (image_id == IMG_BOOTPATCH)
    {
        return BOOTPATCH_DEFAULT_SIZE;
    }
#endif

    /* IMG_OTA <= image_id < IMG_DFU_MAX, image in bank */
    if (!is_ota_support_bank_switch())
    {
        if (image_id == IMG_OTA)
        {
            return 0;
        }

        image_size = flash_partition_size_get(PARTITION_FLASH_OTA_TMP);
    }
    else
    {
#if 0 //TODO: Workaround for debug
#if (CONFIG_APP_NANDBOOT == 1)
        temp_bank_addr = get_active_ota_bank_addr();
        // DFU_PRINT_INFO3("active bank addr: 0x%08x, bank0 addr: 0x%08x, bank1 addr: 0x%08x",
        //         get_active_ota_bank_addr(), ota_bank0_addr, flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1));  //now all psram address

        if (image_id == IMG_OTA)
        {
            image_size = OTA_HEADER_SIZE;
        }
        else
        {
            if (!check_ota_header(temp_bank_addr))
            {
                return 0;
            }

            image_size = ((T_IMG_HEADER_FORMAT *)temp_bank_addr)->psram_image_info[(image_id - IMG_BANK_FIRST) *
                                                                                                               2 + 1];
        }
#endif
#else
#if (CONFIG_APP_NANDBOOT == 1)
        if (ota_bank0_addr == dfu_get_otabank_flash_addr(true))
#else
        if (ota_bank0_addr == get_active_ota_bank_addr())
#endif
        {
            temp_bank_addr = flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1);
        }
        else
        {
            temp_bank_addr = ota_bank0_addr;
        }

        if (image_id == IMG_OTA)
        {
            image_size = OTA_HEADER_SIZE;
        }
        else
        {
            if (!check_ota_header(temp_bank_addr))
            {
                return 0;
            }

#if (CONFIG_APP_NANDBOOT == 1)
            dfu_common_flash_read(temp_bank_addr + offsetof(T_IMG_HEADER_FORMAT,
                                                            image_info) + offsetof(T_IMAGE_INFO, image_exe_size)
                                  + (image_id - IMG_BANK_FIRST) * 8, (uint8_t *)&image_size, 4);
#else
            image_size = ((T_IMG_HEADER_FORMAT *)temp_bank_addr)->image_info[(image_id -
                                                                              IMG_BANK_FIRST)].image_exe_size;
#endif
        }
#endif
    }

    return image_size;
}

#if (CONFIG_APP_NANDBOOT == 1)
static uint32_t get_active_nand_addr_by_img_id(IMG_ID image_id)
{
    uint32_t image_addr = 0;

    if (image_id < IMG_OCCD || image_id >= IMAGE_MAX)
    {
        return image_addr;
    }

    if (image_id == IMG_OTA) // ota_addr is the same as factory_addr
    {
        image_addr = dfu_get_otabank_flash_addr(true);  //ota_header_addr
    }
    else if (image_id == IMG_BOOTPATCH)
    {
        image_addr = dfu_get_bootpatch_flash_addr(true);
    }
    else
    {
        uint32_t ota_header_addr = get_active_ota_bank_addr();

        if (check_header_valid(ota_header_addr, IMG_OTA) != IMG_CHECK_PASS)
        {
            return image_addr;
        }
        image_addr = *get_image_load_addr_in_bank(ota_header_addr, image_id);
    }

    return image_addr;
}

static uint32_t get_pre_image_nand_addr_by_img_id(PRE_IMG_ID image_id)
{
    uint32_t image_addr = 0;
    if (image_id < PRE_IMG_SYSPATCH || image_id >= PRE_IMAGE_MAX)
    {
        return image_addr;
    }

    if (image_id == PRE_IMG_UPPERSTACK)
    {
        image_addr = UPPERSTACK_ADDR;
    }
    else
    {
        const T_STORAGE_PARTITION_INFO info = storage_partition_get(VP_PARTITION_NAME);
        image_addr = info.address;
    }

    return image_addr;
}

static uint32_t get_pre_image_psram_addr_by_img_id(PRE_IMG_ID image_id)
{
    uint32_t image_addr = 0;

    if (image_id < PRE_IMG_SYSPATCH || image_id >= PRE_IMAGE_MAX)
    {
        return image_addr;
    }

    uint32_t active_bootpatch_addr = dfu_get_bootpatch_flash_addr(true);

    dfu_common_flash_read(active_bootpatch_addr + offsetof(T_IMG_HEADER_FORMAT, image_info) +
                          (image_id - (PRE_IMG_ID)IMAGE_MAX) * 8, (uint8_t *)&image_addr, 4);

    return image_addr;
}

/**
    * @brief    get active bank's image address
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_active_ota_bank_img_nand_addr_by_img_id(IMG_ID image_id)
{
    uint32_t image_addr = 0;

    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && ((USER_IMG_ID)image_id < IMG_USER_DATA_FIRST)))
    {
        return image_addr;
    }

    if ((USER_IMG_ID)image_id < IMG_USER_DATA_MAX && (USER_IMG_ID)image_id >= IMG_USER_DATA_FIRST)
    {
        dfu_get_user_data_info((USER_IMG_ID)image_id, &image_addr, true);
        return image_addr;
    }
#if (CONFIG_APP_NANDBOOT == 1)
    /*to cover case: ota platform ext or upperstack*/
    if (image_id >= PRE_IMG_SYSPATCH && image_id < PRE_IMAGE_MAX)
    {
        image_addr = get_pre_image_nand_addr_by_img_id((PRE_IMG_ID)image_id);
        return image_addr;
    }
#endif

#if (CONFIG_APP_NANDBOOT == 1)
    return get_active_nand_addr_by_img_id(image_id);
#else
    return get_header_addr_by_img_id(image_id);
#endif
}

static uint32_t get_active_psram_addr_by_img_id(IMG_ID image_id)
{
    uint32_t image_addr = 0;

    if (image_id < IMG_OCCD || image_id >= IMAGE_MAX)
    {
        return image_addr;
    }

    if (image_id == IMG_OTA) // ota_addr is the same as factory_addr
    {
        image_addr = get_active_ota_bank_addr();  //ota_header_addr
    }
    else if (image_id == IMG_BOOTPATCH)
    {
        image_addr = dfu_get_bootpatch_flash_addr(true);
    }
    else
    {
        uint32_t ota_header_addr = get_active_ota_bank_addr();

        if (check_header_valid(ota_header_addr, IMG_OTA) != IMG_CHECK_PASS)
        {
            return image_addr;
        }
        image_addr = *get_image_exe_addr_in_bank(ota_header_addr, image_id);
    }

    // FLASH_NOR_IDX_TYPE idx = flash_nor_get_idx_by_addr(image_addr);

    // return flash_nor_get_noncache_addr(idx, image_addr);
#if (CONFIG_APP_NANDBOOT == 1)
    return image_addr;
#else
    return FMC_MAIN1_NON_CACHE_ADDR(image_addr);
#endif
}
#endif

/**
    * @brief    get active bank's image address
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_active_ota_bank_img_addr_by_img_id(IMG_ID image_id)
{
    uint32_t image_addr = 0;

    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && ((USER_IMG_ID)image_id < IMG_USER_DATA_FIRST)))
    {
        return image_addr;
    }

    if ((USER_IMG_ID)image_id < IMG_USER_DATA_MAX && (USER_IMG_ID)image_id >= IMG_USER_DATA_FIRST)
    {
        dfu_get_user_data_info((USER_IMG_ID)image_id, &image_addr, true);
        return image_addr;
    }
#if (CONFIG_APP_NANDBOOT == 1)
    /*to cover case: ota platform ext or upperstack*/
    if (image_id >= PRE_IMG_SYSPATCH && image_id < PRE_IMAGE_MAX)
    {
        image_addr = get_pre_image_psram_addr_by_img_id((PRE_IMG_ID)image_id);
        return image_addr;
    }
#endif

#if (CONFIG_APP_NANDBOOT == 1)
    return get_active_psram_addr_by_img_id(image_id);
#else
    return get_header_addr_by_img_id(image_id);
#endif
}


/**
 * @brief  Get active ota bank image size by images id
 * @param  image_id  image ID
 * @return Image size
 */
uint32_t get_active_ota_bank_img_size_by_img_id(IMG_ID image_id)
{
    uint32_t image_size = 0;
    uint32_t active_bank_addr = get_active_ota_bank_addr();

    /*not support image id*/
    if (image_id < IMG_DFU_FIRST ||
        ((image_id >= IMG_DFU_MAX) && ((USER_IMG_ID)image_id < IMG_USER_DATA_FIRST)))
    {
        //invalid image id
        return image_size;
    }

    if ((USER_IMG_ID)image_id >= IMG_USER_DATA_FIRST && (USER_IMG_ID)image_id < IMG_USER_DATA_MAX)
    {
        dfu_get_user_data_info((USER_IMG_ID)image_id, &image_size, false);
        return image_size;
    }

#if defined CONFIG_SOC_SERIES_RTL87X3G
    if (image_id == IMG_BOOTPATCH)
    {
        return BOOTPATCH_DEFAULT_SIZE;
    }
#endif

    /* IMG_OTA <= image_id < IMG_DFU_MAX, image in bank */
    if (image_id == PRE_IMG_VP)
    {
        const T_STORAGE_PARTITION_INFO info = storage_partition_get(VP_PARTITION_NAME);
        return info.size;
    }
    else
    {
        if (!check_ota_header(active_bank_addr))
        {
            return 0;
        }

        if (image_id == IMG_OTA)
        {
            image_size = OTA_HEADER_SIZE;
        }
        else
        {
#if (CONFIG_APP_NANDBOOT == 1)
            image_size = ((T_IMG_HEADER_FORMAT *)active_bank_addr)->image_info[(image_id -
                                                                                IMG_BANK_FIRST)].image_exe_size;
#else
            image_size = ((T_IMG_HEADER_FORMAT *)active_bank_addr)->image_info[(image_id -
                                                                                IMG_BANK_FIRST)].image_exe_size;
#endif
        }

        return (image_size == 0xFFFFFFFF) ? 0 : image_size;
    }
}

/**
 * @brief  Get active ota bank image version by images id
 * @param  image_id  image ID
 * @return image version
 */
uint64_t dfu_get_active_image_version(IMG_ID image_id)
{
    T_IMG_HEADER_FORMAT *p_header;

    uint64_t version = 0;
    uint32_t size = get_active_ota_bank_img_size_by_img_id(image_id);
    uint32_t addr = get_active_ota_bank_img_addr_by_img_id(image_id);

    if (size == 0 || addr == 0)
    {
        return version;
    }

    p_header = (T_IMG_HEADER_FORMAT *)addr;
    if (image_id == IMG_OTA)
    {
        version = (uint64_t)p_header->common_extra_info.image_release_version;
    }
#if defined CONFIG_SOC_SERIES_RTL87X3G
    else if (image_id == IMG_BOOTPATCH)
    {
        /*bootpatch reused dtcm, must get version from nand flash*/
        uint32_t bootpatch_addr = dfu_get_bootpatch_flash_addr(true);

        dfu_common_flash_read(bootpatch_addr + offsetof(T_IMG_HEADER_FORMAT, git_ver),
                              (uint8_t *)&version, 8);
    }
#endif
    else
    {
        //version = p_header->git_ver.version;
        dfu_common_flash_read(addr + offsetof(T_IMG_HEADER_FORMAT, git_ver),
                              (uint8_t *)&version, 8);
    }
    return version;
}

void dfu_print_all_images_version(void)
{
    IMG_ID image_id;
    T_IMAGE_VERSION_FORMAT image_version;

    for (image_id = IMG_DFU_FIRST; image_id < IMG_DFU_MAX; image_id++)
    {
        uint32_t size = get_active_ota_bank_img_size_by_img_id(image_id);
        if (size == 0)
        {
            continue;
        }

        image_version.ver_info.version = dfu_get_active_image_version(image_id);
        switch (image_id)
        {
        case IMG_OTA:
            {
                DFU_PRINT_INFO6("image:0x%x,version =0x%llx, sub_version:%d.%d.%d.%d", image_id,
                                TRACE_UINT64(image_version.ver_info.version),
                                image_version.ver_info.ota_sub_version._version_major,
                                image_version.ver_info.ota_sub_version._version_minor,
                                image_version.ver_info.ota_sub_version._version_revision,
                                image_version.ver_info.ota_sub_version._version_reserve);
            }
            break;
        case IMG_MCUCONFIG:
            {
                DFU_PRINT_INFO6("image:0x%x,version =0x%llx, sub_version:%d.%d.%d.%d", image_id,
                                TRACE_UINT64(image_version.ver_info.version),
                                image_version.ver_info.app_cfg_sub_version._version_major,
                                image_version.ver_info.app_cfg_sub_version._version_minor,
                                image_version.ver_info.app_cfg_sub_version._version_revision,
                                image_version.ver_info.app_cfg_sub_version._version_reserve);
            }
            break;
        case IMG_MCUAPP:
            {
                DFU_PRINT_INFO6("image:0x%x,version =0x%llx, sub_version:%d.%d.%d.%d", image_id,
                                TRACE_UINT64(image_version.ver_info.version),
                                image_version.ver_info.app_sub_version._version_major,
                                image_version.ver_info.app_sub_version._version_minor,
                                image_version.ver_info.app_sub_version._version_revision,
                                image_version.ver_info.app_sub_version._version_reserve);
            }
            break;
        case IMG_DSPSYSTEM:
        case IMG_DSPAPP:
        case IMG_DSPCONFIG:
            {
                DFU_PRINT_INFO6("image:0x%x,version =0x%llx, sub_version:%d.%d.%d.%d", image_id,
                                TRACE_UINT64(image_version.ver_info.version),
                                image_version.ver_info.dsp_sub_version._version_major,
                                image_version.ver_info.dsp_sub_version._version_minor,
                                image_version.ver_info.dsp_sub_version._version_revision,
                                image_version.ver_info.dsp_sub_version._version_reserve);
            }
            break;
        default:
            {
                DFU_PRINT_INFO6("image:0x%x,version =0x%llx, sub_version:%d.%d.%d.%d", image_id,
                                TRACE_UINT64(image_version.ver_info.version),
                                image_version.ver_info.sub_version._version_major,
                                image_version.ver_info.sub_version._version_minor,
                                image_version.ver_info.sub_version._version_revision,
                                image_version.ver_info.sub_version._version_reserve);
            }
            break;
        }

    }
}

#if defined CONFIG_SOC_SERIES_RTL87X3G
// #if (CONFIG_APP_NANDBOOT == 1)
void dfu_print_active_bootpatch_banknum(void)
{
    uint8_t active_banknum = dfu_get_bootpatch_active_bank_num();

    DFU_PRINT_INFO1("Active Boot Patch: Bank %d", active_banknum);
}
// #else
// void dfu_print_active_bootpatch_banknum(void)
// {
//     uint8_t active_banknum = 0;
//     if (BOOT_PATCH0_ADDR == get_header_addr_by_img_id(IMG_BOOTPATCH))
//     {
//         active_banknum = 0;
//     }
//     else if (BOOT_PATCH1_ADDR == get_header_addr_by_img_id(IMG_BOOTPATCH))
//     {
//         active_banknum = 1;
//     }
//     else
//     {
//         active_banknum = 0xFF;
//     }
//     DFU_PRINT_INFO1("Active Boot Patch: Bank %d", active_banknum);
// }
// #endif
#endif

void dfu_print_flash_map(void)
{
    IMG_ID image_id;
    uint32_t img_addr = 0;
    uint32_t img_size = 0;
#if (CONFIG_APP_NANDBOOT == 1)
    uint32_t nand_img_addr = 0;
#endif

    bool is_enable_bank_switch = is_ota_support_bank_switch();
    APP_PRINT_INFO1("Flash Layout bank switch=%d(0: disable)", is_enable_bank_switch);
    APP_PRINT_INFO1("Active OTA Bank num: %d(0: bank0, 1: bank1)", dfu_get_active_ota_bank_num());
    APP_PRINT_INFO2("OTA Bank0: Addr=0x%08x, size=0x%08x",
                    flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_0),
                    flash_partition_size_get(PARTITION_FLASH_OTA_BANK_0));
    APP_PRINT_INFO2("OTA Bank1: Addr=0x%08x, size=0x%08x",
                    flash_partition_addr_get(PARTITION_FLASH_OTA_BANK_1),
                    flash_partition_size_get(PARTITION_FLASH_OTA_BANK_1));

    for (image_id = IMG_DFU_FIRST; image_id < IMAGE_MAX; image_id++)
    {
        img_addr = get_active_ota_bank_img_addr_by_img_id(image_id);
        img_size = get_active_ota_bank_img_size_by_img_id(image_id);
#if (CONFIG_APP_NANDBOOT == 1)
        nand_img_addr = get_active_ota_bank_img_nand_addr_by_img_id(image_id);
        APP_PRINT_INFO4("Active Bank Image id 0x%x: Nand Addr=0x%08x, Psram Addr=0x%08x, Psram size=0x%08x",
                        image_id, nand_img_addr, img_addr,
                        img_size);
#else
        APP_PRINT_INFO3("Active Bank Image id 0x%x: Addr=0x%08x, size=0x%08x", image_id, img_addr,
                        img_size);
#endif

    }
    if (is_enable_bank_switch)
    {
        for (image_id = IMG_DFU_FIRST; image_id < IMAGE_MAX; image_id++)
        {
            img_addr = get_temp_ota_bank_img_addr_by_img_id(image_id);
            img_size = get_temp_ota_bank_img_size_by_img_id(image_id);
            APP_PRINT_INFO3("Temp Bank Image id 0x%x: Addr=0x%08x, size=0x%08x", image_id, img_addr, img_size);
        }
    }
    DFU_PRINT_INFO2("OCCD:       Addr=0x%08x, size=0x%08x",
                    flash_partition_addr_get(PARTITION_FLASH_OCCD),
                    flash_partition_size_get(PARTITION_FLASH_OCCD));

#if defined CONFIG_SOC_SERIES_RTL87X3G
#if (CONFIG_APP_NANDBOOT == 1)
    DFU_PRINT_INFO2("Boot Patch0: Addr=0x%08x, size=0x%08x",
                    NAND_BOOT_PATCH0_ADDR, NAND_BOOT_PATCH_SIZE);
    DFU_PRINT_INFO2("Boot Patch1: Addr=0x%08x, size=0x%08x",
                    NAND_BOOT_PATCH1_ADDR, NAND_BOOT_PATCH_SIZE);
#else
    DFU_PRINT_INFO2("Boot Patch0: Addr=0x%08x, size=0x%08x",
                    BOOT_PATCH0_ADDR, BOOT_PATCH0_SIZE);
    DFU_PRINT_INFO2("Boot Patch1: Addr=0x%08x, size=0x%08x",
                    BOOT_PATCH1_ADDR, BOOT_PATCH1_SIZE);
#endif
    uint32_t active_bootpatch_addr = dfu_get_bootpatch_flash_addr(true);
    DFU_PRINT_INFO1("Active Boot Patch: Addr=0x%08x", active_bootpatch_addr);
#endif

#if (CONFIG_APP_NANDBOOT == 1)
    for (PRE_IMG_ID pre_image_id = (PRE_IMG_ID)IMAGE_MAX; pre_image_id < PRE_IMAGE_MAX; pre_image_id++)
    {
        dfu_common_flash_read(active_bootpatch_addr + offsetof(T_IMG_HEADER_FORMAT, image_info) +
                              (pre_image_id - (PRE_IMG_ID)IMAGE_MAX) * 8, (uint8_t *)&img_addr, 4);
        dfu_common_flash_read(active_bootpatch_addr + offsetof(T_IMG_HEADER_FORMAT, image_info) +
                              (pre_image_id - (PRE_IMG_ID)IMAGE_MAX) * 8 + 4, (uint8_t *)&img_size, 4);
        APP_PRINT_INFO3("Pre Image id 0x%x: Addr=0x%08x, size=0x%08x", pre_image_id, img_addr,
                        img_size);
    }
#else
    for (PRE_IMG_ID pre_image_id = (PRE_IMG_ID)IMAGE_MAX; pre_image_id < PRE_IMAGE_MAX; pre_image_id++)
    {
        img_addr = *get_image_addr_in_bootpatch(get_header_addr_by_img_id(IMG_BOOTPATCH), pre_image_id);
        img_size = *get_image_size_in_bootpatch(get_header_addr_by_img_id(IMG_BOOTPATCH), pre_image_id);
        APP_PRINT_INFO3("Pre Image id 0x%x: Addr=0x%08x, size=0x%08x", pre_image_id, img_addr,
                        img_size);
    }
#endif

    DFU_PRINT_INFO2("FTL:        Addr=0x%08x, size=0x%08x",
                    flash_partition_addr_get(PARTITION_FLASH_FTL),
                    flash_partition_size_get(PARTITION_FLASH_FTL));
    DFU_PRINT_INFO2("OTA TEMP:   Addr=0x%08x, size=0x%08x",
                    flash_partition_addr_get(PARTITION_FLASH_OTA_TMP),
                    flash_partition_size_get(PARTITION_FLASH_OTA_TMP));
}

/**
* @brief calculate checksum of lenth of buffer in flash.
*
* @param  signature          signature to identify FW.
* @param  offset             offset of the image.
* @param  length             length of data.
* @param  crcValue          ret crc value point.
* @return  0 if buffer checksum calcs successfully, error line number otherwise
*/

uint32_t dfu_check_bufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal)
{
    uint32_t ret = 0;
    uint16_t checksum16 = 0;
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
        ret = __LINE__;
    }

    DFU_PRINT_TRACE2("<==dfu_check_bufcrc: checksum16=0x%x, mCrcVal=%x", checksum16, mCrcVal);

    return ret;
}

/**
    * @brief    write data to flash
    * @param    img_id  image id
    * @param    offset  image offset
    * @param    total_offset  total offset when ota temp mode
    * @param    p_void  point of data
    * @return   0: success; other: fail
    */
uint32_t dfu_write_data_to_flash(uint16_t img_id, uint32_t offset, uint32_t total_offset,
                                 uint32_t length, void *p_void)
{
    uint32_t ret = 0;
    uint32_t dfu_base_addr;
    uint8_t readback_buffer[READ_BACK_BUFFER_SIZE];
    uint32_t read_back_len;
    uint32_t dest_addr;
    uint8_t *p_src = (uint8_t *)p_void;
    uint32_t remain_size = length;
#if (CONFIG_APP_NANDBOOT == 0)
    uint32_t flash_img_addr;
    uint32_t flash_img_size;
#endif
    DFU_PRINT_TRACE3("==>dfu_write_data_to_flash: total_offset=0x%x, offset=%d, length=%d",
                     total_offset, offset, length);

    if (p_void == 0)
    {
        ret = __LINE__;
        goto L_EXIT;
    }

#if (CONFIG_APP_NANDBOOT == 1)
    if (length < (FLASH_NAND_PAGE_SIZE / 4))
    {
        DFU_PRINT_TRACE2("==>dfu_write_data_to_flash: length=%d, check length %d",
                         length, (FLASH_NAND_PAGE_SIZE / 4));
        ret = __LINE__;
        goto L_EXIT;
    }
#endif

    dfu_base_addr = get_temp_ota_bank_img_addr_by_img_id((IMG_ID)img_id);

    if (dfu_base_addr == 0)
    {
        ret = __LINE__;
        goto L_EXIT;
    }

    if (img_id >= IMG_DFU_FIRST && img_id < IMG_DFU_MAX)
    {
        dfu_base_addr += total_offset;
    }
#if (CONFIG_APP_NANDBOOT == 0)
    if (offset == 0)
    {
#if CONFIG_DFU_COMPRESS_OTA
        if (isCompressed(p_void))
        {
            T_COMPRESS_IMG_HEADER_FORMAT *p_header = (T_COMPRESS_IMG_HEADER_FORMAT *)p_void;
            p_header->ctrl_header.ctrl_flag.not_ready = 0x1;
        }
        else
        {
            T_IMG_HEADER_FORMAT *p_header = (T_IMG_HEADER_FORMAT *)p_void;
            p_header->ctrl_header.ctrl_flag.not_ready = 0x1;
        }

#else
        T_IMG_HEADER_FORMAT *p_header = (T_IMG_HEADER_FORMAT *)p_void;
        p_header->ctrl_header.ctrl_flag.not_ready = 0x1;
#endif
    }
#endif

    dest_addr = dfu_base_addr + offset;
    DFU_PRINT_TRACE2("==>dfu_write_data_to_flash:dfu_base_addr=0x%x, dest_addr=0x%x", dfu_base_addr,
                     dest_addr);

#if (CONFIG_APP_NANDBOOT == 1)
    if ((dest_addr % FLASH_NAND_BLOCK_SIZE) == 0)
    {
        dfu_common_flash_erase(dest_addr, DFU_FLASH_ERASE_BLOCK);
    }
    else
    {
        if ((dest_addr / FLASH_NAND_BLOCK_SIZE) != ((dest_addr + length) / FLASH_NAND_BLOCK_SIZE))
        {
            if ((dest_addr + length) % FLASH_NAND_BLOCK_SIZE)
            {
                dfu_common_flash_erase((dest_addr + length) & ~(FLASH_NAND_BLOCK_SIZE - 1),
                                       DFU_FLASH_ERASE_BLOCK);
            }
        }
    }
#else
    if (ota_info.ota_mode == BLE_OTA_MODE)
    {
        if ((dest_addr % FLASH_SECTOR_SIZE) == 0)
        {
            dfu_common_flash_erase(dest_addr, DFU_FLASH_ERASE_SECTOR);
        }
        else
        {
            if ((dest_addr / FLASH_SECTOR_SIZE) != ((dest_addr + length) / FLASH_SECTOR_SIZE))
            {
                if ((dest_addr + length) % FLASH_SECTOR_SIZE)
                {
                    dfu_common_flash_erase((dest_addr + length) & ~(FLASH_SECTOR_SIZE - 1),
                                           DFU_FLASH_ERASE_SECTOR);
                }
            }
        }
    }
    else if (ota_info.erase_size == ERASE_SECTOR_SIZE ||
             img_id == IMG_BOOTPATCH)//boot patch is not in temp and the size is small
    {
        uint32_t start_sector = dest_addr / FLASH_SECTOR_SIZE;
        uint32_t end_sector = (dest_addr + length - 1) / FLASH_SECTOR_SIZE;

        for (uint32_t sector = start_sector; sector <= end_sector; sector++)
        {
            dfu_common_flash_erase(sector * FLASH_SECTOR_SIZE, DFU_FLASH_ERASE_SECTOR);
        }

    }
    else if (ota_info.erase_size == ERASE_BLOCK_SIZE)//user data and temp ota
    {
        uint32_t block_start = dest_addr & ~(FLASH_BLOCK_SIZE - 1);
        uint32_t block_end = block_start + FLASH_BLOCK_SIZE;

        if ((USER_IMG_ID)img_id >= IMG_USER_DATA_FIRST && (USER_IMG_ID)img_id < IMG_USER_DATA_MAX)
        {
            flash_img_addr = get_active_ota_bank_img_addr_by_img_id((IMG_ID)img_id);
            flash_img_size = get_active_ota_bank_img_size_by_img_id((IMG_ID)img_id);
        }
        else
        {
            flash_img_addr = OTA_TMP_ADDR;
            flash_img_size = OTA_TMP_SIZE;
        }

        if ((flash_img_addr > block_start) && (flash_img_addr < block_end))
        {
            // Erase by sector at beginning
            if ((dest_addr + length) <= block_end)
            {
                for (uint8_t i = 0; i < (length + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE; i++)
                {
                    dfu_common_flash_erase(dest_addr + i * FLASH_SECTOR_SIZE, DFU_FLASH_ERASE_SECTOR);
                }
            }
            else
            {
                // Erase a block after erasing sectors
                for (uint8_t i = 0; i < (block_end - dest_addr + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE; i++)
                {
                    dfu_common_flash_erase(dest_addr + i * FLASH_SECTOR_SIZE, DFU_FLASH_ERASE_SECTOR);
                }
                dfu_common_flash_erase(block_end, DFU_FLASH_ERASE_BLOCK);
            }
        }
        else if (((flash_img_addr + flash_img_size - block_end) < FLASH_BLOCK_SIZE) &&
                 (dest_addr + length > block_end))
        {
            // Erase by sector at end
            for (uint8_t i = 0;
                 i < (flash_img_addr + flash_img_size - block_end + FLASH_SECTOR_SIZE - 1) / FLASH_SECTOR_SIZE; i++)
            {
                dfu_common_flash_erase(block_end + i * FLASH_SECTOR_SIZE, DFU_FLASH_ERASE_SECTOR);
            }
        }
        else
        {
            // Erase by block
            if ((dest_addr % FLASH_BLOCK_SIZE) == 0)
            {
                dfu_common_flash_erase(dest_addr, DFU_FLASH_ERASE_BLOCK);
            }
            if (dest_addr + length > block_end)
            {
                dfu_common_flash_erase(block_end, DFU_FLASH_ERASE_BLOCK);
            }
        }
    }
#endif

    dfu_common_flash_write(dest_addr, p_void, length);

    SCB_InvalidateDCache_by_Addr((uint32_t *)dest_addr, length);

    while (remain_size)
    {
        read_back_len = (remain_size >= READ_BACK_BUFFER_SIZE) ? READ_BACK_BUFFER_SIZE : remain_size;
        dfu_common_flash_read(dest_addr, readback_buffer, read_back_len);
        if (memcmp(readback_buffer, p_src, read_back_len) != 0)
        {
            // DFU_PRINT_TRACE1("dest_addr 0x%x", dest_addr);
            // DFU_PRINT_TRACE1("R %b", TRACE_BINARY(read_back_len, readback_buffer));
            // DFU_PRINT_TRACE1("S %b", TRACE_BINARY(read_back_len, p_src));

            ret = __LINE__;
            goto L_EXIT;
        }

        dest_addr += read_back_len;
        p_src += read_back_len;
        remain_size -= read_back_len;
    }

L_EXIT:
    DFU_PRINT_TRACE1("<==dfu_write_data_to_flash: ret=%d", ret);
    return ret;
}

/**
    * @brief    check the integrity of the image
    * @param    img_id    image id
    * @param    offset    address offset
    * @return   true:success ; false: fail
    */
bool dfu_checksum(IMG_ID img_id, uint32_t offset)
{
    bool wdg_en = false;
    bool aon_wdg_en = false;
    T_WDG_MODE bkup_core_wdg_mode = RESET_ALL;
    uint32_t bkup_core_wdg_period = 0;
    uint32_t base_addr = 0;
    uint32_t image_total_length = 0;
    bool ret = false;

    base_addr = get_temp_ota_bank_img_addr_by_img_id(img_id);
    DFU_PRINT_TRACE3("==>dfu_checksum: image_id=0x%x, base_addr=0x%x, offset=%d", img_id, base_addr,
                     offset);

    if (base_addr == 0)
    {
        return false;
    }

    if (img_id >= IMG_DFU_FIRST && img_id < IMG_DFU_MAX)
    {
        base_addr += offset;
    }

#if CONFIG_DFU_COMPRESS_OTA
    T_COMPRESS_IMG_HEADER_FORMAT *p_compressed_header = (T_COMPRESS_IMG_HEADER_FORMAT *)base_addr;
    if (isCompressed(&(p_compressed_header->ctrl_header)))
    {
        image_total_length = p_compressed_header->ctrl_header.payload_len;
    }
    else
    {
        dfu_common_flash_read(base_addr + offsetof(T_IMG_HEADER_FORMAT, ctrl_header) +
                              offsetof(T_IMG_CTRL_HEADER_FORMAT, payload_len),
                              (uint8_t *)&image_total_length, 4);
        image_total_length += sizeof(T_IMG_HEADER_FORMAT);
    }

#else
    dfu_common_flash_read(base_addr + offsetof(T_IMG_HEADER_FORMAT, ctrl_header) +
                          offsetof(T_IMG_CTRL_HEADER_FORMAT, payload_len),
                          (uint8_t *)&image_total_length, 4);
    image_total_length += sizeof(T_IMG_HEADER_FORMAT);
#endif

    /*store wdg config and check wdg enable*/
    wdg_en = wdg_is_enable();
    bkup_core_wdg_period = wdg_get_timeout_ms();
    bkup_core_wdg_mode = wdg_get_mode();
    aon_wdg_en = aon_wdg_is_enable(AON_WDG2);

    /*if ota large img, need modify wdg timeout period*/
    if (wdg_en && image_total_length > 0x100000)
    {
        /*1M and less---4s, 2M and less---8s,..., 8M and less---32s*/
        uint32_t img_align_len = ((image_total_length + (0x100000 - 1)) & (~(0x100000 - 1)));
        uint32_t wdg_period = 4000 * (img_align_len / 0x100000);
        DFU_PRINT_TRACE2("<==dfu_checksum: Change WDG Period to %d ms, image_total_length 0x%x",
                         wdg_period, image_total_length);
        WDG_Start(wdg_period, RESET_ALL);

        if (aon_wdg_en)
        {
            aon_wdg_disable(AON_WDG2);
        }
    }

#if CONFIG_DFU_COMPRESS_OTA
    if (isCompressed(&(p_compressed_header->ctrl_header)))
    {
        ret = check_compressed_image_chksum(p_compressed_header);
    }
    else
    {
        ret = dfu_check_sha256((T_IMG_HEADER_FORMAT *)base_addr);
    }
#else
    ret = dfu_check_sha256((T_IMG_HEADER_FORMAT *)base_addr);
#endif

    if (wdg_en && image_total_length > 0x100000)
    {
        //cppcheck-suppress unknownMacro
        WDG_Start(bkup_core_wdg_period, bkup_core_wdg_mode)
        DFU_PRINT_TRACE2("<==dfu_checksum: Restore WDG Period to %d ms, mode %d",
                         bkup_core_wdg_period, bkup_core_wdg_mode);
    }

    DFU_PRINT_TRACE2("<==dfu_checksum: base_addr=0x%x, ret=%d", base_addr, ret);

    return ret;
}

/**
    * @brief    clear not ready flag of specific image
    * @param    addr    address of the image
    * @return   void
    */
void dfu_set_ready(T_IMG_HEADER_FORMAT *p_header)
{
#if (CONFIG_APP_NANDBOOT == 1)
    DFU_PRINT_TRACE0("==>dfu_set_ready: nand flash TODO!");
#else
    T_IMG_CTRL_HEADER_FORMAT ctrl_header;
    uint32_t ctrl_flag;

    dfu_common_flash_read((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                          sizeof(ctrl_flag));
    DFU_PRINT_TRACE2("==>dfu_set_ready: p_header 0x%x, ctrl_flag 0x%x", p_header, ctrl_flag);

    ctrl_flag &= ~0x4000;
    dfu_common_flash_write((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                           sizeof(ctrl_flag));
    dfu_common_flash_read((uint32_t)&p_header->ctrl_header, (uint8_t *)&ctrl_header,
                          sizeof(ctrl_header));
    DFU_PRINT_TRACE2("<==dfu_set_ready:img_id 0x%x, after ctrl_flag 0x%x", ctrl_header.image_id,
                     ctrl_header.ctrl_flag);
#endif
}

#if CONFIG_DFU_COMPRESS_OTA
/**
 * @brief  set specified image valid bit..
 * @param  p_header      specified image.
 * @return true if ready bit sets to 0, false otherwise
*/
void dfu_set_compressed_ready(T_COMPRESS_IMG_HEADER_FORMAT *p_header)
{
    /* clear not_ready and compressed_not_ready bit */
    T_COMPRESS_IMG_CTRL_HEADER_FORMAT ctrl_header;
    uint16_t ctrl_flag;

    dfu_common_flash_read((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                          sizeof(ctrl_flag));
    DFU_PRINT_TRACE2("==>dfu_set_compressed_ready: p_header 0x%x, ctrl_flag 0x%x", p_header, ctrl_flag);

    ctrl_flag &= ~0x01;
    dfu_common_flash_write((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                           sizeof(ctrl_flag));
    dfu_common_flash_read((uint32_t)&p_header->ctrl_header, (uint8_t *)&ctrl_header,
                          sizeof(ctrl_header));
    DFU_PRINT_TRACE2("<==dfu_set_compressed_ready:img_id 0x%x, after ctrl_flag 0x%x",
                     ctrl_header.image_id,
                     ctrl_header.ctrl_flag);
}
#endif

/**
    * @brief    clear not obsolete flag of specific image
    * @param    addr    address of the image
    * @return   void
    */
void dfu_set_obsolete(T_IMG_HEADER_FORMAT *p_header)
{
#if (CONFIG_APP_NANDBOOT == 1)
    DFU_PRINT_TRACE0("==>dfu_set_obsolete: nand flash TODO!");
#else
    uint16_t ctrl_flag;

    dfu_common_flash_read((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                          sizeof(ctrl_flag));
    ctrl_flag &= ~0x0100;
    dfu_common_flash_write((uint32_t) & (p_header->ctrl_header.ctrl_flag), (uint8_t *)&ctrl_flag,
                           sizeof(ctrl_flag));
#endif
}

T_USER_DATA_ERROR_TYPE dfu_get_user_data_info(USER_IMG_ID image_id,
                                              uint32_t *img_info, bool is_addr)
{
    T_USER_DATA_ERROR_TYPE err_code = USER_DATA_SUCCESS;

    switch (image_id)
    {
    case IMG_USER_DATA1:
        if (USER_DATA1_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA1_ADDR : USER_DATA1_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA2:
        if (USER_DATA2_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA2_ADDR : USER_DATA2_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA3:
        if (USER_DATA3_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA3_ADDR : USER_DATA3_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA4:
        if (USER_DATA4_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA4_ADDR : USER_DATA4_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA5:
        if (USER_DATA5_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA5_ADDR : USER_DATA5_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA6:
        if (USER_DATA6_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA6_ADDR : USER_DATA6_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA7:
        if (USER_DATA7_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA7_ADDR : USER_DATA7_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    case IMG_USER_DATA8:
        if (USER_DATA8_WITH_HEADER)
        {
            *img_info = is_addr ? USER_DATA8_ADDR : USER_DATA8_SIZE;
        }
        else
        {
            err_code = USER_DATA_NOT_SUPPORT_OTA;
            *img_info = 0;
        }
        break;
    default:
        APP_PRINT_ERROR1("error user data image_id 0x%x", image_id);
        err_code = USER_DATA_TYPE_ERROR;
        *img_info = 0;
        break;
    }

    return err_code;
}

/** End of APP_OTA_Exported_Functions
    * @}
    */

/** @} */ /* End of group APP_OTA_SERVICE */
