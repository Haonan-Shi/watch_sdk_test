/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _QDEC_WHEEL_H_
#define _QDEC_WHEEL_H_

#include "rtl876x_qdec.h"
typedef struct
{
    uint16_t    count;
    int64_t    timestamp;
} T_QDEC_DATA;

void qdec_wheel_init(void);
T_QDEC_DATA qdec_wheel_read_data(void);

#endif /* _QDEC_WHEEL_H_ */
