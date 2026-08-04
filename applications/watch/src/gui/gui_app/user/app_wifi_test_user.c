/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_wifi_test_user.h"
#include "gui_img.h"
#include "gui_obj.h"
#include "gui_rect.h"
#include "gui_text.h"
#include "gui_view.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if CONFIG_WIFI
#define CONFIG_WIFI_TEST  1
#endif

#if CONFIG_WIFI_TEST == 1
#include "wifi_gui.h"
#endif

#define WIFI_IP_SEGMENT_COUNT 4
#define WIFI_IP_SEGMENT_LEN 3
#define WIFI_IP_STRING_LEN 16

typedef struct
{
    char segments[WIFI_IP_SEGMENT_COUNT][WIFI_IP_SEGMENT_LEN + 1];
    uint8_t activeSegment;
    bool confirmed;
} ip_input_state_t;

typedef struct
{
    bool running;
    int value;
} transfer_state_t;

static ip_input_state_t iperfInput = {0};
static ip_input_state_t fileInput = {0};
static transfer_state_t iperfDownloadState = {false, 0};
static transfer_state_t fileDownloadState = {false, 0};
static int iperfUploadSpeed = 0;
static int fileUploadData = 0;
static char iperfTargetIp[WIFI_IP_STRING_LEN] = "0.0.0.0";
static char fileTargetIp[WIFI_IP_STRING_LEN] = "0.0.0.0";
static char iperfUploadTargetText[40] = "Target: 0.0.0.0";
static char fileUploadTargetText[40] = "Target: 0.0.0.0";
static char iperfUploadSpeedText[16] = "0";
static char iperfDownloadSpeedText[16] = "0";
static char fileUploadDataText[16] = "0";
static char fileDownloadDataText[16] = "0";
static uint8_t iperfLastVisualSegment = 0;
static uint8_t fileLastVisualSegment = 0;
static bool iperfLastVisualConfirmed = false;
static bool fileLastVisualConfirmed = false;
static bool wifiConnected = true;
static bool randomSeeded = false;

static gui_color_t color_primary_text(void)
{
    return gui_rgb(242, 242, 242);
}

static gui_color_t color_secondary_text(void)
{
    return gui_rgb(153, 153, 153);
}

static gui_color_t color_tertiary_text(void)
{
    return gui_rgb(102, 102, 102);
}

static gui_color_t color_disabled_text(void)
{
    return gui_rgb(85, 85, 85);
}

static gui_color_t color_confirmed(void)
{
    return gui_rgb(76, 217, 100);
}

static gui_color_t color_iperf(void)
{
    return gui_rgb(90, 200, 250);
}

static gui_color_t color_file_upload(void)
{
    return gui_rgb(255, 149, 0);
}

static gui_color_t color_file_download(void)
{
    return gui_rgb(88, 86, 214);
}

static void seed_random_once(void)
{
    if (!randomSeeded)
    {
        srand((unsigned int)time(NULL));
        randomSeeded = true;
    }
}

static bool is_current_view(const char *viewName)
{
    gui_view_t *currentView = gui_view_get_current();

    return currentView != NULL && currentView->base.name != NULL &&
           strcmp(currentView->base.name, viewName) == 0;
}

static void set_text_content_safe(gui_text_t *textObj, const char *text)
{
    if (textObj == NULL || text == NULL)
    {
        return;
    }

    gui_text_content_set(textObj, (void *)text, (uint16_t)strlen(text));
}

static void set_text_color_safe(gui_text_t *textObj, gui_color_t color)
{
    if (textObj == NULL)
    {
        return;
    }

    gui_text_color_set(textObj, color);
}

static void set_rect_color_safe(gui_rounded_rect_t *rectObj, gui_color_t color)
{
    if (rectObj == NULL)
    {
        return;
    }

    gui_rect_set_color(rectObj, color);
}

static void set_rect_opacity_safe(gui_rounded_rect_t *rectObj, uint8_t opacity)
{
    if (rectObj == NULL)
    {
        return;
    }

    gui_rect_set_opacity(rectObj, opacity);
}

static void set_image_src_safe(gui_img_t *imageObj, const char *src)
{
    if (imageObj == NULL || src == NULL)
    {
        return;
    }

    gui_img_set_src(imageObj, (const uint8_t *)src, IMG_SRC_FILESYS);
    gui_img_refresh_size(imageObj);
}

static void reset_input_state(ip_input_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    memset(state, 0, sizeof(*state));
    state->activeSegment = 0;
    state->confirmed = false;
}

static bool segment_has_leading_zero(const char *segment)
{
    size_t length;

    if (segment == NULL)
    {
        return false;
    }

    length = strlen(segment);
    return length > 1 && segment[0] == '0';
}

static bool segment_is_valid(const char *segment, bool allowEmpty)
{
    int value;
    size_t length;

    if (segment == NULL)
    {
        return false;
    }

    length = strlen(segment);
    if (length == 0)
    {
        return allowEmpty;
    }

    if (length > WIFI_IP_SEGMENT_LEN || segment_has_leading_zero(segment))
    {
        return false;
    }

    value = atoi(segment);
    return value >= 0 && value <= 255;
}

static void build_ip_string(const ip_input_state_t *state, char *buffer, size_t bufferSize)
{
    if (buffer == NULL || bufferSize == 0)
    {
        return;
    }

    if (state == NULL)
    {
        snprintf(buffer, bufferSize, "0.0.0.0");
        return;
    }

    snprintf(buffer,
             bufferSize,
             "%s.%s.%s.%s",
             state->segments[0][0] != '\0' ? state->segments[0] : "0",
             state->segments[1][0] != '\0' ? state->segments[1] : "0",
             state->segments[2][0] != '\0' ? state->segments[2] : "0",
             state->segments[3][0] != '\0' ? state->segments[3] : "0");
}

static bool input_is_complete(const ip_input_state_t *state)
{
    int index;

    if (state == NULL)
    {
        return false;
    }

    for (index = 0; index < WIFI_IP_SEGMENT_COUNT; ++index)
    {
        if (!segment_is_valid(state->segments[index], false))
        {
            return false;
        }
    }

    return true;
}

static void begin_input_edit(ip_input_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    state->confirmed = false;
}

static void append_digit(ip_input_state_t *state, char digit)
{
    char *segment;
    size_t length;
    char candidate[WIFI_IP_SEGMENT_LEN + 1];

    if (state == NULL)
    {
        return;
    }

    begin_input_edit(state);
    segment = state->segments[state->activeSegment];
    length = strlen(segment);
    if (length >= WIFI_IP_SEGMENT_LEN)
    {
        return;
    }

    if (length == 1 && segment[0] == '0')
    {
        return;
    }

    memcpy(candidate, segment, length + 1);
    candidate[length] = digit;
    candidate[length + 1] = '\0';
    if (!segment_is_valid(candidate, false))
    {
        return;
    }

    strcpy(segment, candidate);
}

static void move_to_next_segment(ip_input_state_t *state)
{
    if (state == NULL)
    {
        return;
    }

    if (state->activeSegment >= WIFI_IP_SEGMENT_COUNT - 1)
    {
        return;
    }

    if (!segment_is_valid(state->segments[state->activeSegment], false))
    {
        return;
    }

    begin_input_edit(state);
    state->activeSegment++;
}

static void delete_input_char(ip_input_state_t *state)
{
    char *segment;
    size_t length;

    if (state == NULL)
    {
        return;
    }

    begin_input_edit(state);
    segment = state->segments[state->activeSegment];
    length = strlen(segment);
    if (length > 0)
    {
        segment[length - 1] = '\0';
        if (length == 1 && state->activeSegment > 0)
        {
            state->activeSegment--;
        }
        return;
    }

    if (state->activeSegment == 0)
    {
        return;
    }

    state->activeSegment--;
    segment = state->segments[state->activeSegment];
    length = strlen(segment);
    if (length > 0)
    {
        segment[length - 1] = '\0';
    }
}

static const char *get_obj_name(void *obj)
{
    gui_obj_t *guiObj = (gui_obj_t *)obj;

    if (guiObj == NULL)
    {
        return NULL;
    }

    return guiObj->name;
}

static char resolve_digit_from_name(void *obj, const char *prefix)
{
    const char *name = get_obj_name(obj);
    size_t prefixLength;

    if (name == NULL || prefix == NULL)
    {
        return '\0';
    }

    prefixLength = strlen(prefix);
    if (strncmp(name, prefix, prefixLength) != 0 || strncmp(name + prefixLength, "_key_", 5) != 0)
    {
        return '\0';
    }

    name += prefixLength + 5;
    return (*name >= '0' && *name <= '9') ? *name : '\0';
}

static char resolve_iperf_digit(void *obj)
{
    return resolve_digit_from_name(obj, "iperf");
}

static char resolve_file_digit(void *obj)
{
    return resolve_digit_from_name(obj, "file");
}

static void update_input_ui(const ip_input_state_t *state,
                            gui_rounded_rect_t *segmentBackgrounds[WIFI_IP_SEGMENT_COUNT],
                            gui_text_t *segments[WIFI_IP_SEGMENT_COUNT],
                            gui_text_t *dots[WIFI_IP_SEGMENT_COUNT - 1],
                            gui_text_t *confirmedText,
                            gui_text_t *confirmButton,
                            gui_text_t *startButton,
                            gui_rounded_rect_t *startButtonBg,
                            uint8_t *lastActiveSegment,
                            bool *lastConfirmed,
                            gui_color_t accentColor)
{
    int index;
    bool visualsChanged;

    if (state == NULL)
    {
        return;
    }

    visualsChanged = lastActiveSegment == NULL || lastConfirmed == NULL ||
                     *lastActiveSegment != state->activeSegment || *lastConfirmed != state->confirmed;

    for (index = 0; index < WIFI_IP_SEGMENT_COUNT; ++index)
    {
        const char *segmentText = state->segments[index][0] != '\0' ? state->segments[index] : "---";
        bool isActive = index == state->activeSegment && !state->confirmed;
        gui_color_t segmentColor = state->confirmed ? color_confirmed()
                                   : (isActive ? accentColor : color_primary_text());

        set_text_content_safe(segments[index], segmentText);
        set_text_color_safe(segments[index], segmentColor);
        if (visualsChanged)
        {
            set_rect_color_safe(segmentBackgrounds[index], isActive ? accentColor : gui_rgb(255, 255, 255));
            set_rect_opacity_safe(segmentBackgrounds[index], isActive ? 38 : 12);
        }

        if (index < WIFI_IP_SEGMENT_COUNT - 1)
        {
            set_text_color_safe(dots[index], state->confirmed ? color_confirmed() : color_disabled_text());
        }
    }

    gui_obj_hidden((gui_obj_t *)confirmedText, !state->confirmed);
    set_text_color_safe(confirmedText, color_confirmed());
    set_text_color_safe(confirmButton, gui_rgb(0, 0, 0));
    set_text_color_safe(startButton, state->confirmed ? gui_rgb(0, 0, 0) : color_disabled_text());
    if (visualsChanged)
    {
        set_rect_color_safe(startButtonBg, state->confirmed ? color_confirmed() : gui_rgb(255, 255, 255));
        set_rect_opacity_safe(startButtonBg, state->confirmed ? 255 : 15);
        *lastActiveSegment = state->activeSegment;
        *lastConfirmed = state->confirmed;
    }
}

static void update_iperf_input_ui(void)
{
    gui_rounded_rect_t *segmentBackgrounds[WIFI_IP_SEGMENT_COUNT] =
    {
        iperf_ip_seg0_bg,
        iperf_ip_seg1_bg,
        iperf_ip_seg2_bg,
        iperf_ip_seg3_bg,
    };
    gui_text_t *segments[WIFI_IP_SEGMENT_COUNT] =
    {
        iperf_ip_seg0,
        iperf_ip_seg1,
        iperf_ip_seg2,
        iperf_ip_seg3,
    };
    gui_text_t *dots[WIFI_IP_SEGMENT_COUNT - 1] =
    {
        iperf_ip_dot0,
        iperf_ip_dot1,
        iperf_ip_dot2,
    };

    update_input_ui(&iperfInput,
                    segmentBackgrounds,
                    segments,
                    dots,
                    iperf_ip_confirmed,
                    iperf_btn_confirm,
                    iperf_btn_start,
                    iperf_btn_start_bg,
                    &iperfLastVisualSegment,
                    &iperfLastVisualConfirmed,
                    color_iperf());
    gui_fb_change();
}

static void update_file_input_ui(void)
{
    gui_rounded_rect_t *segmentBackgrounds[WIFI_IP_SEGMENT_COUNT] =
    {
        file_ip_seg0_bg,
        file_ip_seg1_bg,
        file_ip_seg2_bg,
        file_ip_seg3_bg,
    };
    gui_text_t *segments[WIFI_IP_SEGMENT_COUNT] =
    {
        file_ip_seg0,
        file_ip_seg1,
        file_ip_seg2,
        file_ip_seg3,
    };
    gui_text_t *dots[WIFI_IP_SEGMENT_COUNT - 1] =
    {
        file_ip_dot0,
        file_ip_dot1,
        file_ip_dot2,
    };

    update_input_ui(&fileInput,
                    segmentBackgrounds,
                    segments,
                    dots,
                    file_ip_confirmed,
                    file_btn_confirm,
                    file_btn_start,
                    file_btn_start_bg,
                    &fileLastVisualSegment,
                    &fileLastVisualConfirmed,
                    color_file_upload());
    gui_fb_change();
}

static void update_wifi_home_ui(void)
{
    set_image_src_safe(wifi_home_wifi_icon,
                       wifiConnected ? "/app_wifi_test/icon_wifi_connected.bin"
                       : "/app_wifi_test/icon_wifi_disconnected.bin");
    set_text_content_safe(wifi_home_status_text, wifiConnected ? "Connected" : "Not Connected");
    set_text_color_safe(wifi_home_status_text, wifiConnected ? color_secondary_text() : gui_rgb(255, 59,
                        48));
    set_text_content_safe((gui_text_t *)wifi_home_ip_text,
                          wifiConnected ? "IP: 192.168.1.100" : "IP: ---.---.---.---");
    set_text_color_safe((gui_text_t *)wifi_home_ip_text, color_primary_text());
}

static void update_upload_target(gui_text_t *targetLabel,
                                 char *buffer,
                                 size_t bufferSize,
                                 const char *prefix,
                                 const char *ip)
{
    if (buffer == NULL || bufferSize == 0)
    {
        return;
    }

    snprintf(buffer, bufferSize, "%s %s", prefix, ip != NULL ? ip : "0.0.0.0");
    set_text_content_safe(targetLabel, buffer);
    set_text_color_safe(targetLabel, color_tertiary_text());
}

static void update_numeric_label(gui_text_t *label, char *buffer, size_t bufferSize, int value)
{
    if (buffer == NULL || bufferSize == 0)
    {
        return;
    }

    snprintf(buffer, bufferSize, "%d", value);
    set_text_content_safe(label, buffer);
}

static void sync_iperf_download_ui(void)
{
    gui_obj_hidden((gui_obj_t *)iperf_dl_running_win, !iperfDownloadState.running);
}

static void sync_file_download_ui(void)
{
    gui_obj_hidden((gui_obj_t *)file_dl_running_win, !fileDownloadState.running);
}

void wifi_home_view_init(void *obj)
{
    GUI_UNUSED(obj);

    /* Send WiFi test enter event to APP */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_ENTER, NULL);
#endif

    update_wifi_home_ui();
    gui_fb_change();
}

void iperf_upload_view_init(void *obj)
{
    GUI_UNUSED(obj);
    reset_input_state(&iperfInput);
    update_iperf_input_ui();
}

void iperf_upload_running_view_init(void *obj)
{
    GUI_UNUSED(obj);
    iperfUploadSpeed = 0;
    update_numeric_label(iperf_upload_speed, iperfUploadSpeedText, sizeof(iperfUploadSpeedText),
                         iperfUploadSpeed);
    update_upload_target(iperf_upload_target,
                         iperfUploadTargetText,
                         sizeof(iperfUploadTargetText),
                         "Target:",
                         iperfTargetIp);
    gui_fb_change();
}

void iperf_download_view_init(void *obj)
{
    GUI_UNUSED(obj);
    seed_random_once();
    iperfDownloadState.running = true;
    iperfDownloadState.value = 800 + (rand() % 401);
    update_numeric_label(iperf_dl_speed,
                         iperfDownloadSpeedText,
                         sizeof(iperfDownloadSpeedText),
                         iperfDownloadState.value);
    sync_iperf_download_ui();
    gui_fb_change();
}

void file_upload_view_init(void *obj)
{
    GUI_UNUSED(obj);
    reset_input_state(&fileInput);
    update_file_input_ui();
}

void file_upload_running_view_init(void *obj)
{
    GUI_UNUSED(obj);
    fileUploadData = 0;
    update_numeric_label(file_upload_data, fileUploadDataText, sizeof(fileUploadDataText),
                         fileUploadData);
    update_upload_target(file_upload_target,
                         fileUploadTargetText,
                         sizeof(fileUploadTargetText),
                         "Target:",
                         fileTargetIp);
    gui_fb_change();
}

void file_download_view_init(void *obj)
{
    GUI_UNUSED(obj);
    seed_random_once();
    fileDownloadState.running = true;
    fileDownloadState.value = 50 + (rand() % 101);
    update_numeric_label(file_dl_data,
                         fileDownloadDataText,
                         sizeof(fileDownloadDataText),
                         fileDownloadState.value);
    sync_file_download_ui();
    gui_fb_change();
}

void iperf_key_num(void *obj, gui_event_t *e)
{
    char digit = resolve_iperf_digit(obj);

    GUI_UNUSED(e);
    if (digit == '\0')
    {
        return;
    }

    append_digit(&iperfInput, digit);
    update_iperf_input_ui();
}

void iperf_key_dot_press(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    move_to_next_segment(&iperfInput);
    update_iperf_input_ui();
}

void iperf_key_delete(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    delete_input_char(&iperfInput);
    update_iperf_input_ui();
}

void iperf_key_confirm(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (!input_is_complete(&iperfInput))
    {
        iperfInput.confirmed = false;
        update_iperf_input_ui();
        return;
    }

    iperfInput.confirmed = true;
    build_ip_string(&iperfInput, iperfTargetIp, sizeof(iperfTargetIp));
    update_iperf_input_ui();
}

void iperf_upload_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (!iperfInput.confirmed)
    {
        return;
    }

    build_ip_string(&iperfInput, iperfTargetIp, sizeof(iperfTargetIp));

    /* Send iperf upload start event to APP */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_IPERF_UPLOAD_START, iperfTargetIp);
#endif

    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testIperfUploadRunningView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void iperf_upload_back(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Stop iperf upload test before going back */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_IPERF_UPLOAD_STOP, NULL);
#endif

    reset_input_state(&iperfInput);
    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testIperfMenuView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void iperf_download_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (iperfDownloadState.running)
    {
        return;
    }

    /* Send iperf download start event to APP */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_IPERF_DOWNLOAD_START, NULL);
#endif

    seed_random_once();
    iperfDownloadState.running = true;
    iperfDownloadState.value = 800 + (rand() % 401);
    sync_iperf_download_ui();
    update_numeric_label(iperf_dl_speed,
                         iperfDownloadSpeedText,
                         sizeof(iperfDownloadSpeedText),
                         iperfDownloadState.value);
    gui_fb_change();
}

void iperf_download_back(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Stop iperf download test before going back */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_IPERF_DOWNLOAD_STOP, NULL);
#endif

    iperfDownloadState.running = false;
    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testIperfMenuView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void file_key_num(void *obj, gui_event_t *e)
{
    char digit = resolve_file_digit(obj);

    GUI_UNUSED(e);
    if (digit == '\0')
    {
        return;
    }

    append_digit(&fileInput, digit);
    update_file_input_ui();
}

void file_key_dot_press(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    move_to_next_segment(&fileInput);
    update_file_input_ui();
}

void file_key_delete(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    delete_input_char(&fileInput);
    update_file_input_ui();
}

void file_key_confirm(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (!input_is_complete(&fileInput))
    {
        fileInput.confirmed = false;
        update_file_input_ui();
        return;
    }

    fileInput.confirmed = true;
    build_ip_string(&fileInput, fileTargetIp, sizeof(fileTargetIp));
    update_file_input_ui();
}

void file_upload_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (!fileInput.confirmed)
    {
        return;
    }

    build_ip_string(&fileInput, fileTargetIp, sizeof(fileTargetIp));

    /* Send file upload start event to APP */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_FILE_UPLOAD_START, fileTargetIp);
#endif

    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testFileUploadRunningView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void file_upload_back(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Stop file upload test before going back */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_FILE_UPLOAD_STOP, NULL);
#endif

    reset_input_state(&fileInput);
    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testFileMenuView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void file_download_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    if (fileDownloadState.running)
    {
        return;
    }

    /* Send file download start event to APP */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_FILE_DOWNLOAD_START, NULL);
#endif

    seed_random_once();
    fileDownloadState.running = true;
    fileDownloadState.value = 50 + (rand() % 101);
    sync_file_download_ui();
    update_numeric_label(file_dl_data,
                         fileDownloadDataText,
                         sizeof(fileDownloadDataText),
                         fileDownloadState.value);
    gui_fb_change();
}

void file_download_back(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    /* Stop file download test before going back */
#if CONFIG_WIFI_TEST == 1
    wifi_gui_to_app(WIFI_GUI_TEST_FILE_DOWNLOAD_STOP, NULL);
#endif

    fileDownloadState.running = false;
    gui_view_switch_direct(gui_view_get_current(),
                           "app_wifi_testFileMenuView",
                           SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void iperf_upload_speed_tick_cb_impl(void)
{
    seed_random_once();
    if (!is_current_view("app_wifi_testIperfUploadRunningView"))
    {
        return;
    }

    iperfUploadSpeed = 800 + (rand() % 401);
    update_numeric_label(iperf_upload_speed, iperfUploadSpeedText, sizeof(iperfUploadSpeedText),
                         iperfUploadSpeed);
    update_upload_target(iperf_upload_target,
                         iperfUploadTargetText,
                         sizeof(iperfUploadTargetText),
                         "Target:",
                         iperfTargetIp);
    gui_fb_change();
}

void iperf_dl_speed_tick_cb_impl(void)
{
    seed_random_once();
    if (!is_current_view("app_wifi_testIperfDownloadView") || !iperfDownloadState.running)
    {
        return;
    }

    iperfDownloadState.value = 800 + (rand() % 401);
    update_numeric_label(iperf_dl_speed,
                         iperfDownloadSpeedText,
                         sizeof(iperfDownloadSpeedText),
                         iperfDownloadState.value);
    gui_fb_change();
}

void iperf_dl_connect_delay_cb_impl(void)
{
}

void file_upload_data_tick_cb_impl(void)
{
    seed_random_once();
    if (!is_current_view("app_wifi_testFileUploadRunningView"))
    {
        return;
    }

    fileUploadData += 50 + (rand() % 101);
    update_numeric_label(file_upload_data, fileUploadDataText, sizeof(fileUploadDataText),
                         fileUploadData);
    update_upload_target(file_upload_target,
                         fileUploadTargetText,
                         sizeof(fileUploadTargetText),
                         "Target:",
                         fileTargetIp);
    gui_fb_change();
}

void file_dl_data_tick_cb_impl(void)
{
    seed_random_once();
    if (!is_current_view("app_wifi_testFileDownloadView") || !fileDownloadState.running)
    {
        return;
    }

    fileDownloadState.value += 50 + (rand() % 101);
    update_numeric_label(file_dl_data,
                         fileDownloadDataText,
                         sizeof(fileDownloadDataText),
                         fileDownloadState.value);
    gui_fb_change();
}

void file_dl_connect_delay_cb_impl(void)
{
}