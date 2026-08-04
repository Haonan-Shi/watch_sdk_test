/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_HFP_H_
#define _APP_HFP_H_

#include "bt_hfp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_HFP App Hfp
  * @brief this file handle hfp profile related process
  * @{
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/

/* Event bus topics for HFP (App -> GUI) */
#define EVENT_BUS_TOPIC_HFP_ALL_TOPIC      "hfp/*"
#define EVENT_BUS_TOPIC_HFP_INCOMING       "hfp/incoming"
#define EVENT_BUS_TOPIC_HFP_ANSWERED       "hfp/answered"
#define EVENT_BUS_TOPIC_HFP_ENDED          "hfp/ended"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum t_app_hfp_call_status
{
    APP_HFP_CALL_IDLE                              = 0x00,
    APP_HFP_VOICE_ACTIVATION_ONGOING               = 0x01,
    APP_HFP_CALL_INCOMING                          = 0x02,
    APP_HFP_CALL_OUTGOING                          = 0x03,
    APP_HFP_CALL_ACTIVE                            = 0x04,
    APP_HFP_CALL_ACTIVE_WITH_CALL_WAITING          = 0x05,
    APP_HFP_CALL_ACTIVE_WITH_CALL_HELD             = 0x06,
    APP_HFP_MULTILINK_CALL_ACTIVE_WITH_CALL_WAIT   = 0x07,
    APP_HFP_MULTILINK_CALL_ACTIVE_WITH_CALL_HOLD   = 0x08,
} T_APP_HFP_CALL_STATUS;

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_HFP_Exported_Functions App Hfp Functions
    * @{
    */
/**
    * @brief  hfp module init.
    * @param  void
    * @return void
    */
void app_hfp_init(void);

/**
    * @brief  get current call status.
    * @param  void
    * @return @ref T_APP_HFP_CALL_STATUS
    */
T_APP_HFP_CALL_STATUS app_hfp_get_call_status(void);

/**
    * @brief  set current call status.
    * @param  T_APP_HFP_CALL_STATUS
    * @return void
    */
void app_hfp_set_call_status(T_APP_HFP_CALL_STATUS call_status);

/**
    * @brief  get active hfp connection br link id.
    * @param  void
    * @return link id
    */
uint8_t app_hfp_get_active_hf_index(void);

/**
    * @brief  set active hfp connection br link .
    * @param  bd_addr active link bt device address
    * @return link id
    */
bool app_hfp_set_active_hf_index(uint8_t *bd_addr);

/**
    * @brief  Used to play incoming voice prompt on a loop
    * @param  bd_addr: remote BT link id
    * @return void
    */
void app_hfp_ring_alert(uint8_t link_id);

/**
    * @brief  stop ring.
    * @param  void
    * @return void
    */
void app_hfp_stop_ring(void);

/**
    * @brief  Get incoming call phone number
    * @return pointer to the phone number string
    */
const char *app_hfp_get_current_call_number(void);

/**
 * @brief Clear stored call number (should be called when new call starts)
 */
void app_hfp_clear_call_number(void);

/**
 * @brief Set dial number for outgoing call (used by GUI to pass number to MMI)
 * @param number: pointer to the phone number string
 * @param len: length of the phone number
 */
void app_hfp_set_dial_number(const char *number, uint8_t len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_HFP_H_ */
