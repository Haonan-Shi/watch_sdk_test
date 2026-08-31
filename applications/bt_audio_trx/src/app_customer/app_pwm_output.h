/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_PWM_OUTPUT_H_
#define _APP_PWM_OUTPUT_H_

#include <stdint.h>
#include <stdbool.h>

void app_pwm_output_init(void);
void app_pwm_output_set(uint32_t freq_hz, uint8_t duty_percent);
void app_pwm_output_start(void);
void app_pwm_output_stop(void);
bool app_pwm_output_is_active(void);
#endif /* _APP_PWM_OUTPUT_H_ */
