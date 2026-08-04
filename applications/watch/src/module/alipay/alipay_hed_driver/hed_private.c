/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:       private.c
 Author:          sunll
 Version:         V1.0
 Date:            2023-12-15
 Description:     comm interface definition
 History:

******************************************************************************/

#include "rtl876x_i2c.h"
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "stdio.h"

#include "hed_private.h"

#include "port_driver_i2c.h"

#include <stdint.h>
#include <string.h>

#include "trace.h"

#if CONFIG_ALIPAY

#define COMMAND_LIST_NUM 60
#define VAR 0xFF

uint8_t g_buf[2600]; //���˿���buf

double_queue_node g_queue_in ;
double_queue_node g_queue_out ;

uint8_t *IIC_Master_Init(void)
{
    return g_buf;
}
void HED_IIC_Init(void)
{
    g_queue_in.q_buf = IIC_Master_Init();
    g_queue_out.q_buf = g_queue_in.q_buf + DEQUE_MAX_SIZE ;
}
/**************************************************************************
* Variable Declaration
***************************************************************************/
static peripheral_bus_driver g_proto_i2c =
{
    //BUS_I2C,
    PERIPHERAL_I2C,
    {NULL},
    proto_i2c_init,
    proto_i2c_deinit,
    proto_i2c_open,
    proto_i2c_close,
    proto_i2c_transceive,
    proto_i2c_reset,
    proto_i2c_control,
    NULL
};


PERIPHERAL_BUS_DRIVER_REGISTER(PERIPHERAL_I2C, g_proto_i2c);

/**************************************************************************
* Global Variable Declaration
***************************************************************************/
static peripheral_bus_driver *g_drivers[MAX_PERIPHERAL_BUS_DRIVER] = {0};

struct driver_holder
{
    peripheral_bus_driver *driver;
    peripheral *periph;
};

#ifdef __MULTITASK
// �����񻷾�
// ÿ�������TLS����һ����ѡ��peripheral_driver����
#ifdef __FREERTOS
#endif

#ifdef __LINUX
#endif
#else
// �����񻷾�
// ͨ��ȫ�ֱ���������ѡ�е�һ��peripheral_driver����
static struct driver_holder g_selected_driver = {NULL, NULL};

#define setSelectedDriver(driver) g_selected_driver.driver = driver
#define setSelectedPeriph(periph) g_selected_driver.periph = periph

#define getSelectedDriver() g_selected_driver.driver
#define getSelectedPeriph() g_selected_driver.periph
#endif

#define checkSelectedDriverAndDevice() \
    do {    \
        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)   \
            return NULL;    \
    }while(0)


/*************************************************
Function:     api_select
Description:  SEѡ��Ӧ��
Input:        periph_type       ��������
              periph_id         SE�豸ID��
Output:       no
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_select(peripheral_type periph_type, uint32_t periph_id)
{
    se_error_t ret = 0;

    //�����ж�
    if (periph_type != PERIPHERAL_I2C)
    {
        LOGE("failed to api_select input params!\n");
        return SE_ERR_PARAM_INVALID;
    }
    //����acl��acl_select�ӿ�
    ret = acl_select(periph_type, periph_id);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call acl_select!\n");
        return ret;
    }
    return SE_SUCCESS;

}

/*************************************************
Function:     api_connect
Description:  SE����Ӧ��
Input:        out_buf         ATR����
              out_buf_len     ATR���ݳ���
Output:       no
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_connect(uint8_t *out_buf, uint32_t *out_buf_len)
{
    se_error_t ret = 0;
    //�����ж�
    if (out_buf == NULL || out_buf_len == NULL)
    {
        LOGE("failed to api_connect output params!");
        return SE_ERR_PARAM_INVALID;
    }
    //����acl��acl_connect�ӿ�
    ret = acl_connect(out_buf, out_buf_len);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call acl_connect, %d!\n", ret);
        return ret;
    };

    return SE_SUCCESS;
}
/*************************************************
Function:     api_disconnect
Description:  SE�Ͽ�����Ӧ��
Input:        no
Output:       no
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_disconnect(void)
{
    se_error_t ret = 0;

    //����acl��acl_disconnect�ӿ�
    ret = acl_disconnect();
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call acl_disconnect!\n");
        return ret;
    };

    return SE_SUCCESS;

}
/*************************************************
Function:     api_transceive
Description:  SE�����Ӧ��
Input:        in_buf      APDU������
              in_buf_len  APDU����������
Output:       out_buf     APDU��Ӧ
              out_buf_len APDU��Ӧ���ݳ���
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
                          uint32_t *out_buf_len)
{
    se_error_t ret = 0;

    //�����ж�
    if (in_buf == NULL || in_buf_len == 0)
    {
        LOGE("failed to api_transceive input params!");
        return SE_ERR_PARAM_INVALID;
    }
    if (out_buf == NULL || out_buf_len == NULL)
    {
        LOGE("failed to api_transceive output params!");
        return SE_ERR_PARAM_INVALID;
    }
    //����acl��acl_transceive�ӿ�
    ret = acl_transceive(in_buf, in_buf_len, out_buf, out_buf_len);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call acl_transceive!\n");
        return ret;
    };

    return SE_SUCCESS;
}

/*************************************************
Function:     api_reset
Description:  SE��λӦ��
Input:        out_buf         ATR����
              out_buf_len     ATR���ݳ���
Output:       no
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_reset(uint8_t *out_buf, uint32_t *out_buf_len)
{
    se_error_t ret = 0;
    //�����ж�
    if (out_buf == NULL || out_buf_len == NULL)
    {
        LOGE("failed to api_reset output params!");
        return SE_ERR_PARAM_INVALID;
    }
    //����acl��acl_reset�ӿ�
    ret = acl_reset(out_buf, out_buf_len);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call acl_reset!\n");
        return ret;
    };

    return SE_SUCCESS;

}

//*************************************************
//info.c
//*************************************************
/*************************************************
Function:     api_sdk_version_number
Description:  ��ȡ����sdk�汾��
Input:        no
Output:       num ���Ͱ汾��
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t  api_sdk_version_number(uint32_t *num)
{

    //�������
    if (num == NULL)
    {
        LOGE("failed to api_sdk_version_number pointer params!\n");
        return SE_ERR_PARAM_INVALID;
    }
    *num = SDK_VERSION_NUM;

    return SE_SUCCESS;
}

/*************************************************
Function:     api_sdk_version_string
Description:  ��ȡ�ַ����汾��
Input:        no
Output:       str �ַ����汾��
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t  api_sdk_version_string(char *str)
{

    //�������
    if (str == NULL)
    {
        LOGE("failed to api_sdk_version_string pointer params!\n");
        return SE_ERR_PARAM_INVALID;
    }

    strcpy(str, SDK_VERSION_STRING);

    return SE_SUCCESS;
}
//*************************************************
//ctrl.c
//*************************************************
/*************************************************
Function:     api_switch_mode
Description:  �л�����ģʽӦ��
Input:        type         ����ģʽ
Output:       no
Return:       �μ�error.h
Others:       no
*************************************************/
//se_error_t api_switch_mode (work_mode  type)
//{
//  se_error_t ret = 0;
//  //�������
//  if(type!=STANDBY&&type!=POWERDOWN)
//  {
//      LOGE("failed to api_switch_mode input params!\n");
//      return SE_ERR_PARAM_INVALID;
//  }
//  //����cmd��apdu_switch_mode�ӿ�
//  ret = apdu_switch_mode (type);
//  if(ret!=SE_SUCCESS)
//  {
//      LOGE("failed to call apdu_switch_mode!\n");
//      return ret;
//  }

//  return SE_SUCCESS;

//}
/*************************************************
Function:     api_control
Description:  �������Ӧ��
Input:        ctrlcode        ��������
              in_buf       ��������
                  in_buf_len   �������ݳ���
Output:       out_buf      �������
              out_buf_len  ������ݳ���
Return:       �μ�error.h
Others:       no
*************************************************/
se_error_t api_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len,
                       uint8_t *out_buf, uint32_t *out_buf_len)
{
    se_error_t ret = 0;
    //�������
    if (in_buf == NULL || in_buf_len == 0)
    {
        LOGE("failed to api_control input params!\n");
        return SE_ERR_PARAM_INVALID;
    }
    if (out_buf == NULL || out_buf_len == NULL)
    {
        LOGE("failed to api_control output params!\n");
        return SE_ERR_PARAM_INVALID;
    }
    //����cmd��apdu_control�ӿ�
    ret = apdu_control(ctrlcode, in_buf, in_buf_len, out_buf, out_buf_len);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call apdu_control!\n");
        return ret;
    }

    return SE_SUCCESS;
}
//*************************************************
//tudu.c
//*************************************************
/*************************************************
    Function:    tpdu_init
    Description: ��������ͷ�Ӻ���
    Input:       isoCase  APDUЭ������
                 cla      CLA
    ins          INS
    p1           P1
    p2           P2
    lc           Lc
    le           LE
    Output:      command ����ṹ��
    Return:      �μ�������񷵻���
    Others:      no
*************************************************/
iso_command_apdu_t *tpdu_init(iso_command_apdu_t *command, int32_t isoCase, int32_t cla,
                              int32_t ins, int32_t p1, int32_t p2, int32_t lc, int32_t le)
{
#ifdef _ASSERT_DEBUG
    ASSERT(command != NULL);
#endif

    //��ֵAPDU������
    if (isoCase >= 0)
    {
        command->isoCase = isoCase;
    }
    if (cla >= 0)
    {
        command->classbyte = cla;
    }
    if (ins >= 0)
    {
        command->instruction = ins;
    }
    if (p1  >= 0)
    {
        command->p1 = p1;
    }
    if (p2  >= 0)
    {
        command->p2 = p2;
    }
    if (lc  >= 0)
    {
        command->lc = lc;
    }
    if (le  >  0)
    {
        command->le = le;
    }

    return command;
}

/*************************************************
    Function:     tpdu_init_with_id
    Description:  ��������ͷ��ʼ���������ͷ
    Input:        commandID ����ID��
    Output:       command ����ṹ��
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
iso_command_apdu_t *tpdu_init_with_id(iso_command_apdu_t *command, uint8_t commandID)
{
    typedef struct
    {
        uint8_t  isoCase;
        uint8_t  classbyte;
        uint8_t  instruction;
        uint8_t  p1;
        uint8_t  p2;
    } command_info_t;
    command_info_t *pc = NULL;
    //�����ʼ���������������������
    command_info_t command_list[COMMAND_LIST_NUM] =
    {
        {ISO_CASE_3, 0x80, 0x50, 0x00, 0x00},   //CMD_SCP_INIT_UPDATE
        {ISO_CASE_3, 0x84, 0x82, VAR, 0x00},    //CMD_SCP_AUTH
        {ISO_CASE_3, VAR, 0xD8, 0x00, 0x00},    //CMD_PUT_KEY
        {ISO_CASE_2, 0x00, 0x84, 0x00, 0x00},   //CMD_GET_RANDOM
        {ISO_CASE_3, 0x00, 0xA4, 0x00, 0x00},   //CMD_SELECT_FILE
        {ISO_CASE_3, VAR, 0xD6, VAR, VAR},      //CMD_WRITE_FILE
        {ISO_CASE_3, VAR, 0xB0, VAR, VAR},      //CMD_READ_FILE
        {ISO_CASE_2, 0x80, 0xC8, VAR, 0x00},    //CMD_GET_INFO
        {ISO_CASE_3, 0x80, 0xE1, 0x00, VAR},    //CMD_SWITCH_MODE
        {ISO_CASE_3, 0x80, 0xCE, VAR, 0x00},    //CMD_CLEAR_FILE
    };
#ifdef _ASSERT_DEBUG
    ASSERT(command != NULL);
    ASSERT(commandID < sizeof(command_list));
#endif
    pc = &command_list[commandID];
    //����command IDָ�ֵ��������
    tpdu_init(command,
              pc->isoCase,
              pc->classbyte,
              pc->instruction,
              pc->p1,
              pc->p2,
              -1,
              -1
             );

    return command;
}
/*************************************************
    Function:     tpdu_set_cla
    Description:  �޸�CLA
    Input:        cla      CLA
    Output:       command ����ṹ��
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
iso_command_apdu_t *tpdu_set_cla(iso_command_apdu_t *command, uint32_t cla)
{
#ifdef _ASSERT_DEBUG
    ASSERT(command != NULL);
#endif
    //����CLA����
    tpdu_init(command, -1, cla, -1, -1, -1,  -1, -1);
    return command;

}
/*************************************************
    Function:     tpdu_set_p1p2
    Description:  �޸�P1P2
    Input:        p1       P1
                  p2       P2
    Output:       command ����ṹ��
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
iso_command_apdu_t *tpdu_set_p1p2(iso_command_apdu_t *command, uint32_t p1, uint32_t p2)
{
#ifdef _ASSERT_DEBUG
    ASSERT(command != NULL);
#endif
    //����P1P2
    tpdu_init(command, -1, -1, -1, p1, p2, -1, -1);
    return command;
}
/*************************************************
    Function:     tpdu_set_le
    Description:  �޸�Le
    Input:        le       LE
    Output:       command ����ṹ��
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
iso_command_apdu_t *tpdu_set_le(iso_command_apdu_t *command, uint32_t le)
{
#ifdef _ASSERT_DEBUG
    ASSERT(command != NULL);
#endif
    //����Le
    tpdu_init(command, -1, -1, -1, -1, -1, -1, le);
    return command;
}

/*************************************************
    Function:     tpdu_pack
    Description:  �Ӻ�������APDU����ṹ�����õ�˫�˶�����
    Input:        command����ṹ��
    Output:       output  �������
                  output_len  ������ݳ���
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
se_error_t tpdu_pack(iso_command_apdu_t *command, uint8_t *output, uint32_t *output_len)
{
    //output Ϊ˫�˶���
    uint8_t front_buffer[5] = {0};
    uint8_t rear_buffer[2] = {0};
    double_queue queue_out = (double_queue)output;

    //�����ж�
    if (command == NULL || output == NULL || output_len == NULL)
    {
        LOGE("failed to tpdu_pack input params!");
        return SE_ERR_PARAM_INVALID;
    }


    //���������������������˫�����
    front_buffer[0] = command->classbyte;
    front_buffer[1] = command->instruction;
    front_buffer[2] = command->p1;
    front_buffer[3] = command->p2;
    //����APDUЭ������ƴװ4������
    //ISO_CASE_1 = 0x01,  // CLA INS P1 P2 00
    //ISO_CASE_2 = 0x02,  // CLA INS P1 P2 Le
    //ISO_CASE_3 = 0x03,  // CLA INS P1 P2 Lc Data
    //ISO_CASE_4 = 0x04   // CLA INS P1 P2 Lc Data Le

    switch (command->isoCase)
    {
    case ISO_CASE_1:
        front_buffer[4] = 0x00;
        queue_front_push(front_buffer, 5, queue_out);
        *output_len = 5;
        break;

    case ISO_CASE_2:
        front_buffer[4] = (command->le) & 0xFF;
        *output_len = 5;
        queue_front_push(front_buffer, 5, queue_out);
        break;

    case ISO_CASE_3:
        front_buffer[4] = queue_size(queue_out) & 0xFF;
        queue_front_push(front_buffer, 5, queue_out);
        *output_len = queue_size(queue_out);
        break;

    case ISO_CASE_4:
        front_buffer[4] = queue_size(queue_out) & 0xFF;
        *output_len = 5 + queue_size(queue_out) + 1;
        queue_front_push(front_buffer, 5, queue_out);
        rear_buffer[1] = (command->le) & 0xFF;
        queue_rear_push(rear_buffer, 1, queue_out);
        break;
    }
    output = (uint8_t *)queue_out;

    return SE_SUCCESS;

}
/*************************************************
    Function:     tpdu_unpack
    Description:  �Ӻ��������״̬�ֺ�����
    Input:         no
    Output:        response-> queue_out   ���˫�˶���
                   response-> status_word ״̬��
    Return:        �μ�������񷵻���
    Others:        no
*************************************************/
se_error_t tpdu_unpack(uint8_t *output, uint32_t *output_len)
{
    double_queue queue_out = (double_queue)output;
    //�����ж�
    if (output == NULL || output_len == NULL)
    {
        LOGE("failed to tpdu_unpack output params!");
        return SE_ERR_PARAM_INVALID;
    }
    //ȥ��״̬��
    queue_rear_pop(2, queue_out);
    output = (uint8_t *)queue_out;
    *output_len = queue_out->q_buf_len;

    return SE_SUCCESS;
}

/*************************************************
    Function:      tpdu_send
    Description:   apdu����ͽ���
    Input:         in_buf      ��������
                   in_buf_len  �������ݳ���
    Output:        output  �������
    output_len     ������ݳ���
    Return:        �μ�������񷵻���
    Others:        no
*************************************************/
se_error_t tpdu_send(uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
{

    double_queue queue_in = (double_queue)input;
    double_queue queue_out = (double_queue)output;
    uint8_t temp_buffer[5] = {0x00};
    se_error_t ret = 0;
    uint32_t temp_len = 0;

    //�����ж�
    if (input == NULL || input_len == 0)
    {
        LOGE("failed to tpdu_send input params!");
        return SE_ERR_PARAM_INVALID;
    }
    if (output == NULL || output_len == NULL)
    {
        LOGE("failed to tpdu_send output params!");
        return SE_ERR_PARAM_INVALID;
    }


    //���������61XX��6CXX���
    if (queue_in->q_buf_len == 5)
    {
        memcpy(temp_buffer, queue_in->q_buf + queue_in->front_node, 5);
    }

    while (1)
    {
        ret = acl_transceive_queue((uint8_t *)queue_in, queue_in->q_buf_len, (uint8_t *)queue_out,
                                   &temp_len);
        //����61XX��6CXX���
        if ((temp_len == 2) && (queue_out->q_buf[queue_out->rear_node - 2] == 0x61))
        {
            memcpy(temp_buffer, "\x00\xC0\x00\x00", 4);
            temp_buffer[4] = queue_out->q_buf[queue_out->rear_node - 1];
            queue_init(queue_in);
            queue_init(queue_out);
            queue_rear_push(temp_buffer, 5, queue_in);
            continue;
        }
        if ((temp_len == 2) && (queue_out->q_buf[queue_out->rear_node - 2] == 0x6C))
        {
            temp_buffer[4] = queue_out->q_buf[queue_out->rear_node - 1];
            queue_init(queue_in);
            queue_init(queue_out);
            queue_rear_push(temp_buffer, 5, queue_in);
            continue;
        }
        break;
    }
    output = (uint8_t *)queue_out;
    *output_len = queue_out->q_buf_len;
    if (ret != SE_SUCCESS)
    {
        return ret;
    }


    return SE_SUCCESS;
}
/*************************************************
    Function:      tpdu_execute
    Description:   apdu����������ͽ���
    Input:         command     command����ṹ��
                   in_buf      ��������
                   in_buf_len  �������ݳ���
    Output:        output      �������
                   output_len  ������ݳ���
    Return:        �μ�������񷵻���
    Others:        no
*************************************************/
//se_error_t tpdu_execute   (iso_command_apdu_t *command, uint8_t *input, uint32_t input_len, uint8_t *output, uint32_t *output_len)
//{

//  se_error_t ret = 0;
//  uint16_t status_word = 0;
//  double_queue queue_in=(double_queue)input;
//  double_queue queue_out=(double_queue)output;
//  uint32_t in_len = 0;
//  uint32_t out_len = 0;
//  uint32_t off = 0;
//  uint32_t change_status = 0;

//  //�����ж�
//  if(command==NULL||input==NULL)
//  {
//      LOGE("failed to tpdu_execute input params!");
//      return SE_ERR_PARAM_INVALID;
//  }
//  if(output==NULL||output_len==NULL)
//  {
//      LOGE("failed to tpdu_execute output params!");
//      return SE_ERR_PARAM_INVALID;
//  }
//  //��APDU�������˫�����
//  ret = tpdu_pack(command,(uint8_t *) queue_in,&in_len);
//  if(ret!=SE_SUCCESS)
//      return ret;
//  //���ͽ���APDU����
//  ret = tpdu_send((uint8_t *) queue_in,queue_in->q_buf_len,(uint8_t *)queue_out,&out_len);
//  //���жϷ����Ƿ���ȷ
//  if(ret!=SE_SUCCESS)
//  {
//      output = (uint8_t *)queue_out;
//    *output_len = out_len;
//      return ret;
//  }
//  //��״̬��ȡ��
//  off = queue_out->rear_node;
//  status_word=((queue_out->q_buf[off-2]<< 8) & 0xFF00) | (queue_out->q_buf[off-1] & 0xFF);
//  //6310��Ҫ�������Ƚ����ݷ���
//  if(status_word==0x6310)
//  {
//      change_status = tpdu_change_error_code(status_word);
//      ret = tpdu_unpack((uint8_t *)queue_out,&out_len);
//      output = (uint8_t *)queue_out;
//      *output_len = out_len;
//      return change_status;
//  }
//  //�������أ�ֱ�ӷ��ش����룬��ȷ��������ȥ״̬��
//  if(status_word!=0x9000)
//      return tpdu_change_error_code(status_word);
//
//  //ͨ���ҷ��ض���������״̬�ִ�������ȥ������������Ч����
//  ret = tpdu_unpack((uint8_t *)queue_out,&out_len);
//  if(ret!=SE_SUCCESS)
//      return ret;
//  output = (uint8_t *)queue_out;
//  *output_len = out_len;

//  return SE_SUCCESS;

//}
/*************************************************
    Function:      tpdu_execute_no_response
    Description:   apdu����������ͽ��գ��޷�������
    Input:         command     command����ṹ��
                   in_buf      ��������
                   in_buf_len  �������ݳ���
    Output:        no
    Return:        �μ�������񷵻���
    Others:        no
*************************************************/
se_error_t tpdu_execute_no_response(iso_command_apdu_t *command, uint8_t *input, uint32_t input_len)
{

    se_error_t ret = 0;
    uint16_t status_word = 0;
    double_queue queue_in = (double_queue)input;
    //double_queue_node queue_out={0};
    uint32_t in_len = 0;
    uint32_t out_len = 0;
    uint32_t off = 0;

    //�����ж�
    if (command == NULL || input == NULL)
    {
        LOGE("failed to tpdu_execute_no_response input params!");
        return SE_ERR_PARAM_INVALID;
    }
    //��APDU�������˫�����
    queue_init(&g_queue_out);
    ret = tpdu_pack(command, (uint8_t *) queue_in, &in_len);
    if (ret != SE_SUCCESS)
    {
        return ret;
    }
    //����APDU����޷���
    ret = tpdu_send((uint8_t *) queue_in, queue_in->q_buf_len, (uint8_t *)&g_queue_out, &out_len);
    off = g_queue_out.front_node;
    status_word = ((g_queue_out.q_buf[off] << 8) & 0xFF00) | (g_queue_out.q_buf[off + 1] & 0xFF);
    //�жϷ����Ƿ���ȷ
    if (ret != SE_SUCCESS)
    {
        return ret;
    }
    //�ж�״̬���Ƿ���ȷ
    if (status_word != 0x9000)
    {
        return tpdu_change_error_code(status_word);
    }

    return SE_SUCCESS;

}


/*************************************************
    Function:     tpdu_change_error_code
    Description:  ״̬��ת��
    Input:        status_word Դ״̬��
    Output:       no
    Return:       �μ�������񷵻���
    Others:       no
*************************************************/
se_error_t tpdu_change_error_code(uint16_t status_word)
{
    se_error_t error = (se_error_t)(status_word & (0x0000FFFF));
    //״̬��װ�����μ�error.h
    if (status_word == 0x9000)
    {
        return SE_SUCCESS;
    }
    else
    {
        return (error | (0x10000000));
    }

}
//*************************************************
//apdu.c
//*************************************************
/*************************************************
    Function:     apdu_switch_mode
    Description:  �л�����ģʽ�����װ
    Input:        type         ����ģʽ
    Output:       no
    Return:       �μ�error.h
    Others:       no
*************************************************/
se_error_t apdu_switch_mode(work_mode  type)
{
    iso_command_apdu_t command = {0};
    se_error_t ret = 0;
    //double_queue_node queue_in ={0} ;
    uint32_t p2 = 0;
    if (type != STANDBY && type != POWERDOWN)
    {
        LOGE("failed to apdu_switch_mode input params!\n");
        return SE_ERR_PARAM_INVALID;
    }

    //˫�˶��г�ʼ��
    queue_init(&g_queue_in);
    //��������ͷ
    tpdu_init_with_id(&command, CMD_SWITCH_MODE);
    //����P1P2
    p2 = (type == STANDBY) ? 0x00 : 0x02;
    tpdu_set_p1p2(&command, 0x00, p2);

    ret = tpdu_execute_no_response(&command, (uint8_t *)&g_queue_in, queue_size(&g_queue_in));
    if (ret != SE_SUCCESS)
    {
        return ret;
    }


    return SE_SUCCESS;
}

/*************************************************
    Function:     apdu_control
    Description:  ������������װ
    Input:        ctrlcode     ��������
                  in_buf       ��������
                  in_buf_len   �������ݳ���
    Output:       out_buf      �������
                  out_buf_len  ������ݳ���
    Return:       �μ�error.h
    Others:       no
*************************************************/
se_error_t apdu_control(ctrl_type ctrlcode, const uint8_t *in_buf, uint32_t in_buf_len,
                        uint8_t *out_buf, uint32_t *out_buf_len)
{
    return SE_SUCCESS;
}



//*************************************************
//acl.c
//*************************************************
/*********************************************************************************
Function:       add_periph
Description:    �����豸���
            1. �����豸�����g_drivers�У��������豸���
Input:      driver ���������������
                periph ������
Output:         no
Return:         ������
Others:         no
*********************************************************************************/
static peripheral *add_periph(peripheral_bus_driver *driver, peripheral *periph)
{
    int i = 0;
    //for (; i<sizeof(driver->periph); i++) {
    for (; i < MAX_PERIPHERAL_DEVICE; i++)
    {
        if (driver->periph[i] == periph)
        {
            return periph;
        }
        else if (driver->periph[i] == NULL)
        {
            driver->periph[i] = periph;
            return periph;
        }
    }

    return NULL;
}

/*********************************************************************************
Function:       add_driver
Description:    ����������������������豸���
              1. �����������������g_drivers�У��������������
              2. ����add_periph ���������豸���
Input:      driver ���������������
                periph ������
Output:         no
Return:         ���������������
Others:         no
*********************************************************************************/
static peripheral_bus_driver *add_driver(peripheral_bus_driver *driver, peripheral *periph)
{
    int i = 0;
    //for (; i<sizeof(g_drivers); i++) {
    for (; i < MAX_PERIPHERAL_BUS_DRIVER; i++)
    {
        if (g_drivers[i] == driver)
        {
            if (add_periph(driver, periph) != NULL)
            {
                return driver;
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            if (g_drivers[i] == NULL)
            {
                g_drivers[i] = driver;
                if (add_periph(driver, periph) != NULL)
                {
                    return driver;
                }
                else
                {
                    return NULL;
                }
            }
        }
    }

    return NULL;
}

/*********************************************************************************
Function:       find_driver
Description:    �����������ͣ���g_driver�в������������������
Input:      type ��������
Output:         no
Return:         ���������������
Others:         no
*********************************************************************************/
static peripheral_bus_driver *find_driver(peripheral_type type)
{
    int i = 0;
    //for (; i<sizeof(g_drivers); i++) {
    for (; i < MAX_PERIPHERAL_BUS_DRIVER; i++)
    {
        if (g_drivers[i]->type == type)
        {
            return g_drivers[i];
        }
    }

    return NULL;
}

/*********************************************************************************
Function:       find_slave_device
Description:    �������������������������IDֵ������������
Input:      driver ���������������
                        dev_id ����ID
Output:         no
Return:         ������
Others:         no
*********************************************************************************/
static peripheral *find_slave_device(peripheral_bus_driver *driver, uint32_t dev_id)
{
    int i = 0;
    // for (; i<sizeof(driver->periph); i++) {
    for (; i < MAX_PERIPHERAL_DEVICE; i++)
    {
        if (driver->periph[i]->id == dev_id)
        {
            return driver->periph[i];
        }
    }

    return NULL;
}



/*********************************************************************************
Function:       _acl_register
Description:    �������������������ͺ�����ID ֵ����ע�ᣬ������ʵ����
            1. ����add_driver ����������������������豸���
Input:      driver ���������������
            periph ������
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t _acl_register(peripheral_bus_driver *driver, peripheral *periph)
{
    if (driver == NULL || periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }
    if (add_driver(driver, periph) == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    return SE_SUCCESS;
}


/*********************************************************************************
Function:       acl_select
Description:    ����ע��Ķ�������У�ѡ����Ҫ����������
            1. ����add_driver ����������������������豸���
            2. ����ѯ������������������豸����洢��g_selected_driver
Input:      type  ��������
            dev_id ����IDֵ
Output:         no
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t acl_select(peripheral_type type, uint32_t dev_id)
{
    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
    {
        return SE_ERR_PARAM_INVALID; //δִ��acl_register ����
    }

    driver = find_driver(type);
    if (driver == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    periph = find_slave_device(driver, dev_id);
    if (periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    setSelectedDriver(driver);
    setSelectedPeriph(periph);

    return SE_SUCCESS;
}

/*********************************************************************************
Function:       acl_connect
Description:    �������輰��ȡ����ATR
            1. ������������ѡ���������������ֻע����һ�����裬
            ��δѡ������裬���ص�һ��ע����豸
            2. ���ݼ��ص���������������init ��ʼ��������open����
Input:      no
Output:     out_buf �������ʼ��ַ
            out_buf_len  ������ݳ��ȵ���ʼ��ַ
Return:         ��������״̬��
Others:         no
*********************************************************************************/
se_error_t acl_connect(uint8_t *out_buf, uint32_t *out_buf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    do
    {
        if ((out_buf == NULL) || (out_buf_len == NULL))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            DBG_DIRECT("line %d: out_buf = NULL\n", __LINE__);
            break;
        }

        //1. ������鼰�����������
        if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
        {
            DBG_DIRECT("line %d: g_drivers[0] = NULL\n", __LINE__);
            ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
            break;
        }

        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)
        {
            if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
            {
                ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
                break;
            }
            //ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
            driver = g_drivers[0];
            periph = g_drivers[0]->periph[0];
        }
        else
        {
            //ִ����acl_select ������ѡ����ָ�����豸
            driver = getSelectedDriver();
            periph = getSelectedPeriph();
        }

        DBG_DIRECT("line %d: driver = %p, periph = %p\n", __LINE__, driver, periph);
        //2.��ʼ���豸
        ret_code = driver->init(periph);
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

        DBG_DIRECT("line %d: driver->init success\n", __LINE__);
        //3.���豸
        ret_code = driver->open(periph, out_buf, out_buf_len);  //���豸,��ȡATR
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       acl_disconnect
Description:    �Ͽ������������
            1. ������������ѡ���������������ֻע����һ�����裬
            ��δѡ������裬���ص�һ��ע����豸
            2. ���ݼ��ص���������������deinit ��ֹ��������close����
Input:      no
Output:     no
Return:     ��������״̬��
Others:     no
*********************************************************************************/
se_error_t acl_disconnect(void)
{
    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    se_error_t ret_code = SE_SUCCESS;

    do
    {
        if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;//δִ��acl_register ����
            break;
        }

        //1.��������
        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)
        {
            //δִ��acl_select ����, ʹ�õ�1 ��ע����豸
            driver = g_drivers[0];
            periph = g_drivers[0]->periph[0];
        }
        else
        {
            //ִ����acl_select ������ѡ����ָ�����豸
            driver = getSelectedDriver();
            periph = getSelectedPeriph();
        }

        //2.��ֹ���豸
        ret_code = driver->deinit(periph);
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

        //3.�ر��豸
        ret_code = driver->close(periph);
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

    }
    while (0);

    return ret_code;
}

/******************************************************************
Function:       acl_transceive_queue
Description:    ��˫����и�ʽ������豸�������������Ӧ
                1.��������Ч��
                2.������������������豸���
                3.ͨ�����صľ��������proto��transceiver�������������ݼ�����������Ӧ
Input:     in_buf ����˫����е���ʼ��ַ
           in_buf_len ����˫����е��������ݳ���
Output:     out_buf ���˫����е���ʼ��ַ
           out_buf_len  ���˫����е�������ݳ��ȵ���ʼ��ַ
Return:        ��������״̬��
Others:        no
******************************************************************/
se_error_t acl_transceive_queue(uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
                                uint32_t *out_buf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    do
    {
        //������
        if ((in_buf == NULL) || (in_buf_len == 0U) || (out_buf == NULL) || (out_buf_len == NULL))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;//δִ��acl_register ����
            break;
        }

        //1.��������
        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)
        {
            //δִ��acl_select ����, ʹ�õ�1 ��ע����豸
            driver = g_drivers[0];
            periph = g_drivers[0]->periph[0];
        }
        else
        {
            //ִ����acl_select ������ѡ����ָ�����豸
            driver = getSelectedDriver();
            periph = getSelectedPeriph();
        }

        //---����proto�е�proto_spi_transceive��proto_i2c_transceive,
        //��ѡ����豸�������������������Ӧ����
        ret_code = driver->transceive(periph, in_buf, in_buf_len, out_buf, out_buf_len);
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

    }
    while (0);

    return ret_code;
}


/******************************************************************
Function:       acl_transceive
Description:    ���������ݴ洢��˫����У��������豸�������ݼ�������Ӧ������˫�����ȡ����Ӧ����
                1.����洢�浵������˫�����
                2.����acl_transceive_queue ��������˫����и�ʽ���ͺͽ�������
                3.�����˫�����ȡ����Ӧ����
Input:     in_buf �������ݵ���ʼ��ַ
           in_buf_len �������ݳ���
Output:    out_buf �������ʼ��ַ
           out_buf_len  ������ݳ��ȵ���ʼ��ַ
Return:    ��������״̬��
Others:    no
******************************************************************/
se_error_t acl_transceive(const uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
                          uint32_t *out_buf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    uint32_t out_len = 0;
    uint16_t front_node = 0;

    //double_queue_node queue_in ={0} ;
    //double_queue_node queue_out ={0} ;

    do
    {
        //������
        if ((in_buf == NULL) || (out_buf == NULL) || (out_buf_len == NULL))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        if ((in_buf_len < ACL_TRANS_DATA_LEN_MIN) || (in_buf_len > ACL_TRANS_DATA_LEN_MAX))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        //˫�˶��г�ʼ��
        queue_init(&g_queue_in);
        queue_init(&g_queue_out);

        //�������ݴ���˫�˶���
        queue_rear_push((uint8_t *)in_buf, in_buf_len, &g_queue_in);

        //������˫������е����ݰ�Э���ʽ���͸��豸��������Ӧ���ݴ洢�����˫�����
        ret_code = acl_transceive_queue((uint8_t *)&g_queue_in, queue_size(&g_queue_in),
                                        (uint8_t *)&g_queue_out, &out_len);
        if (ret_code != SE_SUCCESS)
        {
            return ret_code;
        }

        //��˫�˶��п������������
        front_node = g_queue_out.front_node;
        memcpy(out_buf, &g_queue_out.q_buf[front_node], g_queue_out.q_buf_len);
        *out_buf_len = g_queue_out.q_buf_len;

    }
    while (0);

    return ret_code;
}


/*********************************************************************************
Function:       acl_reset
Description:    ��λ���輰��ȡ����ATR
            1. ������������ѡ���������������ֻע����һ�����裬
            ��δѡ������裬���ص�һ��ע����豸
            2. ���ݼ��ص���������������reset��λ����
Input:      no
Output:     out_buf �������ʼ��ַ
            out_buf_len  ������ݳ��ȵ���ʼ��ַ
Return:     ��������״̬��
Others:     no
*********************************************************************************/
se_error_t acl_reset(uint8_t *out_buf, uint32_t *out_buf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    do
    {
        if ((out_buf == NULL) || (out_buf_len == NULL))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        //1. ������鼰�����������
        if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
        {
            ret_code = SE_ERR_PARAM_INVALID; //δִ��acl_register ����
            break;
        }

        if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)
        {
            if (g_drivers[1] != NULL || g_drivers[0]->periph[1] != NULL)
            {
                ret_code = SE_ERR_NO_SELECT; //ע���˶�����裬����ѡ������
                break;
            }
            //ֻע����һ������ʱ, ����ѡ��ֱ��ʹ�õ�1 ��ע�������
            driver = g_drivers[0];
            periph = g_drivers[0]->periph[0];
        }
        else
        {
            //ִ����acl_select ������ѡ����ָ��������
            driver = getSelectedDriver();
            periph = getSelectedPeriph();
        }


        //2.��λ����
        ret_code = driver->reset(periph, out_buf, out_buf_len);  //��λ����,��ȡATR
        if (ret_code != SE_SUCCESS)
        {
            break;
        }

    }
    while (0);

    return ret_code;
}

/*********************************************************************************
Function:       acl_control
Description:    ���Ϳ�������
            1. ������������ѡ���������������ֻע����һ�����裬
            ��δѡ������裬���ص�һ��ע����豸
            2. ���ݼ��ص���������������control���ƺ���
Input:      ctrlcode ���������
            in_buf �������ʼ��ַ
            in_buf_len  �������ݳ���
Output:     out_buf �������ʼ��ַ
            out_buf_len  ������ݳ��ȵ���ʼ��ַ
Return:     ��������״̬��
Others:     no
*********************************************************************************/
se_error_t acl_control(uint32_t ctrlcode, uint8_t *in_buf, uint32_t in_buf_len, uint8_t *out_buf,
                       uint32_t *out_buf_len)
{
    se_error_t retCode = SE_SUCCESS;

    peripheral_bus_driver *driver = NULL;
    peripheral *periph = NULL;

    if (g_drivers[0] == NULL || g_drivers[0]->periph[0] == NULL)
    {
        return SE_ERR_HANDLE_INVALID;  //δִ��acl_register ����
    }

    if (getSelectedDriver() == NULL || getSelectedDriver() == NULL)
    {
        //δִ��acl_select ����, ʹ�õ�1 ��ע����豸
        driver = g_drivers[0];
        periph = g_drivers[0]->periph[0];
    }
    else
    {
        //ִ����acl_select ������ѡ����ָ�����豸
        driver = getSelectedDriver();
        periph = getSelectedPeriph();
    }


    //��ѡ����豸���Ϳ�����������տ����������Ӧ����
    retCode = driver->control(periph, ctrlcode, in_buf, in_buf_len, out_buf, out_buf_len);

    return retCode;
}

//*************************************************
//util.c
//*************************************************
/*********************************************************************************
Function:       gpio_init
Description:  ����LED������ IO �ȹ��ܵ�GPIO ����
Input:           no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void gpio_init(void)
{
#if 0
    GPIO_InitTypeDef  GPIO_InitStruct = {0};

    //--- Enable  GPIO LED Clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure PA.8  LED */
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);

    /* Configure PA.6  debug*/
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET); //����ͣ���Դ��ͨ
#endif
}


/*********************************************************************************
Function:      mcu_init
Description:  mcu�ϵ��ʼ��
Input:           no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void mcu_init(void)
{
#if 0
    HAL_Init();

    /* Configure the system clock to get correspondent USB clock source */
    SystemClock_Config();

    /* Enable Power Clock*/
    __HAL_RCC_PWR_CLK_ENABLE();

    gpio_init();

#if HAL_UART_PRINTF_ENABLE
    hal_printf_init();
#endif
#endif
}



/*********************************************************************************
Function:      error_handler
Description:  �����쳣ʱ��ʹ��LED��˸��ʾ
Input:           no
Output:         no
Return:         no
Others:         no
*********************************************************************************/
void error_handler(void)
{
    while (1)
    {
        /* Error LED is slowly blinking (1 sec. period) */
        //HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_8);
        //HAL_Delay(1000);

    }
}



/************************************************************************************
Function:       SystemClock_Config
Description:    System Clock Configuration
*
*           HSI48 used as USB clock source (USE_USB_CLKSOURCE_CRSHSI48 defined in main.h)
*             - System Clock source            = HSI
*             - HSI Frequency(Hz)              = 48000000
*             - SYSCLK(Hz)                     = 48000000
*             - HCLK(Hz)                       = 48000000
*             - AHB Prescaler                  = 1
*             - APB1 Prescaler                 = 1
*             - APB2 Prescaler                 = 2
*             - Flash Latency(WS)              = 1
*             - Main regulator output voltage  = Scale1 mode
*
*           PLL(HSE) used as USB clock source (USE_USB_CLKSOURCE_PLL defined in main.h)
*             - System Clock source            = PLL (HSE)
*             - HSE Frequency(Hz)              = 8000000
*             - SYSCLK(Hz)                     = 48000000
*             - HCLK(Hz)                       = 48000000
*             - AHB Prescaler                  = 1
*             - APB1 Prescaler                 = 1
*             - APB2 Prescaler                 = 1
*             - PLL_M                          = 1
*             - PLL_N                          = 24
*             - PLL_P                          = 7
*             - PLL_Q                          = 4
*             - PLL_R                          = 4
*             - Flash Latency(WS)              = 4
*
Input:          no
Output:        no
Return:        no
Others:        no
**************************************************************************************/
void SystemClock_Config(void)
{
#if 0//yuyin
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct = {0};

#if defined (USE_USB_CLKSOURCE_CRSHSI48)
    static RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};
#endif

#if defined (USE_USB_CLKSOURCE_CRSHSI48)

    /* Enable HSI48 Oscillator to be used as system clock source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Select HSI48 as USB clock source */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);


    /* MSI is enabled after System reset, activate PLL with MSI as source */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 24;//40;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        /* Initialization Error */
        while (1);
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        /* Initialization Error */
        while (1);
    }


    /*Configure the clock recovery system (CRS)**********************************/

    /* Enable CRS Clock */
    __HAL_RCC_CRS_CLK_ENABLE();

    /* Default Synchro Signal division factor (not divided) */
    RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;

    /* Set the SYNCSRC[1:0] bits according to CRS_Source value */
    RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;

    /* HSI48 is synchronized with USB SOF at 1KHz rate */
    RCC_CRSInitStruct.ReloadValue =  RCC_CRS_RELOADVALUE_DEFAULT;
    RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;

    RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;

    /* Set the TRIM[5:0] to the default value*/
    RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;

    /* Start automatic synchronization */
    HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);

#elif defined (USE_USB_CLKSOURCE_PLL)

    /* Enable HSE Oscillator and activate PLL with HSE as source
    PLLCLK = (8 * 24) / 4) = 48 MHz */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 24;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLP = 7;
    RCC_OscInitStruct.PLL.PLLQ = 4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /*Select PLL 48 MHz output as USB clock source */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLCLK;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

    /* Select PLL as system clock source and configure the HCLK and PCLK1
    clock dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 |
                                   RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }

#endif /*USE_USB_CLKSOURCE_CRSHSI48*/

    /* Enable Power Controller clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    SystemCoreClockUpdate();
#endif //yuyin
    // g_TIM_clk_time = SystemCoreClock/(1000000*1);   //��ǰapb1Ϊ1��Ƶ����¼timeһ��ʱ�����ڵ�ʱ��ĵ���
}

/*************************************************
Function:      queue_init
Description:   ˫�˶��г�ʼ��
Input:         no
Output:        d_queue     ˫�˶��нṹ��
Return:        �μ�������񷵻���
Others:        no
*************************************************/
double_queue queue_init(double_queue d_queue)
{
    d_queue ->q_buf_len = 0;
    d_queue ->capacity = DEQUE_MAX_SIZE;
    d_queue ->front_node = 20;
    d_queue ->rear_node = 20;
    memset(d_queue->q_buf, 0x00, DEQUE_MAX_SIZE);
    return d_queue;
}

/*************************************************
Function:      queue_front_push
Description:   ˫�˶���ͷ��������
Input:         in_buf       ��������
               in_buf_len   �������ݳ���
Output:        d_queue      ˫�˶��нṹ��
Return:        no
Others:        no
*************************************************/
void queue_front_push(uint8_t *in_buf, uint32_t in_buf_len, double_queue d_queue)
{
    uint32_t off = 0;

    if (d_queue ->front_node == 0)
    {

        d_queue ->front_node = d_queue ->capacity - in_buf_len;

    }
    else
    {
        d_queue ->front_node -= in_buf_len;
    }
    off = d_queue ->front_node;
    memcpy(d_queue->q_buf + off, in_buf, in_buf_len);
    d_queue ->q_buf_len += in_buf_len;

}

/*************************************************
Function:      queue_front_pop
Description:   ˫�˶���ͷɾ������
Input:         in_buf_len   ɾ�����ݳ���
Output:        d_queue      ˫�˶��нṹ��
Return:        no
Others:        no
*************************************************/
void queue_front_pop(uint32_t in_buf_len, double_queue d_queue)
{
    //uint8_t item[DEQUE_MAX_SIZE] = {0x00};
    // uint32_t off = d_queue->front_node;
    if (d_queue ->front_node == (d_queue ->capacity - in_buf_len))
    {
        //memcpy(item,d_queue->q_buf,in_buf_len);
        d_queue ->front_node = 0;

    }
    else
    {
        //memcpy(item,d_queue->q_buf+off,in_buf_len);
        d_queue ->front_node += in_buf_len;
    }

    d_queue ->q_buf_len -= in_buf_len;

    return;
}


/*************************************************
Function:      queue_rear_push
Description:   ˫�˶���β��������
Input:         in_buf       ��������
               in_buf_len   �������ݳ���
Output:        d_queue      ˫�˶��нṹ��
Return:        no
Others:        no
*************************************************/
void queue_rear_push(uint8_t *in_buf, uint32_t in_buf_len, double_queue d_queue)
{
    uint32_t off = d_queue->rear_node;
    memcpy(d_queue->q_buf + off, in_buf, in_buf_len);
    if (d_queue ->rear_node == d_queue ->capacity - in_buf_len)
    {
        d_queue ->rear_node = 0;
    }
    else
    {
        d_queue ->rear_node += in_buf_len;
    }


    d_queue ->q_buf_len += in_buf_len;

}

/*************************************************
Function:      queue_rear_pop
Description:   ˫�˶���βɾ������
Input:         in_buf_len   ɾ�����ݳ���
Output:        d_queue      ˫�˶��нṹ��
Return:        ɾ��������
Others:        no
*************************************************/
void queue_rear_pop(uint32_t in_buf_len, double_queue d_queue)
{
    //uint8_t item[DEQUE_MAX_SIZE] = {0x00};
//        uint32_t off = d_queue->rear_node;
    if (d_queue ->rear_node == 0)
    {
        //off=d_queue ->capacity-in_buf_len;
        //memcpy(item,d_queue->q_buf+off,in_buf_len);
        d_queue ->capacity -= in_buf_len;
    }
    else
    {
        //memcpy(item,d_queue->q_buf+off,in_buf_len);
        d_queue ->rear_node -= in_buf_len;
    }

    d_queue ->q_buf_len -= in_buf_len;

    return;
}

/*************************************************
Function:      queue_rear_pop
Description:   ˫�˶���βɾ������
Input:         in_buf_len   ɾ�����ݳ���
Output:        d_queue      ˫�˶��нṹ��
Return:        no
Others:        no
*************************************************/
uint32_t queue_size(double_queue d_queue)
{
    if (d_queue ->rear_node > d_queue ->front_node)
    {
        return (d_queue ->rear_node - d_queue ->front_node);
    }
    else
    {
        return (d_queue ->front_node - d_queue ->rear_node);
    }
}

//***************************************************
//proto_i2c_impl.c
//***************************************************
/*************************************************
  Function:   proto_i2c_crc16
  Description:  ����ָ������ָ�����ݵ�CRC���
  Input:
            CRCType��CRC������sing
            Length��Ҫ��������ݳ���
            Data��Ҫ��������ݵ���ʼ��ַ
  Return:   crc������
  Others:
*************************************************/
uint16_t proto_i2c_crc16(uint32_t CRCType, uint32_t Length, uint8_t *Data)
{
    uint8_t chBlock = 0;
    uint16_t wCrc = 0;

    wCrc = (CRCType == CRC_A) ? 0x6363 : 0xFFFF;    // CRC_A : ITU-V.41 , CRC_B : ISO 3309

    do
    {
        chBlock = *Data++;
        chBlock = (chBlock ^ (uint8_t)(wCrc & 0x00FF));
        chBlock = (chBlock ^ (chBlock << 4));
        wCrc = (wCrc >> 8) ^ ((uint16_t)chBlock << 8) ^ ((uint16_t)chBlock << 3) ^ ((uint16_t)chBlock >> 4);
    }
    while (--Length);

    if (CRCType != CRC_A)
    {
        wCrc = ~wCrc; // ISO 3309
    }

    return wCrc;
}


/******************************************************************
Function:       porto_i2c_queue_in
Description:    ����HED I2CЭ���ʽ����˫�����������֡ͷ��֡β����
        1.��˫�����ͷ������ NAD, PCB , LEN
        2.��˫�����β������CRC
Input:    periph �豸���
          inbuf ����˫����е���ʼ��ַ
          inbuf_len ����˫����е��������ݳ���
Output:       no
Return:       ��������״̬��
Others:        no
******************************************************************/
se_error_t porto_i2c_queue_in(peripheral *periph, uint8_t *inbuf, uint16_t inbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    double_queue queue_in = (double_queue)inbuf;

    uint8_t i2c_data[4] = {0};
    uint8_t offset = 0;
    uint16_t i2c_crc = 0;

    do
    {
        if (periph == NULL)
        {
            ret_code =  SE_ERR_HANDLE_INVALID;
            break;
        }

        if ((inbuf == NULL) || (inbuf_len == 0U))
        {
            ret_code =  SE_ERR_PARAM_INVALID;
            break;
        }

        //��˫����зֱ�����NAD, PCB, LENH,LENL
        i2c_data[offset++] = PROTO_I2C_NAD;
        i2c_data[offset++] = PROTO_I2C_PCB_I_BLOCK;
        i2c_data[offset++] = inbuf_len >> 8;
        i2c_data[offset++] = inbuf_len;
        queue_front_push(i2c_data, offset, queue_in);

        //����CRC16������˫��������� CRC
        i2c_crc = proto_i2c_crc16(CRC_B, queue_size(queue_in),
                                  &queue_in->q_buf[queue_in->front_node]); //4 //���� CRC
        queue_rear_push((uint8_t *)&i2c_crc, 2, queue_in);

    }
    while (0);

    return ret_code;
}


/******************************************************************
Function:       porto_i2c_send_iblock
Description:    ͨ��I2C �ӿڷ���I ������
        1.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
            ���ͺ���transmit������I ������
Input:    periph �豸���
          inbuf ����˫����е���ʼ��ַ
          inbuf_len ����˫����е��������ݳ���
Output:       no
Return:       ��������״̬��
Others:        no
******************************************************************/
se_error_t porto_i2c_send_iblock(peripheral *periph, uint8_t *inbuf, uint16_t inbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

    do
    {
        if (periph == NULL)
        {
            ret_code = SE_ERR_HANDLE_INVALID;
            break;
        }

        if ((inbuf == NULL) || (inbuf_len == 0U))
        {
            ret_code = SE_ERR_PARAM_INVALID;
            break;
        }

        hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);

        //��������
        ret_code = p_i2c_periph->transmit(p_i2c_periph, inbuf, inbuf_len);

    }
    while (0);

    return ret_code;

}

/******************************************************************
Function:       porto_i2c_send_lblock
Description:    ͨ��I2C �ӿڷ���L , R��ATR ��
        1.���ݿ����ͣ���֯���ʽ��֡����
        2.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
            ���ͺ���transmit�����Ϳ�����
Input:    periph   �豸���
          block_type  ������
          data_len   ������ΪL ��ʱ����ʾ����Ҫ���͵�I������ݳ��ȣ�
          ������Ϊ������ʱ����ֵΪ0
Output:       no
Return:       ��������״̬��
Others:        no
******************************************************************/
se_error_t porto_i2c_send_block(peripheral *periph, uint8_t block_type, uint16_t data_len)
{
    se_error_t ret_code = SE_SUCCESS;
    HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
    uint16_t i2c_crc = 0;
    uint8_t i2c_buf[PROTO_I2C_LR_FRAME_LENGTH] = {0};
    uint8_t offset = 0;

    if (pI2cPeriph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    //ƴ�ӷ�������
    i2c_buf[offset++] = PROTO_I2C_NAD;
    i2c_buf[offset++] = block_type;
    i2c_buf[offset++] = data_len >> 8;
    i2c_buf[offset++] = data_len;

    //����CRC16
    i2c_crc = proto_i2c_crc16(CRC_B, offset, i2c_buf);  //4 // NAD PCB LEN1 LEN2 CRC
    i2c_buf[offset++] = i2c_crc;
    i2c_buf[offset++] = i2c_crc >> 8;

    ret_code = pI2cPeriph->transmit(pI2cPeriph, i2c_buf, offset);

    return ret_code;
}


/******************************************************************
Function:       porto_i2c_send_lblock
Description:    ͨ��I2C �ӿڷ���L ������
Input:    periph �豸���
          data_len L ��ĳ���ֵ������Ҫ���͵�I������ݳ���
Output:   no
Return:   ��������״̬��
Others:   no
******************************************************************/
se_error_t porto_i2c_send_lblock(peripheral *periph, uint16_t data_len)
{
    hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
    return porto_i2c_send_block(periph, PROTO_I2C_PCB_L_BLOCK, data_len);
}


/******************************************************************
Function:       porto_i2c_send_rblock
Description:    ͨ��I2C �ӿڷ���R ������
Input:    periph �豸���
Output:   no
Return:   ��������״̬��
Others:   no
******************************************************************/
se_error_t porto_i2c_send_rblock(peripheral *periph)
{
    hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
    return porto_i2c_send_block(periph, PROTO_I2C_PCB_R_BLOCK, 0);
}


/******************************************************************
Function:       porto_i2c_send_atr_block
Description:    ͨ��I2C �ӿڷ���ATR �����
Input:    periph �豸���
Output:   no
Return:   ��������״̬��
Others:   no
******************************************************************/
se_error_t porto_i2c_send_atr_block(peripheral *periph)
{
    hal_delay(PROTO_I2C_RECEIVE_TO_SEND_BGT);
    return porto_i2c_send_block(periph, PROTO_I2C_PCB_ATR_BLOCK, 0);
}


/******************************************************************
Function:       porto_i2c_receive_block
Description:    ͨ��I2C �ӿڽ��տ�����
                1.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
            ���պ���receive�����տ�����
Input:        periph �豸���
Output:       rbuf ���տ����ݵĴ洢��ַ
              rlen ����Ϣ�ĳ���
Return:       ��������״̬��
Others:        no
******************************************************************/
se_error_t porto_i2c_receive_block(peripheral *periph, uint8_t *rbuf, uint16_t rlen)
{
    se_error_t ret_code = SE_SUCCESS;
    HAL_I2C_PERIPHERAL_STRUCT_POINTER pI2cPeriph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
    i2c_timer_t timer = {0};
    uint8_t crc_err_count = 0;
    uint16_t crc_value = 0;

    if (pI2cPeriph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((rbuf == NULL) || (rlen < PROTO_I2C_FRAME_LENGTH_MINI))
    {
        return SE_ERR_PARAM_INVALID;
    }

    //���ý�������ʱ֡�ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_RECEVIE_FRAME_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        do
        {
            hal_delay(PROTO_I2C_RECEVIE_POLL_TIME);   //delay poll time

            ret_code = pI2cPeriph->receive(pI2cPeriph, rbuf, (uint32_t *)&rlen);
            if (ret_code == SE_SUCCESS)
            {
                break;
            }

            if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
            {
                LOGE("Failed:receive frame overtime,  ErrCode-%08X.", SE_ERR_TIMEOUT);
                return SE_ERR_TIMEOUT;
            }

        }
        while (1);


        if (rbuf[PROTO_I2C_PCB_OFFSET] == PROTO_I2C_PCB_I_BLOCK)
        {
            //compare CRC16
            crc_value = proto_i2c_crc16(CRC_B, rlen - PROTO_I2C_CRC_LENGTH, rbuf);
            if ((rbuf[rlen - PROTO_I2C_CRC_LENGTH] == (crc_value & 0xff)) &&
                (rbuf[rlen - PROTO_I2C_CRC_LENGTH + 1] == crc_value >> 8))
            {
                return ret_code;  //receive data ok
            }
        }
        else if ((rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_R_BLOCK) ||
                 (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_R_NAK_BLOCK) ||
                 (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_L_BLOCK) ||
                 (rbuf[PROTO_I2C_PCB_OFFSET]  == PROTO_I2C_PCB_S_BLOCK))
        {
            //compare CRC16
            crc_value = proto_i2c_crc16(CRC_B, PROTO_I2C_LR_FRAME_LENGTH - PROTO_I2C_CRC_LENGTH, rbuf);
            if ((rbuf[PROTO_I2C_LR_FRAME_LENGTH - PROTO_I2C_CRC_LENGTH] == (crc_value & 0xff)) &&
                (rbuf[PROTO_I2C_LR_FRAME_LENGTH - PROTO_I2C_CRC_LENGTH + 1] == crc_value >> 8))
            {
                return ret_code;  //receive data ok
            }
        }
        else if (rbuf[PROTO_I2C_PCB_OFFSET] == 0xFF)
        {
            continue;
        }
        else
        {
            return SE_ERR_PARAM_INVALID;
        }

        crc_err_count++;  //crc  err
    }
    while (crc_err_count <= PROTO_I2C_RETRY_NUM);

    return SE_ERR_LRC_CRC;
}



/******************************************************************
Function:       porto_i2c_ receive_lrblock
Description:    ͨ��I2C �ӿڽ���L���R�飬L�鳤����Ч��R�鳤��Ϊ0
                1.����porto_i2c_receive_block ��������L�顢R ��� S ��
                2.����������
Input:    periph �豸���
Output:   block_type ����������
          block_len ������ΪL�飬��ʾ ����Ҫ���� I  ����Ϣ�ĳ���
Return:       ��������״̬��
Others:        no
******************************************************************/
se_error_t porto_i2c_receive_lrblock(peripheral *periph, uint8_t *block_type, uint16_t *block_len)
{
    se_error_t ret_code = SE_SUCCESS;
    uint8_t i2c_buf[PROTO_I2C_LR_FRAME_LENGTH];
    uint8_t nad = 0;
    uint8_t pcb = 0;
    uint16_t rec_wtx_count = 0;

    if (periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    do
    {
        ret_code = porto_i2c_receive_block(periph, i2c_buf, PROTO_I2C_LR_FRAME_LENGTH);
        if (ret_code != SE_SUCCESS)
        {
            return ret_code;
        }

        nad = i2c_buf[PROTO_I2C_NAD_OFFSET];
        pcb = i2c_buf[PROTO_I2C_PCB_OFFSET];

        //the NAD recieved is the NAD sended whose high-low bit is changed
        if (((nad >> 4) & 0x0f) != (PROTO_I2C_NAD & 0x0f) ||
            ((nad << 4) & 0xf0) != (PROTO_I2C_NAD & 0xf0))
        {
            return SE_ERR_PARAM_INVALID;
        }

        if ((pcb == PROTO_I2C_PCB_R_BLOCK) || (pcb == PROTO_I2C_PCB_R_NAK_BLOCK) ||
            (pcb == PROTO_I2C_PCB_L_BLOCK))
        {
            *block_type = pcb;
            break;
        }

        else if (pcb ==
                 PROTO_I2C_PCB_S_BLOCK)  //S��  ��Ҫ���¼�ʱ����������
        {
            if (rec_wtx_count >= PROTO_I2C_WTX_NUM)
            {
                return SE_ERR_COMM;    //����WTX����֡����20�ν����ش����Է�ֹSE����Ϸ����������޷��˳�ѭ��
            }
            rec_wtx_count++;
            continue;
        }

        else
        {
            return SE_ERR_PARAM_INVALID;
        }
//      break;
    }
    while (1);

    *block_len = i2c_buf[PROTO_I2C_LEN_OFFSET] * 256 + i2c_buf[PROTO_I2C_LEN_OFFSET +
                                                               1];  //cal output len

    return  ret_code;
}



/******************************************************************
Function:       proto_i2c_receive_iblock
Description:    ͨ��I2C �ӿڽ���I �����ݻ�R��
                1.����porto_i2c_receive_block ��������I �����ݻ�R ��
Input:    periph �豸���
          data_len ������Ϣ�����ݵĳ���
Output:   block_type ����������
          outbuf ���˫����е���ʼ��ַ
Return:   ��������״̬��
Others:   no
******************************************************************/
se_error_t proto_i2c_receive_irblock(peripheral *periph, uint8_t *block_type, uint8_t *outbuf,
                                     uint16_t data_len)
{
    se_error_t ret_code = SE_SUCCESS;
    uint8_t nad = 0;
    uint8_t pcb = 0;
    uint16_t  receive_len = 0;

    if (periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((outbuf == NULL) || (data_len == 0U))
    {
        return SE_ERR_PARAM_INVALID;
    }

    do
    {
        ret_code = porto_i2c_receive_block(periph, outbuf, data_len + PROTO_I2C_FRAME_LENGTH_MINI);
        if (ret_code != SE_SUCCESS)
        {
            return ret_code;
        }

        nad = outbuf[PROTO_I2C_NAD_OFFSET];
        pcb = outbuf[PROTO_I2C_PCB_OFFSET];

        //the NAD recieved is the NAD sended whose high-low bit is changed
        if (((nad >> 4) & 0x0f) != (PROTO_I2C_NAD & 0x0f) ||
            ((nad << 4) & 0xf0) != (PROTO_I2C_NAD & 0xf0))
        {
            return SE_ERR_PARAM_INVALID;
        }

        //����ӦΪI ��� NAK�飬���򱨴�
        if (pcb == PROTO_I2C_PCB_R_NAK_BLOCK)
        {
            *block_type = pcb;
            return ret_code;
        }
        else if (pcb == PROTO_I2C_PCB_I_BLOCK)
        {
            *block_type = pcb;
            break;
        }
        else
        {
            return SE_ERR_PARAM_INVALID;
        }

    }
    while (0);

    receive_len = outbuf[PROTO_I2C_LEN_OFFSET] * 256 + outbuf[PROTO_I2C_LEN_OFFSET + 1];
    if (data_len != receive_len) //L�鳤����I�鳤�Ȳ�һ��
    {
        return SE_ERR_LEN;
    }

    return ret_code;
}

/*********************************************************************************
Function:       proto_i2c_handle
Description:    ����HED I2C ͨ��Э���֡��ʽ��SE�����շ����ݽ������ڼ����
              ����֡���ͼ�LRCУ��ȣ�������ʱ֡��NAK֡������ʱ֡�ط�����
                      ��Ҫ���������º���:
              1. proto_spi_send_frame
              2. proto_spi_receive_frame_head
              3. proto_spi_receive_frame_data
Input:          handle �豸���
           input  ��������ָ��
           input_len �������ݳ���
Output:    output������ݴ洢��ַ
           outputlen������ݳ���
Return:         no
Others:         no
*********************************************************************************/
se_error_t proto_i2c_handle(peripheral *periph, uint8_t *inbuf, uint16_t inbuf_len, uint8_t *outbuf,
                            uint16_t *outbuf_len)
{
    se_error_t ret_code = SE_SUCCESS;
    uint16_t block_len = 0;
    uint8_t nak_count = 0;
    uint8_t block_type = 0;

    do
    {
        if (nak_count > PROTO_I2C_NAK_NUM)
        {
            return SE_ERR_LRC_CRC;
        }
        nak_count++;

        ret_code = porto_i2c_send_lblock(periph, inbuf_len);//����L��
        if (ret_code != SE_SUCCESS)
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return ret_code;//SE_ERR_LRC_CRC;
            }
            //nak_count++;
            //return ret_code;
        }

        ret_code = porto_i2c_receive_lrblock(periph, &block_type, &block_len);  //����R��
        if (ret_code != SE_SUCCESS)
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return ret_code;//SE_ERR_LRC_CRC;
            }
        }

    }
    while (block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

    nak_count = 0;
    do
    {
        if (nak_count > PROTO_I2C_NAK_NUM)
        {
            return SE_ERR_LRC_CRC;
        }
        nak_count++;

        ret_code = porto_i2c_send_iblock(periph, inbuf,
                                         inbuf_len + PROTO_I2C_FRAME_LENGTH_MINI); //����I��
        if (ret_code != SE_SUCCESS)
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return ret_code;//SE_ERR_LRC_CRC;
            }
        }

        ret_code = porto_i2c_receive_lrblock(periph, &block_type,
                                             &block_len);  //����L�� �� R ��
        if (ret_code != SE_SUCCESS)
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return ret_code;//SE_ERR_LRC_CRC;
            }
        }
    }
    while (block_type == PROTO_I2C_PCB_R_NAK_BLOCK);


    if ((block_type == PROTO_I2C_PCB_L_BLOCK) && (block_len != 0))
    {
        //��ȷ�յ�L��,����R��
        nak_count = 0;
        do
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return SE_ERR_LRC_CRC;
            }
            nak_count++;

            //��ȷ�յ�L��,����R��
            ret_code = porto_i2c_send_rblock(periph);
            if (ret_code != SE_SUCCESS)
            {
                if (nak_count > PROTO_I2C_NAK_NUM)
                {
                    return ret_code;//SE_ERR_LRC_CRC;
                }
            }

            ret_code = proto_i2c_receive_irblock(periph, &block_type, outbuf,
                                                 block_len);//����I���R��
            if (ret_code != SE_SUCCESS)
            {
                if (nak_count > PROTO_I2C_NAK_NUM)
                {
                    return ret_code;//SE_ERR_LRC_CRC;
                }
            }
        }
        while (block_type == PROTO_I2C_PCB_R_NAK_BLOCK);
    }
    else
    {
        //Ӧ���յ�L�飬ȴ�յ���R��
        return SE_ERR_COMM;
    }

    *outbuf_len = block_len + PROTO_I2C_FRAME_LENGTH_MINI;

    return ret_code;
}



/***********************************************************************
Function:       proto_i2c_get_atr
Description:   SPIͨ��Э��֮���豸�Ĳ���
                     1. ����proto_i2c_send_atr_block��������SE����ATR����顣
                     2. ����proto_i2c_receive_lrblock����L���R�顣
                     3. �����յ�ΪNAK��R�飬��������proto_i2c_send_atr_block������
                            ��ATR����飬������proto_i2c_receive_lrblock��������L���R�顣
                     4. �����յ�I�飬����proto_i2c_receive_iblock��������ATR���ݡ�
Input:        periph �豸���
Output:      rbuf  ������ATR����ʼ��ַ
             rlen  ������ATR�ĳ���
Return:      ��������״̬��
Others:      no
************************************************************************/
se_error_t proto_i2c_get_atr(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
    se_error_t ret_code = SE_SUCCESS;
    uint16_t block_len = 0;
    uint8_t nak_count = 0;
    uint8_t block_type = 0;

    if (periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((rbuf == NULL) || (rlen == NULL))
    {
        DBG_DIRECT("Error: rbuf or rlen is NULL. %s", __func__);
        return SE_ERR_PARAM_INVALID;
    }

    DBG_DIRECT("proto_i2c_get_atr: rbuf = %p, rlen = %p", rbuf, rlen);
    do
    {
        if (nak_count > PROTO_I2C_NAK_NUM)
        {
            return SE_ERR_LRC_CRC;
        }
        nak_count++;

        ret_code = porto_i2c_send_atr_block(periph);  //����ATR �����
        if (ret_code != SE_SUCCESS)
        {
            return ret_code;
        }

        ret_code = porto_i2c_receive_lrblock(periph, &block_type,
                                             &block_len);  //���� L �� ��R ��
        if (ret_code != SE_SUCCESS)
        {
            return ret_code;
        }
    }
    while (block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

    if ((block_type == PROTO_I2C_PCB_L_BLOCK) && (block_len != 0))
    {
        if (block_len > (PROTO_I2C_ATR_MAX_LEN + PROTO_I2C_FRAME_LENGTH_MINI))
        {
            return SE_ERR_LEN;
        }
        nak_count = 0;
        do
        {
            if (nak_count > PROTO_I2C_NAK_NUM)
            {
                return SE_ERR_LRC_CRC;
            }
            nak_count++;

            //��ȷ�յ�L��,����R��
            ret_code = porto_i2c_send_rblock(periph);
            if (ret_code != SE_SUCCESS)
            {
                return ret_code;
            }

            ret_code = proto_i2c_receive_irblock(periph, &block_type, rbuf,
                                                 block_len);//����I��( ATR ֵ)��R��
            if (ret_code != SE_SUCCESS)
            {
                return ret_code;
            }

        }
        while (block_type == PROTO_I2C_PCB_R_NAK_BLOCK);

    }
    else
    {
        //Ӧ���յ�L�飬ȴ�յ���R��
        return SE_ERR_COMM;
    }

    *rlen = block_len;

    return ret_code;
}


/************************************************************************************
Function:       proto_i2c_init
Description:    ����I2C ͨ��Э��ʵ�ֳ�ʼ��
              1.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
              ��ʼ������init
Input:         periph �豸���
Output:        no
Return:        ��������״̬��
Others:        no
**************************************************************************************/
se_error_t proto_i2c_init(peripheral *periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};

    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:init mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        ret_code = p_i2c_periph->init(p_i2c_periph);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:i2c potocol,  ErrCode-%08X.", ret_code);
        }
        else
        {
            LOGI("Success!");
        }
        break;
    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);

    return ret_code;
}


/************************************************************************************
Function:       proto_i2c_deinit
Description:    ����I2C ͨ��Э��ʵ�ֳ�ʼ��
              1.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
              ��ʼ������deinit
Input:          periph �豸���
Output:        no
Return:        ��������״̬��
Others:        no
**************************************************************************************/
se_error_t proto_i2c_deinit(peripheral *periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:deinit mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        ret_code = p_i2c_periph->deinit(p_i2c_periph);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:i2c potocol,  ErrCode-%08X.", ret_code);
        }
        else
        {
            LOGI("Success!");
        }
        break;
    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);
    return ret_code;
}


/***********************************************************************
Function:       proto_i2c_open
Description:   SPIͨ��Э��֮���豸�Ĳ���
                 1. ͨ��port���豸ע��ĺ����б�ָ�룬����port���lock��������
                 2. ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵĺ���power_on���豸�ϵ�
                 3. ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵĺ������豸��λ
                 4. ����proto_i2c_get_atr��������ȡATR��
                 5. ͨ��port���豸ע��ĺ����б�ָ�룬����unlock����������
Input:        periph �豸���
Output:      rbuf  ������ATR����ʼ��ַ
             rlen  ������ATR�ĳ���
Return:      ��������״̬��
Others:      no
************************************************************************/
se_error_t proto_i2c_open(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
    uint8_t atr_buf[PROTO_I2C_ATR_MAX_LEN + PROTO_I2C_FRAME_LENGTH_MINI] = {0};

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((rbuf == NULL) || (rlen == NULL))
    {
        DBG_DIRECT("rbuf or rlen is NULL!");
        return  SE_ERR_PARAM_INVALID;
    }

    DBG_DIRECT("proto_i2c_open: rbuf = %p, rlen = %p", rbuf, rlen);
    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:open periph mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        //SE �ϵ�
        ret_code = p_i2c_periph->power_on(p_i2c_periph);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol power on,  ErrCode-%08X.", ret_code);
            break;
        }

        //����SE ��RST���Ž��и�λ����
        ret_code = p_i2c_periph->control(p_i2c_periph, PROTO_I2C_CTRL_RST, NULL, 0);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol rst io control,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = proto_i2c_get_atr(periph, atr_buf, rlen);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }
        else if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol get atr,  ErrCode-%08X.", ret_code);
            break;
        }

        memcpy(rbuf, atr_buf + PROTO_I2C_DATA_OFFSET, *rlen);
        LOGI("Open Periph Success!");
        break;

    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);

    return ret_code;
}


/************************************************************************************
Function:       proto_i2c_close
Description:    ����I2C ͨ��Э��ʵ�ֳ�ʼ��
              1.ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵ�
              ��ʼ������power_off
Input:          periph �豸���
Output:        no
Return:        ��������״̬��
Others:        no
**************************************************************************************/
se_error_t proto_i2c_close(peripheral *periph)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};

    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:close periph mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        ret_code = p_i2c_periph->power_off(p_i2c_periph);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:i2c potocol,  ErrCode-%08X.", ret_code);
        }
        else
        {
            LOGI("Close Periph Success!");
        }
        break;

    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);
    return ret_code;
}


/******************************************************************
Function:       proto_i2c_transceive
Description:    ͨ��I2C �ӿ�����豸������������������Ӧʱ�����ô˺���
            1. ͨ��port���豸ע��ĺ����б�ָ�룬����port���lock��������
            2������ HED_I2C ͨ��Э�飬����proto_i2c_handle���������ͼ�����Э��֡���ݡ�
            3��ͨ��port���豸ע��ĺ����б�ָ�룬����unlock����������
Input:     periph �豸���
           sbuf ����˫����е���ʼ��ַ
           slen ����˫����е��������ݳ���
Output:    rbuf ���˫����е���ʼ��ַ
           rlen  ���˫����е�������ݳ��ȵ���ʼ��ַ
Return:        ��������״̬��
Others:  no
******************************************************************/
se_error_t proto_i2c_transceive(peripheral *periph, uint8_t *sbuf,  uint32_t  slen, uint8_t *rbuf,
                                uint32_t *rlen)
{
    se_error_t ret_code = SE_SUCCESS;
//  se_error_t ret_code_bak = SE_SUCCESS;
    i2c_timer_t timer = {0};
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
    double_queue queue_in = (double_queue)sbuf;
    double_queue queue_out = (double_queue)rbuf;
    uint16_t send_len = 0;
    uint16_t rec_len = 0;
    uint8_t *p_output = NULL;
    uint8_t *p_input = NULL;

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((sbuf == NULL) || (rbuf == NULL) || (slen == 0U) || (rlen == NULL))
    {
        return  SE_ERR_PARAM_INVALID;
    }

    //׼���շ�������
    send_len = (uint16_t)slen;
    ret_code = porto_i2c_queue_in(periph, sbuf, slen);
    if (ret_code != SE_SUCCESS)
    {
        return ret_code;
    }

    p_input = &queue_in->q_buf[queue_in->front_node];
    p_output = &queue_out->q_buf[queue_out->front_node];

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:communication mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        //����I2C  ���ݵ��շ�
        ret_code = proto_i2c_handle(periph, p_input, send_len, p_output, &rec_len);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        else if (ret_code == SE_ERR_LRC_CRC)
        {
            LOGE("Failed:check crc,  ErrCode-%08X.", ret_code);
            //����SE ��RST���Ž��и�λ����
            //ret_code_bak = p_i2c_periph->control(p_i2c_periph, PROTO_I2C_CTRL_RST, NULL, NULL);
            //if(ret_code_bak != SE_SUCCESS)
            //{
            //  LOGE("Failed:protocol communication control,  ErrCode-%08X.", ret_code_bak);
            //}
            //p_i2c_periph->power_off(p_i2c_periph);
            break;
        }

        else if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol communication,  ErrCode-%08X.", ret_code);
            break;
        }

        queue_out->q_buf_len = rec_len;
        queue_out->rear_node =  queue_out->front_node + rec_len;

        queue_front_pop(PROTO_I2C_NAD_PCB_LEN_LENGTH, queue_out); //�Ƴ�NAD, PCB, LEN
        queue_rear_pop(PROTO_I2C_CRC_LENGTH, queue_out);//�Ƴ�CRC
        *rlen = queue_size(queue_out);
        LOGI("Communication Success!");
        break;
    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);
    return ret_code;
}

/***********************************************************************
Function:       proto_i2c_reset
Description:   SPIͨ��Э��֮��λ�豸�Ĳ���
                 1. ͨ��port���豸ע��ĺ����б�ָ�룬����port���lock��������
                 2. ͨ��port���豸ע��ĺ����б�ָ�룬����port��i2c�ӿڵĺ������豸��λ
                 3. ����proto_i2c_get_atr��������ȡATR��
                 4. ͨ��port���豸ע��ĺ����б�ָ�룬����unlock����������
Input:        periph �豸���
Output:      rbuf  ������ATR����ʼ��ַ
             rlen  ������ATR�ĳ���
Return:      ��������״̬��
Others:      no
************************************************************************/
se_error_t proto_i2c_reset(peripheral *periph, uint8_t *rbuf, uint32_t *rlen)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;
    uint8_t atr_buf[PROTO_I2C_ATR_MAX_LEN + PROTO_I2C_FRAME_LENGTH_MINI] = {0};

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if ((rbuf == NULL) || (rlen == NULL))
    {
        return  SE_ERR_PARAM_INVALID;
    }

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:reset peirph mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        //����SE ��RST���Ž��и�λ����
        ret_code = p_i2c_periph->control(p_i2c_periph, PROTO_I2C_CTRL_RST, NULL, 0);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol rst io control,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = proto_i2c_get_atr(periph, atr_buf, rlen);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }
        else if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:protocol get atr,  ErrCode-%08X.", ret_code);
            break;
        }

        memcpy(rbuf, atr_buf + PROTO_I2C_DATA_OFFSET, *rlen);
        LOGI("Reset Periph Success!");
        break;
    }
    while (1);

    p_i2c_periph->unlock(p_i2c_periph);
    return ret_code;
}



/******************************************************************
Function:       proto_i2c_control
Description:    ͨ��i2c�ӿ�����豸���Ϳ�������
Input:     periph �豸���
           ctrlcode ���������
           sbuf �������ݵ���ʼ��ַ
           slen �������ݵĳ���
Output:    rbuf ������ݵ���ʼ��ַ
           rlen  ������ݳ��ȵ���ʼ��ַ
Return:    ��������״̬��
Others:    no
******************************************************************/
se_error_t proto_i2c_control(peripheral *periph, uint32_t ctrlcode, uint8_t *sbuf, uint32_t slen,
                             uint8_t  *rbuf, uint32_t *rlen)
{
    se_error_t ret_code = SE_SUCCESS;
    i2c_timer_t timer = {0};
    HAL_I2C_PERIPHERAL_STRUCT_POINTER p_i2c_periph = (HAL_I2C_PERIPHERAL_STRUCT_POINTER)periph;

    if (p_i2c_periph == NULL)
    {
        return SE_ERR_HANDLE_INVALID;
    }

    if (ctrlcode == 0U)
    {
        return  SE_ERR_PARAM_INVALID;
    }

    //�������ȴ��ĳ�ʱʱ��
    timer.interval = PROTO_I2C_COMM_MUTEX_WAIT_TIME;
    timer.start = hal_systick();

    do
    {
        if (i2c_time_get_diff(hal_systick(), timer.start) >= timer.interval)
        {
            ret_code = SE_ERR_TIMEOUT;
            LOGE("Failed:control periph mutex,  ErrCode-%08X.", ret_code);
            break;
        }

        ret_code = p_i2c_periph->lock(p_i2c_periph);
        if (ret_code == SE_ERR_BUSY)
        {
            continue;
        }

        ret_code = p_i2c_periph->control(p_i2c_periph, ctrlcode, sbuf, slen);
        if (ret_code != SE_SUCCESS)
        {
            LOGE("Failed:i2c potocol,  ErrCode-%08X.", ret_code);
        }
        else
        {
            LOGI("Success!");
        }
        break;
    }
    while (1);

    return ret_code;
}

#endif //CONFIG_ALIPAY
