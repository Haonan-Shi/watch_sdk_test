/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_APP_GUI
#include "app_panel.h"
#include "draw_font.h"
#if F_GUI_SIMPLE_SPEED_DEMO
#include "app_simple_speed_demo.h"
#endif
#if F_GUI_SDCARD_LIST_DEMO
#include "app_sdcard_list_demo.h"
#endif
#if F_GUI_CHARGEBOX_DEMO
#include "app_chargebox.h"
#endif
#include "gui_components_init.h"


#if F_GUI_SIMPLE_SPEED_DEMO
GUI_INIT_APP_EXPORT(app_simple_speed_ui_design);
#elif F_GUI_SDCARD_LIST_DEMO
GUI_INIT_APP_EXPORT(app_sdcard_list_ui_design);
#elif F_GUI_CHARGEBOX_DEMO
GUI_INIT_APP_EXPORT(app_chargebox_ui_design);
#endif

#endif
