/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_LINK_UTIL_H_
#define _APP_LINK_UTIL_H_

#include <stdint.h>
#include <stdbool.h>
#include "tts.h"
#include "btm.h"
#include "os_queue.h"
#include "audio_type.h"
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "app_gfps.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_LINK App Link
  * @brief App Link
  * @{
  */

/** max BR/EDR link num */
#define MAX_BR_LINK_NUM                 2

/** max BLE link num */
#define MAX_BLE_LINK_NUM                2

/** bitmask of profiles */
#define A2DP_PROFILE_MASK               0x00000001    /**< A2DP profile bitmask */
#define AVRCP_PROFILE_MASK              0x00000002    /**< AVRCP profile bitmask */
#define HFP_PROFILE_MASK                0x00000004    /**< HFP profile bitmask */
#define RDTP_PROFILE_MASK               0x00000008    /**< Remote Control vendor profile bitmask */
#define SPP_PROFILE_MASK                0x00000010    /**< SPP profile bitmask */
#define IAP_PROFILE_MASK                0x00000020    /**< iAP profile bitmask */
#define PBAP_PROFILE_MASK               0x00000040    /**< PBAP profile bitmask */
#define HSP_PROFILE_MASK                0x00000080    /**< HSP profile bitmask */
#define HID_PROFILE_MASK                0x00000100    /**< HID profile bitmask */
#define GATT_PROFILE_MASK               0x00008000    /**< GATT profile bitmask */
#define GFPS_PROFILE_MASK               0x00010000    /**< GFPS profile bitmask */
#define XIAOAI_PROFILE_MASK             0x00020000    /**< XIAOAI profile bitmask */
#define AMA_PROFILE_MASK                0x00040000    /**< AMA profile bitmask */
#define AVP_PROFILE_MASK                0x00080000    /**< AVP profile bitmask */
#define DID_PROFILE_MASK                0x80000000    /**< DID profile bitmask */

#define ALL_PROFILE_MASK                (A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK | HFP_PROFILE_MASK | \
                                         HSP_PROFILE_MASK | RDTP_PROFILE_MASK | SPP_PROFILE_MASK | \
                                         IAP_PROFILE_MASK | PBAP_PROFILE_MASK)

#define MAX_ADV_SET_NUMBER              6

/**  @brief iap data session status */
typedef enum
{
    DATA_SESSION_CLOSE = 0x00,
    DATA_SESSION_LAUNCH = 0x01,
    DATA_SESSION_OPEN = 0x02,
} T_DATA_SESSION_STATUS;

typedef enum
{
    APP_HF_STATE_STANDBY = 0x00,
    APP_HF_STATE_CONNECTED = 0x01,
} T_APP_HF_STATE;

typedef enum
{
    LE_LOCAL_DISC_CAUSE_UNKNOWN = 0x00,
    LE_LOCAL_DISC_CAUSE_POWER_OFF = 0x01,
    LE_LOCAL_DISC_CAUSE_SWITCH_TO_OTA = 0x02,
    LE_LOCAL_DISC_CAUSE_AUTHEN_FAILED = 0x03,
    LE_LOCAL_DISC_CAUSE_TTS_DISC = 0x04,
    LE_LOCAL_DISC_CAUSE_OTA_RESET = 0x05,
    LE_LOCAL_DISC_CAUSE_GFPS_STOP = 0x06,
    LE_LOCAL_DISC_CAUSE_GFPS_FAILED = 0x07,
    LE_LOCAL_DISC_CAUSE_ROLESWAP = 0x08,
    LE_LOCAL_DISC_CAUSE_SECONDARY = 0x09,
    LE_LOCAL_DISC_CAUSE_TUYA = 0x0A,
    LE_LOCAL_DISC_CAUSE_ENGAGE_STREAM_PRIOIRTY = 0x0B,
    LE_LOCAL_DISC_CAUSE_CONSOLE_UART = 0x0C,
    LE_LOCAL_DISC_CAUSE_GFPS_LINK_FULL = 0x0D,
    LE_LOCAL_DISC_CAUSE_GFPS_FINDER = 0x0E,
    LE_LOCAL_DISC_CAUSE_CHARGING_CASE_OTA = 0x0F,
    LE_LOCAL_DISC_CAUSE_CHARGING_CASE_RECONNECT_PRI_BUD = 0x10,
    LE_LOCAL_DISC_CAUSE_GFPS_PAIR_NOT_RECEIVE_PASSKEY = 0x1A,
    LE_LOCAL_DISC_CAUSE_GFPS_PAIR_NOT_STARTED = 0x1B,
} T_LE_LOCAL_DISC_CAUSE;


/**  @brief  APP's Bluetooth BR/EDR link connection database */
typedef struct
{
    uint8_t             bd_addr[6];
    bool                used;
    uint8_t             id;
    bool                cmd_set_enable;
    uint8_t            *p_embedded_cmd;
    uint8_t            *uart_rx_dt_pkt_ptr;
    void               *tts_handle;
    uint8_t             tts_state;
    uint8_t             tts_seq;
    uint16_t            tts_data_offset;
    uint8_t            *tts_frame_buf;
    uint16_t            tts_frame_len;

    uint8_t             eq_state;
    uint8_t             eq_idx;
    uint8_t             eq_mode;
    uint8_t             eq_type;
    uint8_t             eq_seq;
    uint16_t            eq_data_offset;
    uint8_t            *eq_data_buf;
    uint16_t            eq_data_len;
    T_AUDIO_EFFECT_INSTANCE eq_instance;
    T_AUDIO_EFFECT_INSTANCE nrec_instance;

    uint16_t            embedded_cmd_len;
    uint16_t            uart_rx_dt_pkt_len;

    uint16_t            iap_session_id;
    uint16_t            rfc_frame_size;
    uint8_t             rfc_credit;

    uint16_t            rtk_eap_session_id;

    uint16_t            rfc_spp_frame_size;
    uint8_t             rfc_spp_credit;

    uint16_t            rfc_iap_frame_size;

    uint16_t            rfc_ama_frame_size;
    uint8_t             rfc_ama_credit;


    uint8_t             iap_authen_flag;

    uint8_t             rtk_eap_session_status;

    bool                call_id_type_check;
    bool                call_id_type_num;
    uint32_t            connected_profile;
    uint8_t             tx_event_seqn;
    uint8_t             rx_cmd_seqn;
    uint8_t             resume_fg;
    uint8_t             call_status;
    bool                service_status;
    uint8_t             a2dp_codec_type;
    union
    {
        struct
        {
            uint8_t sampling_frequency;
            uint8_t channel_mode;
            uint8_t block_length;
            uint8_t subbands;
            uint8_t allocation_method;
            uint8_t min_bitpool;
            uint8_t max_bitpool;
        } sbc;
        struct
        {
            uint8_t  object_type;
            uint16_t sampling_frequency;
            uint8_t  channel_number;
            bool     vbr_supported;
            uint32_t bit_rate;
        } aac;
        struct
        {
            uint8_t  info[12];
        } vendor;
    } a2dp_codec_info;
    void                *a2dp_track_handle;
    void                *sco_track_handle;
    uint8_t             sco_seq_num;
    uint8_t             duplicate_fst_sco_data;

    T_BT_LINK_ROLE      acl_link_role;
    uint8_t             pbap_repos;
    bool                sco_initial;
    uint16_t            sco_handle;
    uint8_t             streaming_fg;
    uint8_t             avrcp_play_status;

    uint16_t            acl_handle;
    bool                is_inband_ring;
    uint8_t             *p_gfps_cmd;
    uint16_t            gfps_cmd_len;
    uint8_t             gfps_rfc_chann;

    bool                auth_flag;
    // 0 not encrpyt; 1 encrypted; 2 de-encrpyted (only for b2b link)
    uint8_t             link_encrypt;
    uint8_t             link_role_switch_count;

    T_APP_HF_STATE      app_hf_state;
    uint16_t            src_conn_idx;

    uint16_t            remote_hfp_brsf_capability;
    int8_t              local_rssi;
    uint8_t             disconn_acl_flg: 1;
    uint8_t             acl_link_in_sniffmode_flg: 1;
    uint8_t             roleswitch_check_after_unsniff_flg: 1;
    uint8_t             rtk_vendor_spp_active: 1;
    uint8_t             reserved: 4;

    uint16_t            sniff_mode_disable_flag;
} T_APP_BR_LINK;

typedef void(*P_FUN_LE_LINK_DISC_CB)(uint8_t conn_id, uint8_t local_disc_cause,
                                     uint16_t disc_cause);

typedef struct t_le_disc_cb_entry
{
    struct t_le_disc_cb_entry  *p_next;
    P_FUN_LE_LINK_DISC_CB disc_callback;
} T_LE_DISC_CB_ENTRY;

/**  @brief  App define le link connection database */
typedef struct
{
    uint8_t             bd_addr[6];
    bool                used;
    uint8_t             id;
    bool                cmd_set_enable;
    uint8_t            *p_embedded_cmd;
    void               *tts_handle;
    uint8_t             tts_state;
    uint8_t             tts_seq;
    uint16_t            tts_data_offset;
    uint8_t            *tts_frame_buf;
    uint16_t            tts_frame_len;

    uint8_t             eq_state;
    uint8_t             eq_index;
    uint8_t             eq_type;
    uint8_t             eq_seq;
    uint16_t            eq_data_offset;
    uint8_t            *eq_data_buf;
    uint16_t            eq_data_len;

    uint16_t            embedded_cmd_len;
    uint16_t            mtu_size;
    uint8_t             state;
    uint8_t             conn_id;
    uint8_t             rx_cmd_seqn;
    uint8_t             tx_event_seqn;
    uint8_t             transmit_srv_tx_enable_fg;
    uint8_t             local_disc_cause;
    T_OS_QUEUE          disc_cb_list;
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
    T_GFPS_LINK         gfps_link;
#endif
} T_APP_LE_LINK;

uint32_t app_connected_profiles(void);

/**
    * @brief  get BR/EDR link num wich connected with phone by the mask profile
    * @param  profile_mask the mask profile
    * @return BR/EDR link num
    */
uint8_t app_connected_profile_link_num(uint32_t profile_mask);

/**
    * @brief  get current SCO connected link number
    * @param  void
    * @return SCO connected number
    */
uint8_t app_find_sco_conn_num(void);

/**
    * @brief  get the BR/EDR link by bluetooth address
    * @param  bd_addr bluetooth address
    * @return the BR/EDR link
    */
T_APP_BR_LINK *app_find_br_link(uint8_t *bd_addr);

/**
    * @brief  get the BR/EDR link by tts handle
    * @param  handle tts handle
    * @return the BR/EDR link
    */
T_APP_BR_LINK *app_find_br_link_by_tts_handle(T_TTS_HANDLE handle);

/**
    * @brief  alloc the BR/EDR link by bluetooth address
    * @param  bd_addr bluetooth address
    * @return the BR/EDR link
    */
T_APP_BR_LINK *app_alloc_br_link(uint8_t *bd_addr);

/**
    * @brief  free the BR/EDR link
    * @param  p_link the BR/EDR link
    * @return true: success; false: fail
    */
bool app_free_br_link(T_APP_BR_LINK *p_link);

/**
    * @brief  find the BLE link by connected id
    * @param  conn_id BLE link id(slot)
    * @return the BLE link
    */
T_APP_LE_LINK *app_find_le_link_by_conn_id(uint8_t conn_id);

/**
    * @brief  find the BLE link by bluetooth address
    * @param  bd_addr bluetooth address
    * @return the BLE link
    */
T_APP_LE_LINK *app_find_le_link_by_addr(uint8_t *bd_addr);

/**
    * @brief  find the BLE link by tts handle
    * @param  handle tts handle
    * @return the BLE link
    */
T_APP_LE_LINK *app_find_le_link_by_tts_handle(T_TTS_HANDLE handle);

/**
    * @brief  alloc the BLE link by link id(slot)
    * @param  conn_id BLE link id(slot)
    * @return the BLE link
    */
T_APP_LE_LINK *app_alloc_le_link_by_conn_id(uint8_t conn_id);

/**
    * @brief  free the BLE link
    * @param  p_link the BLE link
    * @return true: success; false: fail
    */
bool app_free_le_link(T_APP_LE_LINK *p_link);

bool app_disconnect_le_link(T_APP_LE_LINK *p_link, T_LE_LOCAL_DISC_CAUSE disc_cause);

bool app_reg_le_link_disc_cb(uint8_t conn_id, P_FUN_LE_LINK_DISC_CB p_fun_cb);

uint8_t app_get_ble_link_num(void);

/**
    * @brief  get the bud2phone link num
    * @param  void
    * @return link num
    */
uint8_t app_find_b2s_link_num(void);


/** End of APP_LINK
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_LINK_UTIL_H_ */
