/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_cycle_tracking_callbacks.h"
#include "../ui/app_cycle_tracking_ui.h"
#include "../user/app_cycle_tracking_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char app_cycle_tracking_time_text_time_str[10];
extern char cycle_tracking_record_time_text_time_str[10];
extern char cycle_tracking_record_time_text_copy_1769762411269_1_time_str[10];
extern char cycle_tracking_symptom_time_text_time_str[10];

// Event callback function implementations

void hg_rect_1769761365236_appj_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "cycle_tracking_record_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void app_cycle_tracking_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void cycle_tracking_record_list0_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "cycle_tracking_record_Menstruation_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void cycle_tracking_record_list1_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "cycle_tracking_symptom_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void cycle_tracking_re_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void cycle_tracking_record_back_icon_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_cycle_tracking_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void cycle_menstruation_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_image_1769762439225_63oo_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "cycle_tracking_record_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void cycle_tracking_symptom_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_image_1769763659121_in3z_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "cycle_tracking_record_view",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void app_cycle_tracking_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(app_cycle_tracking_time_text_time_str, sizeof(app_cycle_tracking_time_text_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)app_cycle_tracking_time_text,
                         app_cycle_tracking_time_text_time_str, strlen(app_cycle_tracking_time_text_time_str));
}

void cycle_tracking_record_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(cycle_tracking_record_time_text_time_str, sizeof(cycle_tracking_record_time_text_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)cycle_tracking_record_time_text,
                         cycle_tracking_record_time_text_time_str, strlen(cycle_tracking_record_time_text_time_str));
}

void cycle_tracking_record_time_text_copy_1769762411269_1_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(cycle_tracking_record_time_text_copy_1769762411269_1_time_str,
             sizeof(cycle_tracking_record_time_text_copy_1769762411269_1_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)cycle_tracking_record_time_text_copy_1769762411269_1,
                         cycle_tracking_record_time_text_copy_1769762411269_1_time_str,
                         strlen(cycle_tracking_record_time_text_copy_1769762411269_1_time_str));
}

void cycle_tracking_symptom_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(cycle_tracking_symptom_time_text_time_str,
             sizeof(cycle_tracking_symptom_time_text_time_str), "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)cycle_tracking_symptom_time_text,
                         cycle_tracking_symptom_time_text_time_str, strlen(cycle_tracking_symptom_time_text_time_str));
}

// Toggle button state callback functions

/* USER CODE BEGIN hg_button_1769763824621_71z7_copy_1769763944802_on_callback */
/**
 * hg_button_1769763824621_71z7_copy_1769763944802 ON state callback
 * Called when button switches to ON state
 */
void hg_button_1769763824621_71z7_copy_1769763944802_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_1769763824621_71z7_copy_1769763944802_on_callback */

/* USER CODE BEGIN hg_button_1769763824621_71z7_copy_1769763944802_off_callback */
/**
 * hg_button_1769763824621_71z7_copy_1769763944802 OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_1769763824621_71z7_copy_1769763944802_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_1769763824621_71z7_copy_1769763944802_off_callback */

/* USER CODE BEGIN hg_button_Constipation_on_callback */
/**
 * hg_button_Constipation ON state callback
 * Called when button switches to ON state
 */
void hg_button_Constipation_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Constipation_on_callback */

/* USER CODE BEGIN hg_button_Constipation_off_callback */
/**
 * hg_button_Constipation OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Constipation_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Constipation_off_callback */

/* USER CODE BEGIN hg_button_Hot_Flash_on_callback */
/**
 * hg_button_Hot_Flash ON state callback
 * Called when button switches to ON state
 */
void hg_button_Hot_Flash_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Hot_Flash_on_callback */

/* USER CODE BEGIN hg_button_Hot_Flash_off_callback */
/**
 * hg_button_Hot_Flash OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Hot_Flash_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Hot_Flash_off_callback */

/* USER CODE BEGIN hg_button_Acne_on_callback */
/**
 * hg_button_Acne ON state callback
 * Called when button switches to ON state
 */
void hg_button_Acne_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Acne_on_callback */

/* USER CODE BEGIN hg_button_Acne_off_callback */
/**
 * hg_button_Acne OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Acne_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Acne_off_callback */

/* USER CODE BEGIN hg_button_HairLoss_on_callback */
/**
 * hg_button_HairLoss ON state callback
 * Called when button switches to ON state
 */
void hg_button_HairLoss_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_HairLoss_on_callback */

/* USER CODE BEGIN hg_button_HairLoss_off_callback */
/**
 * hg_button_HairLoss OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_HairLoss_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_HairLoss_off_callback */

/* USER CODE BEGIN bg_button_Nausea_on_callback */
/**
 * bg_button_Nausea ON state callback
 * Called when button switches to ON state
 */
void bg_button_Nausea_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END bg_button_Nausea_on_callback */

/* USER CODE BEGIN bg_button_Nausea_off_callback */
/**
 * bg_button_Nausea OFF state callback
 * Called when button switches to OFF state
 */
void bg_button_Nausea_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END bg_button_Nausea_off_callback */

/* USER CODE BEGIN hg_button_FeelCold_on_callback */
/**
 * hg_button_FeelCold ON state callback
 * Called when button switches to ON state
 */
void hg_button_FeelCold_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_FeelCold_on_callback */

/* USER CODE BEGIN hg_button_FeelCold_off_callback */
/**
 * hg_button_FeelCold OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_FeelCold_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_FeelCold_off_callback */

/* USER CODE BEGIN hg_button_AbdominalColic_on_callback */
/**
 * hg_button_AbdominalColic ON state callback
 * Called when button switches to ON state
 */
void hg_button_AbdominalColic_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_AbdominalColic_on_callback */

/* USER CODE BEGIN hg_button_AbdominalColic_off_callback */
/**
 * hg_button_AbdominalColic OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_AbdominalColic_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_AbdominalColic_off_callback */

/* USER CODE BEGIN hg_button_Diarrhea_on_callback */
/**
 * hg_button_Diarrhea ON state callback
 * Called when button switches to ON state
 */
void hg_button_Diarrhea_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Diarrhea_on_callback */

/* USER CODE BEGIN hg_button_Diarrhea_off_callback */
/**
 * hg_button_Diarrhea OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Diarrhea_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Diarrhea_off_callback */

/* USER CODE BEGIN hg_button_AbdominalDistension_on_callback */
/**
 * hg_button_AbdominalDistension ON state callback
 * Called when button switches to ON state
 */
void hg_button_AbdominalDistension_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_AbdominalDistension_on_callback */

/* USER CODE BEGIN hg_button_AbdominalDistension_off_callback */
/**
 * hg_button_AbdominalDistension OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_AbdominalDistension_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_AbdominalDistension_off_callback */

/* USER CODE BEGIN hg_button_MemoryDecline_on_callback */
/**
 * hg_button_MemoryDecline ON state callback
 * Called when button switches to ON state
 */
void hg_button_MemoryDecline_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_MemoryDecline_on_callback */

/* USER CODE BEGIN hg_button_MemoryDecline_off_callback */
/**
 * hg_button_MemoryDecline OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_MemoryDecline_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_MemoryDecline_off_callback */

/* USER CODE BEGIN hg_button_PelvicPain_on_callback */
/**
 * hg_button_PelvicPain ON state callback
 * Called when button switches to ON state
 */
void hg_button_PelvicPain_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_PelvicPain_on_callback */

/* USER CODE BEGIN hg_button_PelvicPain_off_callback */
/**
 * hg_button_PelvicPain OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_PelvicPain_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_PelvicPain_off_callback */

/* USER CODE BEGIN hg_button_DrySkin_on_callback */
/**
 * hg_button_DrySkin ON state callback
 * Called when button switches to ON state
 */
void hg_button_DrySkin_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_DrySkin_on_callback */

/* USER CODE BEGIN hg_button_DrySkin_off_callback */
/**
 * hg_button_DrySkin OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_DrySkin_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_DrySkin_off_callback */

/* USER CODE BEGIN hg_button_Fatigue_on_callback */
/**
 * hg_button_Fatigue ON state callback
 * Called when button switches to ON state
 */
void hg_button_Fatigue_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Fatigue_on_callback */

/* USER CODE BEGIN hg_button_Fatigue_off_callback */
/**
 * hg_button_Fatigue OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Fatigue_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Fatigue_off_callback */

/* USER CODE BEGIN hg_button_BreastPain_on_callback */
/**
 * hg_button_BreastPain ON state callback
 * Called when button switches to ON state
 */
void hg_button_BreastPain_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_BreastPain_on_callback */

/* USER CODE BEGIN hg_button_BreastPain_off_callback */
/**
 * hg_button_BreastPain OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_BreastPain_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_BreastPain_off_callback */

/* USER CODE BEGIN hg_button_ChangesAppetite_on_callback */
/**
 * hg_button_ChangesAppetite ON state callback
 * Called when button switches to ON state
 */
void hg_button_ChangesAppetite_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_ChangesAppetite_on_callback */

/* USER CODE BEGIN hg_button_ChangesAppetite_off_callback */
/**
 * hg_button_ChangesAppetite OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_ChangesAppetite_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_ChangesAppetite_off_callback */

/* USER CODE BEGIN hg_button_SleepChanges_on_callback */
/**
 * hg_button_SleepChanges ON state callback
 * Called when button switches to ON state
 */
void hg_button_SleepChanges_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_SleepChanges_on_callback */

/* USER CODE BEGIN hg_button_SleepChanges_off_callback */
/**
 * hg_button_SleepChanges OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_SleepChanges_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_SleepChanges_off_callback */

/* USER CODE BEGIN hg_button_Headache_on_callback */
/**
 * hg_button_Headache ON state callback
 * Called when button switches to ON state
 */
void hg_button_Headache_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_Headache_on_callback */

/* USER CODE BEGIN hg_button_Headache_off_callback */
/**
 * hg_button_Headache OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_Headache_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_Headache_off_callback */

/* USER CODE BEGIN hg_button_LowerBackPain_on_callback */
/**
 * hg_button_LowerBackPain ON state callback
 * Called when button switches to ON state
 */
void hg_button_LowerBackPain_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_LowerBackPain_on_callback */

/* USER CODE BEGIN hg_button_LowerBackPain_off_callback */
/**
 * hg_button_LowerBackPain OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_LowerBackPain_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_LowerBackPain_off_callback */

/* USER CODE BEGIN hg_button_MoodChanges_on_callback */
/**
 * hg_button_MoodChanges ON state callback
 * Called when button switches to ON state
 */
void hg_button_MoodChanges_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_MoodChanges_on_callback */

/* USER CODE BEGIN hg_button_MoodChanges_off_callback */
/**
 * hg_button_MoodChanges OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_MoodChanges_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_MoodChanges_off_callback */

/* USER CODE BEGIN hg_button_SweatingNight_on_callback */
/**
 * hg_button_SweatingNight ON state callback
 * Called when button switches to ON state
 */
void hg_button_SweatingNight_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_SweatingNight_on_callback */

/* USER CODE BEGIN hg_button_SweatingNight_off_callback */
/**
 * hg_button_SweatingNight OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_SweatingNight_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_SweatingNight_off_callback */

/* USER CODE BEGIN hg_button_VaginalDryness_on_callback */
/**
 * hg_button_VaginalDryness ON state callback
 * Called when button switches to ON state
 */
void hg_button_VaginalDryness_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END hg_button_VaginalDryness_on_callback */

/* USER CODE BEGIN hg_button_VaginalDryness_off_callback */
/**
 * hg_button_VaginalDryness OFF state callback
 * Called when button switches to OFF state
 */
void hg_button_VaginalDryness_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END hg_button_VaginalDryness_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
