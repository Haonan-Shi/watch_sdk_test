/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_CYCLE_TRACKING_CALLBACKS_H
#define APP_CYCLE_TRACKING_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void app_cycle_tracking_window_key_0_cb(void *obj, gui_event_t *e);
void cycle_menstruation_window_key_0_cb(void *obj, gui_event_t *e);
void cycle_tracking_re_window_key_0_cb(void *obj, gui_event_t *e);
void cycle_tracking_record_back_icon_bg_clicked_cb(void *obj, gui_event_t *e);
void cycle_tracking_record_list0_bg_clicked_cb(void *obj, gui_event_t *e);
void cycle_tracking_record_list1_bg_clicked_cb(void *obj, gui_event_t *e);
void cycle_tracking_symptom_window_key_0_cb(void *obj, gui_event_t *e);
void hg_image_1769762439225_63oo_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1769763659121_in3z_clicked_cb(void *obj, gui_event_t *e);
void hg_rect_1769761365236_appj_clicked_cb(void *obj, gui_event_t *e);
void app_cycle_tracking_time_text_time_update_cb(void *p);
void cycle_tracking_record_time_text_time_update_cb(void *p);
void cycle_tracking_record_time_text_copy_1769762411269_1_time_update_cb(void *p);
void cycle_tracking_symptom_time_text_time_update_cb(void *p);

// Toggle button state callback function declarations
void hg_button_1769763824621_71z7_copy_1769763944802_on_callback(void);
void hg_button_1769763824621_71z7_copy_1769763944802_off_callback(void);
void hg_button_Constipation_on_callback(void);
void hg_button_Constipation_off_callback(void);
void hg_button_Hot_Flash_on_callback(void);
void hg_button_Hot_Flash_off_callback(void);
void hg_button_Acne_on_callback(void);
void hg_button_Acne_off_callback(void);
void hg_button_HairLoss_on_callback(void);
void hg_button_HairLoss_off_callback(void);
void bg_button_Nausea_on_callback(void);
void bg_button_Nausea_off_callback(void);
void hg_button_FeelCold_on_callback(void);
void hg_button_FeelCold_off_callback(void);
void hg_button_AbdominalColic_on_callback(void);
void hg_button_AbdominalColic_off_callback(void);
void hg_button_Diarrhea_on_callback(void);
void hg_button_Diarrhea_off_callback(void);
void hg_button_AbdominalDistension_on_callback(void);
void hg_button_AbdominalDistension_off_callback(void);
void hg_button_MemoryDecline_on_callback(void);
void hg_button_MemoryDecline_off_callback(void);
void hg_button_PelvicPain_on_callback(void);
void hg_button_PelvicPain_off_callback(void);
void hg_button_DrySkin_on_callback(void);
void hg_button_DrySkin_off_callback(void);
void hg_button_Fatigue_on_callback(void);
void hg_button_Fatigue_off_callback(void);
void hg_button_BreastPain_on_callback(void);
void hg_button_BreastPain_off_callback(void);
void hg_button_ChangesAppetite_on_callback(void);
void hg_button_ChangesAppetite_off_callback(void);
void hg_button_SleepChanges_on_callback(void);
void hg_button_SleepChanges_off_callback(void);
void hg_button_Headache_on_callback(void);
void hg_button_Headache_off_callback(void);
void hg_button_LowerBackPain_on_callback(void);
void hg_button_LowerBackPain_off_callback(void);
void hg_button_MoodChanges_on_callback(void);
void hg_button_MoodChanges_off_callback(void);
void hg_button_SweatingNight_on_callback(void);
void hg_button_SweatingNight_off_callback(void);
void hg_button_VaginalDryness_on_callback(void);
void hg_button_VaginalDryness_off_callback(void);

#endif // APP_CYCLE_TRACKING_CALLBACKS_H
