/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_ALIPAY

#include "app_module_init.h"
#include "app_ble_service_info.h"
#include "alipay_task.h"
#include "alipay_pan.h"
#include "upay_service.h"
#include "alipay_ble_transport.h"
#include "alipay_bind.h"
#include "gap_conn_le.h"           /* le_get_conn_id_by_handle */
#include "app_dlps.h"              /* app_dlps_disable */
#include "clk_mgr.h"               /* clk_mgr_user_create / set_high_performance */
#include "trace.h"
#include <zephyr/sys/util.h>       /* BIT() */

T_SERVER_ID upay_gatt_srv_id = 0xFF;
static T_CLK_USER_HANDLE clk_user_alipay;

static T_APP_RESULT app_upay_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_UPAYS_CALLBACK_DATA *p_upay_cb_data = (T_UPAYS_CALLBACK_DATA *)p_data;
    APP_PRINT_INFO1("upay_gatt_svc_callback:  msg_type %d", type);
    switch (type)
    {
    case GATT_MSG_UPAY_SERVER_READ:
        break;

    case GATT_MSG_UPAY_SERVER_WRITE:
        {
            void alipay_ble_recv_data_handle(const uint8_t *data,  uint32_t len);
            APP_PRINT_INFO2("[AliPay] cb len %d,data %b", p_upay_cb_data->msg_data.write.len,
                            TRACE_BINARY(p_upay_cb_data->msg_data.write.len, p_upay_cb_data->msg_data.write.p_value));
            alipay_ble_recv_data_handle(p_upay_cb_data->msg_data.write.p_value,
                                        (uint32_t)p_upay_cb_data->msg_data.write.len);
        }
        break;

    case GATT_MSG_UPAY_SERVER_CCCD_UPDATE:
        {
            APP_PRINT_INFO0("upay_gatt_svc_callback SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION");
        }
        break;

    default:
        break;
    }

    return app_result;
}

void app_upay_data_send_cb(T_EXT_SEND_DATA_RESULT result)
{
    if (result.cause == GAP_SUCCESS)
    {
        uint8_t conn_id = 0xFF;
        le_get_conn_id_by_handle(result.conn_handle, &conn_id);
        alipay_ble_send_completed_proc(conn_id);
    }
    APP_PRINT_INFO1("app_upay_data_send_cb result.cause %d", result.cause);
}

static void alipay_module_init(void)
{
#if CONFIG_ALIPAY_TRANSIT
    alipay_pan_init();
#endif
    upay_gatt_srv_id = upay_reg_srv(app_upay_gatt_svc_callback, app_upay_data_send_cb);
    alipay_task_init();

    /* TODO(alipay-ui): permanently disabling DLPS at module init is a
     * debug-time workaround that increases standby current. Once the UI
     * lands, switch to disable/enable on alipay UI entry/exit so non-alipay
     * usage can still enter DLPS.
     *
     * Root cause: when DLPS engages, PSRAM enters half-sleep, IO power
     * switches to _LQ, and I2C2 registers have no save/restore (the HAL
     * does not provide one). The first alipay I2C op after DLPS therefore
     * NACKs. While the screen is on the DISPLAY bit blocks DLPS so the
     * issue is hidden; during shell debug, after the screen blanks at 60s
     * DLPS engages and I2C_MasterRead reproducibly returns errcode 7.
     * Holding the ALIPAY bit blocks DLPS and avoids the issue entirely. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_ALIPAY);

    /* TODO(alipay-ui): same lifetime story as the DLPS hold above -
     * once the alipay UI lands, move this to disable/enable on entry/exit
     * so the SoC can drop back to 40MHz when alipay is not in use.
     *
     * Why pin high performance here:
     *   clk_mgr_init (app_lower_init.c) leaves CPU at normal=40MHz, and
     *   the only voter that ever asks for high in this codebase is the
     *   honeygui clk_user registered by gui_port_dc_init. That gives two
     *   broken modes for alipay:
     *     - alipay-only build (no honeygui snippet): nobody votes high,
     *       CPU stays at 40MHz forever; SM2/TLS run ~5x too slow and
     *       transit code retrieval times out / SDK watchdogs trigger.
     *     - alipay + honeygui build: gui votes high while the LCD is on
     *       but flips to normal the moment the screen blanks, which
     *       drops CPU back to 40MHz mid-transit. Reproduces as "transit
     *       works with screen on, fails with screen off".
     *   Voting high from alipay keeps the SoC at 200MHz independent of
     *   screen state and snippet selection.
     *
     * Bitmap mirrors gui_port_dc.c (CPU | SPIC1). SPIC0/SPIC3 are pinned
     * high by clk_mgr_init unconditionally - no voter ever lowers them -
     * so we don't need to claim them here. */
    U_CLK_BITMAP bm;
    bm.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC1);
    clk_user_alipay = clk_mgr_user_create("alipay", bm);
    clk_mgr_set_high_performance(clk_user_alipay);
}
APP_MODULE_INIT(alipay_module_init);

APP_BLE_SERVICE_INFO(alipay, 1);

#endif // CONFIG_ALIPAY