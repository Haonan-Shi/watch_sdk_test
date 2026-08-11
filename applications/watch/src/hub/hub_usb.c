/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_SUPPORT_USB
#include "board.h"
#include "trace.h"
#include "os_timer.h"
#include "os_mem.h"
#include "os_sync.h"
#include "hub_clock.h"
#include "hub_display.h"
#include "hub_task.h"
#include "hub_usb.h"
#include "app_dlps.h"
#include "module_usb.h"
#include "hub_usb.h"
//#include "usb_lib_ext.h"
#include "app_audio_if.h"
#include "app_mmi.h"
#include "os_sched.h"
#include "module_adaptor.h"
#include "playback_playlist.h"
#include "usb_if.h"
#include "app_usb.h"
#include "app_task.h"
#include "app_cfg.h"
#include "usb_app_api.h"

static const T_IO_MODULE_CBS usb_detect_cbs =
{
    wristband_usb_driver_init,
    wristband_usb_enter_dlps,
    wristband_usb_exit_dlps,
    wristband_usb_system_wakeup_dlps_check,
};

void usb_detect_add_hub_task(void)
{
    driver_modules[wristband_sensor_hub_count++] = (T_IO_MODULE_CBS *)&usb_detect_cbs;
}

void wristband_usb_driver_init(void)
{

}

void wristband_usb_enter_dlps(void)
{

}
void wristband_usb_exit_dlps(void)
{

}

bool wristband_usb_system_wakeup_dlps_check(void)
{
    return false;
}

void usb_detect_event_handler(T_IO_MSG msg)
{
    switch (msg.subtype)
    {
    case USB_MSG_ADP_IN:
        if (app_audio_cfg.support_local_source && app_db.usb_status == USB_STOPPED &&
            usb_detect_port_type() == USB_CHARGE_DATA_PORT)
        {
            app_dlps_disable(APP_DLPS_ENTER_CHECK_USB);

            T_IO_MSG play_msg;
            play_msg.type = IO_MSG_TYPE_WRISTBNAD;
            play_msg.subtype = IO_MSG_PREPARE_USB_ENVIRONMENT;
            app_send_msg_to_apptask(&play_msg); //app_db.usb_status will be set in app task
            os_sem_take(usb_sem_handle, 0xffffffff);
#if USE_LIB_V3
            Usb_Start();
#else
            int ret = usb_if_start();
            if (ret != 0 && ret != 1) // start failed
            {
                app_db.usb_status = USB_STOPPED;
            }
#endif
        }
        break;

    case USB_MSG_ADP_OUT:
        if (app_audio_cfg.support_local_source && app_db.usb_status == USB_STARTED)
        {
#if USE_LIB_V3
            Usb_Stop();
#else
            usb_if_stop();
#endif
            app_db.usb_status = USB_STOPPED;

            T_IO_MSG play_msg;
            play_msg.type = IO_MSG_TYPE_WRISTBNAD;
            play_msg.subtype = IO_MSG_HANDLE_USB_PLUG_OUT;
            app_send_msg_to_apptask(&play_msg);  //app_db.usb_status will be set in app task
            os_sem_take(usb_sem_handle, 0xffffffff);

            app_dlps_enable(APP_DLPS_ENTER_CHECK_USB);
        }
        break;

    default:
        break;
    }
}
#endif
