/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "ble_transport.h"
#include "communicate_parse.h"
#include "chatgpt_queue.h"
#include "os_mem.h"
#include "chatgpt_port.h"

/*===================================================================
*                     chatgpt memory
*===================================================================*/
void *chatgpt_realloc(void *mem, uint32_t size)
{
    return realloc(mem, size);
}

void *chatgpt_malloc(uint32_t size)
{
    return malloc(size);
}

void *chatgpt_calloc(uint32_t nblock, uint32_t size)
{
    return calloc(nblock, size);
}

void chatgpt_free(void *pt)
{
    free(pt);
}

/*===================================================================
*                     chatgpt ble
*===================================================================*/

//1. send data
bool chatgpt_port_data_send(uint8_t *data, uint16_t length)
{
    return L1_send(data, length);
}


//2. get state
uint8_t chatgpt_port_get_ble_state(void)
{
    if (RtkWristbandSys.gap_conn_state == GAP_CONN_STATE_CONNECTED)
    {
        return 1;
    }

    return 0;
}


//3. receive handler
uint8_t chatgpt_port_ble_rev_proc(uint8_t key, uint8_t *pBuf, uint16_t length)
{
    return chatgpt_ble_rev_proc(key, pBuf, length);
}

//4. send completed handler
void chatgpt_port_ble_send_completed_proc(void)
{
    chatgpt_ble_send_completed_proc();
}




