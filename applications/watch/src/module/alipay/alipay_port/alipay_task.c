/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#if CONFIG_ALIPAY
#include <os_msg.h>
#include <os_task.h>
#include "trace.h"
#include "alipay_task.h"
#include "wdg.h"
#include "section.h"
#include "hs_private.h"
#include "alipay_config.h"
#include "alipay_ble_transport.h"
#include "alipay_bind.h"
#include "alipay_account_manage.h"
#include "hed_private.h"
#include "alipay_mem.h"
#include "trace.h"
#include "rtl876x_i2c.h"
#include "port_driver_i2c.h"
#include "os_sched.h"
#if CONFIG_ALIPAY_TRANSIT
#include "alipay_pan.h"
#include "alipay_transit.h"
#endif //CONFIG_ALIPAY_TRANSIT
#include "mbedtls/platform.h"

/*============================================================================*
 *                              Macros
 *============================================================================*/
#define ALIPAY_TASK_PRIORITY             1        //!< Task priorities
/* Was 512*40 = 20KB. The "get transit code" path runs mbedTLS RSA/ECC big-
 * number ops plus QR-code generation, whose combined stack usage exceeds
 * 20KB. The overflow corrupted the stack -- on return the CPU read the
 * stack-fill pattern 0xAA, jumped PC to 0xAAAAAAAA, and the watchdog fired
 * shortly after. Bumped to 40KB. The stack is allocated from the
 * RAM_TYPE_DATA_ON heap (heap_sram 52KB + PSRAM fallback 128KB), which can
 * satisfy a 40KB allocation. */
#define ALIPAY_TASK_STACK_SIZE           512 * 80  //!<  Task stack size = 40KB

#define MAX_ALIPAY_NUM_MESSAGE       0x20  //alipay message queue size

#define TRANSIT_CARD_NUM        20
#define QRCODEGEN_VERSION_MAX  25//fortest
#define ALIPAY_QRCODEGEN_BUFFER_LEN_FOR_VERSION(n)  ((((n) * 4 + 17) * ((n) * 4 + 17) + 7) / 8 + 1)
#define ALIPAY_QRCODEGEN_BUFFER_LEN_MAX             ALIPAY_QRCODEGEN_BUFFER_LEN_FOR_VERSION(QRCODEGEN_VERSION_MAX)
/*============================================================================*
 *                              Variables
 *============================================================================*/

void *alipay_task_handle = NULL;   //!< alipay Task handle
void *alipay_queue_handle = NULL;   //!< alipay queue handle


/*SHM_DATA_SECTION*/ static alipay_device_status_ g_alipay_status = {.status = 0xff};
/*SHM_DATA_SECTION*/ static uint8_t alipay_paycode_ready = 0;
/*SHM_DATA_SECTION*/ static uint8_t g_alipay_unbind_flag = 0;
/*SHM_DATA_SECTION*/ alipay_tansit_CardBaseVO_t *p_transit_card = NULL;
/*SHM_DATA_SECTION*/ static t_alipay_transit_list g_transit_card_list = {0};
/*SHM_DATA_SECTION*/ static t_alipay_transitCode g_default_transitCode = {0};
/*============================================================================*
 *                              Functions
 *============================================================================*/
void alipay_task(void *p_param);

/**
 * @brief  Initialize alipay task
 * @return void
 */
void alipay_task_init()
{
    void csi_heap_init(void);
    csi_heap_init();

    extern int mbedtls_platform_set_calloc_free(void *(*calloc_func)(size_t, size_t),
                                                void (*free_func)(void *));
    int result = mbedtls_platform_set_calloc_free(csi_calloc, csi_free);
    if (result != 0)
    {
        DBG_DIRECT("Failed to set memory functions");
    }
    else
    {
        DBG_DIRECT("mbedtls_platform_set_calloc_free success");
    }

    se_error_t ret = 0;
    uint8_t com_outbuf[300] = {0};
    uint32_t outlen = 0;
    uint8_t atr[30] = {0};
    uint32_t atr_len = 0;
    ret = api_register(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
    if (ret != SE_SUCCESS)
    {
        LOGE("[Alipay] failed to i2c api_register\n");
        //return ret;
    }

    ret = api_select(PERIPHERAL_I2C, I2C_PERIPHERAL_SE0);
    if (ret != SE_SUCCESS)
    {
        LOGE("[Alipay] failed to i2c api_select\n");
        //return ret;
    }

    ret = api_connect(com_outbuf, &outlen);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to api_connect\n");
        //return ret;
    }
    hal_printf("api_connect atr:\n");

    atr_len = 0;

    ret = api_reset(atr, &atr_len);
    if (ret != SE_SUCCESS)
    {
        LOGE("failed to api_reset\n");
        return ;
    }

    os_task_create(&alipay_task_handle, "alipay", alipay_task, 0, ALIPAY_TASK_STACK_SIZE,
                   ALIPAY_TASK_PRIORITY);
}


bool alipay_send_msg_to_alipay_task(T_ALIPAY_MSG *p_msg)
{
    if (alipay_queue_handle == NULL)
    {
        AliPay_LOG("[Alipay] alipay_queue_handle is null");
        return false;
    }
    if (os_msg_send(alipay_queue_handle, p_msg, 0) == false)
    {
        AliPay_LOG("[Alipay] alipay_send_msg_to_alipay_task fail to queue");
        return false;
    }

    AliPay_LOG("[Alipay] alipay_send_msg_to_alipay_task success to queue");
    return true;
}

/**
 * @brief        create app task queue
 * @param[in]    void
 * @return       void
 */
void alipay_task_queue_create(void)
{
    os_msg_queue_create(&alipay_queue_handle, "alipay_queue", MAX_ALIPAY_NUM_MESSAGE,
                        sizeof(T_ALIPAY_MSG));
}


/**
 * @brief        alipay task to handle events & messages
 * @param[in]    p_params    Parameters sending to the task
 * @return       void
 */

void alipay_task(void *p_param)
{
    T_ALIPAY_MSG alipay_msg;
    alipay_task_queue_create();

#if CONFIG_ALIPAY_TRANSIT
    p_transit_card = csi_malloc(TRANSIT_CARD_NUM * sizeof(alipay_tansit_CardBaseVO_t));

    uint32_t transitCode_len = (ALIPAY_QRCODEGEN_BUFFER_LEN_MAX > 512) ?
                               ALIPAY_QRCODEGEN_BUFFER_LEN_MAX : 512;
    char *p_transitCode = csi_malloc(transitCode_len);
    char errot_msg[128] = {0};
    uint32_t err_msg_len = sizeof(errot_msg);

    uint32_t transitCode_default_len = 512;
    uint8_t *p_default_transitCode = csi_malloc(transitCode_default_len);
#endif//#if CONFIG_ALIPAY_TRANSIT

    while (true)
    {
        if (os_msg_recv(alipay_queue_handle, &alipay_msg, 0xFFFFFFFF) == true)
        {
            AliPay_LOG("[Alipay] alipay task msg type %d ", alipay_msg.type);
            if (alipay_msg.type == ALIPAY_MSG_GET_BIND_STRING)
            {
#if 1
                //HS_IIC_Init();
                char uid[256] = {0};
                int len = sizeof(uid);
                //int alipay_pre_init(void);
                retval_e ret_code = 0;
                EXTERNC retval_e alipay_get_binding_code(PARAM_OUT char *result, PARAM_INOUT int *len_result);
                ret_code += alipay_get_binding_code(uid, &len);

                //qrcode_gen_text(uid, len, 200);
#endif
            }

            else if (ALIPAY_MSG_GET_BIND_STATUS == alipay_msg.type)
            {
                os_sched_suspend();
                g_alipay_status.status = 0xff;
                g_alipay_status.binded = alipay_get_binding_status();
                //binding_status_e status ;
                if (g_alipay_status.binded == 0x00)
                {
                    alipay_query_binding_result(&g_alipay_status.status);
                    AliPay_LOG("[Alipay] 01 alipay_query_binding_result:status %d ", g_alipay_status.status);
                }
                else
                {
                    g_alipay_status.status = ALIPAY_STATUS_BINDING_OK;
                    AliPay_LOG("[Alipay] 02 alipay_query_binding_result:status %d ", g_alipay_status.status);
                }
                g_alipay_status.triggle = false;
                os_sched_resume();
            }

            else if (alipay_msg.type == ALIPAY_MSG_GET_PAYCODE)
            {
                static uint8_t pay_code[128] = {0};
                retval_e alipay_get_paycode(PARAM_OUT uint8_t *paycode, PARAM_INOUT uint32_t *len_paycode);
                uint32_t length = sizeof(pay_code);
                retval_e ret = alipay_get_paycode(pay_code, &length);
                APP_PRINT_INFO3("[AliPay] code ret = %d, length %d, %b", ret, length, TRACE_BINARY(20, pay_code));
                ///qrcode_gen_text(pay_code, length, ALIPAY_QRCODE_SIZE);

                alipay_paycode_ready = 0x01;
            }
#if CONFIG_ALIPAY_TRANSIT
            if (alipay_msg.type == ALIPAY_MSG_GET_DEFAULT_TRANSIT_LIST)
            {
                APP_PRINT_INFO1("[Alipay] ALIPAY_MSG_GET_DEFAULT_TRANSIT_LIST, g_default_transitCode.is_valid %d",
                                g_default_transitCode.is_valid);

                if (g_default_transitCode.is_valid)
                {
                    //alipay_transit_get_the_last_transitCode();
                    // the default transCode is the return value
                    if (alipay_msg.func)
                    {
                        alipay_msg.func(alipay_msg.type, (void *)NULL);
                    }
                }
                else
                {
                    if (p_transit_card != NULL)
                    {
                        //try to get card list offline
                        uint32_t card_len =  sizeof(alipay_tansit_CardBaseVO_t) * TRANSIT_CARD_NUM;
                        uint32_t g_card_num = TRANSIT_CARD_NUM;
                        retval_e ret = alipay_transit_get_card_list_offline(p_transit_card, &card_len, &g_card_num);
                        APP_PRINT_INFO2("[Alipay][TRANSIT] alipay_transit_get_card_list_offline ret %d, card_num %d", ret,
                                        g_card_num);

                        if (ret != 0)
                        {
                            if (ret == 0x05/*RV_IO_ERROR*/)
                            {
                                //try to get transit list online
                                T_ALIPAY_MSG alipay_msg_new = {.type = ALIPAY_MSG_GET_CARD_LIST_ONLINE, .u.data = 0, .func = alipay_msg.func};
                                alipay_send_msg_to_alipay_task(&alipay_msg_new);
                            }
                        }
                        else
                        {
                            //send msg back to gui
                            g_transit_card_list.is_valid = 0x01;
                            g_transit_card_list.p_card_list = p_transit_card;
                            g_transit_card_list.card_num = g_card_num;

                            if (alipay_msg.func)
                            {
                                alipay_msg.func(alipay_msg.type, (void *)NULL);
                            }
                        }
                    }
                }
            }
            else if (alipay_msg.type == ALIPAY_MSG_GET_CARD_LIST_ONLINE)
            {
                APP_PRINT_INFO0("[Alipay] ALIPAY_MSG_GET_CARD_LIST_ONLINE ");
                /*check android tcp agent ready*/
                if (alipay_get_network_status() == BT_STATUS_CONNECTED)// agent enable
                {
//                    g_transit_card_list.is_valid = 0x00;
//                    g_transit_card_list.p_card_list = NULL;
//                    g_transit_card_list.card_num = 0;

                    //get card list online
                    if (p_transit_card != NULL)
                    {
                        //g_transit_card_list.is_valid = 0x00;
                        //g_transit_card_list.status = TRANSIT_STATUS_AGENT_ENABLE;

                        uint32_t card_len =  sizeof(alipay_tansit_CardBaseVO_t) * TRANSIT_CARD_NUM;
                        uint32_t g_card_num = TRANSIT_CARD_NUM;
                        retval_e ret = alipay_transit_get_card_list_online(p_transit_card, &card_len, &g_card_num);
                        APP_PRINT_INFO1("[Alipay][TRANSIT] card_num %d", g_card_num);

                        if (ret != 0)
                        {
                            APP_PRINT_INFO1("[alipay][transit] alipay_transit_get_card_list_online failed, ret %d", ret);

                            if (ret == 0x07/*RV_NETWORK_ERROR*/)
                            {
                                g_transit_card_list.status = TRANSIT_STATUS_NETWORK_ERROR;
                            }
                            else if (ret == 26 /*RV_SERVER_FAIL_ERROR*/)
                            {
                                g_transit_card_list.status = TRANSIT_STATUS_CARD_NUM_EXPIRED;
                            }
                            else if (ret == 15 /*RV_BUF_TOO_SHORT*/)
                            {
                                g_transit_card_list.status = TRANSIT_STATUS_BUFFER_TOO_SHORT;
                            }
                            else/*other unknown error*/
                            {
                                g_transit_card_list.status = TRANSIT_STATUS_LIST_OTHER_ERR;
                            }
//                            g_transit_card_list.error_code = ret;
//                            g_transit_card_list.is_valid = 0x00;
//                            g_transit_card_list.p_card_list = NULL;
//                            g_transit_card_list.card_num = 0;
                        }
                        else
                        {
                            //send msg back to gui
                            g_transit_card_list.is_valid = 0x01;
                            g_transit_card_list.p_card_list = p_transit_card;
                            g_transit_card_list.card_num = g_card_num;
                            g_transit_card_list.status = TRANSIT_STATUS_ONLINE_SUCCESS;
                            g_transit_card_list.error_code = 0;
                        }

                        if (alipay_msg.func)
                        {
                            alipay_msg.func(alipay_msg.type, NULL);
                        }
                    }
                }
//#if (CONFIG_ALIPAY_TRANSIT == 1)

                else//agent disable
                {
                    //send msg back to gui
//                    g_transit_card_list.is_valid = 0x00;
//                    g_transit_card_list.p_card_list = NULL;
//                    g_transit_card_list.card_num = 0;

                    g_transit_card_list.status = TRANSIT_STATUS_AGENT_DISABLE;

                    if (alipay_msg.func)
                    {
                        alipay_msg.func(alipay_msg.type, (void *)NULL);
                    }
                }
//#endif
            }

            else if (alipay_msg.type == ALIPAY_MSG_GET_DEFAULT_TRANSIT_CODE)
            {
                uint32_t index = alipay_msg.u.data;
                APP_PRINT_INFO2("[alipay] ALIPAY_MSG_GET_DEFAULT_TRANSIT_CODE index %d, g_default_transitCode.is_valid %d",
                                index, g_default_transitCode.is_valid);

//              EXTERNC retval_e alipay_transit_get_the_last_transitCode(PARAM_OUT char* title,
//                                                                       PARAM_OUT uint8_t* transitcode,
//                                                                       PARAM_INOUT uint32_t* len_transitcode,
//                                                                       PARAM_OUT char* error_message,
//                                                                       PARAM_IN uint32_t len_error_message);
//              transitCode_default_len = (ALIPAY_QRCODEGEN_BUFFER_LEN_MAX > 512)? ALIPAY_QRCODEGEN_BUFFER_LEN_MAX: 512;
//              DBG_DIRECT("[alipay] alipay_transit_get_the_last_transitCode, cardNo %s, cardType: %s", p_transit_card[index].cardNo, p_transit_card[index].cardType);
//              retval_e default_ret = alipay_transit_get_the_last_transitCode(p_transit_card[index].cardNo, p_transit_card[index].cardType,
//                                                                       (uint8_t *)p_transitCode, &transitCode_default_len, errot_msg, err_msg_len);


                if (g_default_transitCode.is_valid == 0x01)//valid
                {
//                  g_default_transitCode.is_valid = 0x01;
//                  g_default_transitCode.index = index;
//                  g_default_transitCode.p_transitCode = (uint8_t*)p_transitCode;
//                  g_default_transitCode.transitCode_len = transitCode_default_len;
//                  g_default_transitCode.p_default_card = &p_transit_card[index];
                    if (alipay_msg.func)
                    {
                        alipay_msg.func(alipay_msg.type, (void *)NULL);
                    }
                }
                else
                {
                    g_default_transitCode.is_valid = 0x00;
                    g_default_transitCode.index = ~0;
                    g_default_transitCode.p_transitCode = NULL;
                    g_default_transitCode.transitCode_len = 0;
                    g_default_transitCode.p_default_card = NULL;

                    T_ALIPAY_MSG alipay_msg_new = {.type = ALIPAY_MSG_GET_DEFAULT_TRANSIT_LIST, .u.data = index, .func = alipay_msg.func};
                    alipay_send_msg_to_alipay_task(&alipay_msg_new);
                }
            }

            else if (alipay_msg.type == ALIPAY_MSG_GET_TRANSIT_CODE)
            {
                bool is_need_update = false;
                uint32_t index = alipay_msg.u.data;
                APP_PRINT_INFO1("[alipay] ALIPAY_MSG_GET_TRANSIT_CODE index %d", index);

                /*1. check card exist*/
                alipay_transit_card_status_t card_status = {0};
                retval_e ret = alipay_transit_check_card_status(p_transit_card[index].cardNo,
                                                                p_transit_card[index].cardType, &card_status);
                if (card_status.is_exists ==  false)
                {
                    //not exist and update
                    is_need_update = true;
                }
                else//exist
                {
                    if ((card_status.remain_use_count <= 0) || (card_status.expire_timestamp <= 0))
                    {
                        //connect internet and update
                        is_need_update = true;
                    }
                }

                if (is_need_update)
                {
#if 0//(CONFIG_ALIPAY_TRANSIT == 1)
                    //check internet ready
                    if (alipay_get_network_status() == BT_STATUS_DISCONNECT)

                    {
                        g_default_transitCode.status = TRANSIT_STATUS_AGENT_DISABLE;//g_default_transitCode.sta
                        g_default_transitCode.is_valid = 0x00;
                        g_default_transitCode.index = ~0;
                        g_default_transitCode.p_transitCode = NULL;
                        g_default_transitCode.transitCode_len = 0;
                        g_default_transitCode.p_default_card = NULL;
                        APP_PRINT_INFO0("[alipay] ALIPAY_MSG_GET_TRANSIT_CODE TCP_STATUS_DISABLE");

                        if (alipay_msg.func)
                        {
                            alipay_msg.func(alipay_msg.type, NULL);
                        }
                        continue;
                    }
#endif //#if (CONFIG_ALIPAY_TRANSIT == 1)
                }
                //else
                {
                    g_default_transitCode.status = TRANSIT_STATUS_INVALID;
                    transitCode_len = (ALIPAY_QRCODEGEN_BUFFER_LEN_MAX > 512) ? ALIPAY_QRCODEGEN_BUFFER_LEN_MAX : 512;
                    retval_e res = alipay_transit_get_TransitCode(p_transit_card[index].cardNo,
                                                                  p_transit_card[index].cardType, (uint8_t *)p_transitCode, &transitCode_len, errot_msg, err_msg_len);
                    AliPay_LOG("[alipay] alipay_transit_get_TransitCode res %d, transitCode_len %d", res,
                               transitCode_len);
                    if (res == 0)
                    {
                        g_default_transitCode.status = TRANSIT_STATUS_CARD_SUCCESS;
                        g_default_transitCode.is_valid = 0x01;
                        g_default_transitCode.index = index;
                        g_default_transitCode.p_transitCode = (uint8_t *)p_transitCode;
                        g_default_transitCode.transitCode_len = transitCode_len;
                        g_default_transitCode.p_default_card = &p_transit_card[index];

                    }
                    else
                    {
                        if (res == 7/*RV_NETWORK_ERROR*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_NETWORK_ERROR;//g_default_transitCode.sta
                        }
                        else if (res == 54 /*RV_BUSCARDDATA_INVALID*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_BUSCARDDATA_INVALID;
                        }
                        else if (res == 26 /*RV_SERVER_FAIL_ERROR*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_SERVER_FAIL;
                        }
                        else if (res == 15 /*RV_BUF_TOO_SHORT*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_BUF_TOO_SHORT;//
                        }
                        else if (res == 43 /*RV_UNSUPPORTED_CARD*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_UNSUPPORTED;
                        }
                        else if (res == 40/*RV_CARD_DATA_LIMITED*/)
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_DATA_LIMITED;//g_default_transitCode.sta
                        }
                        else //unknown error
                        {
                            g_default_transitCode.status = TRANSIT_STATUS_CARD_OTHER_UNKNOWN_ERROR;
                        }
                        g_default_transitCode.is_valid = 0x00;
                        g_default_transitCode.index = ~0;
                        g_default_transitCode.p_transitCode = NULL;
                        g_default_transitCode.transitCode_len = 0;
                        g_default_transitCode.p_default_card = NULL;

                    }
                    if (alipay_msg.func)
                    {
                        alipay_msg.func(alipay_msg.type, NULL);
                    }
                }
            }
            else if (ALIPAY_MSG_TRANSIT_CODE_CHECK_UPDATE == alipay_msg.type)
            {
                if (g_transit_card_list.is_valid == 0x01)
                {
                    alipay_tansit_CardBaseVO_t *p_cardlist =  g_transit_card_list.p_card_list;

                    for (uint8_t index  = 0; index < g_transit_card_list.card_num; index ++)
                    {
                        bool l_is_need_update = false;
                        alipay_transit_card_status_t card_status = {0};
                        EXTERNC retval_e alipay_transit_check_card_status(PARAM_IN char *cardNo, PARAM_IN char *cardType,
                                                                          PARAM_OUT alipay_transit_card_status_t *status);
                        retval_e ret = alipay_transit_check_card_status(p_cardlist[index].cardNo,
                                                                        p_cardlist[index].cardType, &card_status);
                        APP_PRINT_INFO3("[alipay] ALIPAY_MSG_TRANSIT_CODE_CHECK_UPDATE index %d card_status.is_exists %d, card_status.remain_use_count %d",
                                        index, card_status.is_exists, card_status.remain_use_count);
                        if ((ret == 0x00) && (card_status.remain_use_count < 5))
                        {
                            l_is_need_update = true;
                        }

                        if (card_status.is_exists == 0x00)
                        {
                            l_is_need_update = true;
                        }

                        if (l_is_need_update)
                        {
                            EXTERNC retval_e alipay_transit_update_card_data(PARAM_IN char *cardNo, PARAM_IN char *cardType,
                                                                             PARAM_OUT char *error_message, PARAM_IN uint32_t len_error_message);
                            retval_e update_ret = alipay_transit_update_card_data(p_cardlist[index].cardNo,
                                                                                  p_cardlist[index].cardType, errot_msg, err_msg_len);
                            if (update_ret == 0)
                            {

                            }
                            else if (update_ret == 0x05/*RV_NETWORK_ERROR(5)*/)
                            {

                            }
                            APP_PRINT_INFO1("[alipay] ALIPAY_MSG_TRANSIT_CODE_CHECK_UPDATE update_ret %d", update_ret);
                        }


                    }
                }
            }
            else if (ALIPAY_MSG_TRANSIT_SLIENT_UPDATE == alipay_msg.type)
            {
                //try to get transit list online
                T_ALIPAY_MSG alipay_msg0 = {.type = ALIPAY_MSG_GET_CARD_LIST_ONLINE, .u.data = 0};
                alipay_send_msg_to_alipay_task(&alipay_msg0);

                //try to get transit list online
                T_ALIPAY_MSG alipay_msg1 = {.type = ALIPAY_MSG_TRANSIT_CODE_CHECK_UPDATE, .u.data = 0};
                alipay_send_msg_to_alipay_task(&alipay_msg1);
            }
#endif //CONFIG_ALIPAY_TRANSIT
            else if (ALIPAY_MSG_DEVICE_UNBIND == alipay_msg.type)
            {
                //unbind alipay device
                g_alipay_unbind_flag = false;
                if (0/*RV_OK*/ == alipay_unbinding())
                {
                    g_alipay_unbind_flag = true;
                    memset(&g_transit_card_list, 0, sizeof(g_transit_card_list));
                    memset(&g_default_transitCode, 0, sizeof(g_default_transitCode));

                    g_alipay_status.binded = 0x00;
                    g_alipay_status.status = 0xff;
                    g_alipay_status.triggle = false;
                }
                else
                {
                    g_alipay_unbind_flag = false;
                }
            }
        }
    }

//    (void)csi_free(p_transit_card);
//    (void)csi_free(p_transitCode);
}

alipay_device_status_ *alipay_task_get_bind_status(void)
{
    //g_alipay_status.done = false;
    return &g_alipay_status;
}

uint8_t *alipay_task_get_paycode_status(void)
{
    //g_alipay_status.done = false;
    return &alipay_paycode_ready;
}

#if CONFIG_ALIPAY_TRANSIT
bool alipay_task_get_transit_card_list(t_alipay_transit_list *p_transit_stg)
{
    if (p_transit_stg == 0)
    {
        return false;
    }

    if (g_transit_card_list.is_valid == 0x01)
    {
        memcpy(p_transit_stg, &g_transit_card_list, sizeof(g_transit_card_list));
    }
    else
    {
        memcpy(p_transit_stg, &g_transit_card_list, sizeof(g_transit_card_list));
//      memset(p_transit_stg, 0, sizeof(t_alipay_transit_list));
//      p_transit_stg->status = g_transit_card_list.status;
        return false;
    }

    return true;
}

bool alipay_task_get_transit_online_card_list(t_alipay_transit_list *p_transit_stg)
{
    if (p_transit_stg == 0)
    {
        return false;
    }

    if ((g_transit_card_list.is_valid == 0x01) &&
        (g_transit_card_list.status == TRANSIT_STATUS_ONLINE_SUCCESS))
    {
        memcpy(p_transit_stg, &g_transit_card_list, sizeof(g_transit_card_list));
    }
    else
    {
        //memset(p_transit_stg, 0, sizeof(t_alipay_transit_list));
        p_transit_stg->status = g_transit_card_list.status;
        return false;
    }

    return true;
}

bool alipay_task_get_default_transit_code(t_alipay_transitCode *p_transitCode)
{
    if (g_default_transitCode.is_valid)
    {
        APP_PRINT_INFO0("[Alipay] alipay_task_get_default_transit_code default card success!");
        memcpy(p_transitCode, &g_default_transitCode, sizeof(g_default_transitCode));
        return g_default_transitCode.is_valid;
    }
    APP_PRINT_INFO1("[Alipay] alipay_task_get_default_transit_code g_default_transitCode.is_valid %d!",
                    g_default_transitCode.is_valid);
    return false;
}

bool alipay_task_get_transitCode(uint32_t index, t_alipay_transitCode *p_transitCode)
{

    if (p_transitCode == NULL)
    {
        APP_PRINT_INFO0("[Alipay] alipay_transit_get_TransitCode p_transitCode null!");
        return false;
    }

    if (g_default_transitCode.is_valid == 0x00)
    {
        APP_PRINT_INFO0("[Alipay] alipay_transit_get_TransitCode not valid!");
        memset(p_transitCode, 0, sizeof(g_default_transitCode));
        p_transitCode->status = g_default_transitCode.status;
        return false;
    }

    if (index != g_default_transitCode.index)
    {
        APP_PRINT_INFO0("[Alipay] alipay_transit_get_TransitCode index error!");
        memset(p_transitCode, 0, sizeof(g_default_transitCode));
        return false;
    }

    //if (g_default_transitCode.is_valid == 0x01)
    {
        memcpy(p_transitCode, &g_default_transitCode, sizeof(g_default_transitCode));
    }
    return (p_transitCode->is_valid);
}

uint8_t alipay_task_check_local_card_list_exist(void)
{
    uint32_t card_len =  sizeof(alipay_tansit_CardBaseVO_t) * TRANSIT_CARD_NUM;
    uint32_t g_card_num = TRANSIT_CARD_NUM;
    retval_e ret = alipay_transit_get_card_list_offline(p_transit_card, &card_len, &g_card_num);
    APP_PRINT_INFO2("[Alipay][TRANSIT] alipay_task_check_local_card_list_exist ret %d, card_num %d",
                    ret, g_card_num);

    return ret;
}

/**
 * transit silent update
*/
uint8_t alipay_task_transit_slient_update(void)
{
    //try to get transit list online
    T_ALIPAY_MSG alipay_msg = {.type = ALIPAY_MSG_TRANSIT_SLIENT_UPDATE, .u.data = 0};
    if (true == alipay_send_msg_to_alipay_task(&alipay_msg))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
#endif // CONFIG_ALIPAY_TRANSIT
uint8_t alipay_task_unbind_device(void)
{
    return g_alipay_unbind_flag;
}

/** @} */ /* End of group PERIPH_APP_TASK */
/** @} */ /* End of group PERIPH_DEMO */

#endif // CONFIG_ALIPAY
