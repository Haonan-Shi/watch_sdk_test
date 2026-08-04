/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/*
 * Decoupling model (per spec):
 *   - Shell handlers run on the SHELL thread and ONLY event_bus_publish(...) to
 *     this module's own "a2dp_src/cmd/ *" topic.
 *   - That topic is subscribed ASYNC (event_bus_async_send_to_apptask), so the
 *     command callback runs on the APP TASK, where it calls the real public BT /
 *     audio APIs directly -- the same ones bridge_bt_control.c uses:
 *       app_bt_inquiry_start/stop, app_bt_policy_connect_bredr/disconnect_bredr,
 *       app_bt_policy_set_enabled, app_audio_mode_switch, app_audio_start/stop.
 *   - State (inquiry results, headphone conn/disconn, audio play status) is
 *     received via event_bus subscriptions (app_gap / app_bt_policy / app_audio
 *     publish these regardless of the GUI) and cached for `list`/`status`.
 *   - Module init auto-registers at boot via APP_MODULE_INIT (no main edits).
 *
 * Note: the GUI bridge (bridge_bt_control) is NOT compiled in the build, so
 * this module must drive the BT APIs itself rather than forwarding to the
 * bt_control/cmd/ * topics (which would have no subscriber).
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

#include "event_bus.h"
#include "app_task.h"             /* event_bus_async_send_to_apptask                 */
#include "app_module_init.h"      /* APP_MODULE_INIT                                 */
#include "app_main.h"             /* T_APP_AUDIO_MODE, MODE_APP_A2DP_SRC             */
#include "app_audio_if.h"         /* app_audio_*, T_APP_AUDIO_STATE, audio/pl_st     */
#include "app_audio_mode_switch.h"/* app_audio_mode_switch                          */
#include "app_bt_policy_api.h"    /* app_bt_policy_set_enabled/connect/disconnect +
                                     bt_evt headphone topics                         */
#include "app_gap.h"              /* app_bt_inquiry_start/stop + bt_evt inquiry topics*/
#include "app_bond.h"
#include "app_mmi.h"

#include "a2dp_src.h"

/* Mirror of T_SEARCH_RESULT (defined in app_gap.c, not a header): payload of
 * EVENT_BUS_TOPIC_BT_EVT_INQUIRY_RESULT. Layout must match the publisher. */
typedef struct
{
    uint8_t  bd_addr[6];
    uint8_t  nam_len;
    uint16_t device_name[25];   /* UTF-16 */
    uint32_t cod;
} T_A2DP_SEARCH_RESULT;

#define A2DP_MAX_DEVICES   16

K_MUTEX_DEFINE(s_lock);

static T_A2DP_SEARCH_RESULT s_dev[A2DP_MAX_DEVICES];
static uint8_t            s_dev_count;
static bool               s_scan_done;
static bool               s_hp_connected;
static uint8_t            s_conn_addr[6];     /* addr of the currently connected headphone */
static bool               s_have_conn;
static uint8_t            s_target_addr[6];   /* last addr we issued connect to */
static bool               s_have_target;
static T_APP_AUDIO_STATE  s_play_state = APP_AUDIO_STATE_STOP;

static T_EVENT_BUS_SUBSCRIBER_HANDLE s_cmd_h;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_evt_h;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_audio_h;

/* ---------- helpers ---------- */

static bool parse_mac(const char *s, uint8_t out_be[6])
{
    unsigned int v[6];
    if (sscanf(s, "%x:%x:%x:%x:%x:%x", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) != 6)
    {
        return false;
    }
    for (int i = 0; i < 6; i++)
    {
        out_be[i] = (uint8_t)v[i];
    }
    return true;
}

static void cache_add(const T_A2DP_SEARCH_RESULT *r)
{
    k_mutex_lock(&s_lock, K_FOREVER);
    for (uint8_t i = 0; i < s_dev_count; i++)
    {
        if (memcmp(s_dev[i].bd_addr, r->bd_addr, 6) == 0)
        {
            s_dev[i] = *r;                 /* refresh existing entry */
            k_mutex_unlock(&s_lock);
            return;
        }
    }
    if (s_dev_count < A2DP_MAX_DEVICES)
    {
        s_dev[s_dev_count++] = *r;
    }
    k_mutex_unlock(&s_lock);
}

/* ---------- event_bus state callbacks (sync; run in publisher/app-task ctx) ---------- */

static int32_t evt_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    const char *t = ev->topic;
    if (strcmp(t, EVENT_BUS_TOPIC_BT_EVT_INQUIRY_RESULT) == 0)
    {
        if (ev->data && ev->data_len >= sizeof(T_A2DP_SEARCH_RESULT))
        {
            cache_add((const T_A2DP_SEARCH_RESULT *)ev->data);
        }
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_BT_EVT_INQUIRY_CMPL) == 0)
    {
        s_scan_done = true;
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_CONN) == 0)
    {
        s_hp_connected = true;
        /* app_bt_policy publishes the connected earphone's bd_addr as payload;
         * record it so `list`/`status` can show which device is connected and
         * so play/disconnect have a default target without a prior `connect`. */
        if (ev->data && ev->data_len >= 6)
        {
            const uint8_t *a = (const uint8_t *)ev->data;
            k_mutex_lock(&s_lock, K_FOREVER);
            memcpy(s_conn_addr, a, 6);
            s_have_conn = true;
            memcpy(s_target_addr, a, 6);
            s_have_target = true;
            k_mutex_unlock(&s_lock);
            printk("[a2dp_src] headphone connected: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   a[5], a[4], a[3], a[2], a[1], a[0]);
        }
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_DISCONN) == 0)
    {
        s_hp_connected = false;
        s_have_conn = false;
    }
    return EVENT_BUS_OK;
}

static int32_t audio_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    if (ev->data && ev->data_len >= sizeof(T_APP_AUDIO_STATE))
    {
        s_play_state = *(const T_APP_AUDIO_STATE *)ev->data;
    }
    return EVENT_BUS_OK;
}

/* ---------- gm command callback (async; runs on the APP TASK) ----------
 * Here we call the real public BT/audio APIs directly (same as bridge_bt_control). */

static int32_t cmd_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    const char *t = ev->topic;

    if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_ENTER) == 0)
    {
        app_bond_bt_bond_clear();
        app_bt_policy_set_enabled(true);            /* re-arm BR/EDR in case it was disabled at boot */
        app_audio_mode_switch(MODE_APP_A2DP_SRC);   /* switch role/SDP to A2DP source */
        printk("[a2dp_src] enter: BR/EDR enabled, audio mode -> A2DP_SRC\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_EXIT) == 0)
    {
        app_bt_policy_set_enabled(false);
        printk("[a2dp_src] enter: BR/EDR disabled\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_SCAN_ON) == 0)
    {
        app_bt_inquiry_start();
        printk("[a2dp_src] scan: inquiry started\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_SCAN_OFF) == 0)
    {
        app_bt_inquiry_stop();
        printk("[a2dp_src] scan: inquiry stopped\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_CONNECT) == 0)
    {
        if (ev->data == NULL || ev->data_len < sizeof(T_A2DP_ADDR_DATA))
        {
            return EVENT_BUS_OK;
        }
        T_A2DP_ADDR_DATA *p = (T_A2DP_ADDR_DATA *)ev->data;
        /* must save to cache before connect earphone */
        app_bt_bond_temp_cache_save_to_search();
        app_bt_policy_connect_bredr(p->bd_addr);    /* connects A2DP as ROLE_SRC + AVRCP */
        printk("[a2dp_src] connect: %02x:%02x:%02x:%02x:%02x:%02x\n",
               p->bd_addr[5], p->bd_addr[4], p->bd_addr[3],
               p->bd_addr[2], p->bd_addr[1], p->bd_addr[0]);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_DISCONNECT) == 0)
    {
        if (ev->data == NULL || ev->data_len < sizeof(T_A2DP_ADDR_DATA))
        {
            return EVENT_BUS_OK;
        }
        T_A2DP_ADDR_DATA *p = (T_A2DP_ADDR_DATA *)ev->data;
        app_bt_policy_disconnect_bredr(p->bd_addr);
        printk("[a2dp_src] disconnect: %02x:%02x:%02x:%02x:%02x:%02x\n",
               p->bd_addr[5], p->bd_addr[4], p->bd_addr[3],
               p->bd_addr[2], p->bd_addr[1], p->bd_addr[0]);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_PLAY) == 0)
    {
        app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
        printk("[a2dp_src] play: app_audio_start -> state 0x%x\n", app_audio_get_play_status());

    }
    else if (strcmp(t, EVENT_BUS_TOPIC_A2DP_CMD_STOP) == 0)
    {
        app_mmi_handle_action(MMI_AV_STOP);
        printk("[a2dp_src] stop -> state 0x%x\n", app_audio_get_play_status());
    }
    return EVENT_BUS_OK;
}

/* ---------- shell handlers (SHELL thread; publish only) ---------- */

static int cmd_enter(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_ENTER, NULL, 0);
    shell_print(sh, "a2dp_src: enter A2DP-source test scenario (queued to app task)");
    return 0;
}

static int cmd_exit(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_EXIT, NULL, 0);
    shell_print(sh, "a2dp_src: exit A2DP-source test scenario (queued to app task)");
    return 0;
}


static int cmd_scan(const struct shell *sh, size_t argc, char **argv)
{
    bool on = !(argc >= 2 && strcmp(argv[1], "off") == 0);
    if (on)
    {
        k_mutex_lock(&s_lock, K_FOREVER);
        s_dev_count = 0;
        s_scan_done = false;
        k_mutex_unlock(&s_lock);
        event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_SCAN_ON, NULL, 0);
        shell_print(sh, "a2dp_src: inquiry start queued (then 'a2dp_src list')");
    }
    else
    {
        event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_SCAN_OFF, NULL, 0);
        shell_print(sh, "a2dp_src: inquiry stop queued");
    }
    return 0;
}

static int cmd_list(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    k_mutex_lock(&s_lock, K_FOREVER);
    uint8_t n = s_dev_count;
    shell_print(sh, "a2dp_src: %u device(s)%s", n, s_scan_done ? " (scan done)" : "");
    for (uint8_t i = 0; i < n; i++)
    {
        char name[26];
        uint8_t j;
        for (j = 0; j < s_dev[i].nam_len && j < 25; j++)
        {
            uint16_t ch = s_dev[i].device_name[j];
            name[j] = (ch >= 0x20 && ch < 0x7f) ? (char)ch : '.';
        }
        name[j] = '\0';
        const uint8_t *a = s_dev[i].bd_addr;
        shell_print(sh, "  [%u] %02x:%02x:%02x:%02x:%02x:%02x  cod=0x%06x  %s",
                    i, a[5], a[4], a[3], a[2], a[1], a[0], (unsigned)s_dev[i].cod, name);
    }
    k_mutex_unlock(&s_lock);
    return 0;
}

static int cmd_connect(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2)
    {
        shell_error(sh, "usage: a2dp_src connect <idx|aa:bb:cc:dd:ee:ff>");
        return -EINVAL;
    }

    T_A2DP_ADDR_DATA d;
    if (strchr(argv[1], ':') != NULL)
    {
        uint8_t be[6];
        if (!parse_mac(argv[1], be))
        {
            shell_error(sh, "bad MAC: %s", argv[1]);
            return -EINVAL;
        }
        /* displayed/typed form is big-endian; bd_addr is little-endian */
        for (int i = 0; i < 6; i++)
        {
            d.bd_addr[i] = be[5 - i];
        }
    }
    else
    {
        int idx = atoi(argv[1]);
        k_mutex_lock(&s_lock, K_FOREVER);
        if (idx < 0 || idx >= (int)s_dev_count)
        {
            k_mutex_unlock(&s_lock);
            shell_error(sh, "bad index %d (have %u)", idx, s_dev_count);
            return -EINVAL;
        }
        memcpy(d.bd_addr, s_dev[idx].bd_addr, 6);
        k_mutex_unlock(&s_lock);
    }

    memcpy(s_target_addr, d.bd_addr, 6);
    s_have_target = true;
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_CONNECT, &d, sizeof(d));
    shell_print(sh, "a2dp_src: connect requested (queued to app task)");
    return 0;
}

static int cmd_play(const struct shell *sh, size_t argc, char **argv)
{
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_PLAY, NULL, 0);
    shell_print(sh, "a2dp_src: play the preload mp3 music");
    return 0;
}

static int cmd_stop(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_STOP, NULL, 0);
    shell_print(sh, "a2dp_src: stop (queued to app task)");
    return 0;
}

static int cmd_disconnect(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    if (!s_have_target)
    {
        shell_error(sh, "no target; run 'a2dp_src connect' first");
        return -EINVAL;
    }
    T_A2DP_ADDR_DATA d;
    memcpy(d.bd_addr, s_target_addr, 6);
    event_bus_publish(EVENT_BUS_TOPIC_A2DP_CMD_DISCONNECT, &d, sizeof(d));
    shell_print(sh, "a2dp_src: disconnect requested (queued to app task)");
    return 0;
}

static int cmd_status(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    k_mutex_lock(&s_lock, K_FOREVER);
    shell_print(sh, "a2dp_src status:");
    shell_print(sh, "  devices cached : %u%s", s_dev_count, s_scan_done ? " (scan done)" : "");
    shell_print(sh, "  headphone conn : %s", s_hp_connected ? "yes" : "no");
    if (s_have_conn)
    {
        const uint8_t *a = s_conn_addr;
        shell_print(sh, "  connected addr : %02x:%02x:%02x:%02x:%02x:%02x",
                    a[5], a[4], a[3], a[2], a[1], a[0]);
    }
    shell_print(sh, "  play state     : 0x%02x", s_play_state);
    if (s_have_target)
    {
        const uint8_t *a = s_target_addr;
        shell_print(sh, "  target addr    : %02x:%02x:%02x:%02x:%02x:%02x",
                    a[5], a[4], a[3], a[2], a[1], a[0]);
    }
    k_mutex_unlock(&s_lock);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(a2dp_src_subcmds,
                               SHELL_CMD(enter,      NULL, "Enter A2DP-source test scenario (BR/EDR on, mode->SRC)", cmd_enter),
                               SHELL_CMD(exit,       NULL, "Exit  A2DP-source test scenario (BR/EDR off)",           cmd_exit),
                               SHELL_CMD(scan,       NULL, "Scan BT devices: a2dp_src scan [on|off]",                 cmd_scan),
                               SHELL_CMD(list,       NULL, "List discovered devices (indexed)",                      cmd_list),
                               SHELL_CMD(connect,    NULL, "Connect headphone: a2dp_src connect <idx|MAC>",           cmd_connect),
                               SHELL_CMD(play,       NULL, "Play local music: a2dp_src play [file|synth]",            cmd_play),
                               SHELL_CMD(stop,       NULL, "Stop playback",                                          cmd_stop),
                               SHELL_CMD(disconnect, NULL, "Disconnect the A2DP link",
                                         cmd_disconnect),
                               SHELL_CMD(status,     NULL, "Print cached BT + audio state",                          cmd_status),
                               SHELL_SUBCMD_SET_END
                              );
SHELL_CMD_REGISTER(a2dp_src, &a2dp_src_subcmds,
                   "A2DP-source play-music test (CONFIG_SHELL_APP_A2DP_SRC)", NULL);

/* ---------- module init (auto-called at boot via APP_MODULE_INIT) ---------- */

static void a2dp_src_module_init(void)
{
    /* register is idempotent: harmless if the owner already registered the topic */
    event_bus_topic_register(EVENT_BUS_TOPIC_A2DP_ALL_TOPIC);
    event_bus_topic_register(EVENT_BUS_TOPIC_BT_EVT_ALL_TOPIC);
    event_bus_topic_register(EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED);

    /* commands -> async -> app task (real BT/audio APIs called there) */
    event_bus_subscribe_async(&s_cmd_h, EVENT_BUS_TOPIC_A2DP_ALL_TOPIC,
                              event_bus_async_send_to_apptask, NULL, cmd_cb);
    /* state -> sync cache update */
    event_bus_subscribe(&s_evt_h,   EVENT_BUS_TOPIC_BT_EVT_ALL_TOPIC,            evt_cb);
    event_bus_subscribe(&s_audio_h, EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED,   audio_cb);

}
APP_MODULE_INIT(a2dp_src_module_init);
