/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "stdio.h"
#include "trace.h"
#include "os_msg.h"
#include "os_sync.h"
#include "rtl876x_uart.h"
#include "rtl876x_rcc.h"
#include "rtl876x_gdma.h"
#include "rtl876x_nvic.h"
#include "rtl876x_gpio.h"
#include "app_msg.h"
#include "app_cfg.h"
#include "app_dlps.h"
#include "console.h"
#include "vector_table.h"
#include "section.h"
#include "rtl876x.h"
#include "pm.h"
#include "app_task.h"
#include "dma_channel.h"
#include "mp_cmd.h"
#include "os_mem.h"
#include "console_uart.h"
#include "app_msg_handle.h"
#include "app_cmd.h"
#define UART_TX_DMA_CHANNEL_NUM     uart_tx_ch_num
#define UART_TX_DMA_CHANNEL         DMA_CH_BASE(uart_tx_ch_num)
#define UART_TX_DMA_IRQ             DMA_CH_IRQ(uart_tx_ch_num)

#define UART_RX_DMA_CHANNEL_NUM     uart_rx_ch_num
#define UART_RX_DMA_CHANNEL         DMA_CH_BASE(uart_rx_ch_num)
#define UART_RX_DMA_IRQ             DMA_CH_IRQ(uart_rx_ch_num)

#define RX_GDMA_BUFFER_SIZE         1050

/********************************************
DMA Channel/vector_table/GPIO_InitTypeDef
must follow RTL87x3E
********************************************/
// Data Uart Handler
#define Data_Uart_Handler  UART0_Handler

static void console_uart_handle_tx_done(void);
static bool uart_send_by_cpu_action(uint8_t *buf, uint32_t len);

uint8_t  *uart_rx_buf_addr = NULL;
uint16_t uart_tx_count = 0;

P_CONSOLE_CALLBACK p_console_callback = NULL;

#if (F_APP_AUTO_SUPPORT == 0)
bool is_rx_dma = true;
bool is_tx_dma = true;
#else
bool is_rx_dma = false;
bool is_tx_dma = false;
#endif

uint8_t uart_rx_ch_num  = 0xA5;
uint8_t uart_tx_ch_num  = 0xA5;

static uint8_t *uart_tx_curr_addr;
static uint32_t uart_tx_len;
static uint8_t tx_empty_need_handle = 0;

/**

 * @brief  Uart interface process received data accordint to interrupt.
 *         If enable GDMA, GDMA will carry data from UART peripheral. GDMA only need idle interrupt.
 *         GDMA can trigger data uart interrupt every receive 128 bytes data, and
 *         the number has been setting in uart driver init.
 *         When uart rx timeout interrupt cause, app will read UART peripheral data until
 *         UART FIFO is empty.
 * @param  void
 * @return void
 */
//RAM_TEXT_SECTION
void Data_Uart_Handler(void)
{
    uint32_t int_status = 0;
    uint16_t rx_count = 0;
    uint16_t fifo_count;

    UART_MaskINTConfig(UART0, UART_INT_MASK_TX_FIFO_EMPTY, ENABLE);

    int_status = UART_GetIID(UART0);

    if (UART_GetFlagState(UART0, UART_FLAG_RX_IDLE) == SET)
    {
        //clear Flag
        UART_INTConfig(UART0, UART_INT_IDLE, DISABLE);

        if (is_rx_dma)
        {
            //suspend will cause FIFO length + 1
            //read DMA FIFO length before suspend
            GDMA_SuspendCmd(UART_RX_DMA_CHANNEL, ENABLE);
            /*should waiting or fifo_count will not right when rx one byte or three byte*/
            while (GDMA_GetFIFOStatus(UART_RX_DMA_CHANNEL) != SET);
            //read DMA FIFO length before suspend
            fifo_count = GDMA_GetTransferLen(UART_RX_DMA_CHANNEL);
            GDMA_Cmd(UART_RX_DMA_CHANNEL_NUM, DISABLE);
//            APP_PRINT_ERROR1("uart idle: rx_count %d", fifo_count);
            rx_count = fifo_count;
            if (p_console_callback)
            {
                p_console_callback(CONSOLE_EVT_DATA_IND, uart_rx_buf_addr, rx_count);
            }

            GDMA_SetDestinationAddress(UART_RX_DMA_CHANNEL, (uint32_t)uart_rx_buf_addr);
            GDMA_SuspendCmd(UART_RX_DMA_CHANNEL, DISABLE);
            GDMA_Cmd(UART_RX_DMA_CHANNEL_NUM, ENABLE);
            UART_INTConfig(UART0, UART_INT_IDLE, ENABLE);
        }
    }
    if ((int_status == UART_INT_ID_RX_TMEOUT) || (int_status == UART_INT_ID_RX_LEVEL_REACH))
    {
        rx_count = UART_GetRxFIFOLen(UART0);
        UART_ReceiveData(UART0, uart_rx_buf_addr, rx_count);
//        APP_PRINT_ERROR1("uart rx timeout reach level: rx_count %d", rx_count);
        if (p_console_callback)
        {
            p_console_callback(CONSOLE_EVT_DATA_IND, uart_rx_buf_addr, rx_count);
        }
    }
    else if (int_status == UART_INT_ID_TX_EMPTY)
    {
        UART_INTConfig(UART0, UART_INT_FIFO_EMPTY, (FunctionalState)DISABLE);
        UART_GetIID(UART0);
        if (uart_tx_len)
        {
            uart_send_by_cpu_action(uart_tx_curr_addr, uart_tx_len);
        }
        else
        {
            if (tx_empty_need_handle)
            {
                tx_empty_need_handle = 0;
                console_uart_handle_tx_done();
            }
        }
    }
    UART_MaskINTConfig(UART0, UART_INT_MASK_TX_FIFO_EMPTY, DISABLE);
}

/**
 * @brief  GDMA0 channel 2 used for receive UART peripheral data.
 *         First clear rx GDMA interrupt mask and disable rx GDMA channel interrupt.
 *         Save data to UART rx buffer and set GDMA destination address.
 *         Then enable rx GDMA channel interrupt.
 *         Send IO_MSG_UART_RX event to app task.
 * @param  void
 * @return void
 */
void console_rx_dma_handler(void)
{
    uint16_t rx_count;

    GDMA_INTConfig(UART_RX_DMA_CHANNEL_NUM, GDMA_INT_Transfer, DISABLE);
    GDMA_ClearINTPendingBit(UART_RX_DMA_CHANNEL_NUM, GDMA_INT_Transfer);
    GDMA_Cmd(UART_RX_DMA_CHANNEL_NUM, DISABLE);

    rx_count = GDMA_GetTransferLen(UART_RX_DMA_CHANNEL);

//    APP_PRINT_ERROR1("GDMA0_Channel2_Handler: rx_count %d", rx_count);

    if (p_console_callback)
    {
        p_console_callback(CONSOLE_EVT_DATA_IND, uart_rx_buf_addr, rx_count);
    }

    GDMA_SetDestinationAddress(UART_RX_DMA_CHANNEL, (uint32_t)uart_rx_buf_addr);
    GDMA_INTConfig(UART_RX_DMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
    GDMA_Cmd(UART_RX_DMA_CHANNEL_NUM, ENABLE);
}

/**
 * @brief  GDMA0 channel 1 used for send UART peripheral data.
 *         When GDMA has been finished send all data, this interrupt will be cause.
 *         Clear tx GDMA channel all interrupt mask.
 *         Send IO_MSG_UART_TX event to app task, notify app to next tx action.
 * @param  void
 * @return void
 */
void console_tx_dma_handler(void)
{
    //T_IO_MSG uart_msg;
    //uart_msg.type = IO_MSG_TYPE_UART;
    //uart_msg.subtype = IO_MSG_UART_TX;

    GDMA_ClearAllTypeINT(UART_TX_DMA_CHANNEL_NUM);

    console_uart_handle_tx_done();
//    DBG_DIRECT("console_tx_dma_handler");
}

/**
    * @brief  UART tx channel start send data.
    *         Set GDMA source address and buffer size according to incoming parameters.
    *         Command GDMA tx channel to send data with os lock.
    * @param  void
    * @return void
    */
bool console_uart_dma_write(uint8_t *buf, uint32_t len)
{
    uint32_t s;
//    DBG_DIRECT("console_uart_dma_write = 0x%x, len = %d",buf, len);
    GDMA_SetSourceAddress(UART_TX_DMA_CHANNEL, (uint32_t)buf);
    GDMA_SetBufferSize(UART_TX_DMA_CHANNEL, len);
    s = os_lock();
    GDMA_Cmd(UART_TX_DMA_CHANNEL_NUM, ENABLE);
    os_unlock(s);

    return true;
}


/**
  * @brief  UART send data by cpu action.
  * @param  void
  * @return void
  */
static bool uart_send_by_cpu_action(uint8_t *buf, uint32_t len)
{
    uint32_t tx_len = len > UART_TX_FIFO_SIZE ? UART_TX_FIFO_SIZE : len;

    UART_SendData(UART0, buf, tx_len);

    uart_tx_curr_addr = buf + tx_len;
    uart_tx_len = len - tx_len;

    tx_empty_need_handle = 1;
    UART_INTConfig(UART0, UART_INT_FIFO_EMPTY, ENABLE);

    return true;
}


/**
  * @brief  UART send data by cpu action.
  * @param  void
  * @return void
  */
bool console_uart_write(uint8_t *buf, uint32_t len)
{
    //APP_PRINT_ERROR2("console_uart_write: %p, %d", buf, len);

    uart_tx_count = len;
    app_dlps_disable(APP_DLPS_ENTER_CHECK_CONSOLE);

    if (is_tx_dma)
    {
        return console_uart_dma_write(buf, len);
    }
    else
    {
        return uart_send_by_cpu_action(buf, len);
    }
}

/**
 * @brief  console uart handle tx data done, static function.
 * @param  void
 * @return void
 */
static void console_uart_handle_tx_done(void)
{
    uint16_t tx_count;

    /* GDMA_GetTransferLen(UART_TX_DMA_CHANNEL) is cleared when Channel disabled,
     * we just use the global variable to save it.
     * Let hardware to fix the DMA issue.
     */
    tx_count = uart_tx_count;
    app_dlps_enable(APP_DLPS_ENTER_CHECK_CONSOLE);

    if (p_console_callback)
    {
        p_console_callback(CONSOLE_EVT_DATA_XMIT, NULL, tx_count);
    }
}

/**
 * @brief  UART driver initial.
 *         Include APB peripheral clock config, UART parameter config
 * @param  void
 * @return void
 */
void console_uart_driver_init(void)
{
    UART_InitTypeDef uart_init;
    NVIC_InitTypeDef uart_nvic;

    /* Turn on UART clock */
    RCC_PeriphClockCmd(APBPeriph_UART0, APBPeriph_UART0_CLOCK, ENABLE);
    RamVectorTableUpdate(UART0_VECTORn, Data_Uart_Handler);

    UART_StructInit(&uart_init);
    switch (console_cfg_const.data_uart_baud_rate)
    {
    case BAUD_RATE_9600:
        uart_init.div = 271;
        uart_init.ovsr = 10;
        uart_init.ovsr_adj = 0x24A;
        break;

    case BAUD_RATE_19200:
        uart_init.div = 165;
        uart_init.ovsr = 7;
        uart_init.ovsr_adj = 0x5AD;
        break;

    case BAUD_RATE_115200:
        uart_init.div = 20;
        uart_init.ovsr = 12;
        uart_init.ovsr_adj = 0x252;
        break;

    case BAUD_RATE_230400:
        uart_init.div = 10;
        uart_init.ovsr = 12;
        uart_init.ovsr_adj = 0x252;
        break;

    case BAUD_RATE_460800:
        uart_init.div = 5;
        uart_init.ovsr = 12;
        uart_init.ovsr_adj = 0x252;
        break;

    case BAUD_RATE_921600:
        uart_init.div = 4;
        uart_init.ovsr = 5;
        uart_init.ovsr_adj = 0x3F7;
        break;

    case BAUD_RATE_2000000:
        uart_init.div = 2;
        uart_init.ovsr = 5;
        uart_init.ovsr_adj = 0;
        break;

    case BAUD_RATE_3000000:
        uart_init.div = 1;
        uart_init.ovsr = 8;
        uart_init.ovsr_adj = 0x492;
        break;

    case BAUD_RATE_4000000:
        uart_init.div = 1;
        uart_init.ovsr = 5;
        uart_init.ovsr_adj = 0;
        break;

    default:
        break;
    }

    uart_init.dmaEn = UART_DMA_ENABLE;
    uart_init.TxDmaEn = is_tx_dma ? ENABLE : DISABLE;
    uart_init.TxWaterlevel = 15;
    uart_init.RxDmaEn = is_rx_dma ? ENABLE : DISABLE;
    uart_init.RxWaterlevel = 1;

    //uart_init.rxTriggerLevel = UART_RX_FIFO_TRIGGER_LEVEL_8BYTE;
    UART_Init(UART0, &uart_init);
    if (!is_rx_dma)
    {
        UART_INTConfig(UART0, UART_INT_RD_AVA | UART_INT_IDLE, ENABLE);
    }
    else
    {
        UART_INTConfig(UART0, UART_INT_IDLE, ENABLE);
    }

    /* Enable UART IRQ */
    uart_nvic.NVIC_IRQChannel = UART0_IRQn;
    uart_nvic.NVIC_IRQChannelPriority = 5;
    uart_nvic.NVIC_IRQChannelCmd = (FunctionalState)ENABLE;
    NVIC_Init(&uart_nvic);
}


/**
 * @brief  UART tx dma driver initial.
 *         Include APB peripheral clock config, GDMA parameter config
 * @param  void
 * @return void
 */
void console_uart_tx_dma_init(void)
{
    GDMA_InitTypeDef tx_gdma;
    NVIC_InitTypeDef tx_nvic;
    if (uart_tx_ch_num == 0xA5)
    {
        if (!GDMA_channel_request(&uart_tx_ch_num, console_tx_dma_handler, true))
        {
            is_tx_dma = false;
            return;
        }
    }
    DBG_DIRECT("DMA uart_tx_ch_num = %d", uart_tx_ch_num);
    /*-------GDMA Tx IRQ init-------*/
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);

    /*-------UART Tx GDMA configuration -------*/
    GDMA_StructInit(&tx_gdma);
    tx_gdma.GDMA_ChannelNum = UART_TX_DMA_CHANNEL_NUM;
    tx_gdma.GDMA_DIR = GDMA_DIR_MemoryToPeripheral;
    tx_gdma.GDMA_BufferSize = 0;
    tx_gdma.GDMA_SourceInc = DMA_SourceInc_Inc;
    tx_gdma.GDMA_DestinationInc  = DMA_DestinationInc_Fix;
    tx_gdma.GDMA_SourceDataSize  = GDMA_DataSize_Byte;
    tx_gdma.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    tx_gdma.GDMA_SourceMsize     = GDMA_Msize_1;
    tx_gdma.GDMA_DestinationMsize = GDMA_Msize_1;
    tx_gdma.GDMA_DestinationAddr = (uint32_t)(&(UART0->RB_THR));
    tx_gdma.GDMA_DestHandshake   = GDMA_Handshake_UART0_TX;
    tx_gdma.GDMA_ChannelPriority = 5; //channel prority between 0 to 6
    GDMA_Init(UART_TX_DMA_CHANNEL, &tx_gdma);

    /*-------GDMA Tx IRQ init-------*/
    tx_nvic.NVIC_IRQChannel = UART_TX_DMA_IRQ;
    tx_nvic.NVIC_IRQChannelPriority = 5;
    tx_nvic.NVIC_IRQChannelCmd = (FunctionalState)ENABLE;
    NVIC_Init(&tx_nvic);
    GDMA_INTConfig(UART_TX_DMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);
}


/**
 * @brief  UART rx dma driver initial.
 *         Include APB peripheral clock config, GDMA parameter config
 * @param  void
 * @return void
 */
void console_uart_rx_dma_init(void)
{
    if (uart_rx_ch_num == 0xA5)
    {
        if (!GDMA_channel_request(&uart_rx_ch_num, console_rx_dma_handler, true))
        {
            is_rx_dma = false;
            return;
        }

    }
    DBG_DIRECT("DMA uart_rx_ch_num = %d", uart_rx_ch_num);

    GDMA_InitTypeDef rx_gdma;
    NVIC_InitTypeDef rx_nvic;
    /*-------UART Rx GDMA configuration -------*/
    GDMA_StructInit(&rx_gdma);
    rx_gdma.GDMA_ChannelNum      = UART_RX_DMA_CHANNEL_NUM;
    rx_gdma.GDMA_DIR             = GDMA_DIR_PeripheralToMemory;
    rx_gdma.GDMA_BufferSize      = RX_GDMA_BUFFER_SIZE;
    rx_gdma.GDMA_SourceInc       = DMA_SourceInc_Fix;
    rx_gdma.GDMA_DestinationInc  = DMA_DestinationInc_Inc;
    rx_gdma.GDMA_SourceDataSize  = GDMA_DataSize_Byte;
    rx_gdma.GDMA_DestinationDataSize = GDMA_DataSize_Byte;
    rx_gdma.GDMA_SourceMsize     = GDMA_Msize_1;
    rx_gdma.GDMA_DestinationMsize = GDMA_Msize_1;
    rx_gdma.GDMA_SourceAddr   = (uint32_t)(&(UART0->RB_THR));
    rx_gdma.GDMA_DestinationAddr = (uint32_t)uart_rx_buf_addr;
    rx_gdma.GDMA_SourceHandshake = GDMA_Handshake_UART0_RX;
    rx_gdma.GDMA_ChannelPriority = 5; //channel prority between 0 to 6
    GDMA_Init(UART_RX_DMA_CHANNEL, &rx_gdma);

    /*-------GDMA Rx IRQ init-------*/
    rx_nvic.NVIC_IRQChannel = UART_RX_DMA_IRQ;
    rx_nvic.NVIC_IRQChannelPriority = 5;
    rx_nvic.NVIC_IRQChannelCmd = (FunctionalState)ENABLE;
    NVIC_Init(&rx_nvic);
    GDMA_INTConfig(UART_RX_DMA_CHANNEL_NUM, GDMA_INT_Transfer, ENABLE);

    //GDMA_Cmd(UART_TX_DMA_CHANNEL_NUM, ENABLE);
    GDMA_Cmd(UART_RX_DMA_CHANNEL_NUM, ENABLE);
}

/**
 * @brief console uart exit dlps set
 *
 * @param None
 * @retval None
 */
void console_uart_enter_low_power(POWERMode mode)
{
    if (console_cfg_const.enable_rx_wake_up)
    {
        System_WakeUpPinEnable(console_cfg_const.rx_wake_up_pinmux, PAD_WAKEUP_POL_LOW);
        System_WakeUpInterruptEnable(console_cfg_const.rx_wake_up_pinmux);
    }
    else
    {
        {
            System_WakeUpPinEnable(console_cfg_const.data_uart_rx_pinmux, PAD_WAKEUP_POL_LOW);
            System_WakeUpInterruptEnable(console_cfg_const.data_uart_rx_pinmux);
        }
    }
}

/**
 * @brief console uart enter dlps set
 *
 * @param mode current low power mode to enter
 *
 */
void console_uart_exit_low_power(POWERMode mode)
{
    if (!console_cfg_const.enable_rx_wake_up)
    {
        uint8_t uart_rx_pinmux = 0xFF;
        {
            uart_rx_pinmux = console_cfg_const.data_uart_rx_pinmux;
        }

        if (System_WakeUpInterruptValue(uart_rx_pinmux) == SET)
        {
            Pad_ClearWakeupINTPendingBit(uart_rx_pinmux);
            T_IO_MSG dlps_msg;

            dlps_msg.type = IO_MSG_TYPE_GPIO;
            dlps_msg.subtype = IO_MSG_GPIO_UART_WAKE_UP;

            /* Send MSG to APP task */
            if (!app_send_msg_to_apptask(&dlps_msg))
            {
                APP_PRINT_ERROR0("app_dlps_exit_callback: Send DLPS msg error");
            }
        }
    }

    /*need init gdma angin for exit dlps can not restort gdma setting*/
    if (is_tx_dma)
    {
        console_uart_tx_dma_init();
    }

    if (is_rx_dma)
    {
        console_uart_rx_dma_init();
    }
}


/**
 * @brief  UART driver initial.
 *         Include APB peripheral clock config, UART GPIO parameter config and
 *         GDMA channel parameter config. Enable UART GPIO and GDMA channel interrupt.
 *         Create UART rx queue for communication between UART and app task.
 * @param  void
 * @return void
 */
bool console_uart_init(P_CONSOLE_CALLBACK p_callback)
{
    p_console_callback = p_callback;

    console_uart_driver_init();

    if (is_tx_dma)
    {
        console_uart_tx_dma_init();
    }

    if (is_rx_dma)
    {
        console_uart_rx_dma_init();
    }

    return p_console_callback(CONSOLE_EVT_OPENED, NULL, 0);
}


void app_console_init(void)
{
    uart_rx_buf_addr = os_mem_alloc(RAM_TYPE_DATA_ON, RX_GDMA_BUFFER_SIZE);
    if (uart_rx_buf_addr == NULL)
    {
        APP_PRINT_ERROR0("app_console_init: uart rx buffer init failed!");
        return;
    }

    Pinmux_Config(console_cfg_const.data_uart_tx_pinmux, UART0_TX);
    Pad_Config(console_cfg_const.data_uart_tx_pinmux,
               PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);

    Pinmux_Config(console_cfg_const.data_uart_rx_pinmux, UART0_RX);
    Pad_Config(console_cfg_const.data_uart_rx_pinmux,
               PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

    T_CONSOLE_PARAM console_param;
    T_CONSOLE_OP    console_op;
    {
        console_param.tx_buf_size   = 1050;//CONSOLE_TX_BUFFER_SMALL;
        console_param.rx_buf_size   = 1050;//CONSOLE_RX_BUFFER_SMALL;
    }

    console_op.init = console_uart_init;
    console_op.tx_wakeup_enable = NULL; //console_uart_tx_wakeup_enable;
    console_op.rx_wakeup_enable = NULL; //console_uart_rx_wakeup_enable;

    {
        console_op.write = console_uart_write;
    }
    console_op.wakeup = NULL;

    console_init(&console_param, &console_op);
    console_set_mode(CONSOLE_MODE_BINARY);

    mp_cmd_module_init();
    mp_cmd_register();
}
