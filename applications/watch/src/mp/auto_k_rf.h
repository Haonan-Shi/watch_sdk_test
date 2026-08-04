/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUTO_K_RF_H_
#define _AUTO_K_RF_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/** @defgroup APP_RWS_MP App mp
  * @brief App mp
  * @{
  */

/** @brief single tone event */
typedef enum
{
    SINGLE_TONE_EVT_OPENED,     /* single tone open completed */
    SINGLE_TONE_EVT_CLOSED,     /* single tone close completed */
    SINGLE_TONE_EVT_DATA_IND,   /* single tone rx data indicated */
    SINGLE_TONE_EVT_DATA_XMIT,  /* single tone tx data transmitted */
    SINGLE_TONE_EVT_ERROR,      /* single tone error occurred */
} T_SINGLE_TONE_EVT;

/** @brief P_SINGLE_TONE_CALLBACK */
typedef bool (*P_SINGLE_TONE_CALLBACK)(T_SINGLE_TONE_EVT evt, bool status, uint8_t *p_buf,
                                       uint32_t len);
/**
  * @brief  hci open
  * @param  p_callback P_SINGLE_TONE_CALLBACK
  * @retval true   Success
  * @retval false  Failed
  */
extern bool hci_if_open(P_SINGLE_TONE_CALLBACK p_callback);

/**
  * @brief  hci close
  * @retval true   Success
  * @retval false  Failed
  */
extern bool hci_if_close(void);

/**
  * @brief  hci write
  * @param  pbuf   buffer for data
  * @param  len    buffer length
  * @retval true   Success
  * @retval false  Failed
  */
extern bool hci_if_write(uint8_t *p_buf, uint32_t len);

/**
  * @brief  hci confirm
  * @param  pbuf   buffer
  * @retval true   Success
  * @retval false  Failed
  */
extern bool hci_if_confirm(uint8_t *p_buf);

void hci_cmd_vendor_auto_k_rf(uint8_t rx_channel, int8_t freq_drift_upperbound,
                              int8_t freq_drift_lowerbound, int8_t measure_offset);
void mp_hci_task_init(void);
void get_xtal_cap_value(uint8_t *cap_val);

/** End of APP_RWS_MP
* @}
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif  /* _AUTO_K_RF_H_ */
