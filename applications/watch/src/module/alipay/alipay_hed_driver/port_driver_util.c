/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:         port_driver_util.c
 Author:            zhengwd
 Version:           V1.0
 Date:          2020-04-01
 Description:
 History:

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdarg.h>
#include "port_driver_util.h"
#include "platform_utils.h"
#include "trace.h"

#if CONFIG_ALIPAY

#ifdef _DEBUG


#if HAL_UART_PRINTF_ENABLE

UART_HandleTypeDef hal_uart_handle = {0};


/*********************************************************************************
Function:       hal_printf_init
Description:   uart��ʼ����ʹ��demo��Ĵ��ڴ�ӡ���log��Ϣ��������115200bps
Input:          no
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
void hal_printf_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    HAL_USART1_RX_GPIO_CLK_ENABLE();
    HAL_USART1_TX_GPIO_CLK_ENABLE();

    HAL_USART1_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* LPUART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = HAL_USART1_TX_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = HAL_USART1_TX_AF;

    HAL_GPIO_Init(HAL_USART1_TX_GPIO_PORT, &GPIO_InitStruct);

    /* LPUART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = HAL_USART1_RX_PIN;
    GPIO_InitStruct.Alternate = HAL_USART1_RX_AF;

    HAL_GPIO_Init(HAL_USART1_RX_GPIO_PORT, &GPIO_InitStruct);

    /* LPUART configured as follow:
    - Word Length = 8 Bits
    - Stop Bit = One Stop bit
    - Parity = None
    - BaudRate = 115200 baud
    - Hardware flow control disabled (RTS and CTS signals)
    - Oversampling at 16
    - One bit sampling feature disabled */
    hal_uart_handle.Instance            = LPUART1;
    hal_uart_handle.Init.BaudRate       = 115200;
    hal_uart_handle.Init.WordLength     = UART_WORDLENGTH_8B;
    hal_uart_handle.Init.StopBits       = UART_STOPBITS_1;
    hal_uart_handle.Init.Parity         = UART_PARITY_NONE;
    hal_uart_handle.Init.HwFlowCtl      = UART_HWCONTROL_NONE;
    hal_uart_handle.Init.Mode           = UART_MODE_TX_RX;
    hal_uart_handle.Init.OverSampling   = UART_OVERSAMPLING_16;
    hal_uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

    if (HAL_UART_Init(&hal_uart_handle) != HAL_OK)
    {

    }
}


int fputc(int ch, FILE *f)
{
    uint8_t tmp_ch = 0;

    tmp_ch = ch;
    HAL_UART_Transmit(&hal_uart_handle, &tmp_ch, 1, 100);
    return (ch);
}


#else

#define ITM_Port8(n)    (*((volatile uint8_t *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile uint16_t *)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile uint32_t *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile uint32_t *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever you need here */ };
extern FILE __stdout;
extern FILE __stdin;


int fputc(int ch, FILE *f)
{
    if (DEMCR & TRCENA)
    {
        while (ITM_Port32(0) == 0);
        ITM_Port8(0) = ch;
    }
    return (ch);
}
#endif   //#if HAL_UART_PRINTF_ENABLE

#endif  //#ifdef _DEBUG



//_weak void hal_printf(const char *format, ...)
/*********************************************************************************
Function:       hal_printf
Description:   ��ӡ���log��Ϣ
Input:          format...
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void hal_printf(const char *format, ...)
{
    char printf_buf[256];

    va_list args;
    va_start(args, format);
    //vprintf(format, args);
    vsnprintf(printf_buf, sizeof(printf_buf), format, args);
    va_end(args);

    DBG_DIRECT("%s", printf_buf);
}


/*********************************************************************************
Function:       hal_delay
Description:   ������ʱ
Input:          us, ��ʱ����ֵ����λΪ΢��
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void hal_delay(uint32_t us)
{
//  uint32_t i = 0;
//  uint32_t j = 0;
//
//  for(i=0;i<us;i++)
//  {
//      for(j=0;j<11;j++);
//  }

    platform_delay_us(us);

}

/*********************************************************************************
Function:       hal_systick
Description:   ����Tickֵ������ʱ���ʱ
Input:          no
Output:         no
Return:         ��ʱ��tickֵ����λΪms
Others:         no
*********************************************************************************/
uint32_t hal_systick(void)
{
    return sys_timestamp_get();//HAL_GetTick();
}


#endif //CONFIG_ALIPAY





