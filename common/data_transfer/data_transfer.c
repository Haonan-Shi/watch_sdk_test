/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "stdlib_corecrt.h"
#include "os_timer.h"
#include "trace.h"
#include "os_queue.h"
#include "data_transfer.h"
#include "app_timer.h"

typedef struct t_dt_item
{
    struct t_dt_item *p_next;
    uint8_t  timer_idx;
    uint16_t id;
    T_DT_DB  *data_transfer;
} T_DT_ITEM;

typedef struct
{
    T_OS_QUEUE data_trans_list;
    uint16_t uuid;
} T_DT_LIST;

typedef enum
{
    DATA_TRANS_TIMER_NEXT,
} T_DATA_TRANS_TIMER;

typedef enum
{
    DT_POP_OK                  = 0x00,
    DT_POP_STOP                = 0x01,
    DT_POP_NO_DAT              = 0x02,
    DT_POP_NOT_IDLE            = 0x03,
    DT_POP_HOLD                = 0x04,
    DT_POP_NOT_AVAILABLE       = 0x05,
    DT_POP_NO_SEEK             = 0x06,
} T_DT_POP_STATE;

T_DT_LIST dt_list = {0};

static T_DT_RESEND dt_resend = {0};
static uint8_t data_transfer_timer_id = 0;
/*ref T_DT_MODE,not change order*/
static const T_DT_STATE data_transfer_switch_mode_map[] = {DT_STATE_WAIT_ACK, DT_STATE_NEXT, DT_STATE_NEXT};

static uint16_t data_transfer_get_timeout(T_DT_STATE state)
{
    if (state == DT_STATE_WAIT_READY)
    {
        return dt_resend.wait_ready_ms;
    }
    else if (state == DT_STATE_WAIT_ACK)
    {
        return dt_resend.wait_ack_ms;
    }
    else if (state == DT_STATE_RETRY)
    {
        return dt_resend.retry_ms;
    }
    else
    {
        return 0;
    }
}

static T_DT_ITEM *data_transfer_peek(T_DT_DB  *data_transfer, uint16_t id)
{
    bool checked = false;

    T_DT_ITEM *item = os_queue_peek(&dt_list.data_trans_list, 0);

    while (item != NULL)
    {
        if (data_transfer == NULL)
        {
            if (item->id == id)
            {
                checked = true;
                break;
            }
        }
        else
        {
            if (item->data_transfer == data_transfer)
            {
                checked = true;
                break;
            }
        }

        item = item->p_next;
    }

    return checked ? item : NULL;
}

static void data_transfer_drop_data_queue(T_DT_HANDLE handle)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_DATA *r_dt_data = &data_transfer->queue_data[data_transfer->r_idx];

    APP_PRINT_INFO7("data_transfer_drop_data_queue: handle %p r_idx %d w_idx %d state %d pop %d resend %d result %d",
                    handle,
                    data_transfer->r_idx, data_transfer->w_idx,
                    data_transfer->state, data_transfer->pop,
                    data_transfer->resend, data_transfer->result);

    if (r_dt_data->used == 0)
    {
        goto drop_data;
    }

    if (r_dt_data->pkt_ptr)
    {
        free(r_dt_data->pkt_ptr);
        r_dt_data->pkt_ptr = NULL;
    }

    r_dt_data->pkt_len = 0;
    r_dt_data->used = 0;
    r_dt_data->idx = 0;
    r_dt_data->mode = DT_MODE_NEED_ACK;

    data_transfer->r_idx++;
    if (data_transfer->r_idx >= data_transfer->queue_num)
    {
        data_transfer->r_idx = 0;
    }

drop_data:
    data_transfer->resend = 0;
    data_transfer->state = DT_STATE_IDLE;
}

static void data_transfer_switch_next(T_DT_HANDLE handle)
{
    data_transfer_drop_data_queue(handle);
    data_transfer_pop_data_queue(handle);
}

void data_transfer_switch_check(T_DT_HANDLE handle, uint16_t event_id, bool is_big_end)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_ITEM *item = data_transfer_peek(data_transfer, 0);
    T_DT_DATA *r_dt_data = &data_transfer->queue_data[data_transfer->r_idx];
    uint16_t tx_event_id = 0;

    if (r_dt_data->pkt_ptr != NULL)
    {
        if (is_big_end)
        {
            tx_event_id = ((r_dt_data->pkt_ptr[data_transfer->offset + 1]) |
                           (r_dt_data->pkt_ptr[data_transfer->offset] << 8));
        }
        else
        {
            tx_event_id = ((r_dt_data->pkt_ptr[data_transfer->offset]) |
                           (r_dt_data->pkt_ptr[data_transfer->offset + 1] << 8));
        }
    }

    APP_PRINT_INFO7("data_transfer_switch_check: handle %p event_id 0x%04x 0x%04x state %d offset %d w_idx %d r_idx %d",
                    handle, event_id, tx_event_id, data_transfer->state, data_transfer->offset,
                    data_transfer->w_idx, data_transfer->r_idx);

    if (r_dt_data->pkt_ptr == NULL)
    {
        return;
    }

    if (tx_event_id != event_id)
    {
        return;
    }

    if (data_transfer->state != DT_STATE_WAIT_ACK)
    {
        return;
    }

    if (item != NULL)
    {
        app_stop_timer(&item->timer_idx);
    }
    data_transfer_switch_next(handle);
}

void data_transfer_queue_reset(T_DT_HANDLE handle)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_ITEM *item = data_transfer_peek(data_transfer, 0);

    if (item != NULL)
    {
        app_stop_timer(&item->timer_idx);
    }

    for (uint8_t idx = 0 ; idx < data_transfer->queue_num; idx++)
    {
        if (data_transfer->queue_data[idx].used != 0)
        {
            data_transfer->queue_data[idx].used = 0;

            if (data_transfer->queue_data[idx].pkt_ptr != NULL)
            {
                free(data_transfer->queue_data[idx].pkt_ptr);
                data_transfer->queue_data[idx].pkt_ptr = NULL;
            }
        }
    }

    data_transfer->w_idx = 0;
    data_transfer->r_idx = 0;
    data_transfer->resend = 0;
    data_transfer->state = DT_STATE_IDLE;

    APP_PRINT_INFO0("data_transfer_queue_reset");
}

static T_DT_STATE data_transfer_get_next_state(uint8_t result, T_DT_MODE mode)
{
    T_DT_STATE state = DT_STATE_IDLE;

    if (mode == DT_MODE_NO_COND)
    {
        return DT_STATE_NEXT;
    }

    if (result != DT_OK)
    {
        return DT_STATE_RETRY;
    }

    return data_transfer_switch_mode_map[mode];
}

void data_transfer_cfg_hold_flag(T_DT_HANDLE handle, bool hold)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;

    data_transfer->hold = hold;
}

bool data_transfer_is_empty(T_DT_HANDLE handle)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;

    return (data_transfer->w_idx == data_transfer->r_idx) ? true : false;
}

void data_transfer_pop_data_queue(T_DT_HANDLE handle)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_ITEM *item = data_transfer_peek(data_transfer, 0);
    T_DT_DATA *r_dt_data = &data_transfer->queue_data[data_transfer->r_idx];
    uint16_t time_out = 0;

    data_transfer->pop = DT_POP_OK;

    if (data_transfer->hold)
    {
        data_transfer->state = DT_STATE_WAIT_READY;
        data_transfer->pop = DT_POP_HOLD;
        return;
    }

    if (data_transfer->state != DT_STATE_IDLE)
    {
        data_transfer->pop = DT_POP_NOT_IDLE;
        return;
    }

    if (data_transfer->w_idx == data_transfer->r_idx)
    {
        data_transfer->state = DT_STATE_IDLE;

        if (data_transfer->finish != NULL)
        {
            data_transfer->finish();
        }

        data_transfer->pop = DT_POP_NO_DAT;
        return;
    }

    if (r_dt_data->used == 0)
    {
        data_transfer->state = DT_STATE_IDLE;
        data_transfer->pop = DT_POP_NOT_AVAILABLE;
        return;
    }

    if (item == NULL)
    {
        data_transfer->pop = DT_POP_NO_SEEK;
        return;
    }

    data_transfer->result = data_transfer->send(r_dt_data->idx, r_dt_data->pkt_ptr, r_dt_data->pkt_len);
    data_transfer->state = data_transfer_get_next_state(data_transfer->result, r_dt_data->mode);

    time_out = data_transfer_get_timeout(data_transfer->state);

    if (time_out == 0)
    {
        data_transfer_switch_next(handle);
    }
    else
    {
        app_start_timer(&item->timer_idx, "data_trans", data_transfer_timer_id,
                        DATA_TRANS_TIMER_NEXT, item->id, false, time_out);
    }
}

bool data_transfer_push_data_queue(T_DT_HANDLE handle, uint8_t idx, uint8_t *data,
                                   uint16_t len, T_DT_MODE mode)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_DATA *w_dt_data = &data_transfer->queue_data[data_transfer->w_idx];

    if (w_dt_data->used != 0)
    {
        data_transfer_queue_reset(handle);
        w_dt_data = &data_transfer->queue_data[data_transfer->w_idx];
    }

    w_dt_data->used = 1;
    w_dt_data->pkt_ptr = data;
    w_dt_data->pkt_len = len;
    w_dt_data->idx = idx;
    w_dt_data->mode = mode;

    data_transfer->w_idx++;
    if (data_transfer->w_idx >= data_transfer->queue_num)
    {
        data_transfer->w_idx = 0;
    }

    APP_PRINT_INFO8("data_transfer_push_data_queue: handle %p idx %d r_idx %d w_idx %d state %d pop %d resend %d result %d",
                    handle, idx,
                    data_transfer->r_idx, data_transfer->w_idx,
                    data_transfer->state, data_transfer->pop,
                    data_transfer->resend, data_transfer->result);

    data_transfer_pop_data_queue(handle);

    return true;
}

static void data_transfer_resend(T_DT_HANDLE handle, uint8_t resend_num)
{
    T_DT_DB *data_transfer = (T_DT_DB *)handle;

    if (data_transfer->no_ack != NULL)
    {
        data_transfer->resend = resend_num;
        data_transfer->no_ack();
    }

    data_transfer->resend++;
    if (data_transfer->resend >= resend_num)
    {
        data_transfer_drop_data_queue(handle);
    }

    data_transfer->state = DT_STATE_IDLE;
    data_transfer_pop_data_queue(handle);
}

static void data_transfer_handle_timeout(uint8_t timer_evt, uint16_t param)
{
    T_DT_HANDLE handle = NULL;
    T_DT_ITEM *item = data_transfer_peek(NULL, param);
    uint8_t state = 0;

    if (item == NULL)
    {
        goto data_trans;
    }

    if (item->data_transfer == NULL)
    {
        goto data_trans;
    }

    handle = (T_DT_HANDLE)item->data_transfer;

    switch (timer_evt)
    {
    case DATA_TRANS_TIMER_NEXT:
        {
            app_stop_timer(&item->timer_idx);
            state = item->data_transfer->state;

            switch (state)
            {
            case DT_STATE_NEXT:
                {
                    data_transfer_switch_next(handle);
                }
                break;

            case DT_STATE_WAIT_ACK:
                {
                    data_transfer_resend(handle, dt_resend.wait_ack_num);
                }
                break;

            case DT_STATE_RETRY:
                {
                    data_transfer_resend(handle, dt_resend.retry_num);
                }
                break;

            case DT_STATE_WAIT_READY:
                {
                    item->data_transfer->state = DT_STATE_IDLE;
                    data_transfer_pop_data_queue(handle);
                }
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

data_trans:
    APP_PRINT_TRACE4("data_transfer_handle_timeout: timer_id %d, timer_chann %d item %p state %d",
                     timer_evt, param, item, state);
}

void data_transfer_release(T_DT_HANDLE handle)
{
    int32_t ret = 0;
    T_DT_DB *data_transfer = (T_DT_DB *)handle;
    T_DT_ITEM *item = data_transfer_peek(data_transfer, 0);

    if (handle == NULL)
    {
        ret = 1;
        goto release_print;
    }

    if (item != NULL)
    {
        if (os_queue_delete(&dt_list.data_trans_list, item))
        {
            free(item);
        }
    }

    if (data_transfer->queue_data == NULL)
    {
        ret = 2;
        goto release_print;
    }

    **data_transfer->p_handle = NULL;

    free(data_transfer->queue_data);
    free(data_transfer);

release_print:
    APP_PRINT_TRACE1("data_transfer_release: ret %d", -ret);
}

T_DT_HANDLE data_transfer_create(T_DT_PARA *para)
{
    T_DT_DB *data_transfer = NULL;
    T_DT_DATA *queue_data = NULL;
    T_DT_ITEM *item = NULL;
    int32_t ret = 0;

    data_transfer = calloc(1, sizeof(T_DT_DB));

    if (data_transfer == NULL)
    {
        ret = 1;
        goto create_print;
    }

    queue_data = calloc(para->queue_num, sizeof(T_DT_DATA));

    if (queue_data == NULL)
    {
        free(data_transfer);
        data_transfer = NULL;
        ret = 2;
        goto create_print;
    }

    item = calloc(1, sizeof(T_DT_ITEM));
    if (item == NULL)
    {
        free(queue_data);
        free(data_transfer);
        queue_data = NULL;
        data_transfer = NULL;
        ret = 3;
        goto create_print;
    }

    data_transfer->p_handle = para->p_handle;
    data_transfer->queue_data = queue_data;
    data_transfer->queue_num = para->queue_num;
    data_transfer->send = para->send;
    data_transfer->hold = para->hold;
    data_transfer->start = para->start;
    data_transfer->finish = para->finish;
    data_transfer->no_ack = para->no_ack;
    data_transfer->offset = para->offset;

    item->id = dt_list.uuid++;
    item->data_transfer = data_transfer;
    os_queue_in(&dt_list.data_trans_list, item);

create_print:
    APP_PRINT_TRACE2("data_transfer_create: handle %p ret %d", data_transfer, -ret);

    return (T_DT_HANDLE)data_transfer;
}

void data_transfer_init(T_DT_RESEND *resend)
{
    memcpy((uint8_t *)&dt_resend, (uint8_t *)resend, sizeof(T_DT_RESEND));
    app_timer_reg_cb(data_transfer_handle_timeout, &data_transfer_timer_id);
}
