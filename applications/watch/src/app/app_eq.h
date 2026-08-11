/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_EQ_H
#define APP_EQ_H

#include <stdint.h>
#include <stdbool.h>
#include "eq.h"
#include "eq_utils.h"

/**
 * \brief  Init APP EQ module.
 */
void app_eq_init(void);

/**
 * \brief   Create EQ instance.
 *
 * \param[in]  eq_content_type  EQ content type
 * \param[in]  eq_type          EQ type
 * \param[in]  eq_mode          EQ mode
 * \param[in]  eq_index         EQ index
 *
 * \return  Created EQ instance.
 */
T_AUDIO_EFFECT_INSTANCE app_eq_create(T_EQ_CONTENT_TYPE eq_content_type,
                                      T_EQ_TYPE eq_type, uint8_t eq_mode, uint8_t eq_index);

/**
 * \brief   Set the specific EQ index.
 *
 * \param[in] eq_type   EQ type
 * \param[in] mode      EQ mode
 * \param[in] index     EQ index
 *
 * \return  The status of setting the EQ index.
 * \retval  true        EQ index was set successfully.
 * \retval  false       EQ index was failed to set.
 */
bool app_eq_index_set(T_EQ_TYPE eq_type, uint8_t mode, uint8_t index);

/**
 * \brief   Set the specific EQ index parameter.
 *
 * \param[in] index     EQ index.
 * \param[in] data      EQ parameter buffer.
 * \param[in] len       EQ parameter length.
 *
 * \return  The status of setting the EQ parameter.
 * \retval  true        EQ parameter was set successfully.
 * \retval  false       EQ parameter was failed to set.
 */
bool app_eq_param_set(uint8_t eq_mode, uint8_t index, void *data, uint16_t len);

/**
 * \brief  Check idx accord EQ mode
 */
void app_eq_idx_check_accord_mode(void);

/**
 * \brief   Enable the EQ effect.
 *
 * \param[in]      eq_instance         EQ instance
 * \param[in,out]  audio_eq_enabled    this EQ instance enabled or not
 */
void app_eq_audio_eq_enable(T_AUDIO_EFFECT_INSTANCE *eq_instance, bool *audio_eq_enabled);

#endif // APP_EQ_H
