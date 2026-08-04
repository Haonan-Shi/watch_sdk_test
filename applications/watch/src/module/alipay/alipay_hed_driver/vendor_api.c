/******************************************************************************
 Copyright(C),CEC Huada Electronic Design Co.,Ltd.
 File name:         vendor_api.c
 Author:              sunll
 Version:             V1.0
 Date:              2023-05-10
 Description:     fs interface definition
 History:

******************************************************************************/
#include "vendor_api.h"
#include "hed_private.h"
#include "port_driver_i2c.h"

#if CONFIG_ALIPAY

EXTERNC retval_t csi_user_cmd_process_sync(uint8_t *req, int32_t len_req, uint8_t *rsp,
                                           int32_t *len_rsp)
{
    se_error_t ret_code = SE_SUCCESS;
    uint32_t out_len = 0;
    uint16_t front_node = 0;
    uint8_t head = 0xA0;
    //�����ж�
    if (req == NULL || len_req == 0)
    {
        LOGE("failed to csi_user_cmd_process_sync input params!");
        return RV_UNKNOWN;
    }
    if (rsp == NULL || len_rsp == NULL)
    {
        LOGE("failed to csi_user_cmd_process_sync output params!");
        return RV_UNKNOWN;
    }
    //˫�˶��г�ʼ��
    queue_init(&g_queue_in);
    queue_init(&g_queue_out);

    //�������ݴ���˫�˶���
    queue_rear_push((uint8_t *)req, len_req, &g_queue_in);

    queue_front_push(&head, 1, &g_queue_in);

    //������˫������е����ݰ�Э���ʽ���͸��豸��������Ӧ���ݴ洢�����˫�����
    ret_code = acl_transceive_queue((uint8_t *)&g_queue_in, queue_size(&g_queue_in),
                                    (uint8_t *)&g_queue_out, (uint32_t *)len_rsp);
    if (ret_code != SE_SUCCESS)
    {
        return ret_code;
    }

    //��˫�˶��п������������
    if (g_queue_out.q_buf_len < 2)
    {
        return 1;
    }
    g_queue_out.q_buf_len -= 2;//ȥ��״̬��9000
    front_node = g_queue_out.front_node;
    memcpy(rsp, &g_queue_out.q_buf[front_node], g_queue_out.q_buf_len);
    *len_rsp = g_queue_out.q_buf_len;

    return ret_code;

}

EXTERNC void csi_enter_lpm(void *ctx)
{
    se_error_t ret = 0;

    //����cmd��apdu_switch_mode�ӿ�
    ret = apdu_switch_mode(POWERDOWN);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to call apdu_switch_mode!\n");
        return;
    }

    return;
}

EXTERNC void csi_exit_lpm(void *ctx)
{
    PORT_I2C_SE0_RST_LOW();
    hal_delay(PORT_I2C_SE_RST_LOW_DELAY);  //��λʱ��RST�͵�ƽ����ʱ��
    PORT_I2C_SE0_RST_HIGH();
    hal_delay(PORT_I2C_SE_RST_HIGH_DELAY);  //��λ��RST�ߵ�ƽ����ʱ��
}

#endif //CONFIG_ALIPAY
