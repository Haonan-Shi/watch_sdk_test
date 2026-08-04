/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/*
 * Test scenario: measure board current at 40/100/200 MHz CPU in DLPS mode.
 * CPU wakes every 66 ms, runs for 500 us, then returns to DLPS.
 *
 * Decoupling model (same as other  modules):
 *   Shell thread -> event_bus_publish(dlps/cmd/ *)
 *   App task     <- event_bus_subscribe_async -> executes PM/BT API calls
 *   Test thread  <- spawned by app task, runs the 66ms/500us wake cycle
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <zephyr/drivers/rtc.h>
#include <zephyr/sys/timeutil.h>


#include "event_bus.h"
#include "app_task.h"
#include "app_module_init.h"
#include "app_dlps.h"
#include "app_bt_policy_api.h"
#include "app_bt_policy_int.h"
#include "app_ble_adv.h"
#include "pm.h"
#include "fmc_api_ext.h"
#include "platform_utils.h"
#include "trace.h"

#include "dlps.h"
#include "section.h"
#include "wdg.h"

/*
 * Silence / restore all trace.h logging (the DBG_BUFFER / APP_PRINT_* path).
 * trace_mask[] is a per-level (LEVEL_NUM) 64-bit module bitmap; clearing every
 * module bit on every level disables the lot. Restore reloads the boot-time
 * default mask from sys_init_cfg via log_module_trace_init(NULL).
 *
 * NOTE: this does NOT affect DBG_DIRECT (log_direct bypasses the mask) nor the
 * Zephyr printk() calls in this file -- both still print.
 */
static void dlps_trace_off(void)
{
    for (uint8_t lvl = 0; lvl < LEVEL_NUM; lvl++)
    {
        log_module_bitmap_trace_set((uint64_t)~0ULL, lvl, false);
    }
}

static void dlps_trace_on(void)
{
    log_module_trace_init(NULL);
}

/* ---------- test thread ---------- */

#define DLPS_STACK_SIZE   1024
#define DLPS_PRIORITY     10
#define DLPS_PRINT_CNT       2

/*
 * Wake period. The old implementation used k_sleep(K_USEC(65500)), but with a
 * 100 Hz tick and tickless disabled, k_sleep quantizes to 10 ms, so 65.5 ms
 * became a 70 ms sleep. We instead arm an RTC comparator (32 kHz, ~31 us
 * resolution) that survives DLPS and wakes the CPU precisely.
 */
#define DLPS_WAKE_PERIOD_US   65500u

/*
 * RTC comparator channel used as the DLPS wake source. Channel 0 is reserved
 * for the calendar clock (hub_clock); the "shell" snippet overlay bumps the RTC
 * "channels" property to 2 so the driver's shared ISR also services this
 * channel. This wake source is mandatory in DLPS: the HW/system timers stop, so
 * the k_sleep fallback below cannot wake the CPU and the test would hang.
 */
#define DLPS_RTC_ALARM_CHAN   1

/* Full absolute-time match (sub-second via the NSEC field). */
#define DLPS_RTC_ALARM_MASK                                                   \
    (RTC_ALARM_TIME_MASK_SECOND | RTC_ALARM_TIME_MASK_MINUTE |                   \
     RTC_ALARM_TIME_MASK_HOUR | RTC_ALARM_TIME_MASK_MONTHDAY |                   \
     RTC_ALARM_TIME_MASK_MONTH | RTC_ALARM_TIME_MASK_YEAR |                      \
     RTC_ALARM_TIME_MASK_NSEC)

K_THREAD_STACK_DEFINE(s_dlps_stack, DLPS_STACK_SIZE);
static struct k_thread  s_dlps_thread;
static volatile bool    s_test_active;
static bool             s_thread_created;
static uint32_t         s_cpu_mhz;
static int32_t enable_enter_dlps_print_cnt = DLPS_PRINT_CNT;
static int32_t enable_exit_dlps_print_cnt = DLPS_PRINT_CNT;

static const struct device *const s_rtc_dev = DEVICE_DT_GET(DT_NODELABEL(rtc));
static struct k_sem     s_dlps_sem;
static bool             s_rtc_ready;

/* Arm the RTC alarm channel to fire DLPS_WAKE_PERIOD_US from "now". */
static int dlps_rtc_arm_next(void)
{
    struct rtc_time now;
    int err = rtc_get_time(s_rtc_dev, &now);
    if (err)
    {
        return err;
    }

    /* target = now + wake period, normalized through epoch seconds */
    uint32_t nsec = (uint32_t)now.tm_nsec + DLPS_WAKE_PERIOD_US * 1000u;
    time_t   sec  = timeutil_timegm((struct tm *)&now);
    sec  += nsec / 1000000000u;
    nsec  = nsec % 1000000000u;

    struct tm tm_tgt;
    gmtime_r(&sec, &tm_tgt);

    struct rtc_time alarm = {0};
    alarm.tm_sec  = tm_tgt.tm_sec;
    alarm.tm_min  = tm_tgt.tm_min;
    alarm.tm_hour = tm_tgt.tm_hour;
    alarm.tm_mday = tm_tgt.tm_mday;
    alarm.tm_mon  = tm_tgt.tm_mon;
    alarm.tm_year = tm_tgt.tm_year;
    alarm.tm_wday = tm_tgt.tm_wday;
    alarm.tm_yday = tm_tgt.tm_yday;
    alarm.tm_nsec = nsec;

    return rtc_alarm_set_time(s_rtc_dev, DLPS_RTC_ALARM_CHAN,
                              DLPS_RTC_ALARM_MASK, &alarm);
}

/*
 * RTC alarm callback (runs in the RTC ISR context via the Zephyr RTC driver).
 * Re-arm for the next period right here: the underlying rtl87x3g driver
 * auto-re-arms this channel with a now-stale time just before invoking us, so
 * overwriting it immediately with the correct next-period alarm leaves no
 * window for a spurious early wake. Then release the test thread.
 */
static void dlps_rtc_alarm_cb(const struct device *dev, uint16_t id, void *user_data)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(id);
    ARG_UNUSED(user_data);

    (void)dlps_rtc_arm_next();
    k_sem_give(&s_dlps_sem);
}

/*
 * Print "<tag> [t=<sec>.<usec>]" using the always-on RTC as the time source.
 * The RTC (32 kHz) keeps running through DLPS and is independent of the CPU
 * frequency, so consecutive timestamps let you read the wake interval directly:
 * subtract two values as (sec2-sec1)*1e6 + (usec2-usec1) microseconds.
 */
RAM_TEXT_SECTION
static void dlps_print_ts(const char *tag)
{
    struct rtc_time t;
    uint32_t sec = 0, usec = 0;

    if (s_rtc_ready && rtc_get_time(s_rtc_dev, &t) == 0)
    {
        sec  = (uint32_t)timeutil_timegm((struct tm *)&t);
        usec = (uint32_t)(t.tm_nsec / 1000u);
    }

    printk("%s [t=%u.%06u]\r\n", tag, (unsigned)sec, (unsigned)usec);
}

RAM_TEXT_SECTION
static void dlps_enter_callback(void)
{

// #if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
//         fmc_psram_enter_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
// #endif
// #if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
//         fmc_psram_enter_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
// #endif
    if (enable_enter_dlps_print_cnt > 0)
    {
        printk("app_dlps_enter_callback cpu clk %dmhz\r\n", s_cpu_mhz);
        enable_enter_dlps_print_cnt --;
    }
}

RAM_TEXT_SECTION
static void dlps_exit_callback(void)
{
// #if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
//         fmc_psram_exit_lpm(FMC_SPIC_ID_1, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
// #endif
// #if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
//         fmc_psram_exit_lpm(FMC_SPIC_ID_3, FMC_PSRAM_LPM_DEEP_POWER_DOWN_MODE);
// #endif
    if (enable_exit_dlps_print_cnt > 0)
    {
        printk("app_dlps_exit_callback cpu clk %dmhz\r\n", s_cpu_mhz);
        enable_exit_dlps_print_cnt --;
    }
}

RAM_TEXT_SECTION
static void dlps_test_fn(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    /* Arm the first period; the alarm callback re-arms every subsequent one. */
    bool use_rtc = s_rtc_ready && (dlps_rtc_arm_next() == 0);

    printk("[dlps] test thread: cpu=%u MHz, period=%ums(500us active), wake=%s\n",
           (unsigned)s_cpu_mhz,
           (unsigned)((DLPS_WAKE_PERIOD_US + 500u) / 1000u),
           use_rtc ? "rtc-alarm" : "k_sleep");

    while (s_test_active)
    {
        platform_delay_us(500);        /* 500 us active CPU run */
        // platform_delay_us(500000);
        if (use_rtc)
        {
            /*
             * Block with no kernel timeout so the idle path can enter DLPS;
             * the always-on RTC alarm wakes us precisely at the period and its
             * callback has already armed the next one.
             */
            k_sem_take(&s_dlps_sem, K_FOREVER);
        }
        else
        {
            /* Fallback (RTC unavailable): tick-quantized sleep. */
            k_sleep(K_USEC(DLPS_WAKE_PERIOD_US));
        }
    }

    printk("[dlps] test thread stopped\n");
}

/*
 * Stop the test thread (if running) and WAIT for it to actually terminate
 * before returning. k_thread_join blocks the caller until the target thread
 * is dead, so the static k_thread/stack can be safely reused or left idle.
 * Must be called from a thread context (here: the app task) -- never an ISR.
 */
static void dlps_stop_test(void)
{
    if (s_thread_created)
    {
        s_test_active = false;
        /* If the thread is blocked on the RTC wake, release it so it can exit. */
        k_sem_give(&s_dlps_sem);
        k_thread_join(&s_dlps_thread, K_FOREVER);
        s_thread_created = false;

        if (s_rtc_ready)
        {
            /* Cancel the alarm (mask 0 disables the channel) and detach cb. */
            rtc_alarm_set_time(s_rtc_dev, DLPS_RTC_ALARM_CHAN, 0, NULL);
            rtc_alarm_set_callback(s_rtc_dev, DLPS_RTC_ALARM_CHAN, NULL, NULL);
        }
    }
}

static void dlps_start_test(uint32_t cpu_mhz)
{
    /* stop previous test (if any) and wait until it has truly terminated */
    dlps_stop_test();

    uint32_t actual_mhz = 0;
    int err = pm_cpu_freq_set(cpu_mhz, &actual_mhz);
    if (err != 0)
    {
        printk("[dlps] pm_cpu_freq_set(%u) err=%d\n", (unsigned)cpu_mhz, err);
    }
    s_cpu_mhz = (actual_mhz > 0) ? actual_mhz : cpu_mhz;
    printk("[dlps] CPU freq: req=%u actual=%u MHz\n",
           (unsigned)cpu_mhz, (unsigned)s_cpu_mhz);

    if (s_rtc_ready)
    {
        k_sem_reset(&s_dlps_sem);
        rtc_alarm_set_callback(s_rtc_dev, DLPS_RTC_ALARM_CHAN,
                               dlps_rtc_alarm_cb, NULL);
    }

    s_test_active = true;
    k_thread_create(&s_dlps_thread, s_dlps_stack,
                    K_THREAD_STACK_SIZEOF(s_dlps_stack),
                    dlps_test_fn, NULL, NULL, NULL,
                    DLPS_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&s_dlps_thread, "dlps");
    s_thread_created = true;
}

/* ---------- event_bus app-task callback ---------- */

static T_EVENT_BUS_SUBSCRIBER_HANDLE s_cmd_h;

static int32_t dlps_cmd_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    const char *t = ev->topic;

    if (strcmp(t, EVENT_BUS_TOPIC_DLPS_CMD_ENTER) == 0)
    {
        /* Disable Legacy BT */
        app_bt_policy_event_handle(EVENT_BT_IDLE, NULL);

        /* Stop BLE advertising */
        app_ble_common_adv_stop(0);

        /* BT MAC into deep sleep */
        bt_power_mode_set(BTPOWER_DEEP_SLEEP);

        /* Platform DLPS mode */
        power_mode_set(POWER_DLPS_MODE);

        /* Silence all trace.h logging for the duration of the DLPS test */
        // dlps_trace_off();

        printk("[dlps] enter: BT disabled, BLE adv stopped, DLPS mode active\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_DLPS_CMD_EXIT) == 0)
    {
        /* Destroy the test thread (wait until it has terminated) */
        dlps_stop_test();

        app_dlps_disable(APP_DLPS_ENTER_CHECK_SHELL_APP);

        /* Restore the boot-time trace mask (re-enable trace.h logging) */
        dlps_trace_on();

        /* Enable Legacy BT */
        app_bt_policy_set_enabled(true);

        printk("[dlps] exit: test thread destroyed, DLPS mode inactive\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_DLPS_CMD_RUN) == 0)
    {
        app_dlps_disable(APP_DLPS_ENTER_CHECK_SHELL_APP);
        if (ev->data == NULL || ev->data_len < sizeof(T_DLPS_RUN_DATA))
        {
            return EVENT_BUS_OK;
        }
        T_DLPS_RUN_DATA *p = (T_DLPS_RUN_DATA *)ev->data;
        dlps_start_test(p->cpu_mhz);

        enable_enter_dlps_print_cnt = DLPS_PRINT_CNT;
        enable_exit_dlps_print_cnt = DLPS_PRINT_CNT;

        app_dlps_enable(APP_DLPS_ENTER_CHECK_SHELL_APP);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_DLPS_CMD_STOP) == 0)
    {
        /* Stop and destroy the test thread (wait until it has terminated) */
        dlps_stop_test();

        uint32_t actual = 0;
        pm_cpu_freq_set(200, &actual);

        /* Re-block DLPS so normal app behavior resumes */
        app_dlps_disable(APP_DLPS_ENTER_CHECK_SHELL_APP);

        printk("[dlps] stop: test stopped, CPU->40MHz, DLPS blocked\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_DLPS_CMD_DEEP_SLEEP) == 0)
    {
        enable_enter_dlps_print_cnt = DLPS_PRINT_CNT;
        enable_exit_dlps_print_cnt = DLPS_PRINT_CNT;

        app_dlps_enable(APP_DLPS_ENTER_CHECK_SHELL_APP);
    }

    return EVENT_BUS_OK;
}

/* ---------- shell handlers ---------- */

static int cmd_dlps_enter(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_DLPS_CMD_ENTER, NULL, 0);
    shell_print(sh, "dlps: enter DLPS test scenario (queued to app task)");
    return 0;
}

static int cmd_dlps_exit(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_DLPS_CMD_EXIT, NULL, 0);
    shell_print(sh, "dlps: exit DLPS test scenario (queued to app task)");
    return 0;
}

static int cmd_dlps_run_mhz(const struct shell *sh, uint32_t mhz)
{
    T_DLPS_RUN_DATA d = { .cpu_mhz = mhz };
    event_bus_publish(EVENT_BUS_TOPIC_DLPS_CMD_RUN, &d, sizeof(d));
    shell_print(sh, "dlps: start %u MHz DLPS test (queued to app task)", (unsigned)mhz);
    return 0;
}

static int cmd_dlps_40m(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    return cmd_dlps_run_mhz(sh, 40);
}

static int cmd_dlps_100m(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    return cmd_dlps_run_mhz(sh, 100);
}

static int cmd_dlps_200m(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    return cmd_dlps_run_mhz(sh, 200);
}

static int cmd_dlps_stop(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_DLPS_CMD_STOP, NULL, 0);
    shell_print(sh, "dlps: stop (queued to app task)");
    return 0;
}

static int cmd_dlps_status(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);

    uint32_t cur_mhz = pm_cpu_freq_get();
    uint32_t wakeup_count = 0, total_wakeup_ms = 0, total_sleep_ms = 0;
    power_get_statistics(&wakeup_count, &total_wakeup_ms, &total_sleep_ms);

    shell_print(sh, "dlps status:");
    shell_print(sh, "  test active   : %s", s_test_active ? "yes" : "no");
    shell_print(sh, "  cpu mhz (set) : %u", (unsigned)s_cpu_mhz);
    shell_print(sh, "  cpu mhz (cur) : %u", (unsigned)cur_mhz);
    shell_print(sh, "  dlps wakeups  : %u", (unsigned)wakeup_count);
    shell_print(sh, "  total active  : %u ms", (unsigned)total_wakeup_ms);
    shell_print(sh, "  total sleep   : %u ms", (unsigned)total_sleep_ms);
    return 0;
}

static int cmd_dlps_wakeup(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);

    app_dlps_disable(APP_DLPS_ENTER_CHECK_SHELL_APP);
    shell_print(sh, "wakeup cpu from dlps");
    return 0;
}

static int cmd_dlps_deep_sleep(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);

    event_bus_publish(EVENT_BUS_TOPIC_DLPS_CMD_DEEP_SLEEP, NULL, 0);
    shell_print(sh, "wakeup cpu from dlps");
    return 0;
}



SHELL_STATIC_SUBCMD_SET_CREATE(dlps_subcmds,
                               SHELL_CMD(enter,  NULL, "Enter DLPS test: BT off, BLE adv stop, DLPS mode on", cmd_dlps_enter),
                               SHELL_CMD(40m,    NULL, "Start 40 MHz CPU DLPS test (66ms/500us cycle)",        cmd_dlps_40m),
                               SHELL_CMD(100m,   NULL, "Start 100 MHz CPU DLPS test (66ms/500us cycle)",       cmd_dlps_100m),
                               SHELL_CMD(200m,   NULL, "Start 200 MHz CPU DLPS test (66ms/500us cycle)",       cmd_dlps_200m),
                               SHELL_CMD(deep_sleep,    NULL, "Start deep sleep",
                                         cmd_dlps_deep_sleep),
                               SHELL_CMD(stop,   NULL, "Stop test, restore CPU to 40 MHz",                     cmd_dlps_stop),
                               SHELL_CMD(status, NULL, "Show DLPS statistics",                                 cmd_dlps_status),
                               SHELL_CMD(wakeup, NULL, "Wakeup from DLPS before send another cmd",             cmd_dlps_wakeup),
                               SHELL_CMD(exit,   NULL, "Exit DLPS test",                                       cmd_dlps_exit),
                               SHELL_SUBCMD_SET_END
                              );
SHELL_CMD_REGISTER(dlps, &dlps_subcmds,
                   "DLPS power-consumption test", NULL);

/* ---------- module init ---------- */

static void dlps_module_init(void)
{
    /* app_dlps_init should NOT invoke in main.c*/

    /* Block DLPS by default until enter is called */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_SHELL_APP);

    /* Precise DLPS wake source: always-on RTC comparator (channel 1). */
    k_sem_init(&s_dlps_sem, 0, 1);
    s_rtc_ready = device_is_ready(s_rtc_dev);
    if (!s_rtc_ready)
    {
        printk("[dlps] RTC not ready; falling back to k_sleep (10ms-quantized)\n");
    }

    event_bus_topic_register(EVENT_BUS_TOPIC_DLPS_ALL_TOPIC);
    event_bus_subscribe_async(&s_cmd_h, EVENT_BUS_TOPIC_DLPS_ALL_TOPIC,
                              event_bus_async_send_to_apptask, NULL, dlps_cmd_cb);


    power_stage_cb_register(dlps_enter_callback, POWER_STAGE_STORE);
    power_stage_cb_register(dlps_exit_callback, POWER_STAGE_RESTORE);
}
APP_MODULE_INIT(dlps_module_init);
