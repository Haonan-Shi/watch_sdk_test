/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/
#include "app_module_init.h"
#include "bridge_music_player.h"
#include "bridge_now_playing.h"
#include "bridge_recording.h"
#include "bridge_phone_call.h"
#include "bridge_bt_control.h"
#if CONFIG_WALKIE_TALKIE
#include "bridge_intercom.h"
#endif

/*============================================================================*
 *                            Macros
 *============================================================================*/

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                           Constants
 *============================================================================*/

/*============================================================================*
 *                            Variables
 *============================================================================*/


/*============================================================================*
 *                           Private Functions
 *============================================================================*/

static void bridge_module_init(void)
{
    bridge_music_player_init();
    bridge_now_playing_init();
    bridge_recording_init();
    bridge_phone_call_init();
    bridge_bt_control_init();
#if CONFIG_WALKIE_TALKIE
    bridge_intercom_init();
#endif
}

APP_MODULE_INIT(bridge_module_init);
