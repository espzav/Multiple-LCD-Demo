/* LVGL example header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

typedef struct disp_lvgl_weather_s
{
    char * main;
    char * description;

    double temp;
    double temp_min;
    double temp_max;
    double temp_feels;
    double pressure;
    double humidity;

    char * country;
    char * place;

    char * icon;

    struct
    {
        int hour;
        int min;
    } last_update;
} disp_lvgl_weather_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialization of LVGL
 *
 * @return
 *          - TODO
 */
void disp_lvgl_init(void);

/**
 * @brief Draw everything on small rounded display
 *
 * @param disp-pointer to LVGL display handle
 *
 */
void disp_lvgl_draw_rounded_display(lv_disp_t * disp);

/**
 * @brief Draw all into bigger main display
 *
 * @param disp-pointer to LVGL display handle
 *
 */
void disp_lvgl_draw_big_display(lv_disp_t * disp);

/**
 * @brief Draw all into monochrome display
 *
 * @param disp-pointer to LVGL display handle
 *
 */
void disp_lvgl_draw_monochrome_display(lv_disp_t * disp);

/**
 * @brief Update time in seconds on display/s
 *
 */
void disp_lvgl_update_time_sec(void);

/**
 * @brief Update battery status
 *
 * @param volt_mv    -battery voltage in milivolts
 *
 */
void disp_lvgl_update_battery(uint32_t volt_mv);

/**
 * @brief Update WiFi SSID on display/s
 *
 * @param ssid-SSID of the connected WiFi
 *
 */
void disp_lvgl_update_wifi_ssid(const char * ssid);

/**
 * @brief Update weather on display/s
 *
 * @param weather-Weather structure
 *
 */
void disp_lvgl_update_weather(disp_lvgl_weather_t * weather);

/**
 * @brief Update ssid list in WiFi settings TAB
 *
 * @param list-List of WiFi SSIDs separated with \n
 * @param pos_connected-Position of the connected SSID, if found (otherwise it is -1)
 *
 */
void disp_lvgl_update_wifi_scanned_ssid(const char * list, int pos_connected);

#ifdef __cplusplus
}
#endif
