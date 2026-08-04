/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_STOPWATCH_USER_H
#define APP_STOPWATCH_USER_H

#include "../callbacks/app_stopwatch_callbacks.h"
#include "../ui/app_stopwatch_ui.h"

#define TIME_CNT_NUM 10
typedef enum
{
    DEFAULT = 0,
    START,
    STOP,
} STOPWATCH_STATUS;

#define ICON_START_X    25 + 320
#define ICON_START_Y    21 + 414
#define ICON_STOP_X     23 + 320
#define ICON_STOP_Y     23 + 414
#define ICON_MARK_X     21 + 18
#define ICON_MARK_Y     21 + 414
#define ICON_RESET_X    14 + 18
#define ICON_RESET_Y    14 + 414

#define COLOR_START  GUI_COLOR_ARGB8888(0xFF, 0x65, 0xDC, 0x7B) //65DC7B
#define COLOR_STOP   GUI_COLOR_ARGB8888(0xFF, 0xFE, 0x37, 0x2C) //FE372C
#define COLOR_RESET  GUI_COLOR_ARGB8888(0xFF, 0xB7, 0xB7, 0xB7) //B7B7B7
#define COLOR_MARK   GUI_COLOR_ARGB8888(0xFF, 0xFF, 0xFF, 0xFF) //FFFFFF
#define COLOR_HAND   GUI_COLOR_ARGB8888(0xFF, 0xEC, 0x60, 0x2A) //EC602A

extern STOPWATCH_STATUS status;
extern uint32_t time_count; //milsec
extern uint32_t time_count_array[TIME_CNT_NUM]; //milsec
extern uint8_t time_count_index;
extern char count_str[10];
extern char count_str_array[TIME_CNT_NUM][10];
extern const char *lap_str_array[TIME_CNT_NUM];
extern gui_text_t *t_lap_array[6];

void click_button_l(void *obj, gui_event_t *e);
void click_button_r(void *obj, gui_event_t *e);

#endif // APP_STOPWATCH_USER_H
