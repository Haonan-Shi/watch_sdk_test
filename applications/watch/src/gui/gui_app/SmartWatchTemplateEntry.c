/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "gui_api.h"
#include "gui_view.h"
#include "gui_components_init.h"
#include "gui_vfs.h"
#include "hg_romfs.h"
#include "gui_message.h"
#include "app_phone_user.h"

extern const struct romfs_dirent app_romfs_root;

static int app_init(void)
{
    // Mount romfs from embedded data
    // gui_vfs_mount_romfs("/", &app_romfs_root, 0);

    gui_view_create(gui_obj_get_root(), "SmartWatchTemplateMainView", 0, 0, 0, 0);

    /* Register persistent phone-call state subscription on the GUI root so
     * incoming-call view switches still happen when no phone view is active. */
    app_phone_user_init();

    gui_log("app_init called");
    gui_set_keep_active_time(60000);

    return 0;
}

GUI_INIT_APP_EXPORT(app_init);
