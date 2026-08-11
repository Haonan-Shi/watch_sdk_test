/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_cycle_tracking UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.814Z
 */
#include "app_cycle_tracking_ui.h"
#include "../callbacks/app_cycle_tracking_callbacks.h"
#include "../user/app_cycle_tracking_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *cycle_track_list = NULL;
gui_rounded_rect_t *cycle_track_record0 = NULL;
gui_rounded_rect_t *cycle_track_record1 = NULL;
gui_rounded_rect_t *cycle_track_record2 = NULL;
gui_rounded_rect_t *cycle_track_record3 = NULL;
gui_circle_t *cycle_track_record_mark0 = NULL;
gui_rounded_rect_t *cycle_track_record4 = NULL;
gui_circle_t *cycle_track_record_mark1 = NULL;
gui_rounded_rect_t *cycle_track_record5 = NULL;
gui_circle_t *cycle_track_record_mark2 = NULL;
gui_rounded_rect_t *cycle_track_record6 = NULL;
gui_circle_t *cycle_track_record_mark3 = NULL;
gui_text_t *hg_label_1769761165740_60hx = NULL;
gui_rounded_rect_t *hg_rect_1769761365236_appj = NULL;
gui_text_t *Light_Flow_and_Acne_text = NULL;
gui_win_t *app_cycle_tracking_window = NULL;
gui_img_t *hg_image_1769761296861_2c5p = NULL;
gui_text_t *app_cycle_tracking_time_text = NULL;
gui_list_t *cycle_tracking_record_list = NULL;
gui_rounded_rect_t *cycle_tracking_record_list0_bg = NULL;
gui_img_t *cycle_tracking_record0 = NULL;
gui_text_t *cycle_tracking_Menstruation_text = NULL;
gui_rounded_rect_t *cycle_tracking_record_list1_bg = NULL;
gui_img_t *cycle_tracking_record_add1 = NULL;
gui_text_t *cycle_tracking_Symptom_text = NULL;
gui_rounded_rect_t *cycle_tracking_record_list2_bg = NULL;
gui_img_t *cycle_tracking_record_add2 = NULL;
gui_text_t *Spotting_text = NULL;
gui_win_t *cycle_tracking_re_window = NULL;
gui_text_t *cycle_tracking_record_time_text = NULL;
gui_img_t *cycle_tracking_record_back_icon_bg = NULL;
gui_img_t *hg_image_1769761721084_fcvl = NULL;
gui_list_t *cycle_tracking_menstruation_list = NULL;
gui_text_t *cycle_tracking_mens_TEXT = NULL;
gui_rounded_rect_t *cycle_tracking_mens_list1_bg = NULL;
gui_text_t *cycle_tracking_mens_sym_text = NULL;
gui_rounded_rect_t *cycle_tracking_mens_list2_bg = NULL;
gui_text_t *symptomcycle_tracking_mens_sym_text = NULL;
gui_text_t *Blood_loss_text = NULL;
gui_rounded_rect_t *cycle_tracking_mens_list5_bg = NULL;
gui_text_t *Small_Quantity_text = NULL;
gui_rounded_rect_t *cycle_tracking_mens_list6_bg = NULL;
gui_text_t *Medium_Quantity_text = NULL;
gui_rounded_rect_t *cycle_tracking_mens_list7_bg = NULL;
gui_text_t *Quantity_Large_text = NULL;
gui_win_t *cycle_menstruation_window = NULL;
gui_text_t *cycle_tracking_record_time_text_copy_1769762411269_1 = NULL;
gui_img_t *hg_image_1769762439225_63oo = NULL;
gui_list_t *hg_list_1769763676513_n1s4 = NULL;
gui_text_t *hg_label_1769765239790_eu1e = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg2 = NULL;
gui_obj_t *hg_button_1769763824621_71z7_copy_1769763944802 = NULL;
gui_text_t *button_Bladder_Incontinence = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg3 = NULL;
gui_obj_t *hg_button_Constipation = NULL;
gui_text_t *Constipation_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg4 = NULL;
gui_obj_t *hg_button_Hot_Flash = NULL;
gui_text_t *HotFlash_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg5 = NULL;
gui_obj_t *hg_button_Acne = NULL;
gui_text_t *Acne_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg6 = NULL;
gui_obj_t *hg_button_HairLoss = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg7 = NULL;
gui_obj_t *bg_button_Nausea = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg8 = NULL;
gui_obj_t *hg_button_FeelCold = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg9 = NULL;
gui_obj_t *hg_button_AbdominalColic = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg10 = NULL;
gui_obj_t *hg_button_Diarrhea = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg10_copy_1769765685129 = NULL;
gui_obj_t *hg_button_AbdominalDistension = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg12 = NULL;
gui_obj_t *hg_button_MemoryDecline = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg13 = NULL;
gui_obj_t *hg_button_PelvicPain = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg14 = NULL;
gui_obj_t *hg_button_DrySkin = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg15 = NULL;
gui_obj_t *hg_button_Fatigue = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg16 = NULL;
gui_obj_t *hg_button_BreastPain = NULL;
gui_text_t
*hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528
    = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg17 = NULL;
gui_obj_t *hg_button_ChangesAppetite = NULL;
gui_text_t *ChangesAppetite_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg18 = NULL;
gui_obj_t *hg_button_SleepChanges = NULL;
gui_text_t *SleepChanges_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg19 = NULL;
gui_obj_t *hg_button_Headache = NULL;
gui_text_t *Headache_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg20 = NULL;
gui_obj_t *hg_button_LowerBackPain = NULL;
gui_text_t *LowerBackPain_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg21 = NULL;
gui_obj_t *hg_button_MoodChanges = NULL;
gui_text_t *MoodChanges_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg22 = NULL;
gui_obj_t *hg_button_SweatingNight = NULL;
gui_text_t *SweatingNight_text = NULL;
gui_rounded_rect_t *cycle_tracking_symptom_list_bg23 = NULL;
gui_obj_t *hg_button_VaginalDryness = NULL;
gui_text_t *VaginalDryness_text = NULL;
gui_win_t *cycle_tracking_symptom_window = NULL;
gui_text_t *cycle_tracking_symptom_time_text = NULL;
gui_img_t *hg_image_1769763659121_in3z = NULL;

// Time string global variables
char app_cycle_tracking_time_text_time_str[10] = {0};
char cycle_tracking_record_time_text_time_str[10] = {0};
char cycle_tracking_record_time_text_copy_1769762411269_1_time_str[10] = {0};
char cycle_tracking_symptom_time_text_time_str[10] = {0};

// Toggle button callback functions

// hg_button_1769763824621_71z7_copy_1769763944802 dual-state button callback
static bool hg_button_1769763824621_71z7_copy_1769763944802_state = false;

void hg_button_1769763824621_71z7_copy_1769763944802_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_1769763824621_71z7_copy_1769763944802_state =
        !hg_button_1769763824621_71z7_copy_1769763944802_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_1769763824621_71z7_copy_1769763944802_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                        (const uint8_t *)"/app_phone/icon_bg.bin", IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_1769763824621_71z7_copy_1769763944802_get_state(void)
{
    return hg_button_1769763824621_71z7_copy_1769763944802_state;
}

// Set state (external call)
void hg_button_1769763824621_71z7_copy_1769763944802_set_state(bool state)
{
    if (hg_button_1769763824621_71z7_copy_1769763944802_state != state)
    {
        hg_button_1769763824621_71z7_copy_1769763944802_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                            (const uint8_t *)"/app_phone/icon_bg.bin", IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Constipation dual-state button callback
static bool hg_button_Constipation_state = false;

void hg_button_Constipation_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Constipation_state = !hg_button_Constipation_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Constipation_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Constipation,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Constipation, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Constipation_get_state(void)
{
    return hg_button_Constipation_state;
}

// Set state (external call)
void hg_button_Constipation_set_state(bool state)
{
    if (hg_button_Constipation_state != state)
    {
        hg_button_Constipation_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Constipation,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Constipation, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Hot_Flash dual-state button callback
static bool hg_button_Hot_Flash_state = false;

void hg_button_Hot_Flash_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Hot_Flash_state = !hg_button_Hot_Flash_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Hot_Flash_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Hot_Flash, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Hot_Flash, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Hot_Flash_get_state(void)
{
    return hg_button_Hot_Flash_state;
}

// Set state (external call)
void hg_button_Hot_Flash_set_state(bool state)
{
    if (hg_button_Hot_Flash_state != state)
    {
        hg_button_Hot_Flash_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Hot_Flash, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Hot_Flash, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Acne dual-state button callback
static bool hg_button_Acne_state = false;

void hg_button_Acne_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Acne_state = !hg_button_Acne_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Acne_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Acne, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Acne, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Acne_get_state(void)
{
    return hg_button_Acne_state;
}

// Set state (external call)
void hg_button_Acne_set_state(bool state)
{
    if (hg_button_Acne_state != state)
    {
        hg_button_Acne_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Acne, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Acne, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_HairLoss dual-state button callback
static bool hg_button_HairLoss_state = false;

void hg_button_HairLoss_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_HairLoss_state = !hg_button_HairLoss_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_HairLoss_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_HairLoss, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_HairLoss, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_HairLoss_get_state(void)
{
    return hg_button_HairLoss_state;
}

// Set state (external call)
void hg_button_HairLoss_set_state(bool state)
{
    if (hg_button_HairLoss_state != state)
    {
        hg_button_HairLoss_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_HairLoss, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_HairLoss, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// bg_button_Nausea dual-state button callback
static bool bg_button_Nausea_state = false;

void bg_button_Nausea_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    bg_button_Nausea_state = !bg_button_Nausea_state;

    // Switch image based on state and call corresponding callback
    if (bg_button_Nausea_state)
    {
        gui_img_set_src((gui_img_t *)bg_button_Nausea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)bg_button_Nausea, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool bg_button_Nausea_get_state(void)
{
    return bg_button_Nausea_state;
}

// Set state (external call)
void bg_button_Nausea_set_state(bool state)
{
    if (bg_button_Nausea_state != state)
    {
        bg_button_Nausea_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)bg_button_Nausea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)bg_button_Nausea, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_FeelCold dual-state button callback
static bool hg_button_FeelCold_state = false;

void hg_button_FeelCold_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_FeelCold_state = !hg_button_FeelCold_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_FeelCold_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_FeelCold, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_FeelCold, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_FeelCold_get_state(void)
{
    return hg_button_FeelCold_state;
}

// Set state (external call)
void hg_button_FeelCold_set_state(bool state)
{
    if (hg_button_FeelCold_state != state)
    {
        hg_button_FeelCold_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_FeelCold, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_FeelCold, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_AbdominalColic dual-state button callback
static bool hg_button_AbdominalColic_state = false;

void hg_button_AbdominalColic_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_AbdominalColic_state = !hg_button_AbdominalColic_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_AbdominalColic_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_AbdominalColic,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_AbdominalColic, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_AbdominalColic_get_state(void)
{
    return hg_button_AbdominalColic_state;
}

// Set state (external call)
void hg_button_AbdominalColic_set_state(bool state)
{
    if (hg_button_AbdominalColic_state != state)
    {
        hg_button_AbdominalColic_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_AbdominalColic,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_AbdominalColic, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Diarrhea dual-state button callback
static bool hg_button_Diarrhea_state = false;

void hg_button_Diarrhea_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Diarrhea_state = !hg_button_Diarrhea_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Diarrhea_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Diarrhea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Diarrhea, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Diarrhea_get_state(void)
{
    return hg_button_Diarrhea_state;
}

// Set state (external call)
void hg_button_Diarrhea_set_state(bool state)
{
    if (hg_button_Diarrhea_state != state)
    {
        hg_button_Diarrhea_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Diarrhea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Diarrhea, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_AbdominalDistension dual-state button callback
static bool hg_button_AbdominalDistension_state = false;

void hg_button_AbdominalDistension_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_AbdominalDistension_state = !hg_button_AbdominalDistension_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_AbdominalDistension_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_AbdominalDistension,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_AbdominalDistension,
                        (const uint8_t *)"/app_phone/icon_bg.bin", IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_AbdominalDistension_get_state(void)
{
    return hg_button_AbdominalDistension_state;
}

// Set state (external call)
void hg_button_AbdominalDistension_set_state(bool state)
{
    if (hg_button_AbdominalDistension_state != state)
    {
        hg_button_AbdominalDistension_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_AbdominalDistension,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_AbdominalDistension,
                            (const uint8_t *)"/app_phone/icon_bg.bin", IMG_SRC_FILESYS);
        }
    }
}

// hg_button_MemoryDecline dual-state button callback
static bool hg_button_MemoryDecline_state = false;

void hg_button_MemoryDecline_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_MemoryDecline_state = !hg_button_MemoryDecline_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_MemoryDecline_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_MemoryDecline,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_MemoryDecline, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_MemoryDecline_get_state(void)
{
    return hg_button_MemoryDecline_state;
}

// Set state (external call)
void hg_button_MemoryDecline_set_state(bool state)
{
    if (hg_button_MemoryDecline_state != state)
    {
        hg_button_MemoryDecline_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_MemoryDecline,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_MemoryDecline, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_PelvicPain dual-state button callback
static bool hg_button_PelvicPain_state = false;

void hg_button_PelvicPain_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_PelvicPain_state = !hg_button_PelvicPain_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_PelvicPain_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_PelvicPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_PelvicPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_PelvicPain_get_state(void)
{
    return hg_button_PelvicPain_state;
}

// Set state (external call)
void hg_button_PelvicPain_set_state(bool state)
{
    if (hg_button_PelvicPain_state != state)
    {
        hg_button_PelvicPain_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_PelvicPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_PelvicPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_DrySkin dual-state button callback
static bool hg_button_DrySkin_state = false;

void hg_button_DrySkin_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_DrySkin_state = !hg_button_DrySkin_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_DrySkin_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_DrySkin, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_DrySkin, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_DrySkin_get_state(void)
{
    return hg_button_DrySkin_state;
}

// Set state (external call)
void hg_button_DrySkin_set_state(bool state)
{
    if (hg_button_DrySkin_state != state)
    {
        hg_button_DrySkin_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_DrySkin, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_DrySkin, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Fatigue dual-state button callback
static bool hg_button_Fatigue_state = false;

void hg_button_Fatigue_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Fatigue_state = !hg_button_Fatigue_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Fatigue_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Fatigue, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Fatigue, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Fatigue_get_state(void)
{
    return hg_button_Fatigue_state;
}

// Set state (external call)
void hg_button_Fatigue_set_state(bool state)
{
    if (hg_button_Fatigue_state != state)
    {
        hg_button_Fatigue_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Fatigue, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Fatigue, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_BreastPain dual-state button callback
static bool hg_button_BreastPain_state = false;

void hg_button_BreastPain_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_BreastPain_state = !hg_button_BreastPain_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_BreastPain_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_BreastPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_BreastPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_BreastPain_get_state(void)
{
    return hg_button_BreastPain_state;
}

// Set state (external call)
void hg_button_BreastPain_set_state(bool state)
{
    if (hg_button_BreastPain_state != state)
    {
        hg_button_BreastPain_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_BreastPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_BreastPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_ChangesAppetite dual-state button callback
static bool hg_button_ChangesAppetite_state = false;

void hg_button_ChangesAppetite_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_ChangesAppetite_state = !hg_button_ChangesAppetite_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_ChangesAppetite_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_ChangesAppetite,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_ChangesAppetite, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_ChangesAppetite_get_state(void)
{
    return hg_button_ChangesAppetite_state;
}

// Set state (external call)
void hg_button_ChangesAppetite_set_state(bool state)
{
    if (hg_button_ChangesAppetite_state != state)
    {
        hg_button_ChangesAppetite_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_ChangesAppetite,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_ChangesAppetite, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_SleepChanges dual-state button callback
static bool hg_button_SleepChanges_state = false;

void hg_button_SleepChanges_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_SleepChanges_state = !hg_button_SleepChanges_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_SleepChanges_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_SleepChanges,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_SleepChanges, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_SleepChanges_get_state(void)
{
    return hg_button_SleepChanges_state;
}

// Set state (external call)
void hg_button_SleepChanges_set_state(bool state)
{
    if (hg_button_SleepChanges_state != state)
    {
        hg_button_SleepChanges_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_SleepChanges,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_SleepChanges, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_Headache dual-state button callback
static bool hg_button_Headache_state = false;

void hg_button_Headache_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_Headache_state = !hg_button_Headache_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_Headache_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_Headache, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_Headache, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_Headache_get_state(void)
{
    return hg_button_Headache_state;
}

// Set state (external call)
void hg_button_Headache_set_state(bool state)
{
    if (hg_button_Headache_state != state)
    {
        hg_button_Headache_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_Headache, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_Headache, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_LowerBackPain dual-state button callback
static bool hg_button_LowerBackPain_state = false;

void hg_button_LowerBackPain_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_LowerBackPain_state = !hg_button_LowerBackPain_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_LowerBackPain_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_LowerBackPain,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_LowerBackPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_LowerBackPain_get_state(void)
{
    return hg_button_LowerBackPain_state;
}

// Set state (external call)
void hg_button_LowerBackPain_set_state(bool state)
{
    if (hg_button_LowerBackPain_state != state)
    {
        hg_button_LowerBackPain_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_LowerBackPain,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_LowerBackPain, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_MoodChanges dual-state button callback
static bool hg_button_MoodChanges_state = false;

void hg_button_MoodChanges_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_MoodChanges_state = !hg_button_MoodChanges_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_MoodChanges_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_MoodChanges, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                        IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_MoodChanges, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_MoodChanges_get_state(void)
{
    return hg_button_MoodChanges_state;
}

// Set state (external call)
void hg_button_MoodChanges_set_state(bool state)
{
    if (hg_button_MoodChanges_state != state)
    {
        hg_button_MoodChanges_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_MoodChanges, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                            IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_MoodChanges, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_SweatingNight dual-state button callback
static bool hg_button_SweatingNight_state = false;

void hg_button_SweatingNight_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_SweatingNight_state = !hg_button_SweatingNight_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_SweatingNight_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_SweatingNight,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_SweatingNight, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_SweatingNight_get_state(void)
{
    return hg_button_SweatingNight_state;
}

// Set state (external call)
void hg_button_SweatingNight_set_state(bool state)
{
    if (hg_button_SweatingNight_state != state)
    {
        hg_button_SweatingNight_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_SweatingNight,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_SweatingNight, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}

// hg_button_VaginalDryness dual-state button callback
static bool hg_button_VaginalDryness_state = false;

void hg_button_VaginalDryness_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    hg_button_VaginalDryness_state = !hg_button_VaginalDryness_state;

    // Switch image based on state and call corresponding callback
    if (hg_button_VaginalDryness_state)
    {
        gui_img_set_src((gui_img_t *)hg_button_VaginalDryness,
                        (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)hg_button_VaginalDryness, (const uint8_t *)"/app_phone/icon_bg.bin",
                        IMG_SRC_FILESYS);
    }
    gui_fb_change();
}

// Get current state
bool hg_button_VaginalDryness_get_state(void)
{
    return hg_button_VaginalDryness_state;
}

// Set state (external call)
void hg_button_VaginalDryness_set_state(bool state)
{
    if (hg_button_VaginalDryness_state != state)
    {
        hg_button_VaginalDryness_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)hg_button_VaginalDryness,
                            (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)hg_button_VaginalDryness, (const uint8_t *)"/app_phone/icon_bg.bin",
                            IMG_SRC_FILESYS);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void cycle_track_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void cycle_track_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create cycle_track_record0 (hg_rect)
            cycle_track_record0 = gui_rect_create((gui_obj_t *)note, "cycle_track_record0", 2, 38, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            break;
        }
    case 1:
        {
            // Create cycle_track_record1 (hg_rect)
            cycle_track_record1 = gui_rect_create((gui_obj_t *)note, "cycle_track_record1", 2, 38, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            break;
        }
    case 2:
        {
            // Create cycle_track_record2 (hg_rect)
            cycle_track_record2 = gui_rect_create((gui_obj_t *)note, "cycle_track_record2", 2, 38, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            break;
        }
    case 3:
        {
            // Create cycle_track_record3 (hg_rect)
            cycle_track_record3 = gui_rect_create((gui_obj_t *)note, "cycle_track_record3", 2, 36, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            // Create cycle_track_record_mark0 (hg_circle)
            cycle_track_record_mark0 = gui_circle_create((gui_obj_t *)note, "cycle_track_record_mark0", 27, 61,
                                                         20, gui_rgb(254, 106, 76));
            break;
        }
    case 4:
        {
            // Create cycle_track_record4 (hg_rect)
            cycle_track_record4 = gui_rect_create((gui_obj_t *)note, "cycle_track_record4", 2, 36, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            // Create cycle_track_record_mark1 (hg_circle)
            cycle_track_record_mark1 = gui_circle_create((gui_obj_t *)note, "cycle_track_record_mark1", 27, 61,
                                                         20, gui_rgb(226, 136, 111));
            break;
        }
    case 5:
        {
            // Create cycle_track_record5 (hg_rect)
            cycle_track_record5 = gui_rect_create((gui_obj_t *)note, "cycle_track_record5", 2, 36, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            // Create cycle_track_record_mark2 (hg_circle)
            cycle_track_record_mark2 = gui_circle_create((gui_obj_t *)note, "cycle_track_record_mark2", 27, 61,
                                                         20, gui_rgb(226, 136, 111));
            break;
        }
    case 6:
        {
            // Create cycle_track_record6 (hg_rect)
            cycle_track_record6 = gui_rect_create((gui_obj_t *)note, "cycle_track_record6", 2, 36, 50, 80, 25,
                                                  gui_rgb(255, 255, 255));
            // Create cycle_track_record_mark3 (hg_circle)
            cycle_track_record_mark3 = gui_circle_create((gui_obj_t *)note, "cycle_track_record_mark3", 27, 61,
                                                         20, gui_rgb(226, 136, 111));
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void cycle_tracking_record_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void cycle_tracking_record_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create cycle_tracking_record_list0_bg (hg_rect)
            cycle_tracking_record_list0_bg = gui_rect_create((gui_obj_t *)note,
                                                             "cycle_tracking_record_list0_bg", 0, 4, 390, 120, 20, gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(cycle_tracking_record_list0_bg,
                                 (gui_event_cb_t)cycle_tracking_record_list0_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create cycle_tracking_record0 (hg_image)
            cycle_tracking_record0 = gui_img_create_from_fs((gui_obj_t *)note, "cycle_tracking_record0",
                                                            "/app_phone/phone_add_icon.bin", 336, 44, 40, 40);
            // Create cycle_tracking_Menstruation_text (hg_label)
            cycle_tracking_Menstruation_text = gui_text_create((gui_obj_t *)note,
                                                               "cycle_tracking_Menstruation_text", 15, 48, 228, 42);
            gui_text_set((gui_text_t *)cycle_tracking_Menstruation_text, "Menstruation", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 12, 32);
            gui_text_type_set((gui_text_t *)cycle_tracking_Menstruation_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)cycle_tracking_Menstruation_text, LEFT);
            break;
        }
    case 1:
        {
            // Create cycle_tracking_record_list1_bg (hg_rect)
            cycle_tracking_record_list1_bg = gui_rect_create((gui_obj_t *)note,
                                                             "cycle_tracking_record_list1_bg", 0, 4, 390, 120, 20, gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(cycle_tracking_record_list1_bg,
                                 (gui_event_cb_t)cycle_tracking_record_list1_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create cycle_tracking_record_add1 (hg_image)
            cycle_tracking_record_add1 = gui_img_create_from_fs((gui_obj_t *)note, "cycle_tracking_record_add1",
                                                                "/app_phone/phone_add_icon.bin", 336, 44, 40, 40);
            // Create cycle_tracking_Symptom_text (hg_label)
            cycle_tracking_Symptom_text = gui_text_create((gui_obj_t *)note, "cycle_tracking_Symptom_text", 15,
                                                          48, 166, 42);
            gui_text_set((gui_text_t *)cycle_tracking_Symptom_text, "Symptom", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 7, 32);
            gui_text_type_set((gui_text_t *)cycle_tracking_Symptom_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)cycle_tracking_Symptom_text, LEFT);
            break;
        }
    case 2:
        {
            // Create cycle_tracking_record_list2_bg (hg_rect)
            cycle_tracking_record_list2_bg = gui_rect_create((gui_obj_t *)note,
                                                             "cycle_tracking_record_list2_bg", 0, 0, 390, 120, 20, gui_rgb(44, 44, 46));
            // Create cycle_tracking_record_add2 (hg_image)
            cycle_tracking_record_add2 = gui_img_create_from_fs((gui_obj_t *)note, "cycle_tracking_record_add2",
                                                                "/app_phone/phone_add_icon.bin", 336, 44, 40, 40);
            // Create Spotting_text (hg_label)
            Spotting_text = gui_text_create((gui_obj_t *)note, "Spotting_text", 15, 48, 166, 42);
            gui_text_set((gui_text_t *)Spotting_text, "Spotting", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8,
                         32);
            gui_text_type_set((gui_text_t *)Spotting_text, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Spotting_text, LEFT);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void cycle_tracking_menstruation_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void cycle_tracking_menstruation_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create cycle_tracking_mens_TEXT (hg_label)
            cycle_tracking_mens_TEXT = gui_text_create((gui_obj_t *)note, "cycle_tracking_mens_TEXT", 15, 50,
                                                       199, 42);
            gui_text_set((gui_text_t *)cycle_tracking_mens_TEXT, "Menstruation", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 12, 32);
            gui_text_type_set((gui_text_t *)cycle_tracking_mens_TEXT,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)cycle_tracking_mens_TEXT, LEFT);
            break;
        }
    case 1:
        {
            // Create cycle_tracking_mens_list1_bg (hg_rect)
            cycle_tracking_mens_list1_bg = gui_rect_create((gui_obj_t *)note, "cycle_tracking_mens_list1_bg", 0,
                                                           0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create cycle_tracking_mens_sym_text (hg_label)
            cycle_tracking_mens_sym_text = gui_text_create((gui_obj_t *)note, "cycle_tracking_mens_sym_text",
                                                           15, 34, 248, 42);
            gui_text_set((gui_text_t *)cycle_tracking_mens_sym_text, "Amount of Bleeding", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 18, 32);
            gui_text_type_set((gui_text_t *)cycle_tracking_mens_sym_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)cycle_tracking_mens_sym_text, LEFT);
            break;
        }
    case 2:
        {
            // Create cycle_tracking_mens_list2_bg (hg_rect)
            cycle_tracking_mens_list2_bg = gui_rect_create((gui_obj_t *)note, "cycle_tracking_mens_list2_bg", 0,
                                                           0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create symptomcycle_tracking_mens_sym_text (hg_label)
            symptomcycle_tracking_mens_sym_text = gui_text_create((gui_obj_t *)note,
                                                                  "symptomcycle_tracking_mens_sym_text", 15, 34, 285, 42);
            gui_text_set((gui_text_t *)symptomcycle_tracking_mens_sym_text, "No Bleeding", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)symptomcycle_tracking_mens_sym_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)symptomcycle_tracking_mens_sym_text, LEFT);
            break;
        }
    case 3:
        {
            // Create Blood_loss_text (hg_label)
            Blood_loss_text = gui_text_create((gui_obj_t *)note, "Blood_loss_text", 15, 34, 170, 42);
            gui_text_set((gui_text_t *)Blood_loss_text, "Blood Loss", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         10, 32);
            gui_text_type_set((gui_text_t *)Blood_loss_text, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Blood_loss_text, LEFT);
            break;
        }
    case 4:
        {
            // Create cycle_tracking_mens_list5_bg (hg_rect)
            cycle_tracking_mens_list5_bg = gui_rect_create((gui_obj_t *)note, "cycle_tracking_mens_list5_bg", 0,
                                                           0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create Small_Quantity_text (hg_label)
            Small_Quantity_text = gui_text_create((gui_obj_t *)note, "Small_Quantity_text", 15, 34, 180, 42);
            gui_text_set((gui_text_t *)Small_Quantity_text, "Small Quantity", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 14, 32);
            gui_text_type_set((gui_text_t *)Small_Quantity_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Small_Quantity_text, LEFT);
            break;
        }
    case 5:
        {
            // Create cycle_tracking_mens_list6_bg (hg_rect)
            cycle_tracking_mens_list6_bg = gui_rect_create((gui_obj_t *)note, "cycle_tracking_mens_list6_bg", 0,
                                                           0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create Medium_Quantity_text (hg_label)
            Medium_Quantity_text = gui_text_create((gui_obj_t *)note, "Medium_Quantity_text", 15, 34, 328, 42);
            gui_text_set((gui_text_t *)Medium_Quantity_text, "Medium Quantity", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 15, 32);
            gui_text_type_set((gui_text_t *)Medium_Quantity_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Medium_Quantity_text, LEFT);
            break;
        }
    case 6:
        {
            // Create cycle_tracking_mens_list7_bg (hg_rect)
            cycle_tracking_mens_list7_bg = gui_rect_create((gui_obj_t *)note, "cycle_tracking_mens_list7_bg", 0,
                                                           0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create Quantity_Large_text (hg_label)
            Quantity_Large_text = gui_text_create((gui_obj_t *)note, "Quantity_Large_text", 15, 34, 302, 42);
            gui_text_set((gui_text_t *)Quantity_Large_text, "Quantity is Large", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 17, 32);
            gui_text_type_set((gui_text_t *)Quantity_Large_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Quantity_Large_text, LEFT);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void hg_list_1769763676513_n1s4_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_1769763676513_n1s4_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create hg_label_1769765239790_eu1e (hg_label)
            hg_label_1769765239790_eu1e = gui_text_create((gui_obj_t *)note, "hg_label_1769765239790_eu1e", 30,
                                                          39, 317, 42);
            gui_text_set((gui_text_t *)hg_label_1769765239790_eu1e, "Other Symptoms", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 14, 32);
            gui_text_type_set((gui_text_t *)hg_label_1769765239790_eu1e,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1769765239790_eu1e, LEFT);
            break;
        }
    case 1:
        {
            // Create cycle_tracking_symptom_list_bg2 (hg_rect)
            cycle_tracking_symptom_list_bg2 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg2", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_1769763824621_71z7_copy_1769763944802 (hg_button)
            hg_button_1769763824621_71z7_copy_1769763944802 = (gui_obj_t *)gui_img_create_from_fs((
                                                                  gui_obj_t *)note, "hg_button_1769763824621_71z7_copy_1769763944802", "/app_phone/icon_bg.bin", 338,
                                                              25, 72, 72);
            if (hg_button_1769763824621_71z7_copy_1769763944802_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_1769763824621_71z7_copy_1769763944802,
                                 hg_button_1769763824621_71z7_copy_1769763944802_toggle_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create button_Bladder_Incontinence (hg_label)
            button_Bladder_Incontinence = gui_text_create((gui_obj_t *)note, "button_Bladder_Incontinence", 27,
                                                          34, 323, 42);
            gui_text_set((gui_text_t *)button_Bladder_Incontinence, "Bladder Incontinence", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 20, 32);
            gui_text_type_set((gui_text_t *)button_Bladder_Incontinence,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)button_Bladder_Incontinence, LEFT);
            break;
        }
    case 2:
        {
            // Create cycle_tracking_symptom_list_bg3 (hg_rect)
            cycle_tracking_symptom_list_bg3 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg3", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Constipation (hg_button)
            hg_button_Constipation = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                         "hg_button_Constipation", "/app_phone/icon_bg.bin", 338, 25, 72, 72);
            if (hg_button_Constipation_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Constipation,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Constipation, hg_button_Constipation_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create Constipation_text (hg_label)
            Constipation_text = gui_text_create((gui_obj_t *)note, "Constipation_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)Constipation_text, "Constipation", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 12, 32);
            gui_text_type_set((gui_text_t *)Constipation_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Constipation_text, LEFT);
            break;
        }
    case 3:
        {
            // Create cycle_tracking_symptom_list_bg4 (hg_rect)
            cycle_tracking_symptom_list_bg4 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg4", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Hot_Flash (hg_button)
            hg_button_Hot_Flash = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_Hot_Flash",
                                                                      "/app_phone/icon_bg.bin", 341, 25, 72, 72);
            if (hg_button_Hot_Flash_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Hot_Flash, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Hot_Flash, hg_button_Hot_Flash_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create HotFlash_text (hg_label)
            HotFlash_text = gui_text_create((gui_obj_t *)note, "HotFlash_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)HotFlash_text, "Hot Flash", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9,
                         32);
            gui_text_type_set((gui_text_t *)HotFlash_text, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)HotFlash_text, LEFT);
            break;
        }
    case 4:
        {
            // Create cycle_tracking_symptom_list_bg5 (hg_rect)
            cycle_tracking_symptom_list_bg5 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg5", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Acne (hg_button)
            hg_button_Acne = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_Acne",
                                                                 "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_Acne_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Acne, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Acne, hg_button_Acne_toggle_cb, GUI_EVENT_TOUCH_CLICKED,
                                 NULL);
            // Create Acne_text (hg_label)
            Acne_text = gui_text_create((gui_obj_t *)note, "Acne_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)Acne_text, "Acne", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 4, 32);
            gui_text_type_set((gui_text_t *)Acne_text, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Acne_text, LEFT);
            break;
        }
    case 5:
        {
            // Create cycle_tracking_symptom_list_bg6 (hg_rect)
            cycle_tracking_symptom_list_bg6 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg6", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_HairLoss (hg_button)
            hg_button_HairLoss = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_HairLoss",
                                                                     "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_HairLoss_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_HairLoss, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_HairLoss, hg_button_HairLoss_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649",
                                  27, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649,
                         "Hair Loss", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649,
                              LEFT);
            break;
        }
    case 6:
        {
            // Create cycle_tracking_symptom_list_bg7 (hg_rect)
            cycle_tracking_symptom_list_bg7 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg7", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create bg_button_Nausea (hg_button)
            bg_button_Nausea = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "bg_button_Nausea",
                                                                   "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (bg_button_Nausea_state)
            {
                gui_img_set_src((gui_img_t *)bg_button_Nausea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)bg_button_Nausea, bg_button_Nausea_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527",
                                  27, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527,
                         "Nausea", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 6, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527,
                              LEFT);
            break;
        }
    case 7:
        {
            // Create cycle_tracking_symptom_list_bg8 (hg_rect)
            cycle_tracking_symptom_list_bg8 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg8", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_FeelCold (hg_button)
            hg_button_FeelCold = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_FeelCold",
                                                                     "/app_phone/icon_bg.bin", 347, 25, 72, 72);
            if (hg_button_FeelCold_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_FeelCold, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_FeelCold, hg_button_FeelCold_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415",
                                  27, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415,
                         "Feel Cold", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415,
                              LEFT);
            break;
        }
    case 8:
        {
            // Create cycle_tracking_symptom_list_bg9 (hg_rect)
            cycle_tracking_symptom_list_bg9 = gui_rect_create((gui_obj_t *)note,
                                                              "cycle_tracking_symptom_list_bg9", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_AbdominalColic (hg_button)
            hg_button_AbdominalColic = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                           "hg_button_AbdominalColic", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_AbdominalColic_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_AbdominalColic,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_AbdominalColic, hg_button_AbdominalColic_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766",
                                  27, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766,
                         "Abdominal Colic", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 15, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766,
                              LEFT);
            break;
        }
    case 9:
        {
            // Create cycle_tracking_symptom_list_bg10 (hg_rect)
            cycle_tracking_symptom_list_bg10 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg10", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Diarrhea (hg_button)
            hg_button_Diarrhea = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_Diarrhea",
                                                                     "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_Diarrhea_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Diarrhea, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Diarrhea, hg_button_Diarrhea_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608",
                                  27, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608,
                         "Diarrhea", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608,
                              LEFT);
            break;
        }
    case 10:
        {
            // Create cycle_tracking_symptom_list_bg10_copy_1769765685129 (hg_rect)
            cycle_tracking_symptom_list_bg10_copy_1769765685129 = gui_rect_create((gui_obj_t *)note,
                                                                                  "cycle_tracking_symptom_list_bg10_copy_1769765685129", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_AbdominalDistension (hg_button)
            hg_button_AbdominalDistension = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                                "hg_button_AbdominalDistension", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_AbdominalDistension_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_AbdominalDistension,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_AbdominalDistension,
                                 hg_button_AbdominalDistension_toggle_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497",
                                  27, 34, 351, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497,
                         "Abdominal Distension", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 20, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765557497,
                              LEFT);
            break;
        }
    case 11:
        {
            // Create cycle_tracking_symptom_list_bg12 (hg_rect)
            cycle_tracking_symptom_list_bg12 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg12", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_MemoryDecline (hg_button)
            hg_button_MemoryDecline = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                          "hg_button_MemoryDecline", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_MemoryDecline_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_MemoryDecline,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_MemoryDecline, hg_button_MemoryDecline_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944",
                                  44, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944,
                         "Memory Decline", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 14, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944,
                              LEFT);
            break;
        }
    case 12:
        {
            // Create cycle_tracking_symptom_list_bg13 (hg_rect)
            cycle_tracking_symptom_list_bg13 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg13", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_PelvicPain (hg_button)
            hg_button_PelvicPain = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                       "hg_button_PelvicPain", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_PelvicPain_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_PelvicPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_PelvicPain, hg_button_PelvicPain_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817",
                                  30, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817,
                         "Pelvic Pain", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817,
                              LEFT);
            break;
        }
    case 13:
        {
            // Create cycle_tracking_symptom_list_bg14 (hg_rect)
            cycle_tracking_symptom_list_bg14 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg14", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_DrySkin (hg_button)
            hg_button_DrySkin = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_DrySkin",
                                                                    "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_DrySkin_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_DrySkin, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_DrySkin, hg_button_DrySkin_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016",
                                  30, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016,
                         "Dry Skin", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016,
                              LEFT);
            break;
        }
    case 14:
        {
            // Create cycle_tracking_symptom_list_bg15 (hg_rect)
            cycle_tracking_symptom_list_bg15 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg15", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Fatigue (hg_button)
            hg_button_Fatigue = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_Fatigue",
                                                                    "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_Fatigue_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Fatigue, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Fatigue, hg_button_Fatigue_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544",
                                  30, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544,
                         "Fatigue", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 7, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544,
                              LEFT);
            break;
        }
    case 15:
        {
            // Create cycle_tracking_symptom_list_bg16 (hg_rect)
            cycle_tracking_symptom_list_bg16 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg16", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_BreastPain (hg_button)
            hg_button_BreastPain = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                       "hg_button_BreastPain", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_BreastPain_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_BreastPain, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_BreastPain, hg_button_BreastPain_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528 (hg_label)
            hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528
                = gui_text_create((gui_obj_t *)note,
                                  "hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528",
                                  30, 34, 264, 42);
            gui_text_set((gui_text_t *)
                         hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528,
                         "Breast Pain", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1769765239790_eu1e_copy_1769765328854_copy_1769765388662_copy_1769765452543_copy_1769765509712_copy_1769765520649_copy_1769765527527_copy_1769765539415_copy_1769765544766_copy_1769765551608_copy_1769765583944_copy_1769765701817_copy_1769765710016_copy_1769765719544_copy_1769765725528,
                              LEFT);
            break;
        }
    case 16:
        {
            // Create cycle_tracking_symptom_list_bg17 (hg_rect)
            cycle_tracking_symptom_list_bg17 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg17", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_ChangesAppetite (hg_button)
            hg_button_ChangesAppetite = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                            "hg_button_ChangesAppetite", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_ChangesAppetite_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_ChangesAppetite,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_ChangesAppetite, hg_button_ChangesAppetite_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create ChangesAppetite_text (hg_label)
            ChangesAppetite_text = gui_text_create((gui_obj_t *)note, "ChangesAppetite_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)ChangesAppetite_text, "Changes in Appetite", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 19, 32);
            gui_text_type_set((gui_text_t *)ChangesAppetite_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)ChangesAppetite_text, LEFT);
            break;
        }
    case 17:
        {
            // Create cycle_tracking_symptom_list_bg18 (hg_rect)
            cycle_tracking_symptom_list_bg18 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg18", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_SleepChanges (hg_button)
            hg_button_SleepChanges = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                         "hg_button_SleepChanges", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_SleepChanges_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_SleepChanges,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_SleepChanges, hg_button_SleepChanges_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create SleepChanges_text (hg_label)
            SleepChanges_text = gui_text_create((gui_obj_t *)note, "SleepChanges_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)SleepChanges_text, "Sleep Changes", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 13, 32);
            gui_text_type_set((gui_text_t *)SleepChanges_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)SleepChanges_text, LEFT);
            break;
        }
    case 18:
        {
            // Create cycle_tracking_symptom_list_bg19 (hg_rect)
            cycle_tracking_symptom_list_bg19 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg19", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_Headache (hg_button)
            hg_button_Headache = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "hg_button_Headache",
                                                                     "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_Headache_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_Headache, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_Headache, hg_button_Headache_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create Headache_text (hg_label)
            Headache_text = gui_text_create((gui_obj_t *)note, "Headache_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)Headache_text, "Headache", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8,
                         32);
            gui_text_type_set((gui_text_t *)Headache_text, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)Headache_text, LEFT);
            break;
        }
    case 19:
        {
            // Create cycle_tracking_symptom_list_bg20 (hg_rect)
            cycle_tracking_symptom_list_bg20 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg20", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_LowerBackPain (hg_button)
            hg_button_LowerBackPain = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                          "hg_button_LowerBackPain", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_LowerBackPain_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_LowerBackPain,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_LowerBackPain, hg_button_LowerBackPain_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create LowerBackPain_text (hg_label)
            LowerBackPain_text = gui_text_create((gui_obj_t *)note, "LowerBackPain_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)LowerBackPain_text, "Lower Back Pain", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 15, 32);
            gui_text_type_set((gui_text_t *)LowerBackPain_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)LowerBackPain_text, LEFT);
            break;
        }
    case 20:
        {
            // Create cycle_tracking_symptom_list_bg21 (hg_rect)
            cycle_tracking_symptom_list_bg21 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg21", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_MoodChanges (hg_button)
            hg_button_MoodChanges = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                        "hg_button_MoodChanges", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_MoodChanges_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_MoodChanges, (const uint8_t *)"/app_noise/Noise_ok_icon.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_MoodChanges, hg_button_MoodChanges_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create MoodChanges_text (hg_label)
            MoodChanges_text = gui_text_create((gui_obj_t *)note, "MoodChanges_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)MoodChanges_text, "Mood Changes", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 12, 32);
            gui_text_type_set((gui_text_t *)MoodChanges_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)MoodChanges_text, LEFT);
            break;
        }
    case 21:
        {
            // Create cycle_tracking_symptom_list_bg22 (hg_rect)
            cycle_tracking_symptom_list_bg22 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg22", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_SweatingNight (hg_button)
            hg_button_SweatingNight = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                          "hg_button_SweatingNight", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_SweatingNight_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_SweatingNight,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_SweatingNight, hg_button_SweatingNight_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create SweatingNight_text (hg_label)
            SweatingNight_text = gui_text_create((gui_obj_t *)note, "SweatingNight_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)SweatingNight_text, "Sweating at Night", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 17, 32);
            gui_text_type_set((gui_text_t *)SweatingNight_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)SweatingNight_text, LEFT);
            break;
        }
    case 22:
        {
            // Create cycle_tracking_symptom_list_bg23 (hg_rect)
            cycle_tracking_symptom_list_bg23 = gui_rect_create((gui_obj_t *)note,
                                                               "cycle_tracking_symptom_list_bg23", 10, 0, 390, 100, 20, gui_rgb(44, 44, 46));
            // Create hg_button_VaginalDryness (hg_button)
            hg_button_VaginalDryness = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note,
                                                                           "hg_button_VaginalDryness", "/app_phone/icon_bg.bin", 345, 25, 72, 72);
            if (hg_button_VaginalDryness_state)
            {
                gui_img_set_src((gui_img_t *)hg_button_VaginalDryness,
                                (const uint8_t *)"/app_noise/Noise_ok_icon.bin", IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)hg_button_VaginalDryness, hg_button_VaginalDryness_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create VaginalDryness_text (hg_label)
            VaginalDryness_text = gui_text_create((gui_obj_t *)note, "VaginalDryness_text", 27, 34, 264, 42);
            gui_text_set((gui_text_t *)VaginalDryness_text, "Vaginal Dryness", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 15, 32);
            gui_text_type_set((gui_text_t *)VaginalDryness_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)VaginalDryness_text, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_cycle_tracking_view (hg_view)
static void app_cycle_tracking_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_cycle_tracking_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create cycle_track_list (hg_list)
    cycle_track_list = gui_list_create((gui_obj_t *)view, "cycle_track_list", 0, 110, 410, 150, 54, 5,
                                       HORIZONTAL, cycle_track_list_note_design, NULL, false);
    gui_list_set_style(cycle_track_list, LIST_ZOOM);
    gui_list_set_note_num(cycle_track_list, 7);

    // Create hg_label_1769761165740_60hx (hg_label)
    hg_label_1769761165740_60hx = gui_text_create((gui_obj_t *)view, "hg_label_1769761165740_60hx", 128,
                                                  293, 157, 43);
    gui_text_set((gui_text_t *)hg_label_1769761165740_60hx, "TUE, SEP 12", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 11, 26);
    gui_text_type_set((gui_text_t *)hg_label_1769761165740_60hx,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1769761165740_60hx, CENTER);

    // Create app_cycle_tracking_window (hg_window)
    app_cycle_tracking_window = gui_win_create((gui_obj_t *)view, "app_cycle_tracking_window", 0, 0,
                                               410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_cycle_tracking_time_text_time_str, sizeof(app_cycle_tracking_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create app_cycle_tracking_time_text (hg_time_label)
    app_cycle_tracking_time_text = gui_text_create(app_cycle_tracking_window,
                                                   "app_cycle_tracking_time_text", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)app_cycle_tracking_time_text, app_cycle_tracking_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_cycle_tracking_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)app_cycle_tracking_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_cycle_tracking_time_text, RIGHT);

    // Create hg_image_1769761296861_2c5p (hg_image)
    hg_image_1769761296861_2c5p = gui_img_create_from_fs(app_cycle_tracking_window,
                                                         "hg_image_1769761296861_2c5p", "/heart/icon_information.bin", 20, 10, 72, 72);

    gui_obj_add_event_cb(GUI_BASE(app_cycle_tracking_window),
                         (gui_event_cb_t)app_cycle_tracking_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_cycle_tracking_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_cycle_tracking_time_text), 30000, true,
                         app_cycle_tracking_time_text_time_update_cb);

    // Create hg_rect_1769761365236_appj (hg_rect)
    hg_rect_1769761365236_appj = gui_rect_create((gui_obj_t *)view, "hg_rect_1769761365236_appj", 56,
                                                 394, 300, 80, 40, gui_rgb(46, 46, 46));
    gui_obj_add_event_cb(hg_rect_1769761365236_appj,
                         (gui_event_cb_t)hg_rect_1769761365236_appj_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create Light_Flow_and_Acne_text (hg_label)
    Light_Flow_and_Acne_text = gui_text_create((gui_obj_t *)view, "Light_Flow_and_Acne_text", 71, 418,
                                               270, 43);
    gui_text_set((gui_text_t *)Light_Flow_and_Acne_text, "Light Flow and Acne", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 19, 28);
    gui_text_type_set((gui_text_t *)Light_Flow_and_Acne_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)Light_Flow_and_Acne_text, CENTER);
}
GUI_VIEW_INSTANCE("app_cycle_tracking_view", false, app_cycle_tracking_view_switch_in,
                  app_cycle_tracking_view_switch_out, false);

// Create cycle_tracking_record_view (hg_view)
static void cycle_tracking_record_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void cycle_tracking_record_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create cycle_tracking_re_window (hg_window)
    cycle_tracking_re_window = gui_win_create((gui_obj_t *)view, "cycle_tracking_re_window", 0, 0, 410,
                                              110);
    gui_win_enable_blur((gui_win_t *)cycle_tracking_re_window, true);
    gui_win_set_blur_degree((gui_win_t *)cycle_tracking_re_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(cycle_tracking_record_time_text_time_str, sizeof(cycle_tracking_record_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create cycle_tracking_record_time_text (hg_time_label)
    cycle_tracking_record_time_text = gui_text_create(cycle_tracking_re_window,
                                                      "cycle_tracking_record_time_text", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)cycle_tracking_record_time_text,
                 cycle_tracking_record_time_text_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(cycle_tracking_record_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)cycle_tracking_record_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)cycle_tracking_record_time_text, RIGHT);

    // Create cycle_tracking_record_back_icon_bg (hg_image)
    cycle_tracking_record_back_icon_bg = gui_img_create_from_fs(cycle_tracking_re_window,
                                                                "cycle_tracking_record_back_icon_bg", "/app_phone/icon_bg.bin", 20, 10, 72, 72);
    gui_obj_add_event_cb(cycle_tracking_record_back_icon_bg,
                         (gui_event_cb_t)cycle_tracking_record_back_icon_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_image_1769761721084_fcvl (hg_image)
    hg_image_1769761721084_fcvl = gui_img_create_from_fs(cycle_tracking_re_window,
                                                         "hg_image_1769761721084_fcvl", "/app_reminder/reminders_icon5.bin", 44, 23, 25, 47);

    gui_obj_add_event_cb(GUI_BASE(cycle_tracking_re_window),
                         (gui_event_cb_t)cycle_tracking_re_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)cycle_tracking_re_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(cycle_tracking_record_time_text), 30000, true,
                         cycle_tracking_record_time_text_time_update_cb);

    // Create cycle_tracking_record_list (hg_list)
    cycle_tracking_record_list = gui_list_create((gui_obj_t *)view, "cycle_tracking_record_list", 10,
                                                 110, 390, 392, 124, 10, VERTICAL, cycle_tracking_record_list_note_design, NULL, false);
    gui_list_set_style(cycle_tracking_record_list, LIST_CLASSIC);
    gui_list_set_note_num(cycle_tracking_record_list, 3);
    gui_list_set_out_scope(cycle_tracking_record_list, 80);
}
GUI_VIEW_INSTANCE("cycle_tracking_record_view", false, cycle_tracking_record_view_switch_in,
                  cycle_tracking_record_view_switch_out, false);

// Create cycle_tracking_record_Menstruation_view (hg_view)
static void cycle_tracking_record_Menstruation_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void cycle_tracking_record_Menstruation_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create cycle_tracking_menstruation_list (hg_list)
    cycle_tracking_menstruation_list = gui_list_create((gui_obj_t *)view,
                                                       "cycle_tracking_menstruation_list", 10, 108, 390, 394, 100, 5, VERTICAL,
                                                       cycle_tracking_menstruation_list_note_design, NULL, false);
    gui_list_set_style(cycle_tracking_menstruation_list, LIST_CLASSIC);
    gui_list_set_note_num(cycle_tracking_menstruation_list, 7);
    gui_list_set_out_scope(cycle_tracking_menstruation_list, 80);

    // Create cycle_menstruation_window (hg_window)
    cycle_menstruation_window = gui_win_create((gui_obj_t *)view, "cycle_menstruation_window", 0, 0,
                                               410, 110);
    gui_win_enable_blur((gui_win_t *)cycle_menstruation_window, true);
    gui_win_set_blur_degree((gui_win_t *)cycle_menstruation_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(cycle_tracking_record_time_text_copy_1769762411269_1_time_str,
                 sizeof(cycle_tracking_record_time_text_copy_1769762411269_1_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create cycle_tracking_record_time_text_copy_1769762411269_1 (hg_time_label)
    cycle_tracking_record_time_text_copy_1769762411269_1 = gui_text_create(cycle_menstruation_window,
                                                                           "cycle_tracking_record_time_text_copy_1769762411269_1", 300, 18, 80, 32);
    gui_text_set((gui_text_t *)cycle_tracking_record_time_text_copy_1769762411269_1,
                 cycle_tracking_record_time_text_copy_1769762411269_1_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), strlen(cycle_tracking_record_time_text_copy_1769762411269_1_time_str), 28);
    gui_text_type_set((gui_text_t *)cycle_tracking_record_time_text_copy_1769762411269_1,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)cycle_tracking_record_time_text_copy_1769762411269_1, RIGHT);

    // Create hg_image_1769762439225_63oo (hg_image)
    hg_image_1769762439225_63oo = gui_img_create_from_fs(cycle_menstruation_window,
                                                         "hg_image_1769762439225_63oo", "/app_music/delete_icon.bin", 20, 10, 72, 72);
    gui_obj_add_event_cb(hg_image_1769762439225_63oo,
                         (gui_event_cb_t)hg_image_1769762439225_63oo_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_obj_add_event_cb(GUI_BASE(cycle_menstruation_window),
                         (gui_event_cb_t)cycle_menstruation_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)cycle_menstruation_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(cycle_tracking_record_time_text_copy_1769762411269_1), 30000, true,
                         cycle_tracking_record_time_text_copy_1769762411269_1_time_update_cb);
}
GUI_VIEW_INSTANCE("cycle_tracking_record_Menstruation_view", false,
                  cycle_tracking_record_Menstruation_view_switch_in,
                  cycle_tracking_record_Menstruation_view_switch_out, false);

// Create cycle_tracking_symptom_view (hg_view)
static void cycle_tracking_symptom_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void cycle_tracking_symptom_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create hg_list_1769763676513_n1s4 (hg_list)
    hg_list_1769763676513_n1s4 = gui_list_create((gui_obj_t *)view, "hg_list_1769763676513_n1s4", 3,
                                                 106, 407, 396, 100, 5, VERTICAL, hg_list_1769763676513_n1s4_note_design, NULL, false);
    gui_list_set_style(hg_list_1769763676513_n1s4, LIST_CLASSIC);
    gui_list_set_note_num(hg_list_1769763676513_n1s4, 23);

    // Create cycle_tracking_symptom_window (hg_window)
    cycle_tracking_symptom_window = gui_win_create((gui_obj_t *)view, "cycle_tracking_symptom_window",
                                                   0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)cycle_tracking_symptom_window, true);
    gui_win_set_blur_degree((gui_win_t *)cycle_tracking_symptom_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(cycle_tracking_symptom_time_text_time_str,
                 sizeof(cycle_tracking_symptom_time_text_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create cycle_tracking_symptom_time_text (hg_time_label)
    cycle_tracking_symptom_time_text = gui_text_create(cycle_tracking_symptom_window,
                                                       "cycle_tracking_symptom_time_text", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)cycle_tracking_symptom_time_text,
                 cycle_tracking_symptom_time_text_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(cycle_tracking_symptom_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)cycle_tracking_symptom_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)cycle_tracking_symptom_time_text, RIGHT);

    // Create hg_image_1769763659121_in3z (hg_image)
    hg_image_1769763659121_in3z = gui_img_create_from_fs(cycle_tracking_symptom_window,
                                                         "hg_image_1769763659121_in3z", "/app_music/delete_icon.bin", 20, 10, 72, 72);
    gui_obj_add_event_cb(hg_image_1769763659121_in3z,
                         (gui_event_cb_t)hg_image_1769763659121_in3z_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_obj_add_event_cb(GUI_BASE(cycle_tracking_symptom_window),
                         (gui_event_cb_t)cycle_tracking_symptom_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)cycle_tracking_symptom_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(cycle_tracking_symptom_time_text), 30000, true,
                         cycle_tracking_symptom_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("cycle_tracking_symptom_view", false, cycle_tracking_symptom_view_switch_in,
                  cycle_tracking_symptom_view_switch_out, false);
