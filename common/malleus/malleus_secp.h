/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#ifndef _MALLEUS_SECP_H_
#define _MALLEUS_SECP_H_

#include <stdint.h>
#include <string.h>

#define MALLEUS_MAX_FLASH_SIZE           (32)
#define MALLEUS_MAX_BIN_SIZE             (20480)    // Assume SECP bin maximun is 20kB
#define MALLEUS_SAMPLE_RATE_LEN          (9)
#define MALLEUS_SECP_MAX_SE_SIZE         (12)

enum
{
    MALLEUS_SECP_READ_SUC                 = 0x00,
    MALLEUS_SECP_READ_ERR                 = -1,
} ;

enum
{
    MALLEUS_SECP_SUC                      = 0x00,
    MALLEUS_SECP_HEAD_ID_ERR              = 0x01,
    MALLEUS_SECP_HEAD_LEN_ERR             = 0x02,
    MALLEUS_SECP_HEAD_VER_ERR             = 0x03,
    MALLEUS_SECP_HEAD_SERIAL_ERR          = 0x04,
    MALLEUS_SECP_EFFECT_NUM_ERR           = 0x05,
    MALLEUS_SECP_EFFECT_IDX_ERR           = 0x06,
    MALLEUS_SECP_SAMPLE_RATE_ERR          = 0x07,
};

/**
 * @brief  read data starting from flash offset.
 *
 * @param  offset number of bytes to skip from the beginning of section.
 * @param  data buffer for storing the data.
 * @param  len number of bytes to be read.
 * @return nNumber of bytes actually read into buffer.
*/
int32_t malleus_secp_start_read(int32_t offset, uint8_t *data, int32_t len);

/**
 * @brief  reading data starting from the next file offset of the previous read.
 *
 * @param  data buffer for storing the file data.
 * @param  len number of bytes to be read.
 * @return nNumber of bytes actually read into buffer.
*/
int32_t malleus_secp_continue_read(uint8_t *data, int32_t len);

/**
 * @brief  get start address of SECP table
 *
 * @param  sample_rate input data sample rate.
 * @param  num_effect pointer of number effect.
 * @param  effect_type Current sound effect type.
 * @param  start_addr Start address of SECP table.
 * @return status report.
*/
uint8_t malleus_secp_get_start_addr(int32_t sample_rate, uint16_t *num_effect,
                                    uint16_t *effect_type, uint32_t *start_addr);

/**
 * @brief  get number of sound effect.
 *
 * @param  num_effect pointer of number effect.
 * @return status report.
*/
uint8_t malleus_secp_get_effect_num(uint16_t *num_effect);

/**
 * @brief  initialize SECP (Sound Effect Customize Platform) binary file in Flash section.
 *
 * @param  void
 * @return void
*/
void malleus_secp_init(void);
#endif
