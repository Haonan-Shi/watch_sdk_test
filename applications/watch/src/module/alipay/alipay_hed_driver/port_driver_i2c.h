/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:         port_driver_i2c.h
 Author:            zhengwd
 Version:           V1.0
 Date:                      2020-04-07
 Description:
 History:

******************************************************************************/
#ifndef _PORT_STM32L433_I2C_H_
#define _PORT_STM32L433_I2C_H_


/***************************************************************************
* Include Header Files
***************************************************************************/
#include <stdint.h>
#include "hed_private.h"
//#include "stm32l4xx_hal.h"
//#include "stm32l4xx_hal_i2c.h"


#include "rtl876x_i2c.h"
#include "rtl876x_rtc.h"
#include "rtl876x_pinmux.h"
#include "stdio.h"
#include "alipay_config.h"



#ifdef HED_I2C


/**************************************************************************
* Global Macro Definition
***************************************************************************/
#define CRC_A 0
#define CRC_B 1 // ���в�����CRC_A��CRCType��������Ϊ��CRC_B


#define             PORT_I2C_HAL_TIMEOUT     50       //ms
#define     PORT_I2C_SE_RST_LOW_DELAY        200      //us  T7
#define     PORT_I2C_SE_RST_HIGH_DELAY       10000      //us     T6


#define     PORT_I2C_SE_PWR_OFF_DEALY       5000      //us   5ms
#define     PORT_I2C_SE_PWR_ON_DEALY        5000      //us   5ms


/********************����GPIO ����*******************/

#if defined(HED_I2C_SE0)
//SE0 RST ����IO
//#define PORT_I2C_SE0_RST_IO_PORT     GPIOA
//#define PORT_I2C_SE0_RST_IO_PIN      GPIO_PIN_7
////#define PORT_I2C_SE0_RST_IO_PIN      GPIO_PIN_0
//#define PORT_I2C_SE0_RST_IO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

#define PORT_I2C_SE0_RST_LOW()       Pad_Config(I2C_RESET_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_LOW)//HAL_GPIO_WritePin(PORT_I2C_SE0_RST_IO_PORT,PORT_I2C_SE0_RST_IO_PIN,GPIO_PIN_RESET) 
#define PORT_I2C_SE0_RST_HIGH()      Pad_Config(I2C_RESET_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH)//HAL_GPIO_WritePin(PORT_I2C_SE0_RST_IO_PORT,PORT_I2C_SE0_RST_IO_PIN,GPIO_PIN_SET)
#endif

#if defined(HED_I2C_SE1)
//SE1 RST ����IO
#define PORT_I2C_SE1_RST_IO_PORT     GPIOA
#define PORT_I2C_SE1_RST_IO_PIN      GPIO_PIN_1
#define PORT_I2C_SE1_RST_IO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

#define PORT_I2C_SE1_RST_LOW()       HAL_GPIO_WritePin(PORT_I2C_SE1_RST_IO_PORT,PORT_I2C_SE1_RST_IO_PIN,GPIO_PIN_RESET)
#define PORT_I2C_SE1_RST_HIGH()      HAL_GPIO_WritePin(PORT_I2C_SE1_RST_IO_PORT,PORT_I2C_SE1_RST_IO_PIN,GPIO_PIN_SET)
#endif


/********************I2C �ӿ�IO ����*******************/
#define PORT_I2C_ADDRESS_2A        (0x2A<<1)                //be carefull!   the addr is 0x2A.
#define PORT_I2C_ADDRESS_2C        (0x2C<<1)                //be carefull!   the addr is 0x3C.

/* I2C TIMING Register define when I2C clock source is SYSCLK */
/* I2C TIMING is calculated in case of the I2C Clock source is the SYSCLK = 80 MHz */
/* This example use TIMING to 0x00D00E28 to reach 1 MHz speed (Rise time = 120ns, Fall time = 25ns) */
#define I2C_TIMING       0x5033050A//19:6 0x5033020D//1.3:1.2 0x50330708//2:1 0x5033040B//keyi 16:9 0x5033050A//0xC0330A0F 118KHz//0x50330409   400khz //0x50330409   //0x20702991 //0x20D00E1A//0x20D00E28

/* Definition for I2Cx clock resources */
//#define I2Cnx                            I2C2
//#define RCC_PERIPHCLK_I2Cx              RCC_PERIPHCLK_I2C1
//#define RCC_I2CxCLKSOURCE_SYSCLK        RCC_I2C1CLKSOURCE_SYSCLK
//#define I2Cx_CLK_ENABLE()               __HAL_RCC_I2C1_CLK_ENABLE()
//#define I2Cx_SDA_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()
//#define I2Cx_SCL_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOB_CLK_ENABLE()

//#define I2Cx_FORCE_RESET()              __HAL_RCC_I2C1_FORCE_RESET()
//#define I2Cx_RELEASE_RESET()            __HAL_RCC_I2C1_RELEASE_RESET()

///* Definition for I2Cx Pins */
//#define I2Cx_SCL_PIN                    GPIO_PIN_8
//#define I2Cx_SCL_GPIO_PORT              GPIOB
//#define I2Cx_SDA_PIN                    GPIO_PIN_7
//#define I2Cx_SDA_GPIO_PORT              GPIOB
//#define I2Cx_SCL_SDA_AF                 GPIO_AF4_I2C1



/**************************************************************************
* Global Type Definition
***************************************************************************/
enum  PORT_I2C_CTRL
{
    PORT_I2C_CTRL_POWER =       0x00000001,
    PORT_I2C_CTRL_RST =     0x00000002,
    PORT_I2C_CTRL_OTHER =       0x0000000F
} ;


typedef struct   _i2c_comm_param_t
{
    //I2C_InitTypeDef *i2c_handle;
    I2C_InitTypeDef *i2c_handle;
    uint16_t slave_addr;
    uint8_t slave_id;
    bool locked;
} i2c_comm_param_t, *i2c_comm_param_pointer;


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
I2C_PERIPHERAL_DECLARE(I2C_PERIPHERAL_SE0);
I2C_PERIPHERAL_DECLARE(I2C_PERIPHERAL_SE1);



/**************************************************************************
* Global Functon Declaration
***************************************************************************/

extern se_error_t port_i2c_periph_init(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_deinit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_power_on(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_power_off(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_lock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_unlock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph);
extern se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf,
                                           uint32_t  inbuf_len);
extern se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf,
                                          uint32_t *outbuf_len);
extern se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph,
                                          uint32_t ctrlcode, uint8_t *inbuf, uint32_t  inbuf_len);



#endif //#ifdef HED_I2C
#endif/*_PORT_STM32L433_I2C_H*/
