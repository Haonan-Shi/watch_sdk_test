/*
*      Copyright (C) 2020 Apple Inc. All Rights Reserved.
*
*      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
*      which is contained in the License.txt file distributed with the Find My Network ADK,
*      and only to those who accept that license.
*/

#ifndef fmna_malloc_platform_h
#define fmna_malloc_platform_h

#include "stdbool.h"
#include "stdint.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SEND_PAIRING_DATA = 0,
    INITIATE_PAIRING_DATA,
    FINALIZE_PAIRING_DATA,
    SEND_PAIRING_STATUS,
    MFI_RAW_TOKEN,
    PAIRING_RX_BUFFER,
    ENCRYPTED_SN,
    BUF_TYPE_MAX_SIZE        //Maximum value marker
} T_BUF_TYPE;

typedef enum
{
    FMNA_MALLOC_SUCCESS = 0,
    FMNA_MALLOC_ERR_INVALID_TYPE = 1,
    FMNA_MALLOC_ERR_ALREADY_ALLOCATED = 2,
    FMNA_MALLOC_ERR_MALLOC_FAILED = 3,
} fmna_malloc_status_t;

typedef enum
{
    FMNA_FREE_SUCCESS = 0,
    FMNA_FREE_ERR_INVALID_TYPE = 1,
    FMNA_FREE_ERR_NOT_ALLOCATED = 2,
} fmna_free_status_t;

typedef struct
{
    uint16_t buf_len;
    void *p_buf;
} T_MALLOC;

void *fmna_malloc(T_BUF_TYPE type, uint16_t len);
void fmna_free(T_BUF_TYPE type);
void fmna_all_pairing_buf_free(void);

#ifdef __cplusplus
}
#endif

#endif /* fmna_malloc_platform_h */
