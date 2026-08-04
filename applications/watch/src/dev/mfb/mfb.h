/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _QDEC_WHEEL_H_
#define _QDEC_WHEEL_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    MFB_KEY_INVALID,
    MFB_KEY_PRESSED,
    MFB_KEY_RELEASED,
} T_MFB_KEY_STATE;

typedef struct
{
    uint32_t key_id;
    T_MFB_KEY_STATE current_state;
    uint32_t press_timestamp;
    uint32_t release_timestamp;
} T_MFB_KEY;

typedef void (*T_MFB_KEY_CALLBACK)(T_MFB_KEY *state);

void app_mfb_init(void);
T_MFB_KEY app_mfb_get_level(void);
void app_mfb_register_callback(T_MFB_KEY_CALLBACK user_callback);
#endif /* _QDEC_WHEEL_H_ */
