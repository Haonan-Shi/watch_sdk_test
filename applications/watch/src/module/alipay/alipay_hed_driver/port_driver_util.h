#ifndef _PORT_STM32L433_UTIL_H_
#define _PORT_STM32L433_UTIL_H_

#include <stdint.h>
//#include "stm32l4xx_hal.h"
//#include "stm32l4xx_hal_gpio.h"
#include "hed_private.h"
//#include "stm32l4xx_hal_gpio_ex.h"
//#include "stm32l4xx_hal_rcc.h"

#include "rtl876x_i2c.h"
#include "rtl876x_rtc.h"
#include "rtl876x_pinmux.h"
#include "stdio.h"


/**************************************************************************
* Global Type Definition
***************************************************************************/

#ifdef _DEBUG


#if HAL_UART_PRINTF_ENABLE

#define HAL_USART1_CLK_ENABLE()              __HAL_RCC_LPUART1_CLK_ENABLE()

#define HAL_USART1_RX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()
#define HAL_USART1_TX_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOA_CLK_ENABLE()

#define HAL_USART1_TX_GPIO_PORT  GPIOA
#define HAL_USART1_TX_PIN   GPIO_PIN_2

#define HAL_USART1_RX_GPIO_PORT   GPIOA
#define HAL_USART1_RX_PIN    GPIO_PIN_3

#define HAL_USART1_TX_AF  GPIO_AF8_LPUART1
#define HAL_USART1_RX_AF  GPIO_AF8_LPUART1

extern void hal_printf_init(void);

#endif  //#if HAL_UART_PRINTF_ENABLE

extern int fputc(int ch, FILE *f) ;

#else
#define fputc(ch,f)
//#define hal_printf(...)

#endif   //#ifdef _DEBUG

extern void hal_printf(const char *format, ...);

extern void hal_delay(uint32_t us);
extern uint32_t hal_systick(void);


#endif/*_PORT_STM32L433_UTIL_H*/
