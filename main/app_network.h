/* Network application header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#define APP_NET_WEATHER_PLACE_MAX   100

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init WiFi
 *
 */
void app_net_wifi_init(void);

/**
 * @brief Connect to WiFi AP (if connected, disconnect and connect to new params)
 *
 * @param ssid SSID of the AP
 * @param password Password of AP
 * @param save save SSID and password to storage
 *
 */
void app_net_wifi_connect(const char * ssid, const char * password, bool save);

/**
 * @brief Connect to WiFi AP (Loaded from storage)
 *
 */
void app_net_wifi_connect_default(void);

/**
 * @brief Get SSID of the AP to which connected
 *
 * @return
 *          - SSID of the AP to which connected
 */
char * app_net_wifi_get_connected(void);

/**
 * @brief Init time and timezone
 *
 */
void app_net_wifi_time_init(void);

/**
 * @brief Start searching WiFi networks
 *
 */
void app_net_wifi_scan(void);

/**
 * @brief Initialize weather.
 *
 */
void app_net_weather_init(void);

/**
 * @brief Get place of weather
 *
 * @return
 *          - String of the place for weather
 */
char * app_net_weather_get_place(void);

/**
 * @brief Set place of weather
 *
 * @param place String of the place for weather
 * @param update When true, the weather will be updated immediatelly, if WiFi is connected
 *
 * @return
 *          - ESP error code (ESP_ERR_INVALID_SIZE when string is longer than buffer for it)
 */
esp_err_t app_net_weather_set_place(const char * place, bool update);

/**
 * @brief Get update interval for weather
 *
 * @return
 *          - Interval in minutes for update weather
 */
uint16_t app_net_weather_get_update_interval(void);

/**
 * @brief Set update interval for weather
 *
 * @param minutes Interval in minutes for update weather
 * @param update When true, the weather will be updated immediatelly, if WiFi is connected
 *
 */
void app_net_weather_set_update_interval(uint16_t minutes, bool update);

/**
 * @brief Get time zones names
 *
 * @return
 *          - String of all time zone names separated by \n
 */
const char * app_net_time_get_zone_names(void);

/**
 * @brief Set time zone from array
 *
 * @param n Time zone array index
 *
 */
void app_net_time_set_zone(uint8_t n);

/**
 * @brief Get IP addres and put it into buffes as string
 *
 * @param buf-Buffer for print IP address
 * @param bufsize-Free size in buffer
 *
 */
void app_net_wifi_get_str_ip(char * buf, uint32_t bufsize);

/**
 * @brief Get MAC addres and put it into buffes as string
 *
 * @param buf-Buffer for print MAC address
 * @param bufsize-Free size in buffer
 *
 */
void app_net_wifi_get_str_mac(char * buf, uint32_t bufsize);

#ifdef __cplusplus
}
#endif
