/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _BOARD_H_
#define _BOARD_H_
#include <zephyr/dt-bindings/pinctrl/rtl87x3g-pinctrl.h>
#ifdef __cplusplus
extern "C" {
#endif

/* refer by device tree gpio*/
#define TP_INT_GPIO_PIN         &gpioa 2      /*P0_2*/
#define TP_RST_GPIO_PIN         &gpioa 3      /*P0_3*/
#define SD_SDHC_PWR_GPIO_PIN    &gpioa 6      /*P3_4*/
#define EXT_AMP_GPIO_PIN        &gpioa 20     /*P2_5*/
#define KEY1_GPIO_PIN           &gpioa 23     /*P3_0*/
#define KEY2_GPIO_PIN           &gpioa 0      /*ADC_0*/
#define KEY3_GPIO_PIN           &gpioa 24     /*P3_1*/


/* refer by device tree pinctrl*/
#define UART2_TX_PIN            P3_1
#define UART2_RX_PIN            P3_5
#define I2C1_SCL_PIN            ADC_0       /*P0_0*/
#define I2C1_SDA_PIN            ADC_1       /*P0_1*/
#define QDEC_PHASE_A_PIN        P4_0
#define QDEC_PHASE_B_PIN        P4_1
#define SD_SDHC_CLK_PIN            P5_0
#define SD_SDHC_CMD_PIN            P5_1
#define SD_SDHC_DATA0_PIN          P5_2
#define SD_SDHC_DATA1_PIN          P5_3
#define SD_SDHC_DATA2_PIN          P5_4
#define SD_SDHC_DATA3_PIN          P5_5

/* refer by lcd driver*/
#ifndef LCD_QSPI_CS
#define LCD_QSPI_CS                       P9_2
#endif
#ifndef LCD_QSPI_CLK
#define LCD_QSPI_CLK                      P9_4
#endif
#ifndef LCD_QSPI_D0
#define LCD_QSPI_D0                       P9_3
#endif
#ifndef LCD_QSPI_D1
#define LCD_QSPI_D1                       P9_1
#endif
#ifndef LCD_QSPI_D2
#define LCD_QSPI_D2                       P9_0
#endif
#ifndef LCD_QSPI_D3
#define LCD_QSPI_D3                       P9_5
#endif
#ifndef LCD_QSPI_RST
#define LCD_QSPI_RST                      P4_4
#endif
#ifndef LCD_QSPI_TE
#define LCD_QSPI_TE                       P2_2
#endif
#ifndef LCD_AVDD_EN
#define LCD_AVDD_EN                       P2_3
#endif



#ifdef __cplusplus
}
#endif
#endif /* _BOARD_H_ */
