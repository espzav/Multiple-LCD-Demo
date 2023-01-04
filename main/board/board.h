/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "driver/gpio.h"
#include "lcd.h"

/* BOARD TYPES */
#define BOARD_TYPE_HMI      1   /* ESP32-S2-HMI-DevKit-1 */
#define BOARD_TYPE_ESPBOX   2   /* ESP_BOX */
#define BOARD_TYPE_CUSTOM   3   /* Custom board - multiple display example */

/* DISPLAY TOUCH TYPES */
#define BOARD_DISP_TOUCH_GT911      1
#define BOARD_DISP_TOUCH_TT21100    2
#define BOARD_DISP_TOUCH_FT5X06     3

/* DISPLAY GRAPHIC TYPES */
#define BOARD_DISP_LCD_GC9A01   1
#define BOARD_DISP_LCD_RA8875   2
#define BOARD_DISP_LCD_RM68120  3
#define BOARD_DISP_LCD_SH1107   4
#define BOARD_DISP_LCD_DLPC2607 5
#define BOARD_DISP_LCD_NT35510  6
#define BOARD_DISP_LCD_ST7789   7

/* Selected board */
#define BOARD_TYPE BOARD_TYPE_CUSTOM

#if(BOARD_TYPE == BOARD_TYPE_HMI)
    #include "board_hmi.h"
#elif (BOARD_TYPE == BOARD_TYPE_ESPBOX)
    #include "board_espbox.h"
#elif (BOARD_TYPE == BOARD_TYPE_CUSTOM)
    #include "board_custom.h"
#endif
