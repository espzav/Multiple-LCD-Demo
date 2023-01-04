/* Application storage header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#define APP_SD_GAME_SCORE_FILE  "score.txt"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialization of SD card
 *
 */
void app_sd_init(void);

/**
 * @brief Delete file from SD card
 *
 */
void app_sd_deletefile(char * file);

/**
 * @brief Write data to file on SD card
 *
 */
void app_sd_writefile(char * file, char * data, uint32_t data_len);

/**
 * @brief Append data to file on SD card
 *
 */
void app_sd_appendfile(char * file, char * data, uint32_t data_len);

/**
 * @brief Read file from SD card
 *
 */
void app_sd_readfile(char * file, char * data, uint32_t * outlen, uint32_t maxlen);

/**
 * @brief Get size of file on SD card
 *
 */
uint32_t app_sd_get_file_size(char *path);



#ifdef __cplusplus
}
#endif
