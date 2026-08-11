/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "event_bus.h"
#include "app_task.h"
#include "app_module_init.h"
#include "app_bt_policy_api.h"
#include "app_bt_policy_int.h"
#include "app_ble_adv.h"
#include "app_cfg.h"

#include "ble_throughput.h"

#define ADV_DATA_MAX_LE_NAME_LEN  12

static uint8_t adv_data[31] =
{
    /* Core spec. Vol. 3, Part C, Chapter 18 */
    /* Flags */
    /* place holder for Local Name, filled by BT stack. if not present */
    /* BT stack appends Local Name.                                    */
    0x02,            /* length     */
    GAP_ADTYPE_FLAGS,
    GAP_ADTYPE_FLAGS_GENERAL,
    /* Service */
    /* Alipay Service */
    0x03,           /* length     */
    0x03,           /* type="More 16-bit UUIDs available, service uuid 0xFEE7 0xA00A" */
    0x0D, //0x0D
    0xA0, //0xA0
    /* Manufacture specified data*/
    0x09,           /* length     */
    0xFF,           /* type: manufacture specific data*/
    0xC5, 0xFE,     /* company id */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mac address*/

    0x00,           /* length     */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,           /* type="Complete local name" */
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' /* SmartBracelet */
};

/* ---------- BLE adv work (runs in app task context) ---------- */

/*
 * These touch the BLE adv manager / GAP state machine, whose state
 * (le_common_adv.state) is owned and updated by the BT/app task callbacks.
 * They MUST run in the app task, never in the shell thread, otherwise the
 * shell thread races the BT task and app_ble_common_adv_start() can observe a
 * stale state and skip the actual re-enable (adv never recovers on re-entry).
 */
static void ble_throughput_do_enter(void)
{
    uint8_t le_name_len;
    uint8_t bt_bd_addr[6];

    gap_get_param(GAP_PARAM_BD_ADDR, bt_bd_addr);

    le_name_len = strlen((char *)app_cfg_const.device_name_le_default);

    adv_data[11] = bt_bd_addr[5];
    adv_data[12] = bt_bd_addr[4];
    adv_data[13] = bt_bd_addr[3];
    adv_data[14] = bt_bd_addr[2];
    adv_data[15] = bt_bd_addr[1];
    adv_data[16] = bt_bd_addr[0];

    adv_data[17] = le_name_len + 1;
    if (le_name_len >= ADV_DATA_MAX_LE_NAME_LEN)
    {
        adv_data[17] = ADV_DATA_MAX_LE_NAME_LEN + 1;
    }

    memset(&adv_data[19], 0x00, ADV_DATA_MAX_LE_NAME_LEN);
    memcpy(&adv_data[19], app_cfg_nv.device_name_le, adv_data[17] - 1);

    le_set_gap_param(GAP_PARAM_DEVICE_NAME, adv_data[17] - 1, &adv_data[19]);

    ble_ext_adv_mgr_set_adv_data(le_common_adv.adv_handle, sizeof(adv_data), adv_data);

    app_ble_common_adv_start(0);
}

static void ble_throughput_do_exit(void)
{
    app_ble_common_adv_stop(0);
}

/* ---------- event_bus app-task callback ---------- */

static T_EVENT_BUS_SUBSCRIBER_HANDLE s_cmd_h;

static int32_t ble_throughput_cmd_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    const char *t = ev->topic;

    if (strcmp(t, EVENT_BUS_TOPIC_BLE_THROUGHPUT_CMD_ENTER) == 0)
    {
        ble_throughput_do_enter();
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_BLE_THROUGHPUT_CMD_EXIT) == 0)
    {
        ble_throughput_do_exit();
    }

    return EVENT_BUS_OK;
}

/* ---------- shell handlers (shell thread, only publish) ---------- */

static int cmd_ble_throughput_enter(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);

    event_bus_publish(EVENT_BUS_TOPIC_BLE_THROUGHPUT_CMD_ENTER, NULL, 0);

    shell_print(sh, "Enter BLE throughput test. Please test on phone with app: [Throughput Test]");
    return 0;
}

static int cmd_ble_throughput_exit(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);

    event_bus_publish(EVENT_BUS_TOPIC_BLE_THROUGHPUT_CMD_EXIT, NULL, 0);

    shell_print(sh, "Exit BLE throughput test");
    return 0;
}


SHELL_STATIC_SUBCMD_SET_CREATE(ble_throughput_subcmds,
                               SHELL_CMD(enter,  NULL, "Enter BLE throughput test. Please test on phone",
                                         cmd_ble_throughput_enter),
                               SHELL_CMD(exit,   NULL, "Exit BLE throughput test.",                       cmd_ble_throughput_exit),
                               SHELL_SUBCMD_SET_END
                              );
SHELL_CMD_REGISTER(ble_throughput, &ble_throughput_subcmds,
                   "BLE throughput test", NULL);

/* ---------- module init ---------- */

static void ble_throughput_module_init(void)
{
    /* Block DLPS by default until enter is called */

    /* Decoupling: shell publishes, app task runs the BLE adv calls */
    event_bus_topic_register(EVENT_BUS_TOPIC_BLE_THROUGHPUT_ALL_TOPIC);
    event_bus_subscribe_async(&s_cmd_h, EVENT_BUS_TOPIC_BLE_THROUGHPUT_ALL_TOPIC,
                              event_bus_async_send_to_apptask, NULL, ble_throughput_cmd_cb);
}
APP_MODULE_INIT(ble_throughput_module_init);
