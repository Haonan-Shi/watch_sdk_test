/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _DFU_COMMON_H_
#define _DFU_COMMON_H_

#include "flash_map.h"
#include "patch_header_check.h"
#include "rtl876x.h"
#include "wdg.h"
/** @defgroup  APP_OTA_SERVICE APP OTA handle
    * @brief APP OTA Service to implement OTA feature
    * @{
    */
/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_OTA_SERVICE_Exported_Macros App OTA service Exported Macros
    * @brief
    * @{
    */
#define FLASH_OFFSET_TO_NO_CACHE                   0x70000000
// if there is no cache, then UNCACHEABLE_ADDR should equal to FMC_MAIN0_ADDR
#define FMC_MAIN0_ADDR                             (0x70000000)
#define FMC_MAIN0_UNCACHEABLE_ADDR                 (0x70000000)
#define FLASH_TABLE_MAGIC_PATTERN                  0x5a5a12a5
#define FLASH_SECTOR_SIZE                          0x1000
#define FLASH_BLOCK_SIZE                           0x10000
#define FMC_MAIN0_NON_CACHE_ADDR(cache_addr)       ((cache_addr) & ~(FMC_MAIN0_ADDR) | (FMC_MAIN0_UNCACHEABLE_ADDR))

#if (CONFIG_APP_NANDBOOT == 1)
#define FLASH_NAND_BLOCK_SIZE                      (128 * 1024)
#define FLASH_NAND_PAGE_SIZE                       (2 * 1024)
#define FLASH_NAND_PAGE_ALIGN_MASK                 (FLASH_NAND_PAGE_SIZE - 1)
#define FLASH_NAND_BLOCK_ALIGN_MASK                (FLASH_NAND_BLOCK_SIZE - 1)

#define FMC_MAIN1_ADDR                             (0x22000000)
#define FMC_MAIN1_UNCACHEABLE_ADDR                 (0x22000000)
#define FMC_MAIN1_SIZE                             ( 32 * 1024 * 1024)
#define FMC_MAIN1_NON_CACHE_ADDR(cache_addr)       ((cache_addr) & ~(FMC_MAIN1_ADDR) | (FMC_MAIN1_UNCACHEABLE_ADDR))


#define FMC_IS_SPIC1_CACHEABLE_ADDR(addr)          ((addr >= FMC_MAIN1_ADDR) && (addr < FMC_MAIN1_ADDR + FMC_MAIN1_SIZE))
#define FMC_IS_SPIC1_UNCACHEABLE_ADDR(addr)        ((addr >= FMC_MAIN1_UNCACHEABLE_ADDR) && (addr < FMC_MAIN1_UNCACHEABLE_ADDR + FMC_MAIN1_SIZE))
#define FMC_IS_SPIC1_ADDR(addr)                    (FMC_IS_SPIC1_CACHEABLE_ADDR(addr) || FMC_IS_SPIC1_UNCACHEABLE_ADDR(addr))

/*nand flash address and size is fixed */
#define NAND_BOOT_PATCH0_ADDR           0x80000000
#define NAND_BOOT_PATCH1_ADDR           0x80040000
#define NAND_BOOT_PATCH_SIZE            0x00005000  //20K Bytes
#endif

#define OTA_HEADER_SIZE                            1024

#define IMG_DFU_FIRST                   IMG_BOOTPATCH
#if (CONFIG_APP_NANDBOOT == 1)
#define BOOTPATCH_DEFAULT_SIZE          NAND_BOOT_PATCH_SIZE
#else
#define BOOTPATCH_DEFAULT_SIZE          BOOT_PATCH0_SIZE
#endif

#define IMG_BANK_FIRST                  IMG_SBL
#define IMG_DFU_MAX                     PRE_IMAGE_MAX

#define PLATFORM_STATIC_ASSERT(condition, identifier) typedef char PALStaticAssert_##identifier[(condition) ? 1 : -1]

#define SET_VALID_BITMAP(image_id)      (valid_bitmap |= BIT(image_id - IMG_DFU_FIRST))
#define GET_VALID_BITMAP(image_id)      (valid_bitmap >> (image_id - IMG_DFU_FIRST) & BIT0)

#define SET_USER_DATA_VALID_BITMAP(image_id)      (user_data_valid_bitmap |= BIT(image_id - IMG_USER_DATA_FIRST))
#define GET_USER_DATA_VALID_BITMAP(image_id)      (user_data_valid_bitmap >> (image_id - IMG_USER_DATA_FIRST) & BIT0)


/** End of APP_OTA_SERVICE_Exported_Macros
    * @}
    */


/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup APP_OTA_SERVICE_Exported_Types APP OTA Service Exported Types
    * @brief
    * @{
    */
typedef enum
{
    DFU_ACTIVE_RESET = 0xd0,
    DFU_SYSTEM_RESET = 0xd1,
    DFU_TIMEOUT,
    DFU_SWITCH_TO_OTA_MODE,
    DFU_LINK_LOST,
} T_RESET_REASON;

typedef enum
{
    USER_DATA_SUCCESS = 0,
    USER_DATA_NOT_SUPPORT_OTA,
    USER_DATA_TYPE_ERROR,
} T_USER_DATA_ERROR_TYPE;

typedef enum
{
    OTA_BANK0,
    OTA_BANK1,
    OTA_BANK_MAX,
} T_ACTIVE_BANK_NUM;

typedef enum
{
    DFU_FLASH_ERASE_CHIP   = 1,
    DFU_FLASH_ERASE_SECTOR = 2,
    DFU_FLASH_ERASE_BLOCK  = 4,
} DFU_FLASH_ERASE_MODE;

/** End of APP_OTA_SERVICE_Exported_Types
    * @}
    */
extern uint32_t valid_bitmap;
extern uint32_t user_data_valid_bitmap;
/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_OTA_SERVICE_Exported_Functions APP OTA service Functions
    * @brief
    * @{
    */
#if defined CONFIG_SOC_SERIES_RTL87X3G
uint32_t dfu_get_bootpatch_active_bank_num(void);
#endif
/**
 * @brief  Check nand boot flag
 * @param  None
 * @return Result: true: nand flash boot, false: nor flash boot
 */
bool dfu_check_nand_boot_flag(void);

/**
 * @brief  Get image size of bank area
 * @param  image_id  image ID
 * @return bank size
 */
uint32_t get_bank_size_by_img_id(IMG_ID image_id);

/**
 * @brief  Chip reset for dfu
 * @param  reset_mode   reset mode
 * @param  reason       reset reason
 * @return None
 */
void dfu_fw_reboot(T_WDG_MODE reset_mode, T_RESET_REASON reason);

/**
 * @brief  Check ota mode flag, if image need update
 * @param  None
 * @return Result: true: image need update, false: image don't need update
 */
bool dfu_check_ota_mode_flag(void);

/**
 * @brief  Set ota mode flag
 * @param  enable  ota mode flag
 * @return None
 */
void dfu_set_ota_mode_flag(bool enable);

/**
 * @brief  Check whether OTA transport is SPP.
 * @param  None
 * @return Result: true: OTA transport is SPP, false: OTA transport is BLE
 */
bool dfu_check_ota_transport_spp(void);


/**
 * @brief  Set ota transport is SPP
 * @param  is_spp SPP flag
 * @return None
 */
void dfu_set_ota_transport_spp(bool is_spp);

/**
 * @brief  Switch to the OTA mode, if support normal ota app need call it.
 * @param  None
 * @return None
 */
void dfu_switch_to_ota_mode(void);

/**
    * @brief    get the ic type of current fw
    * @param    void
    * @return   ic type
    */
uint8_t dfu_get_ic_type(void);


/**
    * @brief    check if ota header exist
    * @param    header_addr    ota header address
    * @return   true: exist; false: not exist
    */
bool check_ota_header(uint32_t header_addr);

/**
    * @brief    get inactive bank's image address which located on nor or nand flash
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_temp_ota_bank_img_addr_by_img_id(IMG_ID image_id);

/**
    * @brief    get inactive bank's image size
    * @param    image_id   image id
    * @return   image size
    */
uint32_t get_temp_ota_bank_img_size_by_img_id(IMG_ID image_id);

/**
    * @brief    get active bank's image address
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_active_ota_bank_img_addr_by_img_id(IMG_ID image_id);

/**
 * @brief  Get active ota bank image size by images id
 * @param  image_id  image ID
 * @return Image size
 */
uint32_t get_active_ota_bank_img_size_by_img_id(IMG_ID image_id);

/**
    * @brief    get active bank's image nand address
    * @param    image_id   image id
    * @return   image address
    */
uint32_t get_active_ota_bank_img_nand_addr_by_img_id(IMG_ID image_id);

/**
 * @brief  Get active ota bank image version by images id
 * @param  image_id  image ID
 * @return image version
 */
uint64_t dfu_get_active_image_version(IMG_ID image_id);

/**
 * @brief  print all image version by images id
 */
void dfu_print_all_images_version(void);

/**
 * @brief  Get active bank number
 * @param  None
 * @return Active bank number
 */
T_ACTIVE_BANK_NUM dfu_get_active_ota_bank_num(void);

/**
 * @brief  print flash map
 */
void dfu_print_flash_map(void);

#if defined CONFIG_SOC_SERIES_RTL87X3G
/**
 * @brief  print active boot patch bank num
 */
void dfu_print_active_bootpatch_banknum(void);
#endif

/**
* @brief calculate checksum of lenth of buffer in flash.
*
* @param  signature          signature to identify FW.
* @param  offset             offset of the image.
* @param  length             length of data.
* @param  crcValue          ret crc value point.
* @return  0 if buffer checksum calcs successfully, error line number otherwise
*/

uint32_t dfu_check_bufcrc(uint8_t *buf, uint32_t length, uint16_t mCrcVal);

/**
 * @param addr      the ram address mapping of nor or nand flash flash going to be read
 * @param data      data buffer to be read into
 * @param len       read data length
 * @return          true if read successful, otherwise false
 */
bool dfu_common_flash_read(uint32_t addr, void *data, uint32_t len);

/**
 * @param addr      the ram address mapping of nor or nand flash flash going to be written
 * @param data      data buffer to be write into
 * @param len       write data length
 * @return          true if write successful, otherwise false
 */
bool dfu_common_flash_write(uint32_t addr, void *data, uint32_t len);

/**
 * @param addr      the ram address mapping of nor or nand flash going to be erased
 * @param mode      erase mode defined as @ref DFU_FLASH_ERASE_MODE
 * @return          true if erase successful, otherwise false
 */
bool dfu_common_flash_erase(uint32_t addr, DFU_FLASH_ERASE_MODE mode);

/**
 * @brief erase a sector of the flash.
 *
 * @param  signature          signature to identify FW.
 * @param  offset             offset of the image.
 * @return  0 if erase successfully, error line number otherwise
*/
uint32_t dfu_flash_erase_retry(uint16_t signature, uint32_t offset);

/**
    * @brief    write data to flash
    * @param    img_id  image id
    * @param    offset  image offset
    * @param    total_offset  total offset when ota temp mode
    * @param    p_void  point of data
    * @return   0: success; other: fail
    */
uint32_t dfu_write_data_to_flash(uint16_t img_id, uint32_t offset, uint32_t total_offset,
                                 uint32_t length, void *p_void);

/**
    * @brief    check the integrity of the image
    * @param    image_id    image id
    * @param    offset  address offset
    * @return   ture:success ; false: fail
    */
bool dfu_checksum(IMG_ID image_id, uint32_t offset);

/**
    * @brief    clear not ready flag of specific image
    * @param    addr    address of the image
    * @return   void
    */
void dfu_set_ready(T_IMG_HEADER_FORMAT *p_header);

#if CONFIG_DFU_COMPRESS_OTA
/**
 * @brief  set specified image valid bit..
 * @param  p_header      specified image.
 * @return true if ready bit sets to 0, false otherwise
*/
void dfu_set_compressed_ready(T_COMPRESS_IMG_HEADER_FORMAT *p_header);

int isCompressed(const void *structPtr);
#endif

/**
    * @brief    clear not obsolete flag of specific image
    * @param    addr    address of the image
    * @return   void
    */
void dfu_set_obsolete(T_IMG_HEADER_FORMAT *p_header);

uint32_t dfu_get_bootpatch_flash_addr(bool is_active_bank);

T_USER_DATA_ERROR_TYPE dfu_get_user_data_info(USER_IMG_ID image_id,
                                              uint32_t *img_info, bool is_addr);

/** @} */ /* End of group APP_OTA_SERVICE_Exported_Functions */

/** @} */ /* End of group APP_OTA_SERVICE */
#endif
