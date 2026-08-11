/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:         port_driver_i2c.c
 Author:            zhengwd
 Version:           V1.0
 Date:          2020-04-07
 Description:
 History:

******************************************************************************/


/***************************************************************************
* Include Header Files
***************************************************************************/
#include "port_driver_i2c.h"

#include "rtl876x_i2c.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "alipay_config.h"

#include "hed_private.h"
#include "platform_utils.h"

#if  CONFIG_ALIPAY

#ifdef HED_I2C


/**************************************************************************
* Global Variable Declaration
***************************************************************************/
#if defined(HED_I2C_SE0)
I2C_InitTypeDef i2c_comm_handle_slave0 = {0};
i2c_comm_param_t i2c_comm_parm_slave0 = {&i2c_comm_handle_slave0, PORT_I2C_ADDRESS_2A, I2C_PERIPHERAL_SE0, FALSE};
#endif

#if defined(HED_I2C_SE1)
I2C_InitTypeDef i2c_comm_handle_slave1 = {0};
i2c_comm_param_t i2c_comm_parm_slave1 = {&i2c_comm_handle_slave1, PORT_I2C_ADDRESS_2C, I2C_PERIPHERAL_SE1, FALSE};
#endif
static uint8_t g_i2c_device_init[MAX_PERIPHERAL_DEVICE] = {FALSE};


/*********************************************************************************
Function:       HAL_I2C_MspInit
Description:    This function configures the i2c hardware resources
Input:          hi2c I2C handle pointer
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void HAL_I2C_MspInit(I2C_InitTypeDef *hi2c)
{
#if 1//@yuyin



    DBG_DIRECT("[Alipay] HAL_I2C_MspInit");
#else
    GPIO_InitTypeDef  GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef  RCC_PeriphCLKInitStruct = {0};

    /*##-1- Configure the I2C clock source. The clock is derived from the SYSCLK #*/
    RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2Cx;
    RCC_PeriphCLKInitStruct.I2c1ClockSelection = RCC_I2CxCLKSOURCE_SYSCLK;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct);

    /*##-2- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    I2Cx_SCL_GPIO_CLK_ENABLE();
    I2Cx_SDA_GPIO_CLK_ENABLE();
    /* Enable I2Cx clock */
    I2Cx_CLK_ENABLE();

    /*##-3- Configure peripheral GPIO ##########################################*/
    /* I2C TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    //GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
    HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

    /* I2C RX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = I2Cx_SDA_PIN;
    GPIO_InitStruct.Alternate = I2Cx_SCL_SDA_AF;
    HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

#endif
}


/*********************************************************************************
Function:       HAL_I2C_MspDeInit
Description:    This function frees the i2c hardware resources
Input:          hi2c  I2C handle pointer
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void HAL_I2C_MspDeInit(I2C_TypeDef *hi2c)
{
#if 1 //@yuyin
    I2C_DeInit(ALIPAY_I2Cx);
#else
    /*##-1- Reset peripherals ##################################################*/
    I2Cx_FORCE_RESET();
    I2Cx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure I2C Tx as alternate function  */
    HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
    /* Configure I2C Rx as alternate function  */
    HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
#endif
}



#if defined(HED_I2C_SE0)
/*********************************************************************************
Function:       port_i2c_se0_gpio_init
Description:    ����SE0��gpio��ʼ����RST
Input:          no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void port_i2c_se0_gpio_init(void)
{
#if 1 //@yuyin
    // pad control, nothing to be done
    DBG_DIRECT("[Alipay] port_i2c_se0_gpio_init");
#else
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //---SE RST ����IO----
    GPIO_InitStruct.Pin = PORT_I2C_SE0_RST_IO_PIN;
    PORT_I2C_SE0_RST_IO_CLK_ENABLE();
    HAL_GPIO_Init(PORT_I2C_SE0_RST_IO_PORT, &GPIO_InitStruct);
    //PORT_I2C_S0_RST_OFF();   //�ߵ�ƽ
#endif
}
#endif


#if defined(HED_I2C_SE1)
/*********************************************************************************
Function:       port_i2c_se1_gpio_init
Description:    ����SE1��gpio��ʼ����RST
Input:          no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void port_i2c_se1_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //---SE RST ����IO----
    GPIO_InitStruct.Pin = PORT_I2C_SE1_RST_IO_PIN;
    PORT_I2C_SE1_RST_IO_CLK_ENABLE();
    HAL_GPIO_Init(PORT_I2C_SE1_RST_IO_PORT, &GPIO_InitStruct);
    //PORT_I2C_S1_RST_OFF();   //�ߵ�ƽ
}
#endif



/*********************************************************************************
Function:       port_i2c_init
Description:    i2c�ӿڳ�ʼ������ȡ�豸���
Input:          i2c_handle_instance   I2C handle pointer
                i2c_addr ���豸��ַ
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t  port_i2c_init(I2C_InitTypeDef *i2c_handle_instance, uint8_t i2c_addr)
{
    DBG_DIRECT("[Alipay] port_i2c_init");
#if 1//@yuyin
    Pinmux_Deinit(I2C_CLK_PIN);
    Pinmux_Deinit(I2C_SDA_PIN);

    Pinmux_Config(I2C_SDA_PIN, I2C2_DAT);
    Pinmux_Config(I2C_CLK_PIN, I2C2_CLK);

    Pad_PullConfigValue(I2C_SDA_PIN, 1);
    Pad_PullConfigValue(I2C_CLK_PIN, 1);

    Pad_Config(I2C_SDA_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pad_Config(I2C_CLK_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE, PAD_OUT_HIGH);

    RCC_PeriphClockCmd(APBPeriph_I2C2, APBPeriph_I2C2_CLOCK, ENABLE);
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_InitStructure.I2C_Clock = 40000000;
    I2C_InitStructure.I2C_ClockSpeed = 400000;//100k
    I2C_InitStructure.I2C_DeviveMode = I2C_DeviveMode_Master;
    I2C_InitStructure.I2C_AddressMode = I2C_AddressMode_7BIT;
    I2C_InitStructure.I2C_SlaveAddress = i2c_addr >> 1;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;

    I2C_Init(ALIPAY_I2Cx, &I2C_InitStructure);
    I2C_Cmd(ALIPAY_I2Cx, ENABLE);

    void HED_IIC_Init(void);
    HED_IIC_Init();
#else
    //Configure the I2C peripheral
    I2C_InitTypeDef *HEDSEi2cHandle = i2c_handle_instance;

    (*HEDSEi2cHandle).Instance             = I2Cx;
    (*HEDSEi2cHandle).Init.Timing          = I2C_TIMING;
    (*HEDSEi2cHandle).Init.OwnAddress1     = i2c_addr;
    (*HEDSEi2cHandle).Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    (*HEDSEi2cHandle).Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    (*HEDSEi2cHandle).Init.OwnAddress2     = 0xFF;
    (*HEDSEi2cHandle).Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    (*HEDSEi2cHandle).Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(HEDSEi2cHandle) != HAL_OK)
    {
        return SE_ERR_INIT;
    }

    //Enable the Analog I2C Filter
    HAL_I2CEx_ConfigAnalogFilter(HEDSEi2cHandle, I2C_ANALOGFILTER_ENABLE);
#endif
    return SE_SUCCESS;
}



/*********************************************************************************
Function:       port_i2c_deinit
Description:    I2C�ӿ��Ƴ�����ȡ�豸������
Input:          handle  I2C handle pointer
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_deinit(void *handle)
{
    //if(HAL_I2C_DeInit((I2C_InitTypeDef*)handle)!=HAL_OK)
    {
        //return SE_ERR_INIT;
    }

    HAL_I2C_MspDeInit(ALIPAY_I2Cx);

    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_spi_periph_init
Description:   ���豸periphͨ�ų�ʼ��
               1. ����port_i2c_init������ʼ��SPI�ӿ�
               2. ��ʼ������GPIO
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_init(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code = SE_ERR_HANDLE_INVALID;
//          break;
//      }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code = port_i2c_init(p_comm_param->i2c_handle, p_comm_param->slave_addr);
            if (ret_code != SE_SUCCESS)
            {
                break;
            }
            g_i2c_device_init[p_comm_param->slave_id] = TRUE;
        }

        if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
        {
#if defined(HED_I2C_SE0)
            port_i2c_se0_gpio_init();
#if defined(SE_CIU98_D)
            PORT_I2C_SE0_RST_HIGH();   //�ߵ�ƽ
#endif

#if defined(HED_I2C_DUAL)
            port_i2c_se1_gpio_init();
            PORT_I2C_SE1_RST_HIGH();   //�ߵ�ƽ
#endif
#endif
        }

        else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
        {
#if defined(HED_I2C_SE1)
            port_i2c_se1_gpio_init();
            PORT_I2C_SE1_RST_HIGH();   //�ߵ�ƽ
#endif

#if defined(HED_I2C_DUAL)
            port_i2c_se0_gpio_init();
            PORT_I2C_SE0_RST_HIGH();   //�ߵ�ƽ
#endif
        }

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_deinit
Description:    ���豸periphͨ����ֹ��
               1. ����port_i2c_deinit������ֹ��SPI�ӿ�
               2. ����RST����IOΪ�͵�ƽ
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_deinit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code = SE_ERR_HANDLE_INVALID;
//          break;
//      }

        //��ֹ����ǰ�����I2Cʱ���������������Ƿ��ѳ�ʼ�����Է�ֹ�������ѳ�ʼ����I2C�������Ӱ��
        if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
        {
            if ((g_i2c_device_init[I2C_PERIPHERAL_SE0] == TRUE) &&
                (g_i2c_device_init[I2C_PERIPHERAL_SE1] == FALSE))
            {
                ret_code = port_i2c_deinit(p_comm_param->i2c_handle);
            }
#if defined(HED_I2C_SE0)
            PORT_I2C_SE0_RST_LOW();   //�͵�ƽ
#endif
        }

        else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
        {
            if ((g_i2c_device_init[I2C_PERIPHERAL_SE1] == TRUE) &&
                (g_i2c_device_init[I2C_PERIPHERAL_SE0] == FALSE))
            {
                ret_code = port_i2c_deinit(p_comm_param->i2c_handle);
            }
#if defined(HED_I2C_SE1)
            PORT_I2C_SE1_RST_LOW();   //�͵�ƽ
#endif
        }

        g_i2c_device_init[p_comm_param->slave_id] = FALSE;

    }
    while (0);

    return ret_code;
}


/*********************************************************************************
Function:      port_i2c_periph_power_on
Description:   �����ϵ磬Demo�����ϵ���Ʋ���
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_power_on(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    return SE_SUCCESS;
}

/*********************************************************************************
Function:       port_i2c_periph_power_off
Description:   �����µ磬Demo�����µ���Ʋ���
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_power_off(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    return SE_SUCCESS;
}


/*********************************************************************************
Function:       port_i2c_periph_lock
Description:   �Դ��豸periph��������������ʱ���������������ش���״̬��
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_lock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code = SE_ERR_HANDLE_INVALID;
//          break;
//      }

        if (p_comm_param->locked == TRUE)
        {
            ret_code = SE_ERR_BUSY;
            break;
        }
        p_comm_param->locked = TRUE;

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_unlock
Description:   �Դ��豸periph����
Input:          periph �豸���
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t port_i2c_periph_unlock(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph)
{
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    //if(p_comm_param->locked==FALSE)
    //{
    //  return PORT_I2C_ERR_LOCK;
    //}
    p_comm_param->locked = FALSE;

    return SE_SUCCESS;
}


/*********************************************************************************
Function:       port_i2c_periph_transmit
Description:   ͨ��I2C�ӿ�����豸periph���Ͷ��ֽ�����
               1.����mcu hal���HAL_I2C_Master_Transmit�������Ͷ��ֽ�����
Input:       periph �豸���
             inbuf ���������ݵ���ʼ��ַ
             inbuf_len ���������ݵĳ���
Output:      no
Return:      ��������״̬��
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_transmit(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *inbuf,
                                    uint32_t  inbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code = SE_ERR_HANDLE_INVALID;
//          break;
//      }

        if ((inbuf == NULL) || (inbuf_len == 0U))
        {
            ret_code =  SE_ERR_PARAM_INVALID;
            break;
        }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code =  SE_ERR_COMM;
            LOGE("Failed:i2c has no init!,  ErrCode-%08X.", ret_code);
            break;
        }

        //@yuyin
        uint16_t retry_cnt = 0;
        while (retry_cnt < 100)
        {
            I2C_Status res = I2C_MasterWrite(ALIPAY_I2Cx, inbuf, inbuf_len);
            if (res != I2C_Success)
            {
                DBG_DIRECT("[ALIPAY] ERROR!  I2C_MasterWrite: %d, %s, retry_cnt = %d", res, __func__, retry_cnt);
                if (res == I2C_ABRT_7B_ADDR_NOACK)
                {
                    retry_cnt++;
                    platform_delay_ms(1);
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
//      ret_code = HAL_I2C_Master_Transmit(p_comm_param->i2c_handle, (uint16_t)p_comm_param->slave_addr, inbuf, inbuf_len, PORT_I2C_HAL_TIMEOUT);
//      if(ret_code == HAL_BUSY)
//      {
//          ret_code = SE_ERR_BUSY;
//          break;
//      }
//      else if(ret_code == HAL_TIMEOUT)
//      {
//          ret_code = SE_ERR_TIMEOUT;
//          break;
//      }
//      else if(ret_code != HAL_OK)
//      {
//          ret_code = SE_ERR_COMM;
//          break;
//      }

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_receive
Description:   ͨ��I2C �ӿڴӴ��豸periph���ն��ֽ�����
             1.����mcu hal���HAL_I2C_Master_Receive�������ն��ֽ�����
Input:       periph �豸���
             outbuf ���������ݵ���ʼ��ַ
             outbuf_len ���������ݵĳ���
Output:      no
Return:      ��������״̬��
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_receive(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint8_t *outbuf,
                                   uint32_t *outbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code =  SE_ERR_HANDLE_INVALID;
//          LOGE("Failed:i2c handle invalid!,  ErrCode-%08X.", ret_code);
//          break;
//      }

        if ((outbuf == NULL) || (outbuf_len == NULL))
        {
            ret_code =  SE_ERR_PARAM_INVALID;
            LOGE("Failed:i2c param invalid!,  ErrCode-%08X.", ret_code);
            break;
        }

        if (g_i2c_device_init[p_comm_param->slave_id] == FALSE)
        {
            ret_code =  SE_ERR_COMM;
            LOGE("Failed:i2c has no init!,  ErrCode-%08X.", ret_code);
            break;
        }

        //@yuyin
        uint16_t retry_cnt = 0;
        while (retry_cnt < 100)
        {
            I2C_Status res = I2C_MasterRead(I2C2, outbuf, (uint16_t) * outbuf_len);
            if (res != I2C_Success)
            {
                DBG_DIRECT("[Alipay] ERROR!  I2C_MasterRead: %d,%s, retry_cnt = %d", res, __func__, retry_cnt);
                if (res == I2C_ABRT_7B_ADDR_NOACK)
                {
                    retry_cnt++;
                    platform_delay_ms(1);
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
//      ret_code = HAL_I2C_Master_Receive(p_comm_param->i2c_handle, (uint16_t)p_comm_param->slave_addr, outbuf, (uint16_t)*outbuf_len, PORT_I2C_HAL_TIMEOUT);
//      if(ret_code == HAL_BUSY)
//      {
//          ret_code = SE_ERR_BUSY;
//          break;
//      }
//      else if(ret_code == HAL_TIMEOUT)
//      {
//          ret_code = SE_ERR_TIMEOUT;
//          break;
//      }
//      else if(ret_code != HAL_OK)
//      {
//          ret_code = SE_ERR_COMM;
//          break;
//      }

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       port_i2c_periph_control
Description:   ���ݿ�������������ݣ��Դ��豸periph���п��Ʋ���
Input:         periph �豸���
             ctrlcode ������
             inbuf ���Ϳ������ݵ���ʼ��ַ
             inbuf_len ���Ϳ������ݵĳ���
Output:      no
Return:      ��������״̬��
Others:      no
*********************************************************************************/
se_error_t port_i2c_periph_control(HAL_I2C_PERIPHERAL_STRUCT_POINTER periph, uint32_t ctrlcode,
                                   uint8_t *inbuf, uint32_t  inbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    if (periph == NULL)
    {
        ret_code = SE_ERR_HANDLE_INVALID;
        return ret_code;
    }
    i2c_comm_param_pointer p_comm_param = (i2c_comm_param_pointer)periph->extra;

    do
    {
//      if(periph == NULL)
//      {
//          ret_code = SE_ERR_HANDLE_INVALID;
//          break;
//      }

        if (ctrlcode == PORT_I2C_CTRL_RST)
        {
            if (p_comm_param->slave_id == I2C_PERIPHERAL_SE0)
            {
#if defined(HED_I2C_SE0)
                PORT_I2C_SE0_RST_LOW();
                hal_delay(PORT_I2C_SE_RST_LOW_DELAY);  //��λʱ��RST�͵�ƽ����ʱ��
                PORT_I2C_SE0_RST_HIGH();
                hal_delay(PORT_I2C_SE_RST_HIGH_DELAY);  //��λ��RST�ߵ�ƽ����ʱ��
#endif
            }
            else if (p_comm_param->slave_id == I2C_PERIPHERAL_SE1)
            {
#if defined(HED_I2C_SE1)
                PORT_I2C_SE1_RST_LOW();
                hal_delay(PORT_I2C_SE_RST_LOW_DELAY);  //��λʱ��RST�͵�ƽ����ʱ��
                PORT_I2C_SE1_RST_HIGH();
                hal_delay(PORT_I2C_SE_RST_HIGH_DELAY);  //��λ��RST�ߵ�ƽ����ʱ��
#endif
            }
        }
    }
    while (0);
    return ret_code;
}

#if defined(HED_I2C_SE0)
I2C_PERIPHERAL_DEFINE_BEGIN(I2C_PERIPHERAL_SE0)
port_i2c_periph_init,
port_i2c_periph_deinit,
port_i2c_periph_power_on,
port_i2c_periph_power_off,
port_i2c_periph_lock,
port_i2c_periph_unlock,
port_i2c_periph_transmit,
port_i2c_periph_receive,
port_i2c_periph_control,
&i2c_comm_parm_slave0,
I2C_PERIPHERAL_DEFINE_END()

I2C_PERIPHERAL_REGISTER(I2C_PERIPHERAL_SE0);
#endif

#if defined(HED_I2C_SE1)
I2C_PERIPHERAL_DEFINE_BEGIN(I2C_PERIPHERAL_SE1)
port_i2c_periph_init,
port_i2c_periph_deinit,
port_i2c_periph_power_on,
port_i2c_periph_power_off,
port_i2c_periph_lock,
port_i2c_periph_unlock,
port_i2c_periph_transmit,
port_i2c_periph_receive,
port_i2c_periph_control,
&i2c_comm_parm_slave1,
I2C_PERIPHERAL_DEFINE_END()

I2C_PERIPHERAL_REGISTER(I2C_PERIPHERAL_SE1);
#endif


#endif //#ifdef HED_I2C

#endif //CONFIG_ALIPAY
