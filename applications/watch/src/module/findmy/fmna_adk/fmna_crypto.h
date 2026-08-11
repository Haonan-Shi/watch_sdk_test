/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#ifndef fmna_crypto_h
#define fmna_crypto_h

#include "fmna_constants.h"
#include <stdbool.h>

#define SOFTWARE_AUTH_UUID_ADDR                APPLE_AUTH_TOKEN_SAVE_ADDRESS
#define SOFTWARE_AUTH_TOKEN_ADDR               SOFTWARE_AUTH_UUID_ADDR + SOFTWARE_AUTH_UUID_BLEN

extern bool serial_number_read_state;

typedef enum
{
    FMNA_SERIAL_NUMBER_QUERY_TYPE_TAP,
    FMNA_SERIAL_NUMBER_QUERY_TYPE_BT,
} FMNA_Serial_Number_Query_Type_t;

typedef struct
{
    uint8_t  c1[C1_BLEN];
    uint8_t  e2[E2_BLEN];
} __attribute__((packed)) fmna_send_pairing_data_t;

extern fmna_send_pairing_data_t *p_fmna_send_pairing_data;

typedef struct
{
    uint8_t session_nonce[SESSION_NONCE_BLEN];
    uint8_t e1[E1_BLEN];
} __attribute__((packed)) fmna_initiate_pairing_data_t;

extern fmna_initiate_pairing_data_t *p_fmna_initiate_pairing_data;

typedef struct
{
    uint8_t c2[C2_BLEN];
    uint8_t e3[E3_BLEN];
    uint8_t seeds[SEEDS_BLEN];
    uint8_t icloud_id[ICLOUD_IDENTIFIER_BLEN];
    uint8_t s2[S2_BLEN];
} __attribute__((packed)) fmna_finalize_pairing_data_t;

extern fmna_finalize_pairing_data_t *p_fmna_finalize_pairing_data;

typedef struct
{
    uint8_t     c3[C3_BLEN];
    uint32_t    status;
    uint8_t     e4[E4_BLEN];
} __attribute__((packed)) fmna_send_pairing_status_t;

extern fmna_send_pairing_status_t *p_fmna_send_pairing_status;

typedef struct
{
    uint8_t m_software_auth_uuid[SOFTWARE_AUTH_UUID_BLEN];
    uint8_t *p_software_auth_token;
} __attribute__((aligned(SYS_POINTER_BSIZE), packed)) mfi_info_t;

extern uint8_t *p_fmna_encrypted_serial_number_payload;

void fmna_timer_init(void);
void fmna_crypto_init(void);

fmna_ret_code_t fmna_crypto_generate_send_pairing_data_params(void);
fmna_ret_code_t fmna_crypto_finalize_pairing(void);
fmna_ret_code_t fmna_crypto_pairing_complete(void);

fmna_ret_code_t fmna_crypto_roll_primary_key(void);
fmna_ret_code_t fmna_crypto_roll_secondary_key(void);
fmna_ret_code_t fmna_crypto_roll_primary_sk(void);
fmna_ret_code_t fmna_crypto_roll_secondary_sk(void);

void serial_number_read_state_init(void);
fmna_ret_code_t fmna_crypto_generate_serial_number_response(FMNA_Serial_Number_Query_Type_t type);
uint8_t *fmna_crypto_get_serial_number_raw(void);

void fmna_crypto_unpair(void);

void fmna_log_mfi_token_help(void);
void fmna_log_mfi_token(void);
void fmna_log_serial_number(void);

#endif /* fmna_crypto_h */
