/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "os_sched.h"
#include "system_status_api.h"
#include "hal_adp.h"
#include "app_dlps.h"
#include "app_cfg.h"
#include "app_cfg_nv.h"
#include "app_main.h"
#include "app_io_msg.h"
#include "app_adp.h"

#if (CONFIG_SOC_SERIES_RTL8773D == 1 || TARGET_RTL8773DFL == 1)
#include "pmu_api.h"
#endif

#if F_APP_IO_OUTPUT_SUPPORT
#include "app_io_output.h"
#endif


/** @defgroup  APP_ADP  App Adp
  * @brief
  * @{
  */
/**/

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup APP_ADP_Exported_Variables App adp Variables
    * @{
    */
static uint8_t adaptor_plug_in = ADAPTOR_UNPLUG;

/** End of Exported_Variables
    * @}
    */
///@endcond

/*============================================================================*
 *                              Functions
 *============================================================================*/


static void app_adp_wakeup_pol_set(void)
{
    adp_wake_up_enable(ADP_WAKE_UP_GENERAL);
}

uint8_t app_adp_get_plug_state(void)
{
    return adaptor_plug_in;
}

void app_adp_detect(void)
{
    T_ADP_STATE adp_state = ADP_STATE_UNKNOWN;

    while (adp_get_current_state(ADP_DETECT_5V) == ADP_STATE_DETECTING)
    {
        os_delay(10);
    }

    adp_state = adp_get_current_state(ADP_DETECT_5V);

    if (adp_state == ADP_STATE_IN)
    {
        adaptor_plug_in = ADAPTOR_PLUG;

        if (!app_cfg_nv.adaptor_is_plugged)
        {
            app_cfg_nv.adaptor_is_plugged = 1;
        }
    }
    else if (adp_state == ADP_STATE_OUT)
    {
        adaptor_plug_in = ADAPTOR_UNPLUG;

        if (app_cfg_nv.adaptor_is_plugged)
        {
            app_cfg_nv.adaptor_is_plugged = 0;
        }
    }
    app_adp_wakeup_pol_set();
}

static void app_adp_plug_handle(void)
{
#if F_APP_IO_OUTPUT_SUPPORT
    if (app_cfg_const.enable_power_supply_adp_in)
    {
        app_io_output_power_supply(true);
    }
#endif
}

static void app_adp_unplug_handle(void)
{
#if F_APP_IO_OUTPUT_SUPPORT
    if (app_cfg_const.enable_power_supply_adp_in)
    {
        if (app_db.device_state != APP_DEVICE_STATE_ON)
        {
            app_io_output_power_supply(false);
        }
    }
#endif

}

void app_adp_msg_handler(uint16_t type)
{
    switch (type)
    {
    case IO_MSG_GPIO_ADAPTOR_PLUG:
        app_adp_plug_handle();
        break;

    case IO_MSG_GPIO_ADAPTOR_UNPLUG:
        app_adp_unplug_handle();
        break;
    }
}

/* Callback run in timer task, should not run too long */
static void app_adp_stage_change_cb(T_ADP_PLUG_EVENT status, void *user_data)
{
    //uint32_t adp_type = (uint32_t)user_data;
    T_IO_MSG adp_msg;
    bool adp_msg_handle = false;

#if (CONFIG_SOC_SERIES_RTL8773D == 1 || TARGET_RTL8773DFL == 1)
    if (app_cfg_const.thermistor_power_vpa_support)
    {
        if (status == ADP_EVENT_PLUG_IN)
        {
            pmu_ldo_pa_control(true);
        }
        else if (status == ADP_EVENT_PLUG_OUT)
        {
            pmu_ldo_pa_control(false);
        }
    }
#endif

    // adp msg
    adp_msg.type = IO_MSG_TYPE_GPIO;

    switch (status)
    {
    case ADP_EVENT_PLUG_IN:
        {
            if (app_cfg_nv.adaptor_is_plugged == 0)
            {
                adp_msg_handle = true;
                app_cfg_nv.adaptor_is_plugged = 1;
                adaptor_plug_in = ADAPTOR_PLUG;

                adp_msg.subtype = IO_MSG_GPIO_ADAPTOR_PLUG;

                app_adp_wakeup_pol_set();
            }
        }
        break;

    case ADP_EVENT_PLUG_OUT:
        {
            if (app_cfg_nv.adaptor_is_plugged == 1)
            {
                adp_msg_handle = true;
                app_cfg_nv.adaptor_is_plugged = 0;
                adaptor_plug_in = ADAPTOR_UNPLUG;

                adp_msg.subtype = IO_MSG_GPIO_ADAPTOR_UNPLUG;

                //Should be able to power on during power off ing after debounce timeout
                //Keep adaptor_is_plugged to correct condition
                if (app_db.device_state == APP_DEVICE_STATE_OFF_ING)
                {
                    app_cfg_nv.adaptor_is_plugged = 1;
                }
            }
        }
        break;

    default:
        break;
    }

    if (adp_msg_handle)
    {
        app_io_msg_send(&adp_msg);
    }

}

void app_adp_init(void)
{
    adp_wake_up_enable(ADP_WAKE_UP_GENERAL);
    adp_register_state_change_cb(ADP_DETECT_5V, app_adp_stage_change_cb, (void *)ADP_DETECT_5V);
}
