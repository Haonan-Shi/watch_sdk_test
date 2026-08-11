/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _DTS_BOARD_H_
#define _DTS_BOARD_H_
#include <zephyr/dt-bindings/pinctrl/rtl87x3g-pinctrl.h>
#ifdef __cplusplus
extern "C" {
#endif


/* refer by device tree gpio*/
#define TP_INT_GPIO_PIN         &gpioa 2      /*P0_2*/
#define TP_RST_GPIO_PIN         &gpioa 3      /*P0_3*/
#define SDHC0_PWR_GPIO_PIN      &gpioa 6      /*P3_4*/
#define SDHC1_PWR_GPIO_PIN      &gpiob 22     /*P0_5*/
#define EXT_AMP_GPIO_PIN        &gpioa 20     /*P2_5*/
#define KEY1_GPIO_PIN           &gpioa 23     /*P3_0*/
#define KEY2_GPIO_PIN           &gpioa 0      /*ADC_0*/
#define KEY3_GPIO_PIN           &gpioa 24     /*P3_1*/


#define PIN_M2S_GPIO            &gpioa 18     /*P2_3*/
#define PIN_S2M_GPIO            &gpioa 20     /*P2_5*/


#define PIN_SPI0_CS             &gpioa 19     /*P2_4, Used by overlay of master */
#define PIN_SPI0_SLAVE_CS       P2_4          /*P2_4, Used by overlay of slave */
#define PIN_SPI0_SCK            P1_1
#define PIN_SPI0_MOSI           P2_6
#define PIN_SPI0_MISO           ADC_1


/* refer by device tree pinctrl*/
#define UART0_TX_PIN            P3_1
#define UART0_RX_PIN            P3_0
#define UART2_TX_PIN            P4_1
#define UART2_RX_PIN            P4_0
#define I2C1_SCL_PIN            ADC_0       /*P0_0*/
#define I2C1_SDA_PIN            ADC_1       /*P0_1*/
#define SDHC0_CLK_PIN           P5_0
#define SDHC0_CMD_PIN           P5_1
#define SDHC0_DATA0_PIN         P5_2
#define SDHC0_DATA1_PIN         P5_3
#define SDHC0_DATA2_PIN         P5_4
#define SDHC0_DATA3_PIN         P5_5

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

#if F_APP_SPI_ROLE_SLAVE
#define PIN_SLAVE_SPI0_SCK          ADC_0
#define PIN_SLAVE_SPI0_MOSI         P1_0
#define PIN_SLAVE_SPI0_MISO         ADC_1
#define PIN_SLAVE_SPI0_CS           P2_4
#define PIN_S2M_GPIO                P2_3
#define PIN_M2S_GPIO                ADC_2
#endif

#if F_APP_SPI_ROLE_MASTER
#define PIN_MASTER_SPI0_SCK         ADC_0
#define PIN_MASTER_SPI0_MOSI        P1_0
#define PIN_MASTER_SPI0_MISO        ADC_1
#define PIN_MASTER_SPI0_CS          P2_4
#define PIN_S2M_GPIO                P2_3
#define PIN_M2S_GPIO                ADC_2
#endif



#ifdef __cplusplus
}
#endif
#endif /* _BOARD_H_ */
