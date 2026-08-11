/*
*      Copyright (C) 2020 Apple Inc. All Rights Reserved.
*
*      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
*      which is contained in the License.txt file distributed with the Find My Network ADK,
*      and only to those who accept that license.
*/

#include <stdlib.h>
#include "fmna_malloc_platform.h"
#include "trace.h"

static T_MALLOC malloc_buf[BUF_TYPE_MAX_SIZE];

void *fmna_malloc(T_BUF_TYPE type, uint16_t len)
{
    if (type >= BUF_TYPE_MAX_SIZE)
    {
        return NULL;
    }

    if (malloc_buf[type].p_buf != NULL)
    {
        return NULL;
    }

    malloc_buf[type].p_buf = malloc(len);
    if (malloc_buf[type].p_buf == NULL)
    {
        return NULL;
    }

    malloc_buf[type].buf_len = len;
    APP_PRINT_INFO3("fmna_malloc: type:%d, buf %p, len %d", type, malloc_buf[type].p_buf, len);

    return malloc_buf[type].p_buf;
}

void fmna_free(T_BUF_TYPE type)
{
    if (type >= BUF_TYPE_MAX_SIZE)
    {
        return;
    }

    if (malloc_buf[type].p_buf == NULL)
    {
        return;
    }

    APP_PRINT_INFO2("fmna_free: type %d, buf %p", type, malloc_buf[type].p_buf);
    free(malloc_buf[type].p_buf);
    malloc_buf[type].p_buf = NULL;
    malloc_buf[type].buf_len = 0;
    return;
}

void fmna_all_pairing_buf_free(void)
{
    for (uint8_t i = SEND_PAIRING_DATA; i < PAIRING_RX_BUFFER + 1; i++)
    {
        if (malloc_buf[i].p_buf != NULL)
        {
            APP_PRINT_INFO2("fmna_all_pairing_buf_free: type %d, buf %p", i, malloc_buf[i].p_buf);
            free(malloc_buf[i].p_buf);
            malloc_buf[i].p_buf = NULL;
            malloc_buf[i].buf_len = 0;
        }
    }
}
