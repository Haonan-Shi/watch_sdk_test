/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_DATA_TRANSFER_H_
#define _APP_DATA_TRANSFER_H_

#include "data_transfer.h"
#include "app_msg.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    DATA_TRANS_PATH_NONE                           = 0x00,
    DATA_TRANS_PATH_UART                           = 0x01,
    DATA_TRANS_PATH_LE                             = 0x02,
    DATA_TRANS_PATH_SPP                            = 0x03,
    DATA_TRANS_PATH_IAP                            = 0x04,
    DATA_TRANS_PATH_GATT_OVER_BREDR                = 0x05,

    DATA_TRANS_PATH_MAX                            = 0x06,
} T_DATA_TRANS_PATH;

typedef struct
{
    T_DT_HANDLE               *p_handle;
    P_DT_SEND_CB              send;
    P_DT_SEND_CB              send_instance;
    P_DT_START_CB             start;
    P_DT_FIN_CB               finish;

    uint8_t                   queue_num;
} T_DATA_TRANS_MAP;

typedef struct
{
    T_DATA_TRANS_MAP    *map;
    bool                enable;

    T_DT_RESEND         resend;
} T_DATA_TRANS;

extern T_DATA_TRANS data_trans;

/** @defgroup APP_TRANSFER App Transfer
  * @brief App Transfer
  * @{
  */

/*============================================================================*
 *                              Functions
 *============================================================================*/
/** @defgroup APP_TRANSFER_Exported_Functions App Transfer Functions
    * @{
    */

/**
    * @brief  data transfer push data
    *
    * @param  cmd_path distinguish legacy or le path
    * @param  link_idx app link channel index
    * @param  *p_data need to transfer data
    * @param  data_len transfer data length
    * @param  mode switch mode
    * @return bool
    */
bool app_data_transfer_push_data_queue(uint8_t cmd_path, uint8_t idx, uint8_t *data, uint16_t len,
                                       T_DT_MODE mode);

/**
    * @brief  data transfer pop data
    *
    * @param  cmd_patch
    * @return void
    */
void app_data_transfer_pop_data_queue(uint8_t cmd_path);

/**
    * @brief  data transfer queue reset.
    *
    * @param  cmd_path distinguish legacy, le path or UART
    * @return none
    */
void app_data_transfer_queue_reset(uint8_t cmd_path);

/**
    * @brief  data transfer switch next.
    *
    * @param  cmd_path distinguish legacy, le path
    * @param  event_id event id for check
    * @param  is_big_end is big endian
    * @return none
    */
void app_data_transfer_switch(uint8_t cmd_path, uint16_t event_id, bool is_big_end);

/**
    * @brief  data transfer handle io msg
    *
    * @param  void
    * @return none
    */
void app_data_transfer_handle_msg(T_IO_MSG *io_driver_msg_recv);

/**
    * @brief  data transfer module init.
    *
    * @param  void
    * @return none
    */
void app_data_transfer_init(void);

/** @} */ /* End of group APP_TRANSFER_Exported_Functions */

/** End of APP_TRANSFER
* @}
*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_TRANSFER_H_ */
