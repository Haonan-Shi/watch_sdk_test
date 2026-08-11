/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_CMD_H_
#define _APP_CMD_H_

#include <stdint.h>
#include <stdbool.h>

#include "app_report.h"
#include "app_eq.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_CMD App Cmd
  * @brief App Cmd
  * @{
  */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup APP_CMD_Exported_Macros App Cmd Macros
   * @{
   */
#define F_APP_AUTO_SUPPORT 0
#define C_APP_DEVICE_CMD_SUPPORT 0

#define CMD_SET_VER_MAJOR                   0x01
#define CMD_SET_VER_MINOR                   0x08
#define EQ_SPEC_VER_MAJOR                   0x02
#define EQ_SPEC_VER_MINOR_0                 0x00
#define EQ_SPEC_VER_MINOR_1                 0x01
#define EQ_SPEC_VER_MINOR_2                 0x02
#define CMD_SYNC_BYTE                       0xAA

#define GET_STATUS_RWS_STATE                0x00
#define GET_STATUS_RWS_CHANNEL              0x01
#define GET_STATUS_BATTERY_STATUS           0x02
#define GET_STATUS_APT_STATUS               0x03
#define GET_STATUS_APP_STATE                0x04
#define GET_STATUS_BUD_ROLE                 0x05
#define GET_STATUS_APT_NR_STATUS            0x06
#define GET_STATUS_APT_VOL                  0x07
#define GET_STATUS_LOCK_BUTTON              0x08
#define GET_STATUS_FIND_ME                  0x09
#define GET_STATUS_ANC_STATUS               0x0A
#define GET_STATUS_LLAPT_STATUS             0x0B
#define GET_STATUS_RWS_DEFAULT_CHANNEL      0x0C
#define GET_STATUS_RWS_BUD_SIDE             0x0D
#define GET_STATUS_RWS_SYNC_APT_VOL         0x0E

/* for LG (BBLite D-cut) */
#define GET_STATUS_BUD_ROLE_FOR_LG              0xa0
#define GET_STATUS_VOLUME                       0xa1
#define GET_STATUS_MERIDIAN_SOUND_EFFECT_MODE   0xa2
#define GET_STATUS_LIGHT_SENSOR                 0xa3

#if(C_APP_DEVICE_CMD_SUPPORT == 1)
//for CMD_INQUIRY
#define START_INQUIRY                           0x00
#define STOP_INQUIRY                            0x01

//for CMD_SERVICES_SEARCH
#define START_SERVICES_SEARCH                   0x00
#define STOP_SERVICES_SEARCH                    0x01

//for CMD_PAIR_REPLY and CMD_SSP_CONFIRMATION
#define ACCEPT_PAIRING_REQ                      0x00
#define REJECT_PAIRING_REQ                      0x01

//for CMD_SET_PAIRING_CONTROL
#define ENABLE_AUTO_ACCEPT_ACL_ACF_REQ          0x00
#define ENABLE_AUTO_REJECT_ACL_ACF_REQ          0x01
#define FORWARD_ACL_ACF_REQ_TO_HOST             0x02

//for CMD_GET_REMOTE_DEV_ATTR_INFO
#define GET_AVRCP_ATTR_INFO                     0x00
#define GET_PBAP_ATTR_INFO                      0x01
#endif

#if (C_APP_AVRCP_CMD_SUPPORT == 1)
#define ALL_ELEMENT_ATTR                        0x00
#define MAX_NUM_OF_ELEMENT_ATTR                 0x07
#endif

//for CMD_GET_LINK_KEY
#define GET_ALL_LINK_KEY                        0x00
#define GET_SPECIAL_ADDR_LINK_KEY               0x01
#define GET_PRORITY_LINK_KEY                    0x02

#if (C_APP_PBAP_CMD_SUPPORT == 1)
//for CMD_PBAP_DOWNLOAD
#define PBAP_DOWNLOAD_METHOD_NORMAL             0x01
#define PBAP_DOWNLOAD_METHOD_SET_RANGE          0x02
#define PBAP_DOWNLOAD_METHOD_ALL                0x03 //combined method for download the phonebook of ME/SM and CCH
#define PBAP_DOWNLOAD_METHOD_ALL_PB             0x04 //combined method for download the phonebook of ME/SM
#define PBAP_DOWNLOAD_METHOD_CCH                0x05 //The Combined Calls History (cch) contians incoming/outgoing/missed call history

#define PBAP_DOWNLOAD_ME_PB                     0x07
#define PBAP_DOWNLOAD_SM_PB                     0x08

#define PBAP_DOWNLOAD_ME_PB_MASK                0x01 //bitmask for download ME (phone memory) phonebook
#define PBAP_DOWNLOAD_SM_PB_MASK                0x02 //bitmask for download SM (sim card) phonebook
#define PBAP_DOWNLOAD_CCH_MASK                  0x04 //bitmask for download the Combined Calls History (cch)

//for CMD_PBAP_DOWNLOAD_CONTROL
#define PBAP_DOWNLOAD_CONTROL_ABORT             0x01
#define PBAP_DOWNLOAD_CONTROL_SUSPEND           0x02
#define PBAP_DOWNLOAD_CONTROL_CONTINUE          0x03
#endif

//for CMD_GET_FLASH_DATA and EVENT_REPORT_FLASH_DATA
#define START_TRANS                 0x00
#define CONTINUE_TRANS              0x01
#define SUPPORT_IAMGE_TYPE          0x02

#define TRANS_DATA_INFO             0x00
#define CONTINUE_TRANS_DATA         0x01
#define END_TRANS_DATA              0x02
#define SUPPORT_IAMGE_TYPE_INFO     0x03

#define SYSTEM_CONFIGS              0x00
#define ROM_PATCH_IMAGE             0x01
#define APP_IMAGE                   0x02
#define DSP_SYSTEM_IMAGE            0x03
#define DSP_APP_IMAGE               0x04
#define FTL_DATA                    0x05
#define ANC_IMAGE                   0x06
#define LOG_PARTITION               0x07
#define CORE_DUMP_PARTITION         0x08

#define FLASH_ALL                   0xFF
#define ALL_DUMP_IAMGE_MASK         ((0x01 << SYSTEM_CONFIGS) | (0x01 << ROM_PATCH_IMAGE) | (0x01 << APP_IMAGE) \
                                     | (0x01 << DSP_SYSTEM_IMAGE) | (0x01 << DSP_APP_IMAGE) \
                                     | (0x01 << FTL_DATA) |(0x01 << CORE_DUMP_PARTITION))

//for CMD_AUDIO_DSP_CTRL_SEND to capture dsp data
#define VENDOR_SPP_CAPTURE_DSP_LOG      0x01
#define VENDOR_SPP_CAPTURE_DSP_RWA_DATA 0x02
#define H2D_CMD_DSP_DAC_ADC_DATA_TO_MCU 0x1F
#define H2D_SPPCAPTURE_SET              0x0F01
#define CHANGE_MODE_EXIST               0x00
#define CHANGE_MODE_TO_SCO              0x01
#define DSP_CAPTURE_DATA_START_MASK                  0x01
#define DSP_CAPTURE_DATA_SWAP_TO_MASTER              0x02
#define DSP_CAPTURE_DATA_ENTER_SCO_MODE_MASK         0x04
#define DSP_CAPTURE_DATA_CHANGE_MODE_TO_SCO_MASK     0x08
#define DSP_CAPTURE_RAW_DATA_EXECUTING               0x10
#define DSP_CAPTURE_DATA_LOG_EXECUTING               0x20

/* Define application support status */
#define SNK_SUPPORT_GET_SET_LE_NAME          1
#define SNK_SUPPORT_GET_SET_BR_NAME          1
#define SNK_SUPPORT_GET_SET_VP_LANGUAGE      1
#define SNK_SUPPORT_GET_BATTERY_LEVEL        1
#define SNK_SUPPORT_GET_SET_VAD_STATE        0
#define SNK_SUPPORT_GET_SET_ANC_STATE        0
#define SNK_SUPPORT_GET_SET_VIBRATOR_STATE   0
#define SNK_SUPPORT_GET_SET_VIBRATOR         0
#define SNK_SUPPORT_GET_SET_KEY_REMAP        1  //F_APP_KEY_EXTEND_FEATURE  // in app_flags.h

#define PX_SET_CALIBRATION_NOISE_FLOOR           1
#define PX_REPORT_CALIBRATION_NOISE_FLOOR        1
#define PX_SET_IN_EAR_THRESHOLD                  2
#define PX_REPORT_CALIBRATION_IN_EAR_THRESHOLD   2
#define PX_SET_OUT_EAR_THRESHOLD                 3
#define PX_REPORT_CALIBRATION_OUT_EAR_THRESHOLD  3
#define PX_GET_PX318J_PARA                       4
#define PX_REPORT_PX318J_PARA                    4

//align dsp_driver.h/codec_driver.h define
#define APP_MIC_SEL_DMIC_1                  0x00
#define APP_MIC_SEL_DMIC_2                  0x01
#define APP_MIC_SEL_AMIC_1                  0x02
#define APP_MIC_SEL_AMIC_2                  0x03
#define APP_MIC_SEL_AMIC_3                  0x04
#define APP_MIC_SEL_DISABLE                 0x07

/** End of APP_DEVICE_Exported_Macros
    * @}
    */
/*============================================================================*
 *                              Types
 *============================================================================*/
/** @defgroup APP_CMD_Exported_Types App Cmd Types
  * @{
  */
/**  @brief  embedded uart, spp or le vendor command type.
  *    <b> Only <b> valid when BT SOC connects to external MCU via data uart, spp or le.
  *    refer to SDK audio sample code for definition
  */
typedef enum
{
    CMD_ACK                             = 0x0000,
    CMD_BT_READ_PAIRED_RECORD           = 0x0001,
    CMD_BT_CREATE_CONNECTION            = 0x0002,
    CMD_BT_DISCONNECT                   = 0x0003,
    CMD_MMI                             = 0x0004,
    CMD_LEGACY_DATA_TRANSFER            = 0x0005,
    CMD_ASSIGN_BUFFER_SIZE              = 0x0006,
    CMD_BT_READ_LINK_INFO               = 0x0007,
    CMD_TONE_GEN                        = 0x0008,
    CMD_BT_GET_REMOTE_NAME              = 0x0009,
    CMD_BT_IAP_LAUNCH_APP               = 0x000A,
    CMD_TTS                             = 0x000B,
    CMD_INFO_REQ                        = 0x000C,

    CMD_DAC_GAIN_CTRL                   = 0x000F,
    CMD_ADC_GAIN_CTRL                   = 0x0010,
    CMD_BT_SEND_AT_CMD                  = 0x0011,
    CMD_SET_CFG                         = 0x0012,
    CMD_INDICATION                      = 0x0013,
    CMD_LINE_IN_CTRL                    = 0x0014,
    CMD_LANGUAGE_GET                    = 0x0015,
    CMD_LANGUAGE_SET                    = 0x0016,
    CMD_GET_CFG_SETTING                 = 0x0017,

    CMD_GET_STATUS                      = 0x0018,
    CMD_SUPPROT_MULTILINK               = 0x0019,

    CMD_BT_HFP_DIAL_WITH_NUMBER         = 0x001B,
    CMD_GET_BD_ADDR                     = 0x001C,
    CMD_STRING_MODE                     = 0x001E,
    CMD_SET_VP_VOLUME                   = 0x001F,

    CMD_HONEYGUI_BENCHMARK_START        = 0x00F0,

    CMD_LE_START_ADVERTISING            = 0x0100,
    CMD_LE_STOP_ADVERTISING             = 0x0101,
    CMD_LE_DATA_TRANSFER                = 0x0102,
    CMD_LE_START_SCAN                   = 0x0103,
    CMD_LE_STOP_SCAN                    = 0x0104,
    CMD_LE_GET_ADDR                     = 0x0105,

    CMD_ANCS_REGISTER                   = 0x0110,
    CMD_ANCS_GET_NOTIFICATION_ATTR      = 0x0111,
    CMD_ANCS_GET_APP_ATTR               = 0x0112,
    CMD_ANCS_PERFORM_NOTIFICATION_ACTION = 0x0113,

    CMD_AUDIO_EQ_QUERY                  = 0x0200,
    CMD_AUDIO_EQ_PARAM_SET              = 0x0203,
    CMD_AUDIO_EQ_PARAM_GET              = 0x0204,
    CMD_AUDIO_EQ_INDEX_SET              = 0x0205,
    CMD_AUDIO_EQ_INDEX_GET              = 0x0206,
    CMD_AUDIO_DSP_CTRL_SEND             = 0x0207,
    CMD_AUDIO_CODEC_CTRL_SEND           = 0x0208,
#if 0
    //supported only in cmd set version v0.0.0.1
    CMD_AUDIO_DSP_CTRL_SEND             = 0x0207,
    CMD_AUDIO_CODEC_CTRL_SEND           = 0x0208,
    CMD_DSP_SET_APT_GAIN                = 0x0209,
#endif
    CMD_SET_VOLUME                      = 0x020A,
    CMD_APT_EQ_INDEX_SET                = 0x020B,
    CMD_APT_EQ_INDEX_GET                = 0x020C,
    CMD_DSP_DEBUG_SIGNAL_IN             = 0x020D,  // only support RTL87x3C

#if F_APP_APT_SUPPORT
    CMD_SET_APT_VOLUME_OUT_LEVEL        = 0x020E,
    CMD_GET_APT_VOLUME_OUT_LEVEL        = 0x020F,
#endif

    // for equalizer page
    CMD_AUDIO_EQ_QUERY_PARAM            = 0x0210,
    CMD_SET_TONE_VOLUME_LEVEL           = 0x0211,
    CMD_GET_TONE_VOLUME_LEVEL           = 0x0212,
    CMD_DSP_TOOL_OPERATION              = 0x0213,  // for DSP tool

    CMD_AUDIO_DSP_SCENARIO_CHECK        = 0x0217,

#if CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE
    CMD_DATA_CAPTURE_START_STOP         = 0x0220,
    CMD_DATA_CAPTURE_ENTER_EXIT         = 0x0222,
#endif

    //for good test
    CMD_LED_TEST                        = 0x0300,
    CMD_CLEAR_MP_DATA                   = 0x0301,
    CMD_BT_GET_LOCAL_ADDR               = 0x0302,
    CMD_GET_LEGACY_RSSI                 = 0x0303,
    CMD_GET_RF_POWER                    = 0x0304,
    CMD_GET_CRYSTAL_TRIM                = 0x0305,
    CMD_GET_LINK_KEY                    = 0x0306,
    CMD_GET_COUNTRY_CODE                = 0x0307,
    CMD_GET_FW_VERSION                  = 0x0308,
    CMD_BT_BOND_INFO_CLEAR              = 0x0309,
    CMD_GET_ADC_VALUE_1                 = 0x030A,
    CMD_GET_ADC_VALUE_2                 = 0x030B,
    CMD_GET_UNSIZE_RAM                  = 0x030C,
    CMD_GET_FLASH_DATA                  = 0x030D,
    CMD_MIC_SWITCH                      = 0x030E,
    CMD_GET_PACKAGE_ID                  = 0x030F,
    CMD_SWITCH_TO_HCI_DOWNLOAD_MODE     = 0x0310,
    CMD_GET_PAD_VOLTAGE                 = 0x0311,
    CMD_PX318J_CALIBRATION              = 0x0312,
    CMD_GET_IMAGE_INFO                  = 0x0319,

#if(C_APP_DEVICE_CMD_SUPPORT == 1)
    CMD_GET_LOCAL_DEV_STATE             = 0x0320,
    CMD_INQUIRY                         = 0x0321,
    CMD_SERVICES_SEARCH                 = 0x0322,
    CMD_SET_PAIRING_CONTROL             = 0x0323,
    CMD_SET_PIN_CODE                    = 0x0324,
    CMD_GET_PIN_CODE                    = 0x0325,
    CMD_PAIR_REPLY                      = 0x0326,
    CMD_SSP_CONFIRMATION                = 0x0327,
    CMD_GET_CONNECTED_DEV_ID            = 0x0328,
    CMD_GET_REMOTE_DEV_ATTR_INFO        = 0x0329,
#endif

    CMD_RF_XTAK_K                       = 0x032A,
    CMD_RF_XTAL_K_GET_RESULT            = 0x032B,

#if (C_APP_HFP_CMD_SUPPORT == 1)
    CMD_SEND_DTMF                       = 0x0400,
    CMD_GET_OPERATOR                    = 0x0401,
    CMD_GET_SUBSCRIBER_NUM              = 0x0402,
#endif

#if (C_APP_AVRCP_CMD_SUPPORT == 1)
    CMD_AVRCP_LIST_SETTING_ATTR         = 0x0500,
    CMD_AVRCP_LIST_SETTING_VALUE        = 0x0501,
    CMD_AVRCP_GET_CURRENT_VALUE         = 0x0502,
    CMD_AVRCP_SET_VALUE                 = 0x0503,
    CMD_AVRCP_ABORT_DATA_TRANSFER       = 0x0504,
    CMD_AVRCP_SET_ABSOLUTE_VOLUME       = 0x0505,
    CMD_AVRCP_GET_PLAY_STATUS           = 0x0506,
    CMD_AVRCP_GET_ELEMENT_ATTR          = 0x0507,
#endif

    CMD_OTA_DEV_INFO                    = 0x0600,
    CMD_OTA_IMG_VER                     = 0x0601,
    CMD_OTA_START                       = 0x0602,
    CMD_OTA_PACKET                      = 0x0603,
    CMD_OTA_VALID                       = 0x0604,
    CMD_OTA_RESET                       = 0x0605,
    CMD_OTA_ACTIVE_RESET                = 0x0606,
    CMD_OTA_BUFFER_CHECK_ENABLE         = 0x0607,
    CMD_OTA_BUFFER_CHECK                = 0x0608,
    CMD_OTA_IMG_INFO                    = 0x0609,
    CMD_OTA_SECTION_SIZE                = 0x060A,
    CMD_OTA_DEV_EXTRA_INFO              = 0x060B,
    CMD_OTA_PROTOCOL_TYPE               = 0x060C,
    CMD_OTA_GET_RELEASE_VER             = 0x060D,
    CMD_OTA_INACTIVE_BANK_VER           = 0x060E,
    CMD_OTA_COPY_IMG                    = 0x060F,
    CMD_OTA_CHECK_SHA256                = 0x0610,
    CMD_OTA_ROLESWAP                    = 0x0611,
    CMD_OTA_TEST_EN                     = 0x0612,
    CMD_OTA_KEY_CHECK                   = 0x0613,
    CMD_OTA_REPORT_IMAGE_NUM            = 0x0614,
    CMD_ENTER_NORMAL_OTA                = 0x0615,
    CMD_NORMAL_OTA_CHECK                = 0x0616,

    /* only support RTL87x3C */
    CMD_PLAYBACK_QUERY_INFO                 = 0x0680,
    CMD_PLAYBACK_GET_LIST_DATA              = 0x0681,
    CMD_PLAYBACK_TRANS_START                = 0x0682,
    CMD_PLAYBACK_TRANS_CONTINUE             = 0x0683,
    CMD_PLAYBACK_REPORT_BUFFER_CHECK        = 0x0684,
    CMD_PLAYBACK_VALID_SONG                 = 0x0685,
    CMD_PLAYBACK_TRIGGER_ROLE_SWAP          = 0x0686,
    CMD_PLAYBACK_TRANS_CANCEL               = 0x0687,
    CMD_PLAYBACK_EXIT_TRANS                 = 0x0688,
    CMD_PLAYBACK_PERMANENT_DELETE_SONG      = 0x0689,
    CMD_PLAYBACK_PLAYLIST_ADD_SONG          = 0x068A,
    CMD_PLAYBACK_PLAYLIST_DELETE_SONG       = 0x068B,
    CMD_PLAYBACK_PERMANENT_DELETE_ALL_SONG  = 0x068C,
    CMD_PLAYBACK_GET_SD_SPACE_INFO          = 0x068D,
    CMD_PLAYBACK_GET_FLASH_SPACE_INFO       = 0x068E,
    CMD_PERMANENT_DELETE_ALL_FILE_BY_FORMAT = 0x068F,
    CMD_TRANS_SET_SCENARIO                  = 0x0690,

    CMD_GET_SUPPORTED_MMI_LIST          = 0x0700,
    CMD_GET_SUPPORTED_CLICK_TYPE        = 0x0701,
    CMD_GET_SUPPORTED_CALL_STATUS       = 0x0702,
    CMD_GET_KEY_MMI_MAP                 = 0x0703,
    CMD_SET_KEY_MMI_MAP                 = 0x0704,

    CMD_VENDOR_SEPC                     = 0x0800, //It has been reserved for vendor customer A, please dont't use this value.

    CMD_DFU_START                       = 0x0900,

    //for customize
    CMD_RSV1 = 0x0A00,                  //0x0A00
    CMD_RSV2,                           //0x0A01
    CMD_RSV3,                           //0x0A02
    CMD_SET_MERIDIAN_SOUND_EFFECT_MODE, //0x0A03
    CMD_LG_CUSTOMIZED_FEATURE,          //0x0A04
    CMD_CUSTOMIZED_SITRON_FEATURE,      //0x0A05
    CMD_JSA_CALIBRATION,                //0x0A06
    CMD_MIC_MP_VERIFY_BY_HFP,           //0x0A07
    CMD_GET_DSP_CONFIG_GAIN,            //0x0A08
    CMD_CUSTOMIZED_TOZO_FEATURE,        //0x0A09
    CMD_RSV4,                           //0x0A0A
    CMD_IO_PIN_PULL_HIGH,               //0x0A0B

    //for HCI command
    CMD_HCI                             = 0x0B00,
    CMD_WDG_RESET                       = 0x0B01,
    CMD_DUAL_MIC_MP_VERIFY              = 0x0B02,

    CMD_SOUND_PRESS_CALIBRATION         = 0x0B10,

    //for ANC command
    CMD_ANC_TEST_MODE                   = 0x0C00,
    CMD_ANC_WRITE_GAIN                  = 0x0C01,
    CMD_ANC_READ_GAIN                   = 0x0C02,
    CMD_ANC_BURN_GAIN                   = 0x0C03,
    CMD_ANC_COMPARE                     = 0x0C04,
    CMD_ANC_GEN_TONE                    = 0x0C05,
    CMD_ANC_CONFIG_DATA_LOG             = 0x0C06,
    CMD_ANC_READ_DATA_LOG               = 0x0C07,
    CMD_ANC_READ_MIC_CONFIG             = 0x0C08,
    CMD_ANC_READ_SPEAKER_CHANNEL        = 0x0C09,
    CMD_ANC_READ_REGISTER               = 0x0C0A,
    CMD_ANC_WRITE_REGISTER              = 0x0C0B,
    CMD_ANC_LLAPT_WRITE_GAIN            = 0x0C0C,
    CMD_ANC_LLAPT_READ_GAIN             = 0x0C0D,
    CMD_ANC_LLAPT_BURN_GAIN             = 0x0C0E,
    CMD_ANC_LLAPT_FEATURE_CONTROL       = 0x0C0F,

    CMD_ANC_QUERY                       = 0x0C20,
    CMD_ANC_ENABLE_DISABLE              = 0x0C21,
    CMD_LLAPT_QUERY                     = 0x0C22,
    CMD_LLAPT_ENABLE_DISABLE            = 0x0C23,

    CMD_RAMP_GET_INFO                   = 0x0C26,
    CMD_RAMP_BURN_PARA_START            = 0x0C27,
    CMD_RAMP_BURN_PARA_CONTINUE         = 0x0C28,
    CMD_RAMP_BURN_GRP_INFO              = 0x0C29,
    CMD_RAMP_MULTI_DEVICE_APPLY         = 0x0C2A,

    CMD_LISTENING_MODE_CYCLE_SET        = 0x0C2B,
    CMD_LISTENING_MODE_CYCLE_GET        = 0x0C2C,

    CMD_VENDOR_SPP_COMMAND              = 0x0C2D,

    CMD_APT_VOLUME_INFO                 = 0x0C2E,
    CMD_APT_VOLUME_SET                  = 0x0C2F,
    CMD_APT_VOLUME_STATUS               = 0x0C30,
    CMD_LLAPT_BRIGHTNESS_INFO           = 0x0C31,
    CMD_LLAPT_BRIGHTNESS_SET            = 0x0C32,
    CMD_LLAPT_BRIGHTNESS_STATUS         = 0x0C33,
    CMD_LLAPT_SCENARIO_CHOOSE_INFO      = 0x0C36,
    CMD_LLAPT_SCENARIO_CHOOSE_TRY       = 0x0C37,
    CMD_LLAPT_SCENARIO_CHOOSE_RESULT    = 0x0C38,
    CMD_APT_GET_POWER_ON_DELAY_TIME     = 0x0C39,
    CMD_APT_SET_POWER_ON_DELAY_TIME     = 0x0C3A,

    // OTA Tooling section
    CMD_OTA_TOOLING_PARKING             = 0x0D00,
    CMD_MEMORY_DUMP                     = 0x0D22,

    CMD_GET_LOW_LATENCY_MODE_STATUS     = 0x0E01,
    CMD_GET_EAR_DETECTION_STATUS        = 0x0E02,
    CMD_SET_LOW_LATENCY_LEVEL           = 0x0E04,

    CMD_MP_TEST                         = 0x0F00,

#if (C_APP_PBAP_CMD_SUPPORT == 1)
    CMD_PBAP_DOWNLOAD                   = 0x1000,
    CMD_PBAP_DOWNLOAD_CONTROL           = 0x1001,
    CMD_PBAP_DOWNLOAD_GET_SIZE          = 0x1002,
#endif

#if (F_APP_HEARABLE_SUPPORT == 1)
    /* only support RTL87x3C */
    CMD_HA_SET_PARAM                    = 0x2000,
    CMD_HA_VER_REQ                      = 0x2001,
    CMD_HA_SET_EFFECT_INDEX             = 0x2002,
    CMD_HA_GET_EFFECT_INDEX             = 0x2003,
    CMD_HA_SET_ON_OFF                   = 0x2004,
    CMD_HA_GET_ON_OFF_REQ               = 0x2005,
    CMD_HA_GET_TOOL_EXTEND_REQ          = 0x2006,
    CMD_HA_GET_ANC_APT_REQ              = 0x2007,
    CMD_HA_SET_DSP_PARAM                = 0x2008,
#endif

    CMD_START_RECORD                    = 0x100D,
    CMD_STOP_RECORD                     = 0x100F,
    CMD_START_RECORD_PLAY               = 0x1010,
    CMD_STOP_RECORD_PLAY                = 0x1011,
    CMD_RECORD_PLAY_DATA                = 0x1012,
} T_CMD_ID;

/** @brief  packet type for legacy transfer*/
typedef enum t_pkt_type
{
    PKT_TYPE_SINGLE = 0x00,
    PKT_TYPE_START = 0x01,
    PKT_TYPE_CONT = 0x02,
    PKT_TYPE_END = 0x03
} T_PKT_TYPE;


typedef enum
{
    APP_REMOTE_MSG_CMD_GET_FW_VERSION,
    APP_REMOTE_MSG_CMD_REPORT_FW_VERSION,
    APP_REMOTE_MSG_CMD_GET_OTA_FW_VERSION,
    APP_REMOTE_MSG_CMD_REPORT_OTA_FW_VERSION,
    APP_REMOTE_MSG_DSP_DEBUG_SIGNAL_IN_SYNC,    // only support RTL87x3C

#if (F_APP_HEARABLE_SUPPORT == 1)
    /* only support RTL87x3C */
    APP_REMOTE_MSG_HA_SET_PARAM,
    APP_REMOTE_MSG_HA_SET_EFFECT_INDEX,
    APP_REMOTE_MSG_HA_SET_ON_OFF,
    APP_REMOTE_MSG_HA_SET_DSP_PARAM,
#endif

    APP_REMOTE_MSG_CMD_TOTAL
} T_CMD_REMOTE_MSG;

/**  @brief CMD Set Info Request type. */
typedef enum
{
    CMD_SET_INFO_TYPE_VERSION = 0x00,
    CMD_INFO_GET_CAPABILITY   = 0x01,
} T_CMD_SET_INFO_TYPE;

typedef struct
{
    // Byte 0
    uint8_t snk_support_get_set_le_name : 1;
    uint8_t snk_support_get_set_br_name : 1;
    uint8_t snk_support_get_set_vp_language : 1;
    uint8_t snk_support_get_battery_info : 1;
    uint8_t snk_support_ota : 1;
    uint8_t snk_support_change_channel : 1;
    uint8_t snk_support_rsv1 : 2;

    // Byte 1
    uint8_t snk_support_tts : 1;
    uint8_t snk_support_get_set_rws_state : 1;
    uint8_t snk_support_get_set_apt_state : 1;
    uint8_t snk_support_get_set_eq_state : 1;
    uint8_t snk_support_get_set_vad_state : 1;
    uint8_t snk_support_get_set_anc_state : 1;
    uint8_t snk_support_get_set_llapt_state : 1;
    uint8_t snk_support_get_set_listening_mode_cycle : 1;

    // Byte 2
    uint8_t snk_support_llapt_brightness : 1;
    uint8_t snk_support_anc_eq : 1;
    uint8_t snk_support_apt_eq : 1;
    uint8_t snk_support_tone_volume_adjustment : 1;
    uint8_t snk_support_apt_eq_adjust_separate : 1;
    uint8_t snk_support_rsv5 : 3;

    // Byte 3
    uint8_t snk_support_llapt_scenario_choose : 1;
    uint8_t snk_support_rsv6 : 1;
    uint8_t snk_support_power_on_delay_apply_apt_on : 1;
    uint8_t snk_support_rsv9 : 5;

    // Byte 4
    uint8_t snk_support_ansc : 1;
    uint8_t snk_support_vibrator : 1;
    uint8_t snk_support_change_mfb_func : 1;
    uint8_t snk_support_gaming_mode : 1;
    uint8_t snk_support_gaming_mode_eq : 1;
    uint8_t snk_support_key_remap : 1;
    uint8_t snk_support_HA: 1;
    uint8_t snk_support_local_playback : 1;  //bit39

    // Byte 5
    uint8_t snk_support_rsv5_1 : 2;
    uint8_t snk_support_anc_scenario_choose : 1;
    uint8_t snk_support_rws_key_remap : 1;
    uint8_t snk_support_user_eq : 1;
    uint8_t snk_support_reset_key_map_by_bud : 1;
    uint8_t snk_support_get_set_serial_id : 1;
    uint8_t snk_support_rsv5_3 : 1;

    //byte 6
    uint8_t snk_support_data_capture : 1;
    uint8_t snk_support_anc_apt_coexist : 1; //bit 49
    uint8_t snk_support_spatial_audio : 1;
    uint8_t snk_support_ui_ota_version : 1;
    uint8_t snk_support_anc_apt_scenario_separate : 1; //bit 52
    uint8_t snk_support_3bin_scenario: 1; //bit 53
    uint8_t snk_support_rsv6_2 : 2;

    // Byte 7
    uint8_t snk_support_rsv7_1 : 1;
    uint8_t snk_support_voice_eq : 1; //bit 57
    uint8_t snk_support_anc_llapt_apply_burn : 1; //bit 58
    uint8_t snk_support_spk_eq_independent_cfg : 1;//bit 59
    uint8_t snk_support_spk_eq_compensation_cfg : 1;//bit 60
    uint8_t snk_support_log_status_control : 1;//bit 61
    uint8_t snk_not_support_normal_apt_volume_adjust : 1;//bit 62
    uint8_t snk_not_support_llapt_volume_adjust : 1;//bit 63

    //Byte 8
    uint8_t snk_support_rsv8_1 : 1;
    uint8_t snk_support_listening_mode_custom_cycle : 1;//bit65
    uint8_t snk_support_ullrha : 1;//bit66
    uint8_t snk_support_rsv8_2 : 1;//bit67
    uint8_t snk_support_charger_case : 1;//bit68
    uint8_t snk_support_rsv8_3 : 3;
} T_SNK_CAPABILITY;

/**  @brief  phone send this cmd to soc.*/
typedef enum
{
    TTS_SESSION_OPEN = 0x00,
    TTS_SESSION_PAUSE = 0x01,
    TTS_SESSION_RESUME = 0x02,
    TTS_SESSION_ABORT = 0x03,
    TTS_SESSION_CLOSE = 0x04,
    TTS_SESSION_SEND_SINGLE_FRAME = 0x05,
    TTS_SESSION_SEND_START_FRAME = 0x06,
    TTS_SESSION_SEND_CONTINUE_FRAME = 0x07,
    TTS_SESSION_SEND_END_FRAME = 0x08
} T_TTS_SESSION_CMD_TYPE;

/**  @brief  soc send this event to phone.*/
typedef enum
{
    TTS_SESSION_OPEN_REQ = 0x00,
    TTS_SESSION_PAUSE_REQ = 0x01,
    TTS_SESSION_RESEME_REQ = 0x02,
    TTS_SESSION_ABORT_REQ = 0x03,
    TTS_SESSION_CLOSE_REQ = 0x04,
    TTS_SESSION_SEND_ENCODE_DATA = 0x05
} T_TTS_SESSION_EVENT_TYPE;

/**  @brief  caller id type
  */
typedef enum
{
    CALLER_ID_NUMBER = 0x00,
    CALLER_ID_NAME = 0x01
} T_CALLER_ID_TYPE;

/**  @brief  cmd set status to phone
  */
typedef enum
{
    CMD_SET_STATUS_COMPLETE = 0x00,
    CMD_SET_STATUS_DISALLOW = 0x01,
    CMD_SET_STATUS_UNKNOW_CMD = 0x02,
    CMD_SET_STATUS_PARAMETER_ERROR = 0x03,
    CMD_SET_STATUS_BUSY = 0x04,
    CMD_SET_STATUS_PROCESS_FAIL = 0x05,
    CMD_SET_STATUS_ONE_WIRE_EXTEND = 0x06,
} T_AU_CMD_SET_STATUS;

typedef struct
{
    uint32_t flash_data_start_addr_tmp;
    uint32_t flash_data_start_addr;
    uint32_t flash_data_size;
    uint8_t flash_data_type;
} T_FLASH_DATA;

typedef union
{
    uint8_t d8[16];
    struct
    {
        uint32_t ver_major: 4;      //!< major version
        uint32_t ver_minor: 8;      //!< minor version
        uint32_t ver_revision: 15;  //!< revision version
        uint32_t ver_reserved: 5;   //!< reserved
        uint32_t ver_commitid;      //!< git commit id
        uint8_t customer_name[8];   //!< customer name
    };
} T_PATCH_IMG_VER_FORMAT;

typedef union
{
    uint32_t version;
    struct
    {
        uint32_t ver_major: 8;      //!< major version
        uint32_t ver_minor: 8;      //!< minor version
        uint32_t ver_revision: 8;   //!< revision version
        uint32_t ver_reserved: 8;   //!< reserved
    };
} T_APP_UI_IMG_VER_FORMAT;

typedef union
{
    uint8_t version[4];
    struct
    {
        uint8_t cmd_set_ver_major;
        uint8_t cmd_set_ver_minor;
        uint8_t eq_spec_ver_major;
        uint8_t eq_spec_ver_minor;
    };
} T_SRC_SUPPORT_VER_FORMAT;

typedef enum
{
    LE_RSSI,
    LEGACY_RSSI
} T_RSSI_TYPE;

typedef enum
{
    DSP_TOOL_OPCODE_BRIGHTNESS = 0x0000,
    DSP_TOOL_OPCODE_HW_EQ      = 0x0001,
    DSP_TOOL_OPCODE_GAIN       = 0x0002,
    DSP_TOOL_OPCODE_SW_EQ      = 0x0003,
} T_CMD_DSP_TOOL_OPCODE;

#if (C_APP_PBAP_CMD_SUPPORT == 1)
typedef struct
{
    uint8_t method;
    uint8_t storage;
    uint8_t repos;
    uint8_t phone_book;
    uint8_t download_flag;
    uint16_t browser_start;
    uint16_t browser_end;
    uint16_t list_count;
    uint64_t filter;
} T_PBAP_DOWNLOAD_INFO;
#endif

/** End of APP_CMD_Exported_Types
    * @}
    */
/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_CMD_Exported_Functions App Cmd Functions
    * @{
    */
/**
    * @brief  App process uart or embedded spp vendor command.
    * @param  cmd_ptr command type
    * @param  cmd_len command length
    * @param  cmd_path command path use for distinguish uart,or le,or spp channel.
    * @param  rx_seqn recieved command sequence
    * @param  app_idx received rx command device index
    * @return void
    */
void app_handle_cmd_set(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t rx_seqn,
                        uint8_t app_idx);


#if (F_APP_OTA_TOOLING_SUPPORT == 1)
void app_cmd_ota_tooling_parking(void);
void app_cmd_stop_ota_parking_power_off(void);
#endif

#if(C_APP_DEVICE_CMD_SUPPORT == 1)
bool app_cmd_get_auto_reject_conn_req_flag(void);
bool app_cmd_get_auto_accept_conn_req_flag(void);
bool app_cmd_get_report_attr_info_flag(void);
void app_cmd_set_report_attr_info_flag(bool flag);
#endif

#if (C_APP_PBAP_CMD_SUPPORT == 1)
void app_cmd_pbap_download(uint8_t *bd_addr, uint16_t pb_size);
void app_cmd_pbap_download_check(uint8_t *bd_addr);
bool app_cmd_get_auto_pbap_download_continue_flag(void);
#endif

/** @} */ /* End of group APP_CMD_Exported_Functions */
/** End of APP_CMD
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_CMD_H_ */
