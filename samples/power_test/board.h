/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#define POWER_TEST_UART_TX          P3_1
#define POWER_TEST_UART_RX          P3_0
#define POWER_TEST_UART_BAUD_RATE   BAUD_RATE_115200

#undef  F_APP_EXTERNAL_BUCK_SUPPORT
#define F_APP_EXTERNAL_BUCK_SUPPORT         0

#define KEY_NULL                0
#define KEY0_MASK               0x01
#define KEY1_MASK               0x02
#define KEY2_MASK               0x04
#define KEY3_MASK               0x08
#define KEY4_MASK               0x10
#define KEY5_MASK               0x20
#define KEY6_MASK               0x40
#define KEY7_MASK               0x80

#define KEY_RELEASE             0
#define KEY_PRESS               1

#define MAX_KEY_NUM             8
#define HYBRID_KEY_NUM          8

#define LED_NUM                 3 //MAX: 3 LED
#define SLEEP_LED_PINMUX_NUM    10

#define PWM_TIMER_PINMUX        timer_pwm5

#define HW_TIM_ADP_DET          TIM2
#define SENSOR_LD_TIMER         TIM3
#define LINE_IN_TIMER           TIM3
#define PWM_TIMER               TIM5
#define MULTI_SPK_TIMER         TIM6
#define DSP_TIMER               TIM7 //DSP HW fixed at timer7

#define HW_TIM_IRQn_ADP_DET     TIM2_IRQn
#define SENSOR_LD_TIMER_IRQ     TIM3_IRQn
#define LINE_IN_TIMER_IRQ       TIM3_IRQn
#define PWM_TIMER_IRQ           TIM5_IRQn
#define MULTI_SPK_TIMER_IRQ     TIM6_IRQn

#define APP_ADP_DET_TIMER_HANDLER       TIM2_Handler
#define LINE_IN_TIMER_INTR_HANDLER      TIM3_Handler
#define PWM_TIMER_INTR_HANDLER          TIM5_Handler
#define MULTI_SPK_TIMER_INTR_HANDLER    TIM6_Handler

#define UART_RX_BUFFER_SIZE         1024
#define RX_GDMA_START_ADDR          UART_RX_BUFFER_SIZE
#define RX_GDMA_BUFFER_SIZE         300

/** @defgroup IO Driver Config
  * @note user must config it firstly!! Do not change macro names!!
  * @{
  */

/* if use user define dlps enter/dlps exit callback function */
#define USE_USER_DEFINE_DLPS_EXIT_CB      1
#define USE_USER_DEFINE_DLPS_ENTER_CB     1

#undef F_APP_ONE_WIRE_UART_SUPPORT
#define F_APP_ONE_WIRE_UART_SUPPORT     0

#define F_APP_PSRAM_ENABLE              0
/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* _BOARD_H_ */

