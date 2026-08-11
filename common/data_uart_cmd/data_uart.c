/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdarg.h>
#include <data_uart.h>
#include <os_msg.h>
#include <trace.h>
#include <app_msg.h>
#include <board.h>
#include <rtl876x.h>
#include <rtl876x_nvic.h>
#include <rtl876x_uart.h>
#include <rtl876x_rcc.h>
#include <rtl876x_pinmux.h>
#include "vector_table.h"

static void *h_event_q;
static void *h_io_q;

int data_uart_send_char(int ch)
{
    UART_SendData(UART0, (uint8_t *)&ch, 1);
    /* wait tx fifo empty */
    while (UART_GetFlagState(UART0, UART_FLAG_THR_TSR_EMPTY) != SET);

    return ch;
}

int data_uart_vsprintf(char *buf, const char *fmt, va_list args)
{
    char *p, *s = buf;

    while (*fmt != '\0')
    {
        if (*fmt != '%')
        {
            if (buf)
            {
                *s++ = *fmt;  // Output to the buffer
            }
            else
            {
                data_uart_send_char(*fmt);
            }
            ++fmt;
            continue;
        }

        ++fmt;  // skip '%'

        if (*fmt == 's')  // String
        {
            p = va_arg(args, char *);
            while (*p != '\0')
            {
                if (buf)
                {
                    *s++ = *p;
                }
                else
                {
                    data_uart_send_char(*p);
                }
                p++;
            }
        }
        else if (*fmt == 'c')  // Character
        {
            char c = (char)va_arg(args, int);
            if (buf)
            {
                *s++ = *p;
            }
            else
            {
                data_uart_send_char(*p);
            }
        }
        else
        {
            char tmp[20], *q = tmp;
            int shift = 28;
            int alt = 0;

            if ((*fmt >= '0') && (*fmt <= '9'))
            {
                // Parse width
                int width = 0;
                unsigned char fch = *fmt;
                while (fch >= '0' && fch <= '9')
                {
                    width = width * 10 + (fch - '0');
                    fch = *++fmt;
                }
                shift = (width - 1) * 4;
            }

            if (*fmt == 'x' || *fmt == 'X' || *fmt == 'p' || *fmt == 'P')   // x: Lowercase hexadecimal number,
            {
                // X: Uppercase hexadecimal number,
                long h = va_arg(args, long);                                // p: Pointer, P: Uppercase pointer
                int ncase = (*fmt & 0x20);
                if (*fmt == 'p' || *fmt == 'P')
                {
                    alt = 1;
                }
                if (alt)
                {
                    *q++ = '0';
                    *q++ = 'x' | ncase;
                }
                for (; shift >= 0; shift -= 4)
                {
                    *q++ = "0123456789ABCDEF"[(h >> shift) & 0xF] | ncase;
                }
            }
            else if (*fmt == 'd')  // Decimal integer
            {
                int i = va_arg(args, int);
                char *r;
                if (i < 0)
                {
                    *q++ = '-';
                    i = -i;
                }
                p = q;      /* save beginning of digits */
                do
                {
                    *q++ = '0' + (i % 10);
                    i /= 10;
                }
                while (i);
                /* reverse digits, stop in middle */
                r = q;      /* don't alter q */
                while (--r > p)
                {
                    i = *r;
                    *r = *p;
                    *p++ = i;
                }
            }

            for (p = tmp; p < q; ++p)
            {
                if (buf)
                {
                    *s++ = *p;
                }
                else
                {
                    data_uart_send_char(*p);
                }
            }
        }
        ++fmt;
    }

    if (buf)
    {
        *s = '\0';
    }

    return s - buf; // Return the length of the generated string.
}

/**
 * @brief  Print the trace information through data uart.
 * @param[in] fmt   Print parameters.
 * @return void
 *
 * <b>Example usage</b>
 * \code{.c}
    void test(void)
    {
        data_uart_print("GAP scan stop\r\n");
    }
 * \endcode
 */
void data_uart_print(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    (void)data_uart_vsprintf(0, fmt, args);
    va_end(args);
}

/****************************************************************************/
/* UART0 interrupt                                                           */
/****************************************************************************/
void UART0_Handler(void)
{
    uint8_t rx_char;
    T_IO_MSG io_driver_msg_send;
    uint8_t event  = EVENT_IO_TO_APP;
    io_driver_msg_send.type = IO_MSG_TYPE_UART;

    uint32_t interrupt_id = 0;
    /* read interrupt id */
    interrupt_id = UART_GetIID(UART0);

    /* disable interrupt */
    UART_INTConfig(UART0, UART_INT_RD_AVA | UART_INT_LINE_STS, DISABLE);

    switch (interrupt_id)
    {
    /* tx fifo empty, not enable */
    case UART_INT_ID_TX_EMPTY:
        break;

    /* rx data valiable */
    case UART_INT_ID_RX_LEVEL_REACH:
        UART_ReceiveData(UART0, &rx_char, 1);
        io_driver_msg_send.subtype = rx_char;

        if (os_msg_send(h_io_q, &io_driver_msg_send, 0) == false)
        {
        }
        else if (os_msg_send(h_event_q, &event, 0) == false)
        {
        }
        break;

    case UART_INT_ID_RX_TMEOUT:
        break;

    /* receive line status interrupt */
    case UART_INT_ID_LINE_STATUS:
        {
            DBG_DIRECT("Line status error!!!!\n");
        }
        break;

    default:
        break;
    }

    /* enable interrupt again */
    UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);

    return;
}

/**
 * @brief  Initializes the Data UART0.
 *
 * When data uart receives data, data uart will send an event IO_MSG_TYPE_UART to evt_queue_handle and send the data to io_queue_handle.
 * @param[in] event_queue_handle   Event queue handle which is created by APP.
 * @param[in] io_queue_handle      IO message queue handle which is created by APP.
 * @return void
 *
 * <b>Example usage</b>
 * \code{.c}
    void app_main_task(void *p_param)
    {
        char event;

        os_msg_queue_create(&io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
        os_msg_queue_create(&evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(unsigned char));

        gap_start_bt_stack(evt_queue_handle, io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

        data_uart_init(evt_queue_handle, io_queue_handle);
        ......
    }
    void app_handle_io_msg(T_IO_MSG io_msg)
    {
        uint16_t msg_type = io_msg.type;
        uint8_t rx_char;

        switch (msg_type)
        {
        case IO_MSG_TYPE_UART:
            // We handle user command informations from Data UART0 in this branch.
            rx_char = (uint8_t)io_msg.subtype;
            user_cmd_collect(&user_cmd_if, &rx_char, sizeof(rx_char), user_cmd_table);
            break;
        default:
            break;
        }
    }
 * \endcode
 */
void data_uart_init(void *event_queue_handle, void *io_queue_handle)
{
    /* Clear UART0 pinmux */
    *((volatile uint32_t *)0x40000298) &= ~((0x7f << 8) | (0x7f));
    h_event_q = event_queue_handle;
    h_io_q = io_queue_handle;

    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);

    Pinmux_Config(DATA_UART_TX_PIN, UART0_TX);
    Pinmux_Config(DATA_UART_RX_PIN, UART0_RX);

    Pad_Config(DATA_UART_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE,
               PAD_OUT_HIGH);
    Pad_Config(DATA_UART_RX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE,
               PAD_OUT_LOW);

    /* uart init */
    UART_InitTypeDef uartInitStruct;
    UART_StructInit(&uartInitStruct);
    uartInitStruct.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_1BYTE;
    UART_Init(UART0, &uartInitStruct);
    UART_INTConfig(UART0, UART_INT_RD_AVA, ENABLE);

    RamVectorTableUpdate(UART0_VECTORn, UART0_Handler);

    /*  Enable UART0 IRQ  */
    NVIC_InitTypeDef nvic_init_struct;
    nvic_init_struct.NVIC_IRQChannel         = UART0_IRQn;
    nvic_init_struct.NVIC_IRQChannelCmd      = ENABLE;
    nvic_init_struct.NVIC_IRQChannelPriority = 5;
    NVIC_Init(&nvic_init_struct);
}

