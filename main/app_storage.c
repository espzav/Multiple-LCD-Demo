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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lcd.h"
#include "esp_lcd_touch.h"
#include "lvgl.h"
#include "disp_lvgl.h"
#include "app_network.h"
#include "nvs.h"

#define STORAGE_NAMESPACE 	"storage"

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "STORAGE";

/*******************************************************************************
* Private functions
*******************************************************************************/


/*******************************************************************************
* Public API functions
*******************************************************************************/

void app_storage_init(void)
{
	esp_err_t err;

    /* Initialization of NVS */
	err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
    	ESP_LOGW(TAG, "No free space in storage or new version found. Storage will be erased now!");

        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
    	ESP_ERROR_CHECK(nvs_flash_erase());

        err = nvs_flash_init();
    }

    ESP_ERROR_CHECK(err);
}

void app_storage_write(const char * key, const void * data, uint32_t datalen)
{
    nvs_handle my_handle;

    assert(data != NULL);

    /* Open storage */
	ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));

	/* Write data */
	ESP_ERROR_CHECK(nvs_set_blob(my_handle, key, data, datalen));

	 /* Commit to storage */
	ESP_ERROR_CHECK(nvs_commit(my_handle));

	/* Close storage */
	nvs_close(my_handle);
}

int app_storage_read(const char * key, void * data, uint32_t maxdatalen)
{
	esp_err_t err;
    nvs_handle my_handle;
    size_t len = 0;

    assert(data != NULL);

    /* Open storage */
	ESP_ERROR_CHECK(nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle));

	/* Read size */
	err = nvs_get_blob(my_handle, key, NULL, &len);
    if (err == ESP_ERR_NVS_NOT_FOUND || err == ESP_ERR_NVS_PART_NOT_FOUND)
        goto ERROR;

    /* Check size */
	if (len > maxdatalen)
        goto ERROR;

    /* Read data */
	err = nvs_get_blob(my_handle, key, data, &len);
    if (err == ESP_ERR_NVS_NOT_FOUND || err == ESP_ERR_NVS_PART_NOT_FOUND)
        len = 0;
    else if (err != ESP_OK)
        ESP_ERROR_CHECK(err);

ERROR:
	/* Close storage */
	nvs_close(my_handle);

    return len;
}
