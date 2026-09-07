/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <assert.h>
#include <string.h>
#include <trace.h>
#define MBEDTLS_ALLOW_PRIVATE_ACCESS //must defined before #include "fm-crypto.h"
#include "fm-crypto.h"
#include "mbedtls/ctr_drbg.h"

const byte KDF_LABEL_UPDATE[] = "update";
const byte KDF_LABEL_DIVERSIFY[] = "diversify";
const byte KDF_LABEL_INTERMEDIATE[] = "intermediate";
const byte KDF_LABEL_CONNECT[] = "connect";
const byte KDF_LABEL_SERVERSS[] = "ServerSharedSecret";
const byte KDF_LABEL_PAIRINGSESS[] = "PairingSession";
const byte KDF_LABEL_SNPROTECTION[] = "SerialNumberProtection";



#define CHECK_RV_RET(_rv_, _val_) if (_rv_) return _val_;
#define CHECK_RV(_rv_) CHECK_RV_RET(_rv_, _rv_);
#define CHECK_RV_GOTO(_rv_, _label_) if (_rv_) goto _label_;


static struct
{
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_entropy_context entropy;
} crypto =
{
    .ctr_drbg = {},
    .entropy = {},
};



/*
 * OpenSSL: SHA256()
 */
int fm_crypto_sha256(word32 msg_nbytes, const byte *msg, byte out[32])
{
    int rv = mbedtls_sha256(msg, msg_nbytes, out, 0);
    return rv;
}


int fm_crypto_ckg_init(fm_crypto_ckg_context_t ctx)
{
    int rv = 0;
    int ret = 0;

    XMEMSET(ctx, 0, sizeof(struct fm_crypto_ckg_context));
    ctx->p = (mbedtls_ecp_point *)os_mem_zalloc(OS_MEM_TYPE_DATA, sizeof(mbedtls_ecp_point));
    if (ctx->p == NULL)
    {
        rv = -1;
        goto cleanup;
    }
    mbedtls_ecp_point_init(ctx->p);

    /*
     * OpenSSL: RAND_bytes()
     */
    rv = mbedtls_ctr_drbg_random(&crypto.ctr_drbg, ctx->r1, sizeof(ctx->r1));
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: EC_KEY_new_by_curve_name(NID_secp224r1)
     */

    mbedtls_ecp_keypair_init(&ctx->key);
    mbedtls_ecp_group_load(&(ctx->key.MBEDTLS_PRIVATE(grp)), MBEDTLS_ECP_DP_SECP224R1);
    rv = mbedtls_ecp_gen_keypair(&(ctx->key.MBEDTLS_PRIVATE(grp)), &(ctx->key.MBEDTLS_PRIVATE(d)),
                                 &(ctx->key.MBEDTLS_PRIVATE(Q)), mbedtls_ctr_drbg_random,
                                 &crypto.ctr_drbg);
    CHECK_RV_GOTO(rv, cleanup);

cleanup:
    if (rv)
    {
        XMEMSET(ctx, 0, sizeof(struct fm_crypto_ckg_context));
    }

    return rv;
}

void fm_crypto_ckg_free(fm_crypto_ckg_context_t ctx)
{
    mbedtls_ecp_keypair_free(&ctx->key);
    if (ctx->p)
    {
        mbedtls_ecp_point_free(ctx->p);
        os_mem_free(ctx->p);
    }

    XMEMSET(ctx, 0, sizeof(struct fm_crypto_ckg_context));
}

int fm_crypto_ckg_gen_c1(fm_crypto_ckg_context_t ctx, byte out[32])
{
    // Construct s || r.
    byte msg[28 + sizeof(ctx->r1)];
    XMEMCPY(msg + 28, ctx->r1, sizeof(ctx->r1));

    /*
     * OpenSSL: BN_bn2bin() + EC_KEY_get0_private_key()
     */
    int rv = mbedtls_mpi_write_binary(&ctx->key.MBEDTLS_PRIVATE(d), msg, 28);
    CHECK_RV(rv);

    /*
     * OpenSSL: SHA256()
     */
    return fm_crypto_sha256(sizeof(msg), msg, out);
}

/*! @function _fm_crypto_points_add
 @abstract Adds two given points on an elliptic curve.

 @param r  Resulting EC point r = s + t.
 @param s  EC point s.
 @param t  EC point t.
 @param dp Curve parameters.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_points_add(mbedtls_ecp_point *r,
                                 mbedtls_ecp_point *s,
                                 mbedtls_ecp_point *t,
                                 const mbedtls_ecp_group *dp)
{
    mbedtls_mpi m;
    int rv = 0;
    mbedtls_mpi_init(&m);

    rv = mbedtls_mpi_lset(&m, 1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecp_muladd((mbedtls_ecp_group *)dp, r, &m, s, &m, t);
    CHECK_RV_GOTO(rv, cleanup);

cleanup:
    mbedtls_mpi_free(&m);
    return rv;
}

int fm_crypto_ckg_gen_c3(fm_crypto_ckg_context_t ctx,
                         const byte c2[89],
                         byte out[60])
{
    /*
     * OpenSSL: EC_KEY_new_by_curve_name(NID_secp224r1)
     */
    mbedtls_ecp_point S;
    mbedtls_ecp_point_init(&S);

    /*
     * Import and check S'.
     *
     * OpenSSL: EC_POINT_set_affine_coordinates_GFp()
     */
    mbedtls_ecp_group grp;
    mbedtls_ecp_group_init(&grp);
    int rv = mbedtls_ecp_group_load(&grp, MBEDTLS_ECP_DP_SECP224R1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecp_point_read_binary(&grp, &S, c2, 1 + 28 * 2);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: EC_POINT_is_on_curve()
     */
    rv = mbedtls_ecp_check_pubkey(&grp, &S);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * Compute S + S'.
     *
     * OpenSSL: EC_POINT_add()
     */
    rv = _fm_crypto_points_add(ctx->p, &ctx->key.MBEDTLS_PRIVATE(Q), &S, &grp);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * C3 := s || r
     *
     * OpenSSL: BN_bn2bin() + EC_KEY_get0_private_key()
     */
    rv = mbedtls_mpi_write_binary(&ctx->key.MBEDTLS_PRIVATE(d), out, 28);
    CHECK_RV_GOTO(rv, cleanup);

    XMEMCPY(out + 28, ctx->r1, sizeof(ctx->r1));

    // Store r'.
    XMEMCPY(ctx->r2, c2 + 1 + 28 * 2, sizeof(ctx->r2));

cleanup:
    mbedtls_ecp_point_free(&S);
    mbedtls_ecp_group_free(&grp);
    return rv;
}

int fm_crypto_ckg_finish(fm_crypto_ckg_context_t ctx,
                         byte p[57],
                         byte skn[32],
                         byte sks[32])
{
    byte x[28] = {0};
    int rv = 0;

    /*
     * OpenSSL: BN_bn2bin() + EC_POINT_get_affine_coordinates_GFp()
     */
    rv = mbedtls_mpi_write_binary(&ctx->p->MBEDTLS_PRIVATE(X), x, 28);
    CHECK_RV(rv);

    byte r1r2[32 * 2];
    XMEMCPY(r1r2, ctx->r1, 32);
    XMEMCPY(r1r2 + 32, ctx->r2, 32);

    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte sk[32 * 2];
    rv = mbed_KDF963(MBED_KDF963_SHA256,
                     x, sizeof(x),       /* Z */
                     r1r2, sizeof(r1r2), /* sharedinfo */
                     sk, sizeof(sk));    /* derived key */
    CHECK_RV(rv);

    XMEMCPY(skn, sk, 32);
    XMEMCPY(sks, sk + 32, 32);

    /*
     * Write uncompressed point in X9.63 format.
     *
     * OpenSSL: BN_bn2bin() + EC_POINT_get_affine_coordinates_GFp()
     */
    p[0] = 0x04;
    rv = mbedtls_mpi_write_binary(&ctx->p->MBEDTLS_PRIVATE(X), p + 1, 28);
    CHECK_RV(rv);

    return mbedtls_mpi_write_binary(&ctx->p->MBEDTLS_PRIVATE(Y), p + 1 + 28, 28);
}

int fm_crypto_roll_sk(const byte sk[32], byte out[32])
{
    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    return mbed_KDF963(MBED_KDF963_SHA256,
                       (uint8_t *)sk, 32,                                        /* Z */
                       (uint8_t *)KDF_LABEL_UPDATE, sizeof(KDF_LABEL_UPDATE) - 1, /* sharedinfo */
                       out, 32);                                       /* derived key */
}

int fm_crypto_derive_ltk(const byte skn[32], byte out[16])
{
    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte ik[32];
    int rv = mbed_KDF963(MBED_KDF963_SHA256,
                         (uint8_t *)skn, 32,                                                   /* Z */
                         (uint8_t *)KDF_LABEL_INTERMEDIATE, sizeof(KDF_LABEL_INTERMEDIATE) - 1, /* sharedinfo */
                         ik, sizeof(ik));                                            /* derived key */
    CHECK_RV(rv);

    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    return mbed_KDF963(MBED_KDF963_SHA256,
                       ik, sizeof(ik),                                   /* Z */
                       (uint8_t *)KDF_LABEL_CONNECT, sizeof(KDF_LABEL_CONNECT) - 1, /* sharedinfo */
                       out, 16);                                         /* derived key */
}

/*! @function _fm_crypto_scmult
 @abstract Scalar multiplication on an elliptic curve.

 @param r  Resulting EC point r = s * B.
 @param s  Scalar s.
 @param B  Base point B.
 @param dp Curve parameters.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_scmult(mbedtls_ecp_point *r,
                             mbedtls_mpi *s,
                             mbedtls_ecp_point *B,
                             const mbedtls_ecp_group *dp)
{
    int rv = mbedtls_ecp_mul((mbedtls_ecp_group *)dp, r, s, B, mbedtls_ctr_drbg_random,
                             &crypto.ctr_drbg);

    return rv;
}

/*! @function _fm_crypto_scmult_reduce
 @abstract Takes a 36-byte value uv, reduces it to a valid scalar s,
           and computes r = s * B.

 @param r  Resulting EC point r = (uv (mod q-1) + 1) * B.
 @param uv 36-byte pre-scalar value.
 @param B  Base point B.
 @param dp Curve parameters.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_scmult_reduce(mbedtls_ecp_point *r,
                                    const byte uv[36],
                                    mbedtls_ecp_point *B,
                                    const mbedtls_ecp_group *dp)
{
    mbedtls_mpi s;
    mbedtls_mpi_init(&s);

    mbedtls_mpi qm1;
    mbedtls_mpi_init(&qm1);

    int rv = mbedtls_mpi_read_binary(&s, uv, 28 + 8);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_copy(&qm1, &dp->N);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_sub_int(&qm1, &qm1, 1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_mod_mpi(&s, &s, &qm1);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * s := u (mod q-1) + 1
     *
     * OpenSSL: BN_add_word()
     */
    rv = mbedtls_mpi_add_int(&s, &s, 1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = _fm_crypto_scmult(r, &s, B, dp);

cleanup:
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&qm1);

    return rv;
}

/*! @function _fm_crypto_scmult_twin_reduce
 @abstract Takes two 36-byte values u and v, reduces them to valid scalars s
           and t, a and computes r = s * P + t * G.

 @param r  Resulting EC point r = (u (mod q-1) + 1) * P + (v (mod q-1) + 1) * G.
 @param u  36-byte pre-scalar value.
 @param v  36-byte pre-scalar value.
 @param P  EC point P.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_scmult_twin_reduce(mbedtls_ecp_point *r,
                                         const byte u[36],
                                         const byte v[36],
                                         mbedtls_ecp_keypair *P)
{
    int rv;
    /*
     * OpenSSL: EC_GROUP_new_by_curve_name(NID_secp224r1) + EC_POINT_new()
     */
    mbedtls_ecp_point r1;
    mbedtls_ecp_point_init(&r1);

    /*
     * OpenSSL: EC_GROUP_new_by_curve_name(NID_secp224r1) + EC_POINT_new()
     */
    mbedtls_ecp_point r2;
    mbedtls_ecp_point_init(&r2);

    /*
     * OpenSSL: EC_GROUP_new_by_curve_name(NID_secp224r1) + EC_POINT_new()
     */
    mbedtls_ecp_point g;
    mbedtls_ecp_point_init(&g);

    rv = mbedtls_mpi_copy(&g.MBEDTLS_PRIVATE(X), &P->MBEDTLS_PRIVATE(grp).G.MBEDTLS_PRIVATE(X));
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_copy(&g.MBEDTLS_PRIVATE(Y), &P->MBEDTLS_PRIVATE(grp).G.MBEDTLS_PRIVATE(Y));
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_lset(&g.MBEDTLS_PRIVATE(Z), 1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = _fm_crypto_scmult_reduce(&r1, u, &P->MBEDTLS_PRIVATE(Q), &P->MBEDTLS_PRIVATE(grp));
    CHECK_RV_GOTO(rv, cleanup);
    /*
     * OpenSSL: EC_POINT_mul()
     */
    rv = _fm_crypto_scmult_reduce(&r2, v, &g, &P->MBEDTLS_PRIVATE(grp));
    CHECK_RV_GOTO(rv, cleanup);

    rv = _fm_crypto_points_add(r, &r1, &r2, &P->MBEDTLS_PRIVATE(grp));

cleanup:
    mbedtls_ecp_point_free(&r1);
    mbedtls_ecp_point_free(&r2);
    mbedtls_ecp_point_free(&g);

    return rv;
}

int fm_crypto_derive_primary_or_secondary_x(const byte sk[32],
                                            const byte p[57],
                                            byte out[28])
{
    int rv = 0;
    /*
     * OpenSSL: EC_KEY_new_by_curve_name(NID_secp224r1)
     */
    mbedtls_ecp_keypair P;
    mbedtls_ecp_keypair_init(&P);

    /*
     * OpenSSL: EC_GROUP_new_by_curve_name(NID_secp224r1) + EC_POINT_new()
     */
    mbedtls_ecp_point r;
    mbedtls_ecp_point_init(&r);

    /*
     * OpenSSL: EC_POINT_set_affine_coordinates_GFp()
     */
    rv = mbedtls_ecp_group_load(&P.MBEDTLS_PRIVATE(grp), MBEDTLS_ECP_DP_SECP224R1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecp_point_read_binary(&P.MBEDTLS_PRIVATE(grp), &P.MBEDTLS_PRIVATE(Q), p, 1 + 28 * 2);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: EC_POINT_is_on_curve()
     */
    rv = mbedtls_ecp_check_pubkey(&P.MBEDTLS_PRIVATE(grp), &P.MBEDTLS_PRIVATE(Q));

    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte at[2 * (28 + 8)];
    rv = mbed_KDF963(MBED_KDF963_SHA256, (uint8_t *)sk, 32,
                     (uint8_t *)KDF_LABEL_DIVERSIFY, sizeof(KDF_LABEL_DIVERSIFY) - 1, /* sharedinfo */
                     at, sizeof(at));
    CHECK_RV_GOTO(rv, cleanup);

    rv = _fm_crypto_scmult_twin_reduce(&r, at, &at[28 + 8], &P);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: BN_bn2bin() + EC_POINT_get_affine_coordinates_GFp()
     */
    rv = mbedtls_mpi_write_binary(&r.MBEDTLS_PRIVATE(X), out, 28);

cleanup:
    mbedtls_ecp_keypair_free(&P);
    mbedtls_ecp_point_free(&r);
    return rv;
}

int fm_crypto_derive_server_shared_secret(const byte seeds[32],
                                          const byte seedk1[32],
                                          byte out[32])
{
    byte ikm[2 * 32];
    XMEMCPY(ikm, seeds, 32);
    XMEMCPY(ikm + 32, seedk1, 32);

    /*
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    return mbed_KDF963(MBED_KDF963_SHA256,
                       ikm, sizeof(ikm),                                   /* Z */
                       (uint8_t *)KDF_LABEL_SERVERSS, sizeof(KDF_LABEL_SERVERSS) - 1, /* sharedinfo */
                       out, 32);                                           /* derived key */
}

/*! @function _fm_crypto_aes128gcm_encrypt
 @abstract Encrypts a message using AES-128-GCM.

 @param key        128-bit AES key.
 @param iv         128-bit IV.
 @param msg_nbytes Byte length of message.
 @param msg        Message.
 @param out        Output buffer for the ciphertext.
 @param tag        Output buffer for the 128-bit authentication tag.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_aes128gcm_encrypt(const byte key[16],
                                        const byte iv[16],
                                        word32 msg_nbytes,
                                        const byte *msg,
                                        byte *out,
                                        byte *tag)
{
    mbedtls_gcm_context gcm_ctx;
    mbedtls_gcm_init(&gcm_ctx);
    int rv = mbedtls_gcm_setkey(&gcm_ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: EVP_Encrypt*() + EVP_aes_128_gcm()
     */
    rv = mbedtls_gcm_crypt_and_tag(&gcm_ctx, MBEDTLS_GCM_ENCRYPT, msg_nbytes, iv, 16, NULL, 0, msg, out,
                                   16, tag);

cleanup:
    mbedtls_gcm_free(&gcm_ctx);
    return rv;
}


/*! @function _fm_crypto_aes128gcm_decrypt
 @abstract Decrypts a ciphertext using AES-128-GCM.

 @param key       128-bit AES key.
 @param iv        128-bit IV.
 @param ct_nbytes Byte length of ciphertext.
 @param ct        Ciphertext.
 @param tag       128-bit authentication tag.
 @param out       Output buffer for the plaintext.

 @return 0 on success, a negative value on error.
 */
static int _fm_crypto_aes128gcm_decrypt(const byte key[16],
                                        const byte iv[16],
                                        word32 ct_nbytes,
                                        const byte *ct,
                                        const byte *tag,
                                        byte *out)
{
    mbedtls_gcm_context gcm_ctx;
    mbedtls_gcm_init(&gcm_ctx);
    int rv = mbedtls_gcm_setkey(&gcm_ctx, MBEDTLS_CIPHER_ID_AES, key, 128);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_gcm_auth_decrypt(&gcm_ctx, ct_nbytes, iv, 16, NULL, 0, tag, 16, ct, out);

cleanup:
    mbedtls_gcm_free(&gcm_ctx);
    return rv;
}

int fm_crypto_decrypt_e3(const byte serverss[32],
                         word32 e3_nbytes,
                         const byte *e3,
                         word32 *out_nbytes,
                         byte *out)
{
    // E3 has the 16 byte tag appended.
    if (e3_nbytes <= 16)
    {
        return -1;
    }

    if (*out_nbytes < e3_nbytes - 16)
    {
        return -1;
    }

    /*
     * Derive K1 and IV1.
     *
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte key_iv[32];
    int rv = mbed_KDF963(MBED_KDF963_SHA256,
                         (uint8_t *)serverss, 32,                                            /* Z */
                         (uint8_t *)KDF_LABEL_PAIRINGSESS, sizeof(KDF_LABEL_PAIRINGSESS) - 1, /* sharedinfo */
                         key_iv, sizeof(key_iv));                                  /* derived key */
    CHECK_RV(rv);

    /*
     * OpenSSL: EVP_Decrypt*() + EVP_aes_128_gcm()
     */
    rv = _fm_crypto_aes128gcm_decrypt(key_iv, key_iv + 16, e3_nbytes - 16, e3,
                                      e3 + e3_nbytes - 16, out);
    CHECK_RV(rv);

    *out_nbytes = e3_nbytes - 16;
    return 0;
}

int fm_crypto_verify_s2(const byte pub[65],
                        word32 sig_nbytes,
                        const byte *sig,
                        word32 msg_nbytes,
                        const byte *msg)
{

    mbedtls_ecdsa_context ctx;
    mbedtls_ecdsa_init(&ctx);

    int rv = mbedtls_ecp_group_load(&ctx.MBEDTLS_PRIVATE(grp), MBEDTLS_ECP_DP_SECP256R1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecp_point_read_binary(&ctx.MBEDTLS_PRIVATE(grp), &ctx.MBEDTLS_PRIVATE(Q), pub,
                                       1 + 32 * 2);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecp_check_pubkey(&ctx.MBEDTLS_PRIVATE(grp), &ctx.MBEDTLS_PRIVATE(Q));
    CHECK_RV_GOTO(rv, cleanup);

    byte hash[32];
    rv = fm_crypto_sha256(msg_nbytes, msg, hash);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_ecdsa_read_signature(&ctx, hash, sizeof(hash), sig, sig_nbytes);
    CHECK_RV_GOTO(rv, cleanup);

cleanup:
    mbedtls_ecdsa_free(&ctx);

    return rv;
}

int fm_crypto_authenticate_with_ksn(const byte serverss[32],
                                    word32 msg_nbytes,
                                    const byte *msg,
                                    byte out[32])
{
    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    /*
     * Derive KSN.
     *
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte ksn[32];
    int rv = mbed_KDF963(MBED_KDF963_SHA256,
                         (uint8_t *)serverss, 32,                                              /* Z */
                         (uint8_t *)KDF_LABEL_SNPROTECTION, sizeof(KDF_LABEL_SNPROTECTION) - 1, /* sharedinfo */
                         ksn, sizeof(ksn));                                          /* derived key */
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_md_setup(&md_ctx, info, 1);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_md_hmac_starts(&md_ctx, ksn, sizeof(ksn));
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_md_hmac_update(&md_ctx, msg, msg_nbytes);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_md_hmac_finish(&md_ctx, out);
    CHECK_RV_GOTO(rv, cleanup);

cleanup:
    mbedtls_md_free(&md_ctx);
    return rv;
}

int fm_crypto_generate_seedk1(byte out[32])
{
    int rv = mbedtls_ctr_drbg_random(&crypto.ctr_drbg, out, 32);
    return rv;
}

int fm_crypto_encrypt_to_server(const byte pub[65],
                                word32 msg_nbytes,
                                const byte *msg,
                                word32 *out_nbytes,
                                byte *out)
{
    int rv = 0;
    mbedtls_ecp_keypair key;
    mbedtls_ecp_keypair_init(&key);

    mbedtls_ecp_keypair Q;
    mbedtls_ecp_keypair_init(&Q);

    mbedtls_ecp_group_load(&(key.MBEDTLS_PRIVATE(grp)), MBEDTLS_ECP_DP_SECP256R1);
    mbedtls_ecp_group_load(&(Q.MBEDTLS_PRIVATE(grp)), MBEDTLS_ECP_DP_SECP256R1);

    // Need space for Q || C || T.
    if (*out_nbytes < 65 + msg_nbytes + 16)
    {
        return -1;
    }

    word32 x_len = 0;
    /*
     * Import and check Q_E.
     *
     * OpenSSL: EC_POINT_set_affine_coordinates_GFp()
     */
    rv = mbedtls_ecp_point_read_binary(&Q.MBEDTLS_PRIVATE(grp), &Q.MBEDTLS_PRIVATE(Q), pub, 1 + 32 * 2);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * OpenSSL: EC_POINT_is_on_curve()
     */
    rv = mbedtls_ecp_check_pubkey(&Q.MBEDTLS_PRIVATE(grp), &Q.MBEDTLS_PRIVATE(Q));
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * Generate ephemeral key.
     *
     * OpenSSL: EC_KEY_generate_key()
     */
    rv = mbedtls_ecp_gen_keypair(&key.MBEDTLS_PRIVATE(grp), &key.MBEDTLS_PRIVATE(d),
                                 &key.MBEDTLS_PRIVATE(Q), mbedtls_ctr_drbg_random, &crypto.ctr_drbg);
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * Generate shared secret.
     *
     * OpenSSL: ECDH_compute_key()
     */
    byte x[32];
    x_len = (word32)sizeof(x);
    mbedtls_mpi z;
    mbedtls_mpi_init(&z);

    rv = mbedtls_ecdh_compute_shared(&key.MBEDTLS_PRIVATE(grp), &z, &Q.MBEDTLS_PRIVATE(Q),
                                     &key.MBEDTLS_PRIVATE(d), mbedtls_ctr_drbg_random, &crypto.ctr_drbg);
    mbedtls_mpi_write_binary(&z, x, x_len);
    CHECK_RV_GOTO(rv, cleanup);
    assert(x_len == 32);

    // Compute sharedinfo.
    byte info[2 * 65];
    XMEMCPY(info + 65, pub, 65);

    /*
     * OpenSSL: BN_bn2bin() + EC_POINT_get_affine_coordinates_GFp()
     */
    info[0] = 0x04;
    rv = mbedtls_mpi_write_binary(&key.MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(X), info + 1, 32);
    CHECK_RV_GOTO(rv, cleanup);

    rv = mbedtls_mpi_write_binary(&key.MBEDTLS_PRIVATE(Q).MBEDTLS_PRIVATE(Y), info + 1 + 32, 32);
    CHECK_RV_GOTO(rv, cleanup);

    // Copy Q into out.
    XMEMCPY(out, info, 65);

    /*
     * Derive key and IV.
     *
     * OpenSSL: Custom X9.63 KDF implementation using SHA256()
     */
    byte key_iv[32];
    rv = mbed_KDF963(MBED_KDF963_SHA256,
                     x, sizeof(x),            /* Z */
                     info, sizeof(info),      /* sharedinfo */
                     key_iv, sizeof(key_iv)); /* derived key */
    CHECK_RV_GOTO(rv, cleanup);

    /*
     * Encrypt.
     *
     * OpenSSL: EVP_Encrypt*() + EVP_aes_128_gcm()
     */
    rv = _fm_crypto_aes128gcm_encrypt(key_iv, key_iv + 16, msg_nbytes, msg,
                                      out + 65, out + 65 + msg_nbytes);
    CHECK_RV_GOTO(rv, cleanup);

    *out_nbytes = 65 + msg_nbytes + 16;

cleanup:
    mbedtls_ecp_keypair_free(&Q);
    mbedtls_ecp_keypair_free(&key);
    mbedtls_mpi_free(&z);
    return rv;
}

void fm_crypto_init(void)
{
    mbedtls_entropy_init(&crypto.entropy);
    mbedtls_ctr_drbg_init(&crypto.ctr_drbg);

    mbedtls_ctr_drbg_seed(&crypto.ctr_drbg, mbedtls_entropy_func, &crypto.entropy,
                          (const unsigned char *)"Findmy", strlen("Findmy"));
}


