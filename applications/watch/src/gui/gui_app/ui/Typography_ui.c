/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * Typography UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:41.026Z
 */
#include "Typography_ui.h"
#include "../callbacks/Typography_callbacks.h"
#include "../user/Typography_user.h"
#include <stddef.h>

// Component handle definitions
gui_text_t *typography_lable18 = NULL;
gui_text_t *typography_lable1 = NULL;
gui_text_t *typography_lable2 = NULL;
gui_text_t *typography_lable3 = NULL;
gui_text_t *typography_lable4 = NULL;
gui_text_t *typography_lable5 = NULL;
gui_text_t *typography_lable19 = NULL;
gui_text_t *typography_lable6 = NULL;
gui_text_t *typography_lable17 = NULL;
gui_text_t *typography_lable8 = NULL;
gui_text_t *typography_lable9 = NULL;
gui_text_t *typography_lable10 = NULL;
gui_text_t *typography_lable20 = NULL;
gui_text_t *typography_lable11 = NULL;
gui_text_t *typography_lable12 = NULL;
gui_text_t *typography_lable13 = NULL;
gui_text_t *typography_lable14 = NULL;
gui_text_t *typography_lable15 = NULL;
gui_text_t *typography_lable21 = NULL;
gui_text_t *typography_lable22 = NULL;
gui_text_t *typography_lable23 = NULL;
gui_text_t *typography_lable24 = NULL;
gui_text_t *typography_lable25 = NULL;
gui_text_t *typography_lable26 = NULL;
gui_text_t *typography_lable27 = NULL;
gui_text_t *typography_lable28 = NULL;
gui_text_t *typography_lable29 = NULL;
gui_text_t *typography_lable30 = NULL;
gui_text_t *typography_lable31 = NULL;
gui_text_t *typography_lable32 = NULL;
gui_text_t *typography_lable33 = NULL;
gui_text_t *typography_lable34 = NULL;
gui_text_t *typography_lable35 = NULL;
gui_text_t *typography_lable36 = NULL;
gui_text_t *typography_lable37 = NULL;
gui_text_t *typography_lable38 = NULL;
gui_text_t *typography_lable39 = NULL;
gui_text_t *typography_lable40 = NULL;
gui_text_t *typography_lable41 = NULL;
gui_text_t *typography_lable42 = NULL;
gui_text_t *typography_lable43 = NULL;
gui_text_t *typography_lable44 = NULL;
gui_text_t *lbl_3 = NULL;


// Create view_1 (hg_view)
static void view_1_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void view_1_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create typography_lable18 (hg_label)
    typography_lable18 = gui_text_create((gui_obj_t *)view, "typography_lable18", 24, 32, 160, 18);
    gui_text_set((gui_text_t *)typography_lable18, "LightItalic", GUI_FONT_SRC_BMP, gui_rgb(229, 148,
                 55), 11, 16);
    gui_text_type_set((gui_text_t *)typography_lable18,
                      "/font/Inter24pt_LightItalic_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable18, LEFT);

    // Create typography_lable1 (hg_label)
    typography_lable1 = gui_text_create((gui_obj_t *)view, "typography_lable1", 24, 60, 150, 34);
    gui_text_set((gui_text_t *)typography_lable1, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable1,
                      "/font/Inter24pt_LightItalic_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable1, LEFT);

    // Create typography_lable2 (hg_label)
    typography_lable2 = gui_text_create((gui_obj_t *)view, "typography_lable2", 24, 88, 150, 38);
    gui_text_set((gui_text_t *)typography_lable2, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable2,
                      "/font/Inter24pt_LightItalic_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable2, LEFT);

    // Create typography_lable3 (hg_label)
    typography_lable3 = gui_text_create((gui_obj_t *)view, "typography_lable3", 24, 118, 150, 42);
    gui_text_set((gui_text_t *)typography_lable3, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable3,
                      "/font/Inter24pt_LightItalic_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable3, LEFT);

    // Create typography_lable4 (hg_label)
    typography_lable4 = gui_text_create((gui_obj_t *)view, "typography_lable4", 24, 154, 150, 46);
    gui_text_set((gui_text_t *)typography_lable4, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable4,
                      "/font/Inter24pt_LightItalic_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable4, LEFT);

    // Create typography_lable5 (hg_label)
    typography_lable5 = gui_text_create((gui_obj_t *)view, "typography_lable5", 24, 194, 150, 50);
    gui_text_set((gui_text_t *)typography_lable5, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable5,
                      "/font/Inter24pt_LightItalic_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable5, LEFT);

    // Create typography_lable19 (hg_label)
    typography_lable19 = gui_text_create((gui_obj_t *)view, "typography_lable19", 220, 32, 160, 18);
    gui_text_set((gui_text_t *)typography_lable19, "Medium", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55), 6,
                 16);
    gui_text_type_set((gui_text_t *)typography_lable19,
                      "/font/Inter24pt_Medium_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable19, LEFT);

    // Create typography_lable6 (hg_label)
    typography_lable6 = gui_text_create((gui_obj_t *)view, "typography_lable6", 220, 60, 150, 34);
    gui_text_set((gui_text_t *)typography_lable6, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable6, "/font/Inter24pt_Medium_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable6, LEFT);

    // Create typography_lable17 (hg_label)
    typography_lable17 = gui_text_create((gui_obj_t *)view, "typography_lable17", 220, 88, 150, 38);
    gui_text_set((gui_text_t *)typography_lable17, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable17,
                      "/font/Inter24pt_Medium_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable17, LEFT);

    // Create typography_lable8 (hg_label)
    typography_lable8 = gui_text_create((gui_obj_t *)view, "typography_lable8", 220, 118, 150, 42);
    gui_text_set((gui_text_t *)typography_lable8, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable8, "/font/Inter24pt_Medium_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable8, LEFT);

    // Create typography_lable9 (hg_label)
    typography_lable9 = gui_text_create((gui_obj_t *)view, "typography_lable9", 220, 154, 150, 46);
    gui_text_set((gui_text_t *)typography_lable9, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable9, "/font/Inter24pt_Medium_size36_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable9, LEFT);

    // Create typography_lable10 (hg_label)
    typography_lable10 = gui_text_create((gui_obj_t *)view, "typography_lable10", 220, 194, 150, 50);
    gui_text_set((gui_text_t *)typography_lable10, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable10,
                      "/font/Inter24pt_Medium_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable10, LEFT);

    // Create typography_lable20 (hg_label)
    typography_lable20 = gui_text_create((gui_obj_t *)view, "typography_lable20", 24, 276, 160, 18);
    gui_text_set((gui_text_t *)typography_lable20, "MediumItalic", GUI_FONT_SRC_BMP, gui_rgb(229, 148,
                 55), 12, 16);
    gui_text_type_set((gui_text_t *)typography_lable20,
                      "/font/Inter24pt_MediumItalic_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable20, LEFT);

    // Create typography_lable11 (hg_label)
    typography_lable11 = gui_text_create((gui_obj_t *)view, "typography_lable11", 24, 304, 150, 34);
    gui_text_set((gui_text_t *)typography_lable11, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable11,
                      "/font/Inter24pt_MediumItalic_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable11, LEFT);

    // Create typography_lable12 (hg_label)
    typography_lable12 = gui_text_create((gui_obj_t *)view, "typography_lable12", 24, 332, 150, 38);
    gui_text_set((gui_text_t *)typography_lable12, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable12,
                      "/font/Inter24pt_MediumItalic_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable12, LEFT);

    // Create typography_lable13 (hg_label)
    typography_lable13 = gui_text_create((gui_obj_t *)view, "typography_lable13", 24, 362, 150, 42);
    gui_text_set((gui_text_t *)typography_lable13, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable13,
                      "/font/Inter24pt_MediumItalic_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable13, LEFT);

    // Create typography_lable14 (hg_label)
    typography_lable14 = gui_text_create((gui_obj_t *)view, "typography_lable14", 24, 398, 150, 46);
    gui_text_set((gui_text_t *)typography_lable14, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable14,
                      "/font/Inter24pt_MediumItalic_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable14, LEFT);

    // Create typography_lable15 (hg_label)
    typography_lable15 = gui_text_create((gui_obj_t *)view, "typography_lable15", 24, 438, 150, 50);
    gui_text_set((gui_text_t *)typography_lable15, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable15,
                      "/font/Inter24pt_MediumItalic_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable15, LEFT);

    // Create typography_lable21 (hg_label)
    typography_lable21 = gui_text_create((gui_obj_t *)view, "typography_lable21", 220, 276, 160, 18);
    gui_text_set((gui_text_t *)typography_lable21, "SemiBold", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55),
                 8, 16);
    gui_text_type_set((gui_text_t *)typography_lable21,
                      "/font/Inter24pt_SemiBold_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable21, LEFT);

    // Create typography_lable22 (hg_label)
    typography_lable22 = gui_text_create((gui_obj_t *)view, "typography_lable22", 220, 304, 150, 34);
    gui_text_set((gui_text_t *)typography_lable22, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable22,
                      "/font/Inter24pt_SemiBold_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable22, LEFT);

    // Create typography_lable23 (hg_label)
    typography_lable23 = gui_text_create((gui_obj_t *)view, "typography_lable23", 220, 332, 150, 38);
    gui_text_set((gui_text_t *)typography_lable23, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable23,
                      "/font/Inter24pt_SemiBold_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable23, LEFT);

    // Create typography_lable24 (hg_label)
    typography_lable24 = gui_text_create((gui_obj_t *)view, "typography_lable24", 220, 362, 150, 42);
    gui_text_set((gui_text_t *)typography_lable24, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable24,
                      "/font/Inter24pt_SemiBold_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable24, LEFT);

    // Create typography_lable25 (hg_label)
    typography_lable25 = gui_text_create((gui_obj_t *)view, "typography_lable25", 220, 398, 150, 46);
    gui_text_set((gui_text_t *)typography_lable25, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable25,
                      "/font/Inter24pt_SemiBold_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable25, LEFT);

    // Create typography_lable26 (hg_label)
    typography_lable26 = gui_text_create((gui_obj_t *)view, "typography_lable26", 220, 438, 150, 50);
    gui_text_set((gui_text_t *)typography_lable26, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable26,
                      "/font/Inter24pt_SemiBold_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable26, LEFT);
}
GUI_VIEW_INSTANCE("view_1", false, view_1_switch_in, view_1_switch_out, false);

// Create view_2 (hg_view)
static void view_2_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void view_2_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create typography_lable27 (hg_label)
    typography_lable27 = gui_text_create((gui_obj_t *)view, "typography_lable27", 24, 32, 160, 18);
    gui_text_set((gui_text_t *)typography_lable27, "Italic", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55), 6,
                 16);
    gui_text_type_set((gui_text_t *)typography_lable27,
                      "/font/Inter_24pt_Italic_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable27, LEFT);

    // Create typography_lable28 (hg_label)
    typography_lable28 = gui_text_create((gui_obj_t *)view, "typography_lable28", 24, 60, 150, 34);
    gui_text_set((gui_text_t *)typography_lable28, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable28,
                      "/font/Inter_24pt_Italic_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable28, LEFT);

    // Create typography_lable29 (hg_label)
    typography_lable29 = gui_text_create((gui_obj_t *)view, "typography_lable29", 24, 88, 150, 38);
    gui_text_set((gui_text_t *)typography_lable29, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable29,
                      "/font/Inter_24pt_Italic_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable29, LEFT);

    // Create typography_lable30 (hg_label)
    typography_lable30 = gui_text_create((gui_obj_t *)view, "typography_lable30", 24, 118, 150, 42);
    gui_text_set((gui_text_t *)typography_lable30, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable30,
                      "/font/Inter_24pt_Italic_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable30, LEFT);

    // Create typography_lable31 (hg_label)
    typography_lable31 = gui_text_create((gui_obj_t *)view, "typography_lable31", 24, 154, 150, 46);
    gui_text_set((gui_text_t *)typography_lable31, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable31,
                      "/font/Inter_24pt_Italic_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable31, LEFT);

    // Create typography_lable32 (hg_label)
    typography_lable32 = gui_text_create((gui_obj_t *)view, "typography_lable32", 24, 194, 150, 50);
    gui_text_set((gui_text_t *)typography_lable32, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable32,
                      "/font/Inter_24pt_Italic_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable32, LEFT);

    // Create typography_lable33 (hg_label)
    typography_lable33 = gui_text_create((gui_obj_t *)view, "typography_lable33", 220, 32, 160, 18);
    gui_text_set((gui_text_t *)typography_lable33, "Light", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55), 5,
                 16);
    gui_text_type_set((gui_text_t *)typography_lable33,
                      "/font/Inter_24pt_Light_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable33, LEFT);

    // Create typography_lable34 (hg_label)
    typography_lable34 = gui_text_create((gui_obj_t *)view, "typography_lable34", 220, 60, 150, 34);
    gui_text_set((gui_text_t *)typography_lable34, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable34,
                      "/font/Inter_24pt_Light_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable34, LEFT);

    // Create typography_lable35 (hg_label)
    typography_lable35 = gui_text_create((gui_obj_t *)view, "typography_lable35", 220, 88, 150, 38);
    gui_text_set((gui_text_t *)typography_lable35, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable35,
                      "/font/Inter_24pt_Light_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable35, LEFT);

    // Create typography_lable36 (hg_label)
    typography_lable36 = gui_text_create((gui_obj_t *)view, "typography_lable36", 220, 118, 150, 42);
    gui_text_set((gui_text_t *)typography_lable36, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable36,
                      "/font/Inter_24pt_Light_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable36, LEFT);

    // Create typography_lable37 (hg_label)
    typography_lable37 = gui_text_create((gui_obj_t *)view, "typography_lable37", 220, 154, 150, 46);
    gui_text_set((gui_text_t *)typography_lable37, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable37,
                      "/font/Inter_24pt_Light_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable37, LEFT);

    // Create typography_lable38 (hg_label)
    typography_lable38 = gui_text_create((gui_obj_t *)view, "typography_lable38", 220, 194, 150, 50);
    gui_text_set((gui_text_t *)typography_lable38, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable38,
                      "/font/Inter_24pt_Light_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable38, LEFT);

    // Create typography_lable39 (hg_label)
    typography_lable39 = gui_text_create((gui_obj_t *)view, "typography_lable39", 122, 276, 160, 18);
    gui_text_set((gui_text_t *)typography_lable39, "Regular", GUI_FONT_SRC_BMP, gui_rgb(229, 148, 55),
                 7, 16);
    gui_text_type_set((gui_text_t *)typography_lable39,
                      "/font/Inter_24pt_Regular_size16_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable39, LEFT);

    // Create typography_lable40 (hg_label)
    typography_lable40 = gui_text_create((gui_obj_t *)view, "typography_lable40", 122, 304, 150, 34);
    gui_text_set((gui_text_t *)typography_lable40, "Aa 24", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)typography_lable40,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable40, LEFT);

    // Create typography_lable41 (hg_label)
    typography_lable41 = gui_text_create((gui_obj_t *)view, "typography_lable41", 122, 332, 150, 38);
    gui_text_set((gui_text_t *)typography_lable41, "Aa 28", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 28);
    gui_text_type_set((gui_text_t *)typography_lable41,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable41, LEFT);

    // Create typography_lable42 (hg_label)
    typography_lable42 = gui_text_create((gui_obj_t *)view, "typography_lable42", 122, 362, 150, 42);
    gui_text_set((gui_text_t *)typography_lable42, "Aa 32", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 32);
    gui_text_type_set((gui_text_t *)typography_lable42,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable42, LEFT);

    // Create typography_lable43 (hg_label)
    typography_lable43 = gui_text_create((gui_obj_t *)view, "typography_lable43", 122, 398, 150, 46);
    gui_text_set((gui_text_t *)typography_lable43, "Aa 36", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 36);
    gui_text_type_set((gui_text_t *)typography_lable43,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable43, LEFT);

    // Create typography_lable44 (hg_label)
    typography_lable44 = gui_text_create((gui_obj_t *)view, "typography_lable44", 122, 438, 150, 50);
    gui_text_set((gui_text_t *)typography_lable44, "Aa 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 40);
    gui_text_type_set((gui_text_t *)typography_lable44,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)typography_lable44, LEFT);
}
GUI_VIEW_INSTANCE("view_2", false, view_2_switch_in, view_2_switch_out, false);

// Create view_3 (hg_view)
static void view_3_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void view_3_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create lbl_3 (hg_label)
    lbl_3 = gui_text_create((gui_obj_t *)view, "lbl_3", 51, 116, 220, 50);
    gui_text_set((gui_text_t *)lbl_3, "中文 40", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 9, 40);
    gui_text_type_set((gui_text_t *)lbl_3, "/font/NotoSansSC_Regular_size40_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)lbl_3, LEFT);
}
GUI_VIEW_INSTANCE("view_3", false, view_3_switch_in, view_3_switch_out, false);
