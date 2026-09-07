/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                      Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

typedef void *T_DT_HANDLE;
typedef void (*P_DT_START_CB)(void);
typedef void (*P_DT_FIN_CB)(void);
typedef uint8_t (*P_DT_SEND_CB)(uint8_t idx, uint8_t *data, uint32_t len);
typedef void (*P_DT_NO_ACK_CB)(void);

typedef enum
{
    DT_OK                     = 0x00,
    DT_NOT_CONN               = 0x01,
    DT_NOT_READY              = 0x02,
    DT_SEND_FAIL              = 0x03,
} T_DT_RESULT;

typedef enum
{
    DT_MODE_NEED_ACK          = 0x00,
    DT_MODE_NO_NEED_ACK       = 0x01,
    DT_MODE_NO_COND           = 0x02,
} T_DT_MODE;

typedef enum
{
    DT_STATE_IDLE             = 0x00,
    DT_STATE_NEXT             = 0x01,
    DT_STATE_WAIT_ACK         = 0x02,
    DT_STATE_RETRY            = 0x03,
    DT_STATE_WAIT_READY       = 0x04,

    DT_STATE_MAX              = 0x05,
} T_DT_STATE;

typedef struct
{
    uint8_t                   retry_num;
    uint16_t                  retry_ms;

    uint8_t                   wait_ack_num;
    uint16_t                  wait_ack_ms;

    uint16_t                  wait_ready_ms;
} T_DT_RESEND;

typedef struct
{
    uint8_t                   used;
    uint8_t                   *pkt_ptr;
    uint16_t                  pkt_len;
    uint8_t                   idx;
    T_DT_MODE                 mode;
} T_DT_DATA;

typedef struct
{
    T_DT_HANDLE               **p_handle;
    uint8_t                   queue_num;
    P_DT_START_CB             start;
    P_DT_FIN_CB               finish;
    P_DT_SEND_CB              send;
    P_DT_NO_ACK_CB            no_ack;
    uint16_t                  offset;
    bool                      hold;
} T_DT_PARA;

typedef struct
{
    T_DT_HANDLE               **p_handle;
    T_DT_DATA                 *queue_data;
    uint8_t                   queue_num;
    uint8_t                   w_idx;
    uint8_t                   r_idx;
    uint8_t                   resend;
    T_DT_STATE                state;
    P_DT_START_CB             start;
    P_DT_FIN_CB               finish;
    P_DT_SEND_CB              send;
    P_DT_NO_ACK_CB            no_ack;
    uint16_t                  offset;
    bool                      hold;
    uint8_t                   pop;
    uint8_t                   result;
} T_DT_DB;


bool data_transfer_push_data_queue(T_DT_HANDLE handle, uint8_t idx, uint8_t *data,
                                   uint16_t len, uint8_t mode);
void data_transfer_switch_check(T_DT_HANDLE handle, uint16_t event_id, bool is_big_end);
void data_transfer_queue_reset(T_DT_HANDLE handle);
void data_transfer_pop_data_queue(T_DT_HANDLE handle);
void data_transfer_cfg_hold_flag(T_DT_HANDLE handle, bool hold);
bool data_transfer_is_empty(T_DT_HANDLE handle);
void data_transfer_release(T_DT_HANDLE handle);
T_DT_HANDLE data_transfer_create(T_DT_PARA *para);
void data_transfer_init(T_DT_RESEND *resend);
#ifdef  __cplusplus
}
#endif      /* __cplusplus */

#endif
