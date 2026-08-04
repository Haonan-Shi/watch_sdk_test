/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __GSENSOR_STK8321_H__
#define __GSENSOR_STK8321_H__
#include "stdint.h"
#include "hub_gsensor.h"



void gsensor_read(uint8_t reg, uint8_t *p_data, uint16_t len);
void gsensor_write(uint8_t reg, uint8_t data);
uint8_t gsensor_get_fifo_length(void);
bool gsensor_get_fifo_data(uint8_t len, AxesRaw_t *buf);
void gsensor_init(void);
void gsensor_enable(void);
void gsensor_disable(void);

#endif
