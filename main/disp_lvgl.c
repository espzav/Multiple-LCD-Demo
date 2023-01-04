/* LVGL SPI LCD display example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "disp_lvgl.h"
#include "app_network.h"
#include "app_battery.h"
#include "board.h"
#include "lv_100ask_2048/lv_100ask_2048.h"
#include "app_sd.h"
#include "list.h"
#include "freertos/FreeRTOS.h"
#include "esp_lvgl_port.h"

#ifndef PI
#define PI  (3.14159f)
#endif

/*******************************************************************************
* Types definitions
*******************************************************************************/

typedef struct {
    lv_obj_t * scr;
    lv_obj_t * img_logo;
    lv_obj_t * img_text;
    lv_obj_t * lbl_time;
    lv_obj_t * lbl_main_time;
    lv_obj_t * lbl_wifi;
    lv_obj_t * tabview;

    /* Monochrome time and date */
    lv_obj_t * lbl_monochrome_time;
    lv_obj_t * lbl_monochrome_date;

    /* Keyboard on touch screen */
    lv_obj_t * keyboard;

    /* Bigger display main screen */
    lv_obj_t * main_screen;

    /* Clock */
    lv_obj_t * clock_meter;
    lv_meter_indicator_t * indic_hour;
    lv_meter_indicator_t * indic_min;
    lv_meter_indicator_t * indic_sec;

    /* Weather */
    lv_obj_t * img_weather;
    lv_obj_t * lbl_weather_temp;
    lv_obj_t * lbl_weather_place;
    lv_obj_t * lbl_weather_desc;
    /* Weather - small display */
    lv_obj_t * img_small_weather;
    lv_obj_t * lbl_small_weather;
    /* Date - small display */
    lv_obj_t * lbl_small_date;
    /* Battery */
    lv_obj_t * img_bat;
    lv_obj_t * lbl_bat;

    /* WiFi */
    lv_obj_t * dd_ssid;
    lv_obj_t * ta_wifi_pass;
    lv_obj_t * btn_refresh;
    lv_obj_t * cont_wifi_info;
    lv_obj_t * lbl_info_ssid;
    lv_obj_t * lbl_info_ip;
    lv_obj_t * lbl_info_mac;

    /* Game */
    lv_obj_t * tab_game;
    lv_obj_t * obj_2048;
    lv_obj_t * game_over;
    lv_obj_t * lv_score_list;
    list_t score_list;

    lv_group_t * tp_group;

    lv_obj_t * arc[3];
    int count_val;
} disp_lvgl_disp_ctx_t;

typedef struct lvgl_weather_icon_s
{
    const char * name;
    const lv_img_dsc_t * img;
} lvgl_weather_icon_t;

typedef struct game_score_s
{
    int score;
    int best_tile;
    char * name;
} game_score_t;

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "LVGL";

// LVGL image declare
LV_IMG_DECLARE(img_background)
LV_IMG_DECLARE(esp_logo)
LV_IMG_DECLARE(esp_text)
/* Weather images */
LV_IMG_DECLARE(weather_01d)
LV_IMG_DECLARE(weather_02d)
LV_IMG_DECLARE(weather_03d)
LV_IMG_DECLARE(weather_04d)
LV_IMG_DECLARE(weather_09d)
LV_IMG_DECLARE(weather_10d)
LV_IMG_DECLARE(weather_11d)
LV_IMG_DECLARE(weather_13d)
LV_IMG_DECLARE(weather_50d)
LV_IMG_DECLARE(weather_01n)
LV_IMG_DECLARE(weather_02n)
LV_IMG_DECLARE(weather_10n)


// LVGL font declare
LV_FONT_DECLARE(digital_30)
LV_FONT_DECLARE(digital_50_bold)

static const char * WEEKDAYS[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

static const char * WEEKDAYS_SHORT[] =
{
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
};

static const lvgl_weather_icon_t weather_icons[] =
{
    {"01d", &weather_01d},
    {"02d", &weather_02d},
    {"03d", &weather_03d},
    {"04d", &weather_04d},
    {"09d", &weather_09d},
    {"10d", &weather_10d},
    {"11d", &weather_11d},
    {"13d", &weather_13d},
    {"50d", &weather_50d},

    {"01n", &weather_01n},
    {"02n", &weather_02n},
    {"03n", &weather_03d},
    {"04n", &weather_04d},
    {"09n", &weather_09d},
    {"10n", &weather_10n},
    {"11n", &weather_11d},
    {"13n", &weather_13d},
    {"50n", &weather_50d},
};

static disp_lvgl_disp_ctx_t disp_ctx;
static time_t last_time;
static int last_min = -1;
static int last_sec = -1;
static bool last_game_status = true;
/*******************************************************************************
* Private functions
*******************************************************************************/

static void anim_zoom_cb(void * var, int32_t v)
{
    lv_img_set_zoom(var, v);
}

static void anim_x_cb(void * var, int32_t v)
{
    lv_obj_set_x(var, v);
}

static void anim_y_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

static void anim_img_opa_cb(void * var, int32_t v)
{
    lv_obj_set_style_img_opa(var, v, 0);
}

static void anim_text_opa_cb(void * var, int32_t v)
{
    lv_obj_set_style_text_opa(var, v, 0);
}

static void anim_logo_done_cb(lv_anim_t * anim)
{
}

static void anim_logo_zoom_done_cb(lv_anim_t * anim)
{
    /* Logo animation (example of multiple animation) */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, anim->var);
    lv_anim_set_values(&a, 0, -360);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_start(&a);

    lv_anim_set_values(&a, 0, -210);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_start(&a);

    lv_anim_set_values(&a, lv_img_get_zoom(anim->var), 100);
    lv_anim_set_exec_cb(&a, anim_zoom_cb);
    lv_anim_set_ready_cb(&a, anim_logo_done_cb);
    lv_anim_start(&a);
}

static void lvgl_show_clock(lv_obj_t * screen, uint32_t size)
{
    disp_ctx.clock_meter = lv_meter_create(screen);
    lv_obj_set_size(disp_ctx.clock_meter, size, size);
    lv_obj_center(disp_ctx.clock_meter);

    /* Black background color */
    lv_obj_set_style_bg_color(disp_ctx.clock_meter, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(disp_ctx.clock_meter, 0, 0);
    lv_obj_set_style_pad_top(screen, 0, 0);
    lv_obj_set_style_pad_bottom(screen, 0, 0);
    lv_obj_set_style_pad_left(screen, 0, 0);
    lv_obj_set_style_pad_right(screen, 0, 0);
    lv_obj_set_style_pad_top(disp_ctx.clock_meter, 0, 0);
    lv_obj_set_style_pad_bottom(disp_ctx.clock_meter, 0, 0);
    lv_obj_set_style_pad_left(disp_ctx.clock_meter, 0, 0);
    lv_obj_set_style_pad_right(disp_ctx.clock_meter, 0, 0);

    /* Create a scale for minutes and seconds */
    /* 61 ticks in a 360 degrees range (the last and the first line overlaps) */
    lv_meter_scale_t * scale_min = lv_meter_add_scale(disp_ctx.clock_meter);
    //lv_meter_set_scale_ticks(disp_ctx.clock_meter, scale_min, 61, 1, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_range(disp_ctx.clock_meter, scale_min, 0, 60, 360, 270);

    /* Create another scale for the hours. It's only visual and contains only major ticks */
    lv_meter_scale_t * scale_hour = lv_meter_add_scale(disp_ctx.clock_meter);
    lv_meter_set_scale_ticks(disp_ctx.clock_meter, scale_hour, 12, 0, 0, lv_palette_main(LV_PALETTE_GREY));               /*12 ticks*/
    lv_meter_set_scale_major_ticks(disp_ctx.clock_meter, scale_hour, 1, 2, 20, lv_color_white(), 10);    /*Every tick is major*/
    lv_meter_set_scale_range(disp_ctx.clock_meter, scale_hour, 1, 12, 330, 300);      /*[1..12] values in an almost full circle*/

    LV_IMG_DECLARE(clock_minute)
    LV_IMG_DECLARE(clock_hour)

    /*Add a the hands from images*/
    disp_ctx.indic_hour = lv_meter_add_needle_img(disp_ctx.clock_meter, scale_min, &clock_hour, 5, 5);
    disp_ctx.indic_min = lv_meter_add_needle_img(disp_ctx.clock_meter, scale_min, &clock_minute, 5, 5);
    disp_ctx.indic_sec = lv_meter_add_needle_line(disp_ctx.clock_meter, scale_min, 1, lv_palette_lighten(LV_PALETTE_RED, 1), 1);
}

static const lv_img_dsc_t * lvgl_get_weather_image(char * icon)
{
    for (int i=0; i<(sizeof(weather_icons)/sizeof(lvgl_weather_icon_t)); i++) {
        if (strcmp(icon, weather_icons[i].name) == 0) {
            return weather_icons[i].img;
        }
    }

    return NULL;
}

static void btn_new_game_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED && disp_ctx.obj_2048) {
        lv_100ask_2048_set_new_game(disp_ctx.obj_2048);
    }
}

static void btn_close_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED && disp_ctx.obj_2048) {
        lv_obj_del(disp_ctx.game_over);
        disp_ctx.game_over = NULL;
    }
}

static void put_score(game_score_t * s)
{
    list_item_t * item = NULL;
    game_score_t * f;

    assert(s != NULL);

    /* Put score into list (sorted) */
    f = list_get_first(&disp_ctx.score_list, &item);
    while (item != NULL) {
        if (f && s->score > f->score) {
            list_add_before(&disp_ctx.score_list, item, s, sizeof(game_score_t), NULL);
            return;
        }
        f = list_get_next(&disp_ctx.score_list, &item);
    }

    list_add(&disp_ctx.score_list, s, sizeof(game_score_t));
}

static void parse_score(char * line)
{
    char * p;
    int cnt = 0;
    int line_len = strlen(line);
    game_score_t score;

    assert(line != NULL);

    p = &line[0];
    for (int i=0; (i <= line_len); i++) {
        if (line[i] == '\t' || line[i] == '\0') {
            line[i] = '\0';
            switch (cnt)
            {
            case 0:
                score.score = strtol(p, NULL, 10);
                break;
            case 1:
                score.best_tile = strtol(p, NULL, 10);
                break;
            case 2:
                score.name = p;
                break;
            }
            p = &line[i+1];
            cnt++;
        }
    }
    put_score(&score);
}

static void load_game_scores(lv_obj_t * screen)
{
    char text[100];
    lv_obj_clean(screen);
    list_clear(&disp_ctx.score_list);
    lv_list_add_text(screen, "  #    SCORE  TILE  NICKNAME");

    char * p = NULL;
    char * buff;
    uint32_t size = 0;
    uint32_t filesize = app_sd_get_file_size(APP_SD_GAME_SCORE_FILE);
    if (filesize > 0) {
        buff = malloc(filesize+1);
        if (buff != NULL) {
            buff[0] = '\0';
            app_sd_readfile(APP_SD_GAME_SCORE_FILE, buff, &size, filesize+1);
            buff[size] = '\0';

            p = &buff[0];
            for (int i=0; (i < size && p != NULL); i++) {
                if (buff[i] == '\n') {
                    buff[i] = '\0';
                    parse_score(p);
                    p = &buff[i+1];
                }
            }

            list_item_t * item = NULL;
            game_score_t * f = NULL;
            int cnt = 1;
            f = list_get_first(&disp_ctx.score_list, &item);
            while (item != NULL && f != NULL) {
                if (f) {
                    snprintf(text, 100, "%2d. %6d  %4d   %s", cnt, f->score, f->best_tile, f->name);
                    lv_list_add_btn(screen, NULL, text);
                }
                f = list_get_next(&disp_ctx.score_list, &item);
                cnt++;
                if (cnt > 10) {
                    break;
                }
            }

            /* Free after all printed, because string are not allocated */
            free(buff);
            list_clear(&disp_ctx.score_list);
        }
    }

}

static void btn_clear_score_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED) {
        app_sd_deletefile(APP_SD_GAME_SCORE_FILE);
        load_game_scores(disp_ctx.lv_score_list);
    }
}

static void btn_savename_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_user_data(e);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED && disp_ctx.obj_2048) {
        uint16_t score = lv_100ask_2048_get_score(disp_ctx.obj_2048);
        uint16_t best_tile = lv_100ask_2048_get_best_tile(disp_ctx.obj_2048);
        char data[100];
        const char * ta_name = lv_textarea_get_text(ta);
        snprintf(data, sizeof(data), "%d\t%d\t%s\n", score, best_tile, ta_name);
        app_sd_appendfile(APP_SD_GAME_SCORE_FILE, data, strlen(data));
        load_game_scores(disp_ctx.lv_score_list);

        lv_obj_del(disp_ctx.game_over);
        disp_ctx.game_over = NULL;
        lv_100ask_2048_set_new_game(disp_ctx.obj_2048);
    }
}

static void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(kb, ta);
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }

    if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
}

static void lvgl_show_game_over(lv_obj_t * screen, uint32_t width, uint32_t height)
{
    uint16_t score = lv_100ask_2048_get_score(disp_ctx.obj_2048);
    uint16_t best_tile = lv_100ask_2048_get_best_tile(disp_ctx.obj_2048);

    if (disp_ctx.game_over) {
        lv_obj_del(disp_ctx.game_over);
        disp_ctx.game_over = NULL;
    }

    /* Game Over object */
    disp_ctx.game_over = lv_obj_create(screen);
    lv_obj_set_size(disp_ctx.game_over, (width-340), (height/2));
	lv_obj_align(disp_ctx.game_over, LV_ALIGN_CENTER, 0, -80);
	lv_obj_set_style_bg_color(disp_ctx.game_over, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(disp_ctx.game_over, 240, 0);
    lv_obj_set_style_border_color(disp_ctx.game_over, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_border_opa(disp_ctx.game_over, 150, 0);

    lv_obj_t * label = lv_label_create(disp_ctx.game_over);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(label, lv_color_make(0xFF, 0x00, 0xFF), 0);
    if (best_tile >= 2048) {
        lv_label_set_text_static(label, "YOU WIN");
    } else {
        lv_label_set_text_static(label, "GAME OVER");
    }

    lv_obj_t * label_score = lv_label_create(disp_ctx.game_over);
	lv_obj_align(label_score, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_text_font(label_score, &lv_font_montserrat_22, 0);
    lv_obj_set_style_text_color(label_score, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_label_set_text_fmt(label_score, "Your score: %d      Best tile: %d", score, best_tile);
	lv_obj_align_to(label_score, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t * txt_name = lv_textarea_create(disp_ctx.game_over);
    lv_textarea_set_one_line(txt_name, true);
    lv_textarea_set_placeholder_text(txt_name, "Put your nickname");
    /* Add event to weather text area with keyboard param */
    lv_obj_add_event_cb(txt_name, ta_event_cb, LV_EVENT_ALL, disp_ctx.keyboard);
    /* Set textarea for keyboard */
    lv_keyboard_set_textarea(disp_ctx.keyboard, txt_name);
	lv_obj_align(txt_name, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t * btn_save_name = lv_btn_create(disp_ctx.game_over);
	lv_obj_align_to(btn_save_name, txt_name, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    label = lv_label_create(btn_save_name);
    lv_label_set_text_static(label, "Save Nick");
    lv_obj_add_event_cb(btn_save_name, btn_savename_cb, LV_EVENT_ALL, txt_name);

    lv_obj_t * btn_close = lv_btn_create(disp_ctx.game_over);
    lv_obj_set_size(btn_close, 40, 40);
    lv_obj_set_style_radius(btn_close, 20, 0);
	lv_obj_align(btn_close, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(btn_close, 100, 0);
    label = lv_label_create(btn_close);
    lv_label_set_text_static(label, LV_SYMBOL_CLOSE"");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_close, btn_close_cb, LV_EVENT_ALL, NULL);
}

static void lvgl_show_game(lv_obj_t * screen, uint32_t width, uint32_t height)
{
    lv_obj_set_style_pad_top(screen, 5, 0);
    lv_obj_set_style_pad_bottom(screen, 5, 0);

    lv_obj_t * block1 = lv_obj_create(screen);
    lv_obj_set_size(block1, (width/2)-70, height-40);
	lv_obj_set_style_bg_color(block1, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(block1, 20, 0);
    lv_obj_set_style_border_color(block1, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(block1, 150, 0);
	lv_obj_align(block1, LV_ALIGN_TOP_LEFT, 30, 10);


    lv_obj_t * block2 = lv_obj_create(screen);
    lv_obj_set_size(block2, (width/2)-70, height-40);
	lv_obj_set_style_bg_color(block2, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(block2, 20, 0);
    lv_obj_set_style_border_color(block2, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(block2, 150, 0);
	lv_obj_align(block2, LV_ALIGN_TOP_RIGHT, -30, 10);

    disp_ctx.obj_2048 = lv_100ask_2048_create(block1);
	lv_obj_align(disp_ctx.obj_2048, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label = lv_label_create(block2);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_label_set_text_static(label, "The 2048 Game");

    label = lv_label_create(block2);
    lv_obj_set_width(label, height-80);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);
	lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text_static(label, "Join blocks with the same numbers by moving UP, DOWN, LEFT and RIGHT over the playing view. By joining the same numbers, the new block will have sum of these joined blocks. When the result block will have number 2048, you are the winnner!\n\nThe game was made by 100askTeam.");

    /* Button new game */
    lv_obj_t * btn_new_game = lv_btn_create(block2);
	lv_obj_align(btn_new_game, LV_ALIGN_BOTTOM_MID, 0, 0);
    label = lv_label_create(btn_new_game);
    lv_label_set_text_static(label, "New Game");
    lv_obj_add_event_cb(btn_new_game, btn_new_game_cb, LV_EVENT_ALL, NULL);
}

static void lvgl_show_sd(lv_obj_t * screen, const lv_font_t * font, uint32_t size)
{
    lv_obj_t * block = lv_obj_create(screen);
    lv_obj_set_size(block, size+20, size);
	lv_obj_set_style_bg_color(block, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_border_color(block, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(block, 150, 0);
	lv_obj_align(block, LV_ALIGN_TOP_RIGHT, -30, 10);
    lv_obj_set_style_pad_top(block, 10, 0);
    lv_obj_set_style_pad_bottom(block, 0, 0);
    lv_obj_set_style_pad_left(block, 5, 0);
    lv_obj_set_style_pad_right(block, 5, 0);

    lv_obj_t * label = lv_label_create(block);
    lv_label_set_text_static(label, "TOP 10 PLAYERS");
	lv_obj_set_style_text_font(label, &lv_font_unscii_16, 0);
	lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    /* List */
    disp_ctx.lv_score_list = lv_list_create(block);
    lv_obj_set_style_pad_left(disp_ctx.lv_score_list, 0, 0);
    lv_obj_set_style_pad_right(disp_ctx.lv_score_list, 0, 0);
    lv_obj_set_style_pad_row(disp_ctx.lv_score_list, 5, 0);
    lv_obj_set_size(disp_ctx.lv_score_list, size, size-40);
    lv_obj_set_style_bg_opa(disp_ctx.lv_score_list, 0, LV_PART_MAIN );
    lv_obj_set_style_border_opa(disp_ctx.lv_score_list, 0, LV_PART_MAIN );
	lv_obj_align(disp_ctx.lv_score_list, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(disp_ctx.lv_score_list, &lv_font_unscii_8, 0);

    load_game_scores(disp_ctx.lv_score_list);
}

static void lvgl_show_weather(lv_obj_t * screen, const lv_font_t * font, uint32_t size)
{
    lv_obj_t * block = lv_obj_create(screen);
    lv_obj_set_size(block, size+20, size);
	lv_obj_set_style_bg_color(block, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_border_color(block, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(block, 150, 0);
	lv_obj_align(block, LV_ALIGN_TOP_LEFT, 30, 10);

    disp_ctx.img_weather = lv_img_create(block);
    lv_img_set_src(disp_ctx.img_weather, &weather_01d);
	lv_obj_align(disp_ctx.img_weather, LV_ALIGN_TOP_MID, 0, 10);
    lv_img_set_zoom(disp_ctx.img_weather, (size > 150 ? 512 : 300));

	disp_ctx.lbl_weather_temp = lv_label_create(block);
	lv_obj_set_style_text_font(disp_ctx.lbl_weather_temp, &lv_font_montserrat_30, 0);
	lv_label_set_text(disp_ctx.lbl_weather_temp, "-- °C");
    lv_obj_set_style_text_align(disp_ctx.lbl_weather_temp, LV_ALIGN_CENTER, 0);
	lv_obj_align(disp_ctx.lbl_weather_temp, LV_ALIGN_CENTER, 0, 10);

	disp_ctx.lbl_weather_place = lv_label_create(block);
	lv_obj_set_style_text_font(disp_ctx.lbl_weather_place, font, 0);
	lv_label_set_text(disp_ctx.lbl_weather_place, "----");
	lv_obj_align(disp_ctx.lbl_weather_place, LV_ALIGN_BOTTOM_MID, 0, -50);

	disp_ctx.lbl_weather_desc = lv_label_create(block);
	lv_obj_set_style_text_font(disp_ctx.lbl_weather_desc, font, 0);
	lv_label_set_text(disp_ctx.lbl_weather_desc, "----");
    lv_obj_set_style_text_align(disp_ctx.lbl_weather_desc, LV_ALIGN_CENTER, 0);
	lv_obj_align(disp_ctx.lbl_weather_desc, LV_ALIGN_BOTTOM_MID, 0, -20);

    /* Weather API key is not set! */
    if (strlen(CONFIG_WEATHER_API_KEY) == 0) {
	    lv_label_set_text(disp_ctx.lbl_weather_place, "Weather API key is not set!");
    }
}

static void btn_weather_place_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_user_data(e);

    assert(ta != NULL);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED) {
        const char * ta_place = lv_textarea_get_text(ta);
        if(ta_place != NULL)
            app_net_weather_set_place(ta_place, true);
    }
}

static void dd_time_zone_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    /* Drop-Down changed */
    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t n = lv_dropdown_get_selected(obj);
        /* Set selected time zone */
        app_net_time_set_zone(n);
    }
}

static void spin_weather_interval_plus_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * spinbox = lv_event_get_user_data(e);

    assert(spinbox != NULL);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_increment(spinbox);
    }
}

static void spin_weather_interval_minus_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * spinbox = lv_event_get_user_data(e);

    assert(spinbox != NULL);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_LONG_PRESSED_REPEAT) {
        lv_spinbox_decrement(spinbox);
    }
}

static void btn_weather_interval_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * spinbox = lv_event_get_user_data(e);

    assert(spinbox != NULL);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED) {
        uint32_t interval = lv_spinbox_get_value(spinbox);
        app_net_weather_set_update_interval((uint16_t)interval, true);
    }
}

static void lvgl_show_settings(lv_obj_t * screen, const lv_font_t * font, uint32_t width)
{
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* WEATHER PLACE */
    lv_obj_t * cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, width, 80);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Label - weather place */
    lv_obj_t * lbl_weather_place = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_weather_place, font, 0);
	lv_label_set_text_static(lbl_weather_place, "Weather place: ");
	lv_obj_align(lbl_weather_place, LV_ALIGN_LEFT_MID, 0, 0);

    /* Textarea - weather place */
    lv_obj_t * ta_weather_place = lv_textarea_create(cont_row);
    lv_textarea_set_one_line(ta_weather_place, true);
    lv_textarea_set_max_length(ta_weather_place, APP_NET_WEATHER_PLACE_MAX);
    lv_textarea_set_text(ta_weather_place, app_net_weather_get_place());
	lv_obj_align(ta_weather_place, LV_ALIGN_LEFT_MID, 0, 0);
    /* Add event to weather text area with keyboard param */
    lv_obj_add_event_cb(ta_weather_place, ta_event_cb, LV_EVENT_ALL, disp_ctx.keyboard);
    /* Set textarea for keyboard */
    lv_keyboard_set_textarea(disp_ctx.keyboard, ta_weather_place);

    /* Save button - weather place */
    lv_obj_t * btn_weather_place = lv_btn_create(cont_row);
    lv_obj_t * label = lv_label_create(btn_weather_place);
    lv_label_set_text_static(label, "Save");
	lv_obj_align(btn_weather_place, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btn_weather_place, btn_weather_place_event_cb, LV_EVENT_ALL, ta_weather_place);


    /* WEATHER UPDATE INTERVAL */
    cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, width, 80);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Label - weather update interval */
    lv_obj_t * lbl_weather_interval = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_weather_interval, font, 0);
	lv_label_set_text_static(lbl_weather_interval, "Weather update interval (min): ");
	lv_obj_align(lbl_weather_interval, LV_ALIGN_LEFT_MID, 0, 0);

    /* Minus button */
    lv_obj_t * btn_minus = lv_btn_create(cont_row);
    lv_obj_set_style_bg_img_src(btn_minus, LV_SYMBOL_MINUS, 0);

    /* Spinbox - weather update interval */
    lv_obj_t * spin_weather_interval = lv_spinbox_create(cont_row);
    lv_spinbox_set_value(spin_weather_interval, app_net_weather_get_update_interval());
    lv_spinbox_set_rollover(spin_weather_interval, true);
    lv_spinbox_set_range(spin_weather_interval, 5, 60);
    lv_spinbox_set_digit_format(spin_weather_interval, 2, 0);
    lv_spinbox_step_prev(spin_weather_interval);
    lv_obj_set_width(spin_weather_interval, 100);
    lv_spinbox_set_step(spin_weather_interval, 5);

    uint32_t h = lv_obj_get_height(spin_weather_interval);

    /* Plus button */
    lv_obj_t * btn_plus = lv_btn_create(cont_row);
    lv_obj_set_style_bg_img_src(btn_plus, LV_SYMBOL_PLUS, 0);

    lv_obj_set_size(btn_minus, h, h);
    lv_obj_set_size(btn_plus, h, h);
    lv_obj_add_event_cb(btn_minus, spin_weather_interval_minus_event_cb, LV_EVENT_ALL, spin_weather_interval);
    lv_obj_add_event_cb(btn_plus, spin_weather_interval_plus_event_cb, LV_EVENT_ALL,  spin_weather_interval);

    /* Save button - weather place */
    lv_obj_t * btn_weather_interval = lv_btn_create(cont_row);
    label = lv_label_create(btn_weather_interval);
    lv_label_set_text_static(label, "Save");
	lv_obj_align(btn_weather_interval, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btn_weather_interval, btn_weather_interval_event_cb, LV_EVENT_ALL, spin_weather_interval);


    /* TIME ZONE */
    cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, width, 80);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Label - time zone */
    lv_obj_t * lbl_time_zone = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_time_zone, font, 0);
	lv_label_set_text_static(lbl_time_zone, "Time zone: ");
	lv_obj_align(lbl_time_zone, LV_ALIGN_LEFT_MID, 0, 0);

    /* Select - time zone */
    lv_obj_t * dd_time_zone = lv_dropdown_create(cont_row);
    lv_obj_set_size(dd_time_zone, lv_obj_get_width(ta_weather_place), lv_obj_get_height(ta_weather_place));
    lv_dropdown_set_options(dd_time_zone, app_net_time_get_zone_names());
    lv_obj_add_event_cb(dd_time_zone, dd_time_zone_event_cb, LV_EVENT_ALL, NULL);


    /* RESET */
    cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, width, 80);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Reset button */
    lv_obj_t * btn_clear_score = lv_btn_create(cont_row);
    label = lv_label_create(btn_clear_score);
    lv_label_set_text_static(label, "Clean score table");
	lv_obj_align(btn_clear_score, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_clear_score, btn_clear_score_event_cb, LV_EVENT_ALL, NULL);

    /* Label - brightness */
    /*lv_obj_t * lbl_brightness = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_brightness, font, 0);
	lv_label_set_text_static(lbl_brightness, "Brightness: ");
	lv_obj_align(lbl_brightness, LV_ALIGN_LEFT_MID, 0, 0);*/

    /* Slider - brightness */
    /*lv_obj_t * slider_brightness = lv_slider_create(cont_row);
    lv_slider_set_range(slider_brightness, 0, 100);
    lv_obj_center(slider_brightness);
    lv_obj_add_event_cb(slider_brightness, slider_brightness_event_cb, LV_EVENT_VALUE_CHANGED, NULL);*/
}

static void lvgl_show_info(lv_obj_t * screen, const lv_font_t * font, uint32_t width)
{
    lv_obj_t * img_logo = lv_img_create(screen);
    lv_img_set_src(img_logo, &esp_logo);
    lv_obj_align(img_logo, LV_ALIGN_TOP_LEFT, 10, 10);
    if (width < 400) {
        lv_img_set_zoom(img_logo, 128); /* Half size */
    }

    lv_obj_t * img_text = lv_img_create(screen);
    lv_img_set_src(img_text, &esp_text);
    lv_obj_set_style_img_recolor(img_text, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_align_to(img_text, img_logo, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    if (width < 400) {
        lv_img_set_zoom(img_text, 128); /* Half size */
    }

    lv_obj_t * lbl_info = lv_label_create(screen);
	lv_obj_set_style_text_font(lbl_info, font, 0);
    lv_obj_set_width(lbl_info, width);
    lv_label_set_long_mode(lbl_info, LV_LABEL_LONG_WRAP);
    lv_label_set_text_static(lbl_info, "This example shows handling multiple LCD displays with touch from one MCU. It shows time synchronized with SNTP server and weather synchronized with https://openweathermap.org/. It allows search WiFi networks and connect to selected. It can change the weather place, weather update interval and time zone. All settings are saved into NVS and loaded after each restart.\n\nThere is used LVGL as a graphic library.");

    lv_obj_set_style_text_align(lbl_info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(lbl_info, LV_ALIGN_BOTTOM_MID, 0, 0);

}

static void btn_refresh_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    assert(obj != NULL);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED) {
        /* Disable refresh button */
        lv_obj_add_state(obj, LV_STATE_DISABLED);
        app_net_wifi_scan();
    }
}

static void btn_connect_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    /* Button clicked */
    if (code == LV_EVENT_CLICKED) {
        char ssid[32] = "";
        lv_dropdown_get_selected_str(disp_ctx.dd_ssid, ssid, sizeof(ssid));
        const char * pass = lv_textarea_get_text(disp_ctx.ta_wifi_pass);
        if(strlen(ssid) > 0)
            app_net_wifi_connect(ssid, pass, true);
    }
}

static void lvgl_show_wifi(lv_obj_t * screen, const lv_font_t * font, uint32_t width)
{
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* WiFi SSID and PASSWORD */
    lv_obj_t * cont_row = lv_obj_create(screen);
    lv_obj_set_size(cont_row, width, 150);
    lv_obj_align(cont_row, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_top(cont_row, 2, 0);
    lv_obj_set_style_pad_bottom(cont_row, 2, 0);
    lv_obj_set_flex_align(cont_row, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Label - ssid */
    lv_obj_t * lbl_ssid = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_ssid, font, 0);
	lv_label_set_text_static(lbl_ssid, "SSID: ");
	lv_obj_align(lbl_ssid, LV_ALIGN_LEFT_MID, 0, 0);

    /* Select - ssid */
    char * connected_ssid = app_net_wifi_get_connected();
    lv_obj_t * dd_ssid = lv_dropdown_create(cont_row);
    lv_dropdown_set_options(dd_ssid, (connected_ssid ? connected_ssid : ""));
    lv_obj_set_width(dd_ssid, width/3);
    disp_ctx.dd_ssid = dd_ssid;

    /* Refresh button */
    lv_obj_t * btn_refresh = lv_btn_create(cont_row);
    lv_obj_set_style_bg_img_src(btn_refresh, LV_SYMBOL_REFRESH, 0);
    lv_obj_set_size(btn_refresh, width/10, width/10);
    lv_obj_add_event_cb(btn_refresh, btn_refresh_event_cb, LV_EVENT_ALL, NULL);
    disp_ctx.btn_refresh = btn_refresh;

    /* Label - password */
    lv_obj_t * lbl_pass = lv_label_create(cont_row);
	lv_obj_set_style_text_font(lbl_pass, font, 0);
	lv_label_set_text_static(lbl_pass, "Password: ");

    /* Textarea - password */
    lv_obj_t * ta_pass = lv_textarea_create(cont_row);
    lv_textarea_set_one_line(ta_pass, true);
    lv_textarea_set_max_length(ta_pass, APP_NET_WEATHER_PLACE_MAX);
    lv_textarea_set_text(ta_pass, "");
    lv_obj_set_width(ta_pass, width/3);
    disp_ctx.ta_wifi_pass = ta_pass;

    /* Set textarea for keyboard */
    lv_keyboard_set_textarea(disp_ctx.keyboard, ta_pass);
    /* Add event to weather text area with keyboard param */
    lv_obj_add_event_cb(ta_pass, ta_event_cb, LV_EVENT_ALL, disp_ctx.keyboard);

    /* Connect button */
    lv_obj_t * btn_connect = lv_btn_create(cont_row);
    lv_obj_t * label = lv_label_create(btn_connect);
    lv_label_set_text_static(label, "Connect to WiFi");
	lv_obj_align(btn_connect, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btn_connect, btn_connect_event_cb, LV_EVENT_ALL, NULL);


    /* WIFI INFO */
    lv_obj_t * cont_wifi_info = lv_obj_create(screen);
    lv_obj_set_size(cont_wifi_info, width, 130);
    lv_obj_align(cont_wifi_info, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_flex_flow(cont_wifi_info, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_style_pad_top(cont_wifi_info, 2, 0);
    lv_obj_set_style_pad_bottom(cont_wifi_info, 2, 0);
    lv_obj_set_flex_align(cont_wifi_info, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_add_flag(cont_wifi_info, LV_OBJ_FLAG_HIDDEN);
    disp_ctx.cont_wifi_info = cont_wifi_info;

    /* Connected SSID */
    lv_obj_t * lbl_info_ssid = lv_label_create(cont_wifi_info);
    lv_obj_set_width(lbl_info_ssid, width-10);
	lv_obj_set_style_text_font(lbl_info_ssid, font, 0);
	lv_label_set_text_static(lbl_info_ssid, "SSID: ---");
    disp_ctx.lbl_info_ssid = lbl_info_ssid;

    /* IP address */
    lv_obj_t * lbl_info_ip = lv_label_create(cont_wifi_info);
    lv_obj_set_width(lbl_info_ip, width-10);
	lv_obj_set_style_text_font(lbl_info_ip, font, 0);
	lv_label_set_text_static(lbl_info_ip, "IP: ---");
    disp_ctx.lbl_info_ip = lbl_info_ip;

    /* MAC address */
    lv_obj_t * lbl_info_mac = lv_label_create(cont_wifi_info);
    lv_obj_set_width(lbl_info_mac, width-10);
	lv_obj_set_style_text_font(lbl_info_mac, font, 0);
	lv_label_set_text_static(lbl_info_mac, "MAC: ---");
    disp_ctx.lbl_info_mac = lbl_info_mac;


}

static void keyboard_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    /* OK clicked on keyboard */
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_t * ta = lv_keyboard_get_textarea(obj);
        if (ta)
           lv_event_send(ta, LV_EVENT_CANCEL, NULL);

        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

/*******************************************************************************
* Public API functions
*******************************************************************************/

void disp_lvgl_init(void)
{
    memset(&disp_ctx, 0, sizeof(disp_ctx));

    /* Prepare list */
    list_create(&disp_ctx.score_list);

    ESP_LOGI(TAG, "LVGL initialized");
}

void disp_lvgl_draw_rounded_display(lv_disp_t * disp)
{
    assert(disp != NULL);

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    /* Display screen */
    lv_obj_t * scr = lv_disp_get_scr_act(disp);

    /* Black background color */
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(scr, 255, 0);

    /* Create image */
    lv_obj_t * img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &esp_logo);
    lv_obj_center(img_logo);
    lv_img_set_zoom(img_logo, 200);

    /* Clock */
    lvgl_show_clock(scr, 240);

#if ROUNDED_DISP_TIME_DIGIT
	// Create time label
	disp_ctx.lbl_time = lv_label_create(scr);
	lv_obj_set_style_text_font(disp_ctx.lbl_time, &digital_50_bold, 0);
	lv_label_set_text(disp_ctx.lbl_time, "00:00:00");
	lv_obj_align(disp_ctx.lbl_time, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_opa(disp_ctx.lbl_time, 0, 0);

    /* Green text color */
    lv_obj_set_style_text_color(disp_ctx.lbl_time, lv_color_make(0x00, 0xFF, 0x00), 0);
#endif

    /* Date */
    disp_ctx.lbl_small_date = lv_label_create(scr);
	lv_label_set_text(disp_ctx.lbl_small_date, "--");
    lv_obj_set_style_text_opa(disp_ctx.lbl_small_date, 0, 0);
    lv_obj_align(disp_ctx.lbl_small_date, LV_ALIGN_CENTER, 0, -40);

    /* Weather */
    disp_ctx.img_small_weather = lv_img_create(scr);
    lv_img_set_src(disp_ctx.img_small_weather, &weather_01d);
    lv_img_set_zoom(disp_ctx.img_small_weather, 100);
    lv_obj_set_style_img_opa(disp_ctx.img_small_weather, 0, 0);
    lv_obj_set_style_border_color(disp_ctx.img_small_weather, lv_color_white(), 0);
    lv_obj_align(disp_ctx.img_small_weather, LV_ALIGN_CENTER, -25, 40);
    disp_ctx.lbl_small_weather = lv_label_create(scr);
	lv_label_set_text(disp_ctx.lbl_small_weather, "-- °C");
    lv_obj_set_style_text_opa(disp_ctx.lbl_small_weather, 0, 0);
    lv_obj_set_style_border_color(disp_ctx.lbl_small_weather, lv_color_white(), 0);
    lv_obj_align(disp_ctx.lbl_small_weather, LV_ALIGN_CENTER, 15, 40);

    lv_anim_t a;
    lv_anim_init(&a);

    lv_anim_set_var(&a, img_logo);
    lv_anim_set_values(&a, 255, 40);
    lv_anim_set_delay(&a, 500);
    lv_anim_set_exec_cb(&a, anim_img_opa_cb);
    lv_anim_start(&a);

    lv_anim_set_var(&a, disp_ctx.img_small_weather);
    lv_anim_set_values(&a, 0, 150);
    lv_anim_set_delay(&a, 1500);
    lv_anim_set_exec_cb(&a, anim_img_opa_cb);
    lv_anim_start(&a);

    lv_anim_set_var(&a, disp_ctx.lbl_small_weather);
    lv_anim_set_values(&a, 0, 150);
    lv_anim_set_delay(&a, 1500);
    lv_anim_set_exec_cb(&a, anim_text_opa_cb);
    lv_anim_start(&a);

    lv_anim_set_var(&a, disp_ctx.lbl_small_date);
    lv_anim_set_values(&a, 0, 150);
    lv_anim_set_delay(&a, 1500);
    lv_anim_set_exec_cb(&a, anim_text_opa_cb);
    lv_anim_start(&a);

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

static void scroll_begin_event(lv_event_t * e)
{
    /*Disable the scroll animations. Triggered when a tab button is clicked */
    if(lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN) {
        lv_anim_t * a = lv_event_get_param(e);
        if(a)  a->time = 10;
    }
}

void disp_lvgl_draw_big_display(lv_disp_t * disp)
{
    assert(disp != NULL);

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    /* Create image Logo */
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    disp_ctx.main_screen = scr;

    //lv_obj_t * bg_img = lv_img_create(scr);
    //lv_img_set_src(bg_img, &img_background);
    //lv_img_set_zoom(bg_img, 1024);

    /* Background color */
    lv_obj_set_style_bg_color(scr, lv_color_make(0x19, 0x2B, 0xFE), 0);
    lv_obj_set_style_bg_opa(scr, 255, 0);

    uint32_t bar_h = disp->driver->ver_res/8;

    /* Header bar */
    lv_obj_t * scr_header = lv_obj_create(scr);
    lv_obj_set_size(scr_header, disp->driver->hor_res, bar_h);
    lv_obj_align(scr_header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_border_width(scr_header, 0, 0);
    lv_obj_set_style_bg_color(scr_header, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_grad_color(scr_header, lv_color_make(0x05, 0x05, 0x05), 0);
    lv_obj_set_style_bg_grad_dir(scr_header, LV_GRAD_DIR_VER,0);
    lv_obj_set_style_bg_opa(scr_header, 255, 0);
    lv_obj_set_style_radius(scr_header, 0, 0);
    lv_obj_clear_flag(scr_header, LV_OBJ_FLAG_SCROLLABLE);

	// Create time label
	disp_ctx.lbl_main_time = lv_label_create(scr_header);
	lv_obj_set_style_text_font(disp_ctx.lbl_main_time, (bar_h < 60 ? &digital_30 : &digital_50_bold), 0);
	lv_label_set_text(disp_ctx.lbl_main_time, "00:00:00");
	lv_obj_align(disp_ctx.lbl_main_time, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_content_width(disp_ctx.lbl_main_time, (bar_h < 50 ? 100 : 180));
    lv_obj_set_style_text_align(disp_ctx.lbl_main_time, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_opa(disp_ctx.lbl_main_time, 150, 0);
    lv_obj_set_style_text_color(disp_ctx.lbl_main_time, lv_color_make(0x00, 0xFF, 0x00), 0);

    disp_ctx.lbl_wifi = lv_label_create(scr_header);
	lv_obj_set_style_text_font(disp_ctx.lbl_wifi, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), 0);
	lv_label_set_text(disp_ctx.lbl_wifi, LV_SYMBOL_WIFI" -----");
    if (bar_h < 50) {
	    lv_obj_align(disp_ctx.lbl_wifi, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_text_align(disp_ctx.lbl_wifi, LV_TEXT_ALIGN_LEFT, 0);
    } else {
	    lv_obj_align(disp_ctx.lbl_wifi, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_align(disp_ctx.lbl_wifi, LV_TEXT_ALIGN_CENTER, 0);
    }

    disp_ctx.img_bat = lv_label_create(scr_header);
	lv_obj_set_style_text_font(disp_ctx.img_bat, &lv_font_montserrat_40, 0);
    lv_label_set_text_static(disp_ctx.img_bat, LV_SYMBOL_BATTERY_EMPTY"");
    lv_obj_set_style_text_color(disp_ctx.img_bat, lv_color_hex(0xFF0000), 0);
    lv_obj_align(disp_ctx.img_bat, LV_ALIGN_LEFT_MID, 80, 0);
    disp_ctx.lbl_bat = lv_label_create(disp_ctx.img_bat);
    lv_obj_align(disp_ctx.lbl_bat, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_text_font(disp_ctx.lbl_bat, &lv_font_montserrat_14, 0);
	lv_label_set_text(disp_ctx.lbl_bat, "X");


    /* Tabview */
    disp_ctx.tabview = lv_tabview_create(scr, LV_DIR_BOTTOM, bar_h);
    lv_obj_set_size(disp_ctx.tabview, disp->driver->hor_res, disp->driver->ver_res - bar_h);
    lv_obj_align(disp_ctx.tabview, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(disp_ctx.tabview, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), 0);
    lv_obj_add_event_cb(lv_tabview_get_content(disp_ctx.tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_set_style_bg_opa(disp_ctx.tabview, 180, 0);
	lv_obj_clear_flag(lv_tabview_get_content(disp_ctx.tabview), LV_OBJ_FLAG_SCROLLABLE);
	//lv_obj_add_event_cb(lv_tabview_get_content(disp_ctx.tabview), scroll_begin_event, LV_EVENT_SCROLL_BEGIN, NULL);

    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(disp_ctx.tabview);
    lv_obj_set_style_bg_color(tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tab_btns, lv_palette_lighten(LV_PALETTE_GREEN, 5), 0);
    lv_obj_set_style_border_side(tab_btns, LV_BORDER_SIDE_BOTTOM, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_set_style_bg_opa(tab_btns, 180, 0);

    /* Create a keyboard to use it with an of the text areas */
    disp_ctx.keyboard = lv_keyboard_create(disp_ctx.main_screen);
    lv_obj_set_size(disp_ctx.keyboard,  disp->driver->hor_res, disp->driver->ver_res / 2);
    lv_keyboard_set_mode(disp_ctx.keyboard, LV_KEYBOARD_MODE_TEXT_UPPER);
    lv_obj_add_event_cb(disp_ctx.keyboard, keyboard_event_cb, LV_EVENT_ALL, NULL);

    /* Hide keyboard */
    lv_obj_add_flag(disp_ctx.keyboard, LV_OBJ_FLAG_HIDDEN);

    /* Add 3 tabs (the tabs are page (lv_page) and can be scrolled */
    lv_obj_t * tab_home = lv_tabview_add_tab(disp_ctx.tabview, LV_SYMBOL_HOME" Home");
    disp_ctx.tab_game = lv_tabview_add_tab(disp_ctx.tabview, LV_SYMBOL_PLAY" Game");
    lv_obj_t * tab_wifi = lv_tabview_add_tab(disp_ctx.tabview, LV_SYMBOL_WIFI" Wi-Fi");
    lv_obj_t * tab_settings = lv_tabview_add_tab(disp_ctx.tabview, LV_SYMBOL_SETTINGS" Settings");
    lv_obj_t * tab_info = lv_tabview_add_tab(disp_ctx.tabview, LV_SYMBOL_IMAGE" Info");

    /* Clock */
    //lvgl_show_clock(tab_home, disp->driver->ver_res - 3*bar_h);

    /* Weather */
    lvgl_show_weather(tab_home, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), disp->driver->ver_res - 3*bar_h);

    /* SD */
    lvgl_show_sd(tab_home, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), disp->driver->ver_res - 3*bar_h);

    /* WiFi */
    lvgl_show_wifi(tab_wifi, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), disp->driver->hor_res-bar_h);

    /* Game */
    lvgl_show_game(disp_ctx.tab_game, disp->driver->hor_res, disp->driver->ver_res - 2*bar_h);

    /* Settings */
    lvgl_show_settings(tab_settings, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), disp->driver->hor_res-bar_h);

    /* Info */
    lvgl_show_info(tab_info, (bar_h < 50 ? &lv_font_montserrat_14 : &lv_font_montserrat_20), disp->driver->hor_res-bar_h);

    /* Create image */
    lv_obj_t * img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &esp_logo);
    lv_obj_center(img_logo);

    /* Create image Text */
    lv_obj_t * img_text = lv_img_create(scr);
    lv_img_set_src(img_text, &esp_text);
    lv_obj_align(img_text, LV_ALIGN_CENTER, 0, 100);
    lv_img_set_zoom(img_text, 200);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img_logo);
    lv_anim_set_values(&a, lv_img_get_zoom(img_logo), 300);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_exec_cb(&a, anim_zoom_cb);
    lv_anim_set_ready_cb(&a, anim_logo_zoom_done_cb);
    lv_anim_start(&a);

    lv_anim_set_var(&a, img_text);
    lv_anim_set_values(&a, 255, 0);
    lv_anim_set_exec_cb(&a, anim_img_opa_cb);
    lv_anim_start(&a);

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();

}

void disp_lvgl_draw_monochrome_display(lv_disp_t * disp)
{
    assert(disp != NULL);

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    /* Display screen */
    lv_obj_t * scr = lv_disp_get_scr_act(disp);
    lv_obj_set_width(scr, disp->driver->ver_res);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* ESP Logo image */
    lv_obj_t * img_logo = lv_img_create(scr);
    lv_img_set_src(img_logo, &esp_logo);
    lv_img_set_zoom(img_logo, 70);
    lv_obj_set_x(img_logo, -32);
    lv_obj_set_y(img_logo, -29);
    /* ESP Text image */
    lv_obj_t * img_text = lv_img_create(scr);
    lv_img_set_src(img_text, &esp_text);
    lv_obj_set_style_bg_img_recolor(img_text, lv_color_white(), 0);
    lv_img_set_zoom(img_text, 100);
    lv_obj_set_align(img_text, LV_ALIGN_CENTER);
    lv_obj_set_x(img_text, 15);
    lv_obj_set_y(img_text, -12);
    lv_obj_set_style_img_recolor(img_text, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_img_recolor_opa(img_text, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* Time */
	/*disp_ctx.lbl_monochrome_time = lv_label_create(scr);
    lv_obj_set_width(disp_ctx.lbl_monochrome_time, 128);
	lv_obj_set_style_text_font(disp_ctx.lbl_monochrome_time, &lv_font_montserrat_22, 0);
	lv_label_set_text(disp_ctx.lbl_monochrome_time, "WELCOME");
	lv_obj_align(disp_ctx.lbl_monochrome_time, LV_ALIGN_TOP_MID, 0, 10);*/

    /* Date */
	disp_ctx.lbl_monochrome_date = lv_label_create(scr);
	lv_obj_set_style_text_font(disp_ctx.lbl_monochrome_date, &lv_font_montserrat_14, 0);
	lv_label_set_text(disp_ctx.lbl_monochrome_date, "--------");
    lv_obj_set_width(disp_ctx.lbl_monochrome_date, disp->driver->ver_res);
    lv_label_set_long_mode(disp_ctx.lbl_monochrome_date, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_anim_speed(disp_ctx.lbl_monochrome_date, 30, 0);
	lv_obj_align(disp_ctx.lbl_monochrome_date, LV_ALIGN_BOTTOM_MID, 0, -5);

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

void disp_lvgl_update_time_sec(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);

    /* Nothing to update */
    if(now == last_time)
        return;

    last_time = now;

    /* Get local time by time zone */
    localtime_r(&now, &timeinfo);

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    if (disp_ctx.lbl_time != NULL)
	    lv_label_set_text_fmt(disp_ctx.lbl_time, "%d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    if (disp_ctx.lbl_main_time != NULL)
	    lv_label_set_text_fmt(disp_ctx.lbl_main_time, "%d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    //if (disp_ctx.lbl_monochrome_time)
	    //lv_label_set_text_fmt(disp_ctx.lbl_monochrome_time, "%d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    if (disp_ctx.lbl_monochrome_date) {
        if(disp_ctx.obj_2048 && lv_tabview_get_tab_act(disp_ctx.tabview) == 1) {
            bool status = lv_100ask_2048_get_status(disp_ctx.obj_2048);
            uint16_t score = lv_100ask_2048_get_score(disp_ctx.obj_2048);
            uint16_t best_tile = lv_100ask_2048_get_best_tile(disp_ctx.obj_2048);
            if (best_tile >= 2048) {
                status = true;
            }
            if (status) {
                if (last_game_status != status) {
                    lvgl_show_game_over(disp_ctx.tab_game, 800, 360);
                }
                lv_label_set_text_fmt(disp_ctx.lbl_monochrome_date, "GAME OVER! Score: %d, best tile: %d", score, lv_100ask_2048_get_best_tile(disp_ctx.obj_2048));
            } else {
                lv_label_set_text_fmt(disp_ctx.lbl_monochrome_date, " Score: %08d", score);
            }
            last_game_status = status;
        } else {
            lv_label_set_text_fmt(disp_ctx.lbl_monochrome_date, "Today is %s %d.%d.%d   ", WEEKDAYS[timeinfo.tm_wday], timeinfo.tm_mday, (timeinfo.tm_mon+1), (1900+timeinfo.tm_year));
        }
    }

    if(disp_ctx.lbl_small_date)
        lv_label_set_text_fmt(disp_ctx.lbl_small_date, " %s %d.%d.", WEEKDAYS_SHORT[timeinfo.tm_wday], timeinfo.tm_mday, (timeinfo.tm_mon+1));


    if(disp_ctx.clock_meter != NULL)
    {
        /* Update only when changed */
        if(timeinfo.tm_min != last_min) {
            if (timeinfo.tm_hour > 12)
                timeinfo.tm_hour -= 12;
            else if (timeinfo.tm_hour == 0)
                timeinfo.tm_hour = 12;

            /* Move hour indicator between hours when changing minutes */
            int hour_ind = ((5*timeinfo.tm_hour) + (timeinfo.tm_min > 12 ? floor(timeinfo.tm_min/12) : 0));
            if(hour_ind > 60)
                hour_ind -= 60;

            /* Hour value is divided into same scale like minutes - moving hour pointer between two hours */
            lv_meter_set_indicator_end_value(disp_ctx.clock_meter, disp_ctx.indic_hour, hour_ind);
            lv_meter_set_indicator_end_value(disp_ctx.clock_meter, disp_ctx.indic_min, timeinfo.tm_min);
            last_min = timeinfo.tm_min;
        }

        /* Update only when changed */
        if(timeinfo.tm_sec != last_sec) {
            lv_meter_set_indicator_end_value(disp_ctx.clock_meter, disp_ctx.indic_sec, timeinfo.tm_sec);
            last_sec = timeinfo.tm_sec;
        }
    }

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

void disp_lvgl_update_battery(uint32_t volt_mv)
{
    lv_color_t img_color;
    lv_color_t lbl_color = lv_color_hex(0xFFFFFF);
    char * symbol;

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    if (disp_ctx.img_bat && disp_ctx.lbl_bat) {
        uint8_t percent = ((double)100 / (APP_BATTERY_MAX_MV - APP_BATTERY_MIN_MV)) * (volt_mv - APP_BATTERY_MIN_MV);

        if (volt_mv < APP_BATTERY_MIN_MV) {
            percent = 0;
        } else if (volt_mv > APP_BATTERY_MAX_MV || percent > 100) {
            percent = 100;
        }

        if (percent > 90) {
            /* Green color */
            img_color = lv_color_hex(0x329C04);
            symbol = LV_SYMBOL_BATTERY_FULL;
        } else if (percent > 70) {
            /* Green color */
            img_color = lv_color_hex(0x329C04);
            symbol = LV_SYMBOL_BATTERY_3;
        } else if (percent > 50) {
            /* Orange color */
            img_color = lv_color_hex(0xFFA800);
            symbol = LV_SYMBOL_BATTERY_2;
        } else if (percent > 20) {
            /* Orange color */
            img_color = lv_color_hex(0xFFA800);
            symbol = LV_SYMBOL_BATTERY_1;
        } else {
            /* Red color */
            img_color = lv_color_hex(0xFF0000);
            lbl_color = lv_color_hex(0xFF0000);
            symbol = LV_SYMBOL_BATTERY_EMPTY;
        }

        lv_label_set_text_static(disp_ctx.img_bat, symbol);
        lv_obj_set_style_text_color(disp_ctx.img_bat, img_color, 0);
        lv_obj_set_style_text_color(disp_ctx.lbl_bat, lbl_color, 0);
        if (volt_mv < APP_BATTERY_MIN_MV) {
            lv_label_set_text_static(disp_ctx.lbl_bat, "");
            lv_obj_set_style_text_opa(disp_ctx.img_bat, 0, 0);
            lv_obj_set_style_text_opa(disp_ctx.lbl_bat, 0, 0);
        }else{
            lv_label_set_text_fmt(disp_ctx.lbl_bat, "%d %%", percent);
            lv_obj_set_style_text_opa(disp_ctx.img_bat, 255, 0);
            lv_obj_set_style_text_opa(disp_ctx.lbl_bat, 255, 0);
        }
    }



    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

void disp_lvgl_update_wifi_ssid(const char * ssid)
{
	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    if(disp_ctx.lbl_wifi != NULL)
    {
        if (ssid == NULL) {
            lv_label_set_text(disp_ctx.lbl_wifi, LV_SYMBOL_WIFI" -----");
            /* Hide WiFi info */
            lv_obj_add_flag(disp_ctx.cont_wifi_info, LV_OBJ_FLAG_HIDDEN);
            /* Print WiFi SSID */
            lv_label_set_text(disp_ctx.lbl_info_ssid, "SSID: ---");
        } else {
            lv_label_set_text_fmt(disp_ctx.lbl_wifi, LV_SYMBOL_WIFI" %s", ssid);
            /* Show WiFi info */
            lv_obj_clear_flag(disp_ctx.cont_wifi_info, LV_OBJ_FLAG_HIDDEN);
            /* Print WiFi SSID */
            lv_label_set_text_fmt(disp_ctx.lbl_info_ssid, "SSID: %s", ssid);

            char ipbuf[20];
            /* Get IP address */
            app_net_wifi_get_str_ip(ipbuf, sizeof(ipbuf));
            /* Print IP address */
            lv_label_set_text_fmt(disp_ctx.lbl_info_ip, "IPv4: %s", ipbuf);
            /* Get MAC address */
            app_net_wifi_get_str_mac(ipbuf, sizeof(ipbuf));
            /* Print IP address */
            lv_label_set_text_fmt(disp_ctx.lbl_info_mac, "MAC: %s", ipbuf);
        }
    }

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

void disp_lvgl_update_weather(disp_lvgl_weather_t * weather)
{
    assert(weather != NULL);

	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    /* Weather image */
    if (disp_ctx.img_weather != NULL) {
        const lv_img_dsc_t * img = lvgl_get_weather_image(weather->icon);
        if(img)
            lv_img_set_src(disp_ctx.img_weather, img);
    }

    /* Weather place */
    if (disp_ctx.lbl_weather_place != NULL) {
	    lv_label_set_text_fmt(disp_ctx.lbl_weather_place, "%s, %s (Updated: %d:%02d)", (weather->place ? weather->place : "---"), (weather->country ? weather->country : "---"), weather->last_update.hour, weather->last_update.min);
        lv_obj_set_style_text_align(disp_ctx.lbl_weather_place, LV_ALIGN_CENTER, 0);
    }

    /* Weather description */
    if (disp_ctx.lbl_weather_desc != NULL) {
	    lv_label_set_text_fmt(disp_ctx.lbl_weather_desc, "%s (%s)", weather->main, weather->description);
        lv_obj_set_style_text_align(disp_ctx.lbl_weather_desc, LV_ALIGN_CENTER, 0);
    }

    /* Weather temperature */
    if (disp_ctx.lbl_weather_temp != NULL) {
	    lv_label_set_text_fmt(disp_ctx.lbl_weather_temp, "%.1f °C", weather->temp);
    }

    /* Weather image small */
    if (disp_ctx.img_small_weather != NULL) {
        const lv_img_dsc_t * img = lvgl_get_weather_image(weather->icon);
        if(img)
            lv_img_set_src(disp_ctx.img_small_weather, img);
    }

    /* Weather temperature small */
    if (disp_ctx.lbl_small_weather != NULL) {
	    lv_label_set_text_fmt(disp_ctx.lbl_small_weather, "%.1f °C", weather->temp);
    }

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}

void disp_lvgl_update_wifi_scanned_ssid(const char * list, int pos_connected)
{
	/* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    if(disp_ctx.dd_ssid != NULL && list != NULL)
    {
        /* Put list into drop-down and select connected */
        lv_dropdown_set_options(disp_ctx.dd_ssid, list);
        if(pos_connected >= 0)
            lv_dropdown_set_selected(disp_ctx.dd_ssid, pos_connected);
    }

    /* Enable refresh button */
    if (disp_ctx.btn_refresh)
        lv_obj_clear_state(disp_ctx.btn_refresh, LV_STATE_DISABLED);

    /* Screen operation done -> release for the other task */
    lvgl_port_unlock();
}
