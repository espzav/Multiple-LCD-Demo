/* Application storage header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialization of storage
 *
 */
void app_storage_init(void);

/**
 * @brief Save data to storage
 *
 * @param key - Storage key
 * @param data - Data to store
 * @param datalen - length of data to store
 *
 */
void app_storage_write(const char * key, const void * data, uint32_t datalen);

/**
 * @brief Read data from storage
 *
 * @param key - Storage key
 * @param data - Buffer for read data
 * @param maxdatalen - Data buffer size
 *
 * @return length of read data
 *
 */
int  app_storage_read(const char * key, void * data, uint32_t maxdatalen);

#ifdef __cplusplus
}
#endif
