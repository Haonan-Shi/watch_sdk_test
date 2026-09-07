/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#ifndef fmna_constants_platform_h
#define fmna_constants_platform_h

#include "app_cfg.h"
//#include "app_flags_customer.h"

#define DEBUG

#define SET_BIT(W, B)  ((W) |= (uint32_t)(1U << (B)))
#define CLR_BIT(W, B) ((W) &= (~(uint32_t)(1U << (B))))
#define IS_SET(W, B) (((W) >> (B)) & 1)

/** @brief  Default minimum advertising interval when device is discoverable (units of 625us, 160=100ms) */
#define DEFAULT_ADVERTISING_INTERVAL_MIN            320
/** @brief  Default maximum advertising interval */
#define DEFAULT_ADVERTISING_INTERVAL_MAX            320

#define FMNA_MANUFACTURER_NAME                  "Realtek"
#define FMNA_MODEL_NAME                         "Realtek Reference Implementation"
#define FMNA_PID                                0xCAFE
#define FMNA_HARDWARE_VERSION                   "1"

#define ARG_COUNT(...) INTERNAL_ARG_COUNT_PRIVATE(0, ##__VA_ARGS__, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0)
#define INTERNAL_ARG_COUNT_PRIVATE( _0,  _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, N, ...) N

#if (CONFIG_SOC_SERIES_RTL8773E || CONFIG_SOC_SERIES_RTL87X3G)
#define FMNA_LOG_ERROR(fmt,...)                 DBG_BUFFER(SUBTYPE_FORMAT, MODULE_APP, LEVEL_ERROR, "FMNA!!!"fmt, ARG_COUNT(__VA_ARGS__),##__VA_ARGS__)
#define FMNA_LOG_WARNING(fmt,...)               DBG_BUFFER(SUBTYPE_FORMAT, MODULE_APP, LEVEL_WARN, "FMNA!!*"fmt, ARG_COUNT(__VA_ARGS__),##__VA_ARGS__)
#define FMNA_LOG_INFO(fmt,...)                  DBG_BUFFER(SUBTYPE_FORMAT, MODULE_APP, LEVEL_INFO, "FMNA!**"fmt, ARG_COUNT(__VA_ARGS__),##__VA_ARGS__)
#define FMNA_LOG_DEBUG(fmt,...)                 DBG_BUFFER(SUBTYPE_FORMAT, MODULE_APP, LEVEL_TRACE, "FMNA"fmt, ARG_COUNT(__VA_ARGS__),##__VA_ARGS__)
#else
#define FMNA_LOG_ERROR(fmt,...)                 DBG_BUFFER(TYPE_BB2, SUBTYPE_FORMAT, MODULE_APP, LEVEL_ERROR, "FMNA!!!"fmt, ARG_COUNT(__VA_ARGS__),__VA_ARGS__)
#define FMNA_LOG_WARNING(fmt,...)               DBG_BUFFER(TYPE_BB2, SUBTYPE_FORMAT, MODULE_APP, LEVEL_WARN, "FMNA!!*"fmt, ARG_COUNT(__VA_ARGS__),__VA_ARGS__)
#define FMNA_LOG_INFO(fmt,...)                  DBG_BUFFER(TYPE_BB2, SUBTYPE_FORMAT, MODULE_APP, LEVEL_INFO, "FMNA!**"fmt, ARG_COUNT(__VA_ARGS__),__VA_ARGS__)
#define FMNA_LOG_DEBUG(fmt,...)                 DBG_BUFFER(TYPE_BB2, SUBTYPE_FORMAT, MODULE_APP, LEVEL_TRACE, "FMNA"fmt, ARG_COUNT(__VA_ARGS__),__VA_ARGS__)
#endif

#define DBG_BUFFER_RETARGET(...)                FMNA_LOG_DEBUG(__VA_ARGS__)

#define FMNA_LOG_HEXDUMP_INFO(p_data, len)      APP_PRINT_INFO1("%b", TRACE_BINARY(len,p_data))
#define FMNA_LOG_HEXDUMP_DEBUG(p_data, len)     APP_PRINT_INFO1("%b", TRACE_BINARY(len,p_data))

#define MAX_SUPPORTED_CONNECTIONS               2
#define GATT_MAX_MTU_SIZE                       247
#define BLE_GAP_ADDR_LEN                        (6)

#define CONN_HANDLE_INVALID                     0xFF
#define CONN_HANDLE_ALL                         0xEF
#define GAP_SEC_KEY_LEN                         16

#define FMNA_SUCCESS                            0        ///< Successful command
#define FMNA_ERROR_INTERNAL                     1        ///< Internal Error
#define FMNA_ERROR_INVALID_STATE                2        ///< Invalid state, operation disallowed in this state
#define FMNA_ERROR_INVALID_LENGTH               3        ///< Invalid Length
#define FMNA_ERROR_INVALID_DATA                 4        ///< Invalid Data
#define FMNA_ERROR_NULL                         5        ///< Null Pointer

/**@brief Macro for calling error handler function if supplied error code any other than FMNA_SUCCESS.
 *
 * @param[in] ERR_CODE Error code supplied to the error handler.
 */
#define APP_ERROR_HANDLER(ERR_CODE)             DBG_BUFFER_RETARGET("[ERROR] %s %d %d", __FUNCTION__, __LINE__, ERR_CODE);
#define FMNA_ERROR_CHECK(ERR_CODE)                           \
    do                                                       \
    {                                                        \
        const uint32_t LOCAL_ERR_CODE = (ERR_CODE);          \
        if (LOCAL_ERR_CODE != FMNA_SUCCESS)                  \
        {                                                    \
            APP_ERROR_HANDLER(LOCAL_ERR_CODE);               \
        }                                                    \
    } while (0)

/**@brief Macro for calling error handler function if supplied boolean value is false.
 *
 * @param[in] BOOLEAN_VALUE Boolean value to be evaluated.
 */
#define APP_ERROR_CHECK_BOOL(ERR_CODE)           DBG_BUFFER_RETARGET("[ERROR] %s %d bool %d", __FUNCTION__, __LINE__, ERR_CODE);
#define FMNA_ERROR_CHECK_BOOL(BOOLEAN_VALUE)                  \
    do                                                        \
    {                                                         \
        const uint32_t LOCAL_BOOLEAN_VALUE = (BOOLEAN_VALUE); \
        if (!LOCAL_BOOLEAN_VALUE)                             \
        {                                                     \
            APP_ERROR_CHECK_BOOL(0);                          \
        }                                                     \
    } while (0)

/** Priority of the application BLE event handler. */
#define FMNA_BLE_CONN_CFG_TAG                    1                         /**< A tag identifying the SoftDevice BLE configuration. */
#define FMNA_BLE_OBSERVER_PRIO                   3
#define FMNA_BLE_CONN_BUFFER_SIZE                8

#define fmna_ret_code_t                         uint32_t

#define ADV_TYPE_SERVICE_DATA                   GAP_ADTYPE_SERVICE_DATA

#define APP_ERROR_CHECK(ERR_CODE)               FMNA_ERROR_CHECK(ERR_CODE)
#define APP_TIMER_MIN_TIMEOUT_TICKS             10
#define MSEC_TO_TIMER_TICKS(ms)                 (uint32_t)(ms)

#define APPLE_AUTH_TOKEN_SAVE_ADDRESS           BKP_DATA2_ADDR
#define APPLE_AUTH_TOKEN_BACKUP_ADDRESS         (BKP_DATA2_ADDR + 0x1000)

/* FTL 4 byte alignment */
#define ALIGN_SIZE(len)                         (len + 3)&(~3)

#define FTL_SAVE_PAIR_STATE_ADDR                APP_FINDMY_INFO_ADDR
#define FTL_SAVE_PAIR_STATE_SIZE                4
#define FTL_SAVE_M_P_ADDR                       (FTL_SAVE_PAIR_STATE_ADDR + FTL_SAVE_PAIR_STATE_SIZE)
#define FTL_SAVE_M_P_SIZE                       ALIGN_SIZE(P_BLEN)
#define FTL_SAVE_PRI_KEY_ADDR                   (FTL_SAVE_M_P_ADDR + FTL_SAVE_M_P_SIZE)
#define FTL_SAVE_PRI_KEY_SIZE                   ALIGN_SIZE(sizeof(fmna_primary_key_t))
#define FTL_SAVE_SEC_KEY_ADDR                   (FTL_SAVE_PRI_KEY_ADDR + FTL_SAVE_PRI_KEY_SIZE)
#define FTL_SAVE_SEC_KEY_SIZE                   ALIGN_SIZE(sizeof(fmna_secondary_key_t))
#define FTL_SAVE_BT_MAC_ADDR                    (FTL_SAVE_SEC_KEY_ADDR + FTL_SAVE_SEC_KEY_SIZE)
#define FTL_SAVE_BT_MAC_SIZE                    (8)
#define FTL_SAVE_NEXT_PW_ROT_INDEX_ADDR         (FTL_SAVE_BT_MAC_ADDR + FTL_SAVE_BT_MAC_SIZE)
#define FTL_SAVE_NEXT_PW_ROT_INDEX_SIZE         (4)
#define FTL_SAVE_SKN_ADDR                       (FTL_SAVE_NEXT_PW_ROT_INDEX_ADDR + FTL_SAVE_NEXT_PW_ROT_INDEX_SIZE)
#define FTL_SAVE_SKN_SIZE                       ALIGN_SIZE(SK_BLEN)
#define FTL_SAVE_SKS_ADDR                       (FTL_SAVE_SKN_ADDR + FTL_SAVE_SKN_SIZE)
#define FTL_SAVE_SKS_SIZE                       ALIGN_SIZE(SK_BLEN)
#define FTL_SAVE_ICLOUD_ID_ADDR                 (FTL_SAVE_SKS_ADDR + FTL_SAVE_SKS_SIZE)
#define FTL_SAVE_ICLOUD_ID_SIZE                 ALIGN_SIZE(ICLOUD_IDENTIFIER_BLEN)
#define FTL_SAVE_BOND_IDX_ADDR                  (FTL_SAVE_ICLOUD_ID_ADDR + FTL_SAVE_ICLOUD_ID_SIZE)
#define FTL_SAVE_BOND_IDX_SIZE                  (4)


#endif /* fmna_constants_platform_h */
