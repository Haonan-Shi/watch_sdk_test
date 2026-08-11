/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "fmna_constants.h"
#include "fmna_platform_includes.h"
#include "fmna_crypto.h"
#include "fmna_state_machine.h"
#include "fmna_connection.h"
#include "fmna_malloc_platform.h"
#include "fmna_version.h"
#include "app_timer.h"
#if CONFIG_REALTEK_FINDMY_SUPPORT_NFC
#include "fmna_nfc.h"
#endif
#if !HARDCODED_PAIRING_ENABLED
#include "fm-crypto.h"

#ifdef GENERATE_RANDOM_SERIAL_NUMBER
//TODO: remove this - random UUID
#endif

fmna_send_pairing_data_t        *p_fmna_send_pairing_data;
fmna_initiate_pairing_data_t    *p_fmna_initiate_pairing_data;
fmna_finalize_pairing_data_t    *p_fmna_finalize_pairing_data;
fmna_send_pairing_status_t      *p_fmna_send_pairing_status;

uint8_t *p_fmna_encrypted_serial_number_payload;

bool serial_number_read_state = false;
static uint8_t m_serial_number[SERIAL_NUMBER_RAW_BLEN];
#ifdef GENERATE_RANDOM_SERIAL_NUMBER
static uint8_t m_serial_number_random_map[SERIAL_NUMBER_RAW_BLEN];
#endif

typedef struct
{
    uint8_t  serial_number[SERIAL_NUMBER_RAW_BLEN];
    uint64_t counter;
    char     op[SERIAL_NUMBER_PAYLOAD_OP_BLEN];
} __attribute__((packed)) serial_number_hmac_payload_t;

static serial_number_hmac_payload_t m_serial_number_hmac_payload;

typedef struct
{
    uint8_t  serial_number[SERIAL_NUMBER_RAW_BLEN];
    uint64_t counter;
    uint8_t  hmac[SERIAL_NUMBER_PAYLOAD_HMAC_BLEN];
    char     op[SERIAL_NUMBER_PAYLOAD_OP_BLEN];
} __attribute__((packed)) serial_number_payload_t;

static serial_number_payload_t m_serial_number_payload;

static struct fm_crypto_ckg_context m_fm_crypto_ckg_ctx;

static uint8_t m_seedk1[SK_BLEN];
static uint8_t m_p[P_BLEN];
static uint8_t m_server_shared_secret[SERVER_SHARED_SECRET_BLEN];
static uint64_t serial_number_query_count = 0;

// Hardcoded values, eventually read these on boot from provisioned keys;
static uint8_t m_q_e[APPLE_SERVER_ENCRYPTION_KEY_BLEN];
static uint8_t m_q_a[APPLE_SERVER_SIG_VERIFICATION_KEY_BLEN];

static const uint8_t server_key[88] = FMNA_SERVER_ENCRYPT_KEY;
static const uint8_t sig_key[88] = FMNA_SIGN_VERIFY_KEY;

static mfi_info_t m_mfi_struct;

typedef struct
{
    uint8_t  session_nonce[SESSION_NONCE_BLEN];
    uint8_t  software_auth_token[SOFTWARE_AUTH_TOKEN_BLEN];
    uint8_t  software_auth_uuid[SOFTWARE_AUTH_UUID_BLEN];
    uint8_t  serial_number[SERIAL_NUMBER_RAW_BLEN];
    uint8_t  product_data[PROD_DATA_MAX_LEN];
    uint32_t fw_version;
    uint8_t  e1[E1_BLEN];
    uint8_t  seedk1[SK_BLEN];
} __attribute__((packed)) e2_generation_encryption_msg_t;

typedef struct
{
    uint8_t  software_auth_uuid[SOFTWARE_AUTH_UUID_BLEN];
    uint8_t  serial_number[SERIAL_NUMBER_RAW_BLEN];
    uint8_t  session_nonce[SESSION_NONCE_BLEN];
    uint8_t  e1[E1_BLEN];
    uint8_t  latest_sw_token[SOFTWARE_AUTH_TOKEN_BLEN];
    uint32_t status;
} __attribute__((packed)) e4_generation_encryption_msg_t;

typedef struct
{
    uint8_t  software_auth_uuid[SOFTWARE_AUTH_UUID_BLEN];
    uint8_t  session_nonce[SESSION_NONCE_BLEN];
    uint8_t  seeds[SEEDS_BLEN];
    uint8_t  h1[H1_BLEN];
    uint8_t  e1[E1_BLEN];
    uint8_t  e3[E3_BLEN];
} __attribute__((packed)) s2_verification_msg_t;

// Union to hold buffers for various encryption, verification messages.
typedef union
{
    e2_generation_encryption_msg_t e2_generation_encryption_msg;
    e4_generation_encryption_msg_t e4_generation_encryption_msg;
    s2_verification_msg_t          s2_verification_msg;
} key_verif_encr_msg_t;

e2_generation_encryption_msg_t decrypted_e2_generation_encryption_msg;

static key_verif_encr_msg_t m_key_verif_encr_msg;

static uint8_t m_current_primary_sk[SK_BLEN];
static uint8_t m_current_secondary_sk[SK_BLEN];

#define FM_CRYPTO_STATUS_SUCCESS 0

static uint8_t fmna_timer_id = 0;
static uint8_t timer_idx_sn_lookup = 0;

typedef enum
{
    APP_TIMER_SN_LOOKUP,
} T_APP_FINDMY_TIMER;

static void fmna_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_INFO1("fmna_timeout_cb: timer_evt %d", timer_evt);
    switch (timer_evt)
    {
    case APP_TIMER_SN_LOOKUP:
        {
            app_stop_timer(&timer_idx_sn_lookup);
            serial_number_read_state = false;
            fmna_free(ENCRYPTED_SN);
        }
        break;

    default:
        break;
    }
}

void fmna_timer_init(void)
{
    app_timer_reg_cb(fmna_timeout_cb, &fmna_timer_id);

}

static void populate_e2_generation_encryption_msg(void)
{
    FMNA_LOG_INFO("populate_e2_generation_encryption_msg");
    // Populate the fields for m_key_verif_encr_msg.e2_generation_encryption_msg.

    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.session_nonce,
           p_fmna_initiate_pairing_data->session_nonce,
           SESSION_NONCE_BLEN);

    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.software_auth_uuid,
           m_mfi_struct.m_software_auth_uuid,
           SOFTWARE_AUTH_UUID_BLEN);

    fmc_flash_nor_read(SOFTWARE_AUTH_TOKEN_ADDR,
                       m_key_verif_encr_msg.e2_generation_encryption_msg.software_auth_token,
                       SOFTWARE_AUTH_TOKEN_BLEN);

    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.serial_number,
           m_serial_number,
           SERIAL_NUMBER_RAW_BLEN);

    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.e1,
           p_fmna_initiate_pairing_data->e1,
           E1_BLEN);

    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.seedk1,
           m_seedk1,
           SK_BLEN);

    m_key_verif_encr_msg.e2_generation_encryption_msg.fw_version = fmna_version_get_fw_version();

    uint8_t product_data[PRODUCT_DATA_BLEN] = PRODUCT_DATA_VAL;
    memcpy(m_key_verif_encr_msg.e2_generation_encryption_msg.product_data,
           product_data,
           PRODUCT_DATA_BLEN);
}

static void populate_e4_generation_encryption_msg(void)
{
    FMNA_LOG_INFO("populate_e4_generation_encryption_msg");
    // Populate the fields for m_key_verif_encr_msg.e4_generation_encryption_msg.

    memcpy(m_key_verif_encr_msg.e4_generation_encryption_msg.session_nonce,
           p_fmna_initiate_pairing_data->session_nonce,
           SESSION_NONCE_BLEN);

    memcpy(m_key_verif_encr_msg.e4_generation_encryption_msg.software_auth_uuid,
           m_mfi_struct.m_software_auth_uuid,
           SOFTWARE_AUTH_UUID_BLEN);

    memcpy(m_key_verif_encr_msg.e4_generation_encryption_msg.serial_number,
           m_serial_number,
           SERIAL_NUMBER_RAW_BLEN);

    memcpy(m_key_verif_encr_msg.e4_generation_encryption_msg.e1,
           p_fmna_initiate_pairing_data->e1,
           E1_BLEN);

    memcpy(m_key_verif_encr_msg.e4_generation_encryption_msg.latest_sw_token,
           m_mfi_struct.p_software_auth_token,
           SOFTWARE_AUTH_TOKEN_BLEN);

    m_key_verif_encr_msg.e4_generation_encryption_msg.status = 0;
}

static void populate_s2_verification_msg(void)
{
    FMNA_LOG_INFO("populate_s2_verification_msg");
    // Populate the fields for m_key_verif_encr_msg.s2_verification_msg.

    memcpy(m_key_verif_encr_msg.s2_verification_msg.session_nonce,
           p_fmna_initiate_pairing_data->session_nonce,
           SESSION_NONCE_BLEN);

    memcpy(m_key_verif_encr_msg.s2_verification_msg.software_auth_uuid,
           m_mfi_struct.m_software_auth_uuid,
           SOFTWARE_AUTH_UUID_BLEN);

    memcpy(m_key_verif_encr_msg.s2_verification_msg.seeds,
           p_fmna_finalize_pairing_data->seeds,
           SEEDS_BLEN);

    ftl_save_to_storage(p_fmna_finalize_pairing_data->icloud_id, FTL_SAVE_ICLOUD_ID_ADDR,
                        FTL_SAVE_ICLOUD_ID_SIZE);

    uint32_t ret = fm_crypto_sha256(C2_BLEN, p_fmna_finalize_pairing_data->c2,
                                    m_key_verif_encr_msg.s2_verification_msg.h1);
    FMNA_ERROR_CHECK(ret);

    memcpy(m_key_verif_encr_msg.s2_verification_msg.e1,
           p_fmna_initiate_pairing_data->e1,
           E1_BLEN);

    memcpy(m_key_verif_encr_msg.s2_verification_msg.e3,
           p_fmna_finalize_pairing_data->e3,
           E3_BLEN);
}

static fmna_ret_code_t roll_sk(uint8_t current_sk[SK_BLEN])
{
    FMNA_LOG_INFO("roll_sk");
    uint8_t new_sk[SK_BLEN];
    int ret_code = fm_crypto_roll_sk(current_sk, new_sk);
    if (ret_code != FM_CRYPTO_STATUS_SUCCESS)
    {
        FMNA_LOG_ERROR("roll_sk: err %d", ret_code);
        return FMNA_ERROR_INTERNAL;
    }
    memcpy(current_sk, new_sk, SK_BLEN);

    return FMNA_SUCCESS;
}
#endif // HARDCODED_PAIRING_ENABLED

#ifdef GENERATE_RANDOM_SERIAL_NUMBER
/** @brief Function for getting vector of random numbers.
 *
 * @param[out] p_buff       Pointer to unit8_t buffer for storing the bytes.
 * @param[in]  length       Number of bytes to take from pool and place in p_buff.
 *
 * @retval     Number of bytes actually placed in p_buff.
 */
static uint8_t random_vector_generate(uint8_t *p_buff, uint8_t size)
{
    uint32_t err_code;
    uint8_t  available;

    nrf_drv_rng_bytes_available(&available);
    uint8_t length = MIN(size, available);

    err_code = nrf_drv_rng_rand(p_buff, length);
    FMNA_ERROR_CHECK(err_code);

    return length;
}

static void generate_random_serial_number(void)
{
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789";

    random_vector_generate((uint8_t *)&m_serial_number_random_map, SERIAL_NUMBER_RAW_BLEN);

    for (uint8_t i = 0; i < SERIAL_NUMBER_RAW_BLEN; i++)
    {
        m_serial_number[i] = charset[m_serial_number_random_map[i] % sizeof(charset)];
    }

    FMNA_LOG_INFO("generate_random_serial_number:");
    FMNA_LOG_HEXDUMP_INFO(m_serial_number, 16);
}
#endif

uint8_t *fmna_crypto_get_serial_number_raw(void)
{
    return m_serial_number;
}

void fmna_crypto_init(void)
{
#if !HARDCODED_PAIRING_ENABLED
    FMNA_LOG_INFO("fmna_crypto_init");

    size_t len;

    mbedtls_base64_decode(m_q_e, APPLE_SERVER_ENCRYPTION_KEY_BLEN, &len, server_key, 88);
    mbedtls_base64_decode(m_q_a, APPLE_SERVER_SIG_VERIFICATION_KEY_BLEN, &len, sig_key, 88);

    //Read software auth token, UUID, serial number from accessory factory registers/ flash.
    uint8_t token_preview[16] = {0};
    fmc_flash_nor_read(SOFTWARE_AUTH_TOKEN_ADDR, token_preview, 16);
    FMNA_LOG_INFO("fmna_crypto_init: m_software_auth_token preview %b", TRACE_BINARY(16,
                  token_preview));

    fmc_flash_nor_read(SOFTWARE_AUTH_UUID_ADDR, (uint8_t *)m_mfi_struct.m_software_auth_uuid,
                       SOFTWARE_AUTH_UUID_BLEN);
    FMNA_LOG_INFO("fmna_crypto_init: m_software_auth_uuid %b", TRACE_BINARY(16,
                                                                            m_mfi_struct.m_software_auth_uuid));

    if (fmna_connection_is_fmna_paired())
    {
        uint8_t read_m_p[FTL_SAVE_M_P_SIZE];
        uint8_t read_skn[FTL_SAVE_SKN_SIZE];
        uint8_t read_sks[FTL_SAVE_SKS_SIZE];
        ftl_load_from_storage(read_m_p, FTL_SAVE_M_P_ADDR, FTL_SAVE_M_P_SIZE);
        ftl_load_from_storage(read_skn, FTL_SAVE_SKN_ADDR, FTL_SAVE_SKN_SIZE);
        ftl_load_from_storage(read_sks, FTL_SAVE_SKS_ADDR, FTL_SAVE_SKS_SIZE);
        memcpy(m_p, read_m_p, P_BLEN);
        memcpy(m_current_primary_sk, read_skn, SK_BLEN);
        memcpy(m_current_secondary_sk, read_sks, SK_BLEN);
    }
    else
    {
        int ret = fm_crypto_ckg_init(&m_fm_crypto_ckg_ctx);

        if (FM_CRYPTO_STATUS_SUCCESS != ret)
        {
            FMNA_LOG_ERROR("fmna_crypto_init: fm_crypto_ckg_init err %d", ret);
            FMNA_ERROR_CHECK(FMNA_ERROR_INTERNAL);
        }
    }
#endif // HARDCODED_PAIRING_ENABLED

#ifdef GENERATE_RANDOM_SERIAL_NUMBER
    generate_random_serial_number();
#else
    fmna_connection_platform_get_serial_number(m_serial_number, SERIAL_NUMBER_RAW_BLEN);
#endif
}

fmna_ret_code_t fmna_crypto_generate_send_pairing_data_params(void)
{
#if !HARDCODED_PAIRING_ENABLED

    // Generate C1, SeedK1, and E2.
    int ret = fm_crypto_ckg_gen_c1(&m_fm_crypto_ckg_ctx, p_fmna_send_pairing_data->c1);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_generate_send_pairing_data_params: fm_crypto_ckg_gen_c1 err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    ret = fm_crypto_generate_seedk1(m_seedk1);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_generate_send_pairing_data_params: fm_crypto_generate_seedk1 err %d",
                       ret);
        return FMNA_ERROR_INTERNAL;
    }

    //Generate E2, see Section 6.2.3 for details.

    populate_e2_generation_encryption_msg();
    FMNA_LOG_INFO("fmna_crypto_generate_send_pairing_data_params: E2 generation encryption msg size %d",
                  sizeof(m_key_verif_encr_msg.e2_generation_encryption_msg));
    uint32_t e2_blen = E2_BLEN;
    ret = fm_crypto_encrypt_to_server((const uint8_t *)m_q_e,
                                      sizeof(m_key_verif_encr_msg.e2_generation_encryption_msg),
                                      (const uint8_t *)&m_key_verif_encr_msg.e2_generation_encryption_msg,
                                      &e2_blen,
                                      p_fmna_send_pairing_data->e2);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_generate_send_pairing_data_params: fm_crypto_encrypt_to_server e2 err %d",
                       ret);
        return FMNA_ERROR_INTERNAL;
    }
    FMNA_LOG_INFO("fmna_crypto_generate_send_pairing_data_params: e2_blen %lu", e2_blen);
#endif //HARCODED_PAIRING_ENABLED

    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_crypto_finalize_pairing(void)
{
#if !HARDCODED_PAIRING_ENABLED
    // Validate S2, decrypt E3, generate C3, generate E4, and send response.

    int ret = fm_crypto_derive_server_shared_secret(p_fmna_finalize_pairing_data->seeds, m_seedk1,
                                                    m_server_shared_secret);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_finalize_pairing: fm_crypto_derive_server_shared_secret err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    populate_s2_verification_msg();
    FMNA_LOG_INFO("fmna_crypto_finalize_pairing: S2 verification msg len %d",
                  sizeof(m_key_verif_encr_msg.s2_verification_msg));
    ret = fm_crypto_verify_s2(m_q_a,
                              S2_BLEN,
                              p_fmna_finalize_pairing_data->s2,
                              sizeof(m_key_verif_encr_msg.s2_verification_msg),
                              (const uint8_t *)(&m_key_verif_encr_msg.s2_verification_msg));
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_finalize_pairing: fm_crypto_verify_s2 err %d", ret);

        //TODO: Do not bypass failure check for signature.
        //return FMNA_ERROR_INTERNAL;
    }

    uint32_t e3_decrypt_plaintext_blen = SOFTWARE_AUTH_TOKEN_BLEN;
    m_mfi_struct.p_software_auth_token = fmna_malloc(MFI_RAW_TOKEN, SOFTWARE_AUTH_TOKEN_BLEN);
    if (m_mfi_struct.p_software_auth_token == NULL)
    {
        APP_PRINT_ERROR0("fmna_crypto_finalize_pairing: malloc failed!");
        return FMNA_ERROR_INTERNAL;
    }

    ret = fm_crypto_decrypt_e3((const uint8_t *)m_server_shared_secret,
                               E3_BLEN,
                               (const uint8_t *)p_fmna_finalize_pairing_data->e3,
                               &e3_decrypt_plaintext_blen,
                               (uint8_t *)m_mfi_struct.p_software_auth_token);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_finalize_pairing: fm_crypto_decrypt_e3 err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }
    FMNA_LOG_INFO("fmna_crypto_finalize_pairing: E3 decrypted token len %d", e3_decrypt_plaintext_blen);
    fmna_log_mfi_token();

    ret = fm_crypto_ckg_gen_c3(&m_fm_crypto_ckg_ctx,
                               p_fmna_finalize_pairing_data->c2,
                               p_fmna_send_pairing_status->c3);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_finalize_pairing: fm_crypto_ckg_gen_c3 err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    // Generate E4, see Section 6.2.6 for details.
    populate_e4_generation_encryption_msg();
    FMNA_LOG_INFO("fmna_crypto_finalize_pairing: E4 generation encryption msg size %d",
                  sizeof(m_key_verif_encr_msg.e4_generation_encryption_msg));
    fmna_free(INITIATE_PAIRING_DATA);

    uint32_t e4_blen = E4_BLEN;
    ret = fm_crypto_encrypt_to_server((const uint8_t *)m_q_e,
                                      sizeof(m_key_verif_encr_msg.e4_generation_encryption_msg),
                                      (const uint8_t *)&m_key_verif_encr_msg.e4_generation_encryption_msg,
                                      &e4_blen,
                                      p_fmna_send_pairing_status->e4);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_finalize_pairing: fm_crypto_encrypt_to_server e4 err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }
    FMNA_LOG_INFO("fmna_crypto_finalize_pairing: e4_blen %lu", e4_blen);

    //new MFi token should be stored before pairing complete is sent
    //Attempt to save MFi Token
    fmna_connection_update_mfi_token_storage(&m_mfi_struct,
                                             SOFTWARE_AUTH_UUID_BLEN + SOFTWARE_AUTH_TOKEN_BLEN);

    fmna_free(MFI_RAW_TOKEN);
#endif // HARDCODED_PAIRING_ENABLED

    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_crypto_pairing_complete(void)
{
#if !HARDCODED_PAIRING_ENABLED
    int ret = fm_crypto_ckg_finish(&m_fm_crypto_ckg_ctx,
                                   m_p,
                                   m_current_primary_sk,
                                   m_current_secondary_sk);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_pairing_complete: fm_crypto_ckg_finish err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    FMNA_LOG_INFO("fmna_crypto_pairing_complete: P");
    FMNA_LOG_HEXDUMP_INFO(m_p, P_BLEN);
    ftl_save_to_storage(m_p, FTL_SAVE_M_P_ADDR, FTL_SAVE_M_P_SIZE);

    FMNA_LOG_INFO("fmna_crypto_pairing_complete: Primary SKN");
    FMNA_LOG_HEXDUMP_INFO(m_current_primary_sk, SK_BLEN);
    ftl_save_to_storage(m_current_primary_sk, FTL_SAVE_SKN_ADDR, FTL_SAVE_SKN_SIZE);

    FMNA_LOG_INFO("fmna_crypto_pairing_complete: Secondary SKN");
    FMNA_LOG_HEXDUMP_INFO(m_current_secondary_sk, SK_BLEN);
    ftl_save_to_storage(m_current_secondary_sk, FTL_SAVE_SKS_ADDR, FTL_SAVE_SKN_SIZE);

    fm_crypto_ckg_free(&m_fm_crypto_ckg_ctx);

#endif //HARDCODED_PAIRING_ENABLED

    return FMNA_SUCCESS;
}

fmna_ret_code_t fmna_crypto_roll_primary_sk(void)
{
    FMNA_LOG_INFO("fmna_crypto_roll_primary_sk");
#if !HARDCODED_PAIRING_ENABLED
    return roll_sk(m_current_primary_sk);
#else
    return FMNA_SUCCESS;
#endif
}

fmna_ret_code_t fmna_crypto_roll_secondary_sk(void)
{
    FMNA_LOG_INFO("fmna_crypto_roll_secondary_sk: m_p");
    FMNA_LOG_HEXDUMP_INFO(m_p, P_BLEN);
#if !HARDCODED_PAIRING_ENABLED
    return roll_sk(m_current_secondary_sk);
#else
    return FMNA_SUCCESS;
#endif
}

fmna_ret_code_t fmna_crypto_roll_primary_key(void)
{
#if !HARDCODED_PAIRING_ENABLED

    // SK(i) -> SK(i+1)
    fmna_ret_code_t fmna_ret_code = fmna_crypto_roll_primary_sk();
    if (fmna_ret_code != FMNA_SUCCESS)
    {
        return fmna_ret_code;
    }

    // SK(i+1) -> Primary_Key(i+1)
    int ret = fm_crypto_derive_primary_or_secondary_x(m_current_primary_sk, m_p,
                                                      m_fmna_current_primary_key.public_key);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_roll_primary_key: fm_crypto_derive_primary err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    // Increment Primary Key index
    m_fmna_current_primary_key.index++;

    FMNA_LOG_INFO("fmna_crypto_roll_primary_key: Curr Primary Key index 0x%x:",
                  m_fmna_current_primary_key.index);
    FMNA_LOG_HEXDUMP_INFO(m_fmna_current_primary_key.public_key, FMNA_PUBKEY_BLEN);

    // SK(i+1) -> LTK(i+1)
    ret = fm_crypto_derive_ltk(m_current_primary_sk, m_fmna_current_primary_key.ltk);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_roll_primary_key: fm_crypto_derive_primary err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    FMNA_LOG_INFO("fmna_crypto_roll_primary_key: Curr LTK");
    FMNA_LOG_HEXDUMP_INFO(m_fmna_current_primary_key.ltk, GAP_SEC_KEY_LEN);
    fmna_connection_set_active_ltk(m_fmna_current_primary_key.ltk);

    ftl_save_to_storage(&m_fmna_current_primary_key, FTL_SAVE_PRI_KEY_ADDR, FTL_SAVE_PRI_KEY_SIZE);
    ftl_save_to_storage(&m_current_primary_sk, FTL_SAVE_SKN_ADDR, FTL_SAVE_SKN_SIZE);

    return FMNA_SUCCESS;

#else // HARDCODED_PAIRING_ENABLED

    return FMNA_SUCCESS;
#endif // HARDCODED_PAIRING_ENABLED
}

fmna_ret_code_t fmna_crypto_roll_secondary_key(void)
{
#if !HARDCODED_PAIRING_ENABLED
    // SK(i) -> SK(i+1)
    fmna_ret_code_t fmna_ret_code = fmna_crypto_roll_secondary_sk();
    if (fmna_ret_code != FMNA_SUCCESS)
    {
        return fmna_ret_code;
    }

    // SK(i+1) -> Secondary_Key(i+1)
    int ret = fm_crypto_derive_primary_or_secondary_x(m_current_secondary_sk, m_p,
                                                      m_fmna_current_secondary_key.public_key);
    if (FM_CRYPTO_STATUS_SUCCESS != ret)
    {
        FMNA_LOG_ERROR("fmna_crypto_roll_secondary_key: fm_crypto_derive_primary err %d", ret);
        return FMNA_ERROR_INTERNAL;
    }

    // Increment Secondary Key index
    m_fmna_current_secondary_key.index++;

    FMNA_LOG_INFO("fmna_crypto_roll_secondary_key: Curr Secondary Key index 0x%x",
                  m_fmna_current_secondary_key.index);
    FMNA_LOG_HEXDUMP_INFO(m_fmna_current_secondary_key.public_key, FMNA_PUBKEY_BLEN);
    ftl_save_to_storage(&m_fmna_current_secondary_key, FTL_SAVE_SEC_KEY_ADDR, FTL_SAVE_SEC_KEY_SIZE);
    ftl_save_to_storage(&m_current_secondary_sk, FTL_SAVE_SKS_ADDR, FTL_SAVE_SKN_SIZE);

    return FMNA_SUCCESS;

#else //HARDCODED_PAIRING_ENABLED

    return FMNA_SUCCESS;
#endif // HARDCODED_PAIRING_ENABLED
}

void serial_number_read_state_init(void)
{
    fmna_ret_code_t ret_code = fmna_crypto_generate_serial_number_response(
                                   FMNA_SERIAL_NUMBER_QUERY_TYPE_BT);
    FMNA_ERROR_CHECK(ret_code);

    if (ret_code == FMNA_SUCCESS)
    {
        serial_number_read_state = true;
        app_start_timer(&timer_idx_sn_lookup, "put_sn_state", fmna_timer_id,
                        APP_TIMER_SN_LOOKUP, 0, false, 300 * 1000);
        APP_PRINT_INFO0("serial_number_read_state_init: success");
    }
}

fmna_ret_code_t fmna_crypto_generate_serial_number_response(FMNA_Serial_Number_Query_Type_t type)
{
#if !HARDCODED_PAIRING_ENABLED
    int ret;
    // Clear the encrypted serial number initially in case of error.
    p_fmna_encrypted_serial_number_payload = fmna_malloc(ENCRYPTED_SN,
                                                         ENCRYPTED_SERIAL_NUMBER_PAYLOAD_BLEN);
    if (p_fmna_encrypted_serial_number_payload == NULL)
    {
        APP_PRINT_ERROR0("fmna_crypto_generate_serial_number_response: malloc failed!");
        return FMNA_ERROR_INTERNAL;
    }

    memset(p_fmna_encrypted_serial_number_payload, 0, ENCRYPTED_SERIAL_NUMBER_PAYLOAD_BLEN);

    serial_number_query_count++;
    FMNA_LOG_INFO("fmna_crypto_generate_serial_number_response: serial_number_query_count %lu",
                  serial_number_query_count);

    memcpy(m_serial_number_hmac_payload.serial_number, m_serial_number, SERIAL_NUMBER_RAW_BLEN);
    memcpy(m_serial_number_payload.serial_number, m_serial_number, SERIAL_NUMBER_RAW_BLEN);

    m_serial_number_hmac_payload.counter = serial_number_query_count;
    m_serial_number_payload.counter      = serial_number_query_count;

    switch (type)
    {
    case FMNA_SERIAL_NUMBER_QUERY_TYPE_TAP:
        strcpy(m_serial_number_hmac_payload.op, "tap");
        strcpy(m_serial_number_payload.op, "tap");
        break;
    case FMNA_SERIAL_NUMBER_QUERY_TYPE_BT:
        strcpy(m_serial_number_hmac_payload.op, "bt");
        strcpy(m_serial_number_payload.op, "bt");
        break;
    default:
        FMNA_LOG_ERROR("fmna_crypto_generate_serial_number_response: Invalid Serial Number Query Type");
        return FMNA_ERROR_INTERNAL;
    }

    ret = fm_crypto_authenticate_with_ksn(m_server_shared_secret,
                                          sizeof(m_serial_number_hmac_payload),
                                          (const uint8_t *)&m_serial_number_hmac_payload,
                                          m_serial_number_payload.hmac);
    if (ret != FM_CRYPTO_STATUS_SUCCESS)
    {
        FMNA_LOG_ERROR("fmna_crypto_generate_serial_number_response: fm_crypto_authenticate_with_ksn err %d",
                       ret);
        return FMNA_ERROR_INTERNAL;
    }

    uint32_t encrypted_serial_number_payload_blen = ENCRYPTED_SERIAL_NUMBER_PAYLOAD_BLEN;
    ret = fm_crypto_encrypt_to_server(m_q_e,
                                      sizeof(m_serial_number_payload),
                                      (const uint8_t *)&m_serial_number_payload,
                                      &encrypted_serial_number_payload_blen,
                                      p_fmna_encrypted_serial_number_payload);
    if (ret != FM_CRYPTO_STATUS_SUCCESS)
    {
        FMNA_LOG_ERROR("fmna_crypto_generate_serial_number_response: encrypted_serial_number_payload_blen err %d",
                       ret);

        // Clear the encrypted serial number in case of fm_crypto_encrypt_to_server error.
        fmna_free(ENCRYPTED_SN);
        return FMNA_ERROR_INTERNAL;
    }
#endif //HARDCODED_PAIRING_ENABLED
    return FMNA_SUCCESS;
}

void fmna_crypto_unpair(void)
{
#if !HARDCODED_PAIRING_ENABLED
    FMNA_LOG_INFO("fmna_crypto_unpair: paired %d", fmna_connection_is_fmna_paired());

    if (fmna_connection_is_fmna_paired())
    {
        // Initialize new CKG context on unpair in preparation for new pairing.
        int ret = fm_crypto_ckg_init(&m_fm_crypto_ckg_ctx);

        if (FM_CRYPTO_STATUS_SUCCESS != ret)
        {
            FMNA_LOG_ERROR("fmna_crypto_unpair: fm_crypto_ckg_init err %d", ret);
            FMNA_ERROR_CHECK(FMNA_ERROR_INTERNAL);
        }
    }
#endif // HARDCODED_PAIRING_ENABLED
}

void fmna_log_mfi_token_help(void)
{
#if !HARDCODED_PAIRING_ENABLED
    fmna_connection_platform_log_token_help(m_mfi_struct.p_software_auth_token,
                                            SOFTWARE_AUTH_TOKEN_BLEN, m_mfi_struct.m_software_auth_uuid, SOFTWARE_AUTH_UUID_BLEN);
#endif // HARDCODED_PAIRING_ENABLED
}

void fmna_log_mfi_token(void)
{
#if !HARDCODED_PAIRING_ENABLED
    fmna_connection_platform_log_token(m_mfi_struct.p_software_auth_token, SOFTWARE_AUTH_TOKEN_BLEN);
#endif // HARDCODED_PAIRING_ENABLED
}

void fmna_log_serial_number(void)
{
    FMNA_LOG_INFO("fmna_log_serial_number: Serial Number %s", TRACE_STRING(m_serial_number));
}

#endif
