/*
*      Copyright (C) 2020 Apple Inc. All Rights Reserved.
*
*      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
*      which is contained in the License.txt file distributed with the Find My Network ADK,
*      and only to those who accept that license.
*/

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include <stdlib.h>
#include "fmna_malloc_platform.h"
#include "trace.h"

static T_MALLOC malloc_buf[BUF_TYPE_MAX_SIZE];

void *fmna_malloc(T_BUF_TYPE type, uint16_t len)
{
    void *p_ret = NULL;
    fmna_malloc_status_t status = FMNA_MALLOC_SUCCESS;

    if (type >= BUF_TYPE_MAX_SIZE)
    {
        status = FMNA_MALLOC_ERR_INVALID_TYPE;
        goto exit;
    }

    if (malloc_buf[type].p_buf != NULL)
    {
        status = FMNA_MALLOC_ERR_ALREADY_ALLOCATED;
        goto exit;
    }

    malloc_buf[type].p_buf = malloc(len);
    if (malloc_buf[type].p_buf == NULL)
    {
        status = FMNA_MALLOC_ERR_MALLOC_FAILED;
        goto exit;
    }

    malloc_buf[type].buf_len = len;
    APP_PRINT_INFO3("fmna_malloc: type %d, buf %p, len %d", type, malloc_buf[type].p_buf, len);
    p_ret = malloc_buf[type].p_buf;

exit:
    if (status != FMNA_MALLOC_SUCCESS)
    {
        APP_PRINT_ERROR3("fmna_malloc: failed reason %d, type %d, len %d", status, type, len);
    }
    return p_ret;
}

void fmna_free(T_BUF_TYPE type)
{
    fmna_free_status_t status = FMNA_FREE_SUCCESS;

    if (type >= BUF_TYPE_MAX_SIZE)
    {
        status = FMNA_FREE_ERR_INVALID_TYPE;
        goto exit;
    }

    if (malloc_buf[type].p_buf == NULL)
    {
        status = FMNA_FREE_ERR_NOT_ALLOCATED;
        goto exit;
    }

    APP_PRINT_INFO2("fmna_free: type %d, buf %p", type, malloc_buf[type].p_buf);
    free(malloc_buf[type].p_buf);
    malloc_buf[type].p_buf = NULL;
    malloc_buf[type].buf_len = 0;

exit:
    if (status != FMNA_FREE_SUCCESS)
    {
        APP_PRINT_ERROR2("fmna_free: failed reason %d, type %d", status, type);
    }

    return;
}

void fmna_all_pairing_buf_free(void)
{
    for (uint8_t i = SEND_PAIRING_DATA; i < PAIRING_RX_BUFFER + 1; i++)
    {
        if (malloc_buf[i].p_buf != NULL)
        {
            free(malloc_buf[i].p_buf);
            malloc_buf[i].p_buf = NULL;
            malloc_buf[i].buf_len = 0;
        }
    }
}

#endif
