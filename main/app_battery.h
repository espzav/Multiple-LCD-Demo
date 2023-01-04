/* Battery application header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define APP_BATTERY_MAX_MV  (4100)
#define APP_BATTERY_MIN_MV  (3300)

/**
 * @brief Init battery ADC
 *
 */
void app_battery_init(void);

/**
 * @brief Get battery voltage [mV]
 *
 * @return
 *          - Battery voltage in milivolts
 */
uint32_t app_battery_get(void);

#ifdef __cplusplus
}
#endif
