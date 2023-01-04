/* LVGL LCD display example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include "driver/sdmmc_host.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_vfs_fat.h"
#include "board.h"
#include "sdmmc_cmd.h"

#define APP_SD_MOUNT_POINT "/sd"
#define APP_SD_MAX_PATH_LEN	100

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Function definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "SD";

static bool app_sd_initialized = false;
static SemaphoreHandle_t app_sd_sem = NULL;
static sdmmc_card_t *app_sdcard = NULL;    // Global uSD card handler
/*******************************************************************************
* Private functions
*******************************************************************************/

static void _sd_lock(void)
{
    /* Wait for the other task done the SD card operation */
    xSemaphoreTake(app_sd_sem, portMAX_DELAY);
}

static void _sd_unlock(void)
{
	/* SD card operation done -> release for the other task */
	xSemaphoreGive(app_sd_sem);
}

static char * _sd_path(const char * filename)
{
	static char path[APP_SD_MAX_PATH_LEN];

	assert(filename != NULL);

	snprintf(path, APP_SD_MAX_PATH_LEN, "%s%s%s", APP_SD_MOUNT_POINT, (filename[0] != '/' ? "/" : ""),  filename);

	return path;
}

static uint32_t _sd_get_file_size(const char * filename)
{
	struct stat st;
	int f = 0;

	assert(filename != NULL);

	memset(&st, 0, sizeof(struct stat));

	f = stat(_sd_path(filename), &st);
	if(f == 0)
		return (uint32_t) st.st_size;

	return 0;
}

static bool _is_file_exists(const char * filename)
{
	struct stat st;
	int f = 0;
	bool ret = false;

	assert(filename != NULL);

	memset(&st, 0, sizeof(struct stat));

	f = stat(_sd_path(filename), &st);
	if(f == 0)
		ret = true;

	return ret;
}


/*******************************************************************************
* Public API functions
*******************************************************************************/

void app_sd_init(void)
{
    const esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 10 * 1024
    };

    const sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    const sdmmc_slot_config_t slot_config = {
        .clk = BOARD_SD_CLK,
        .cmd = BOARD_SD_CMD,
        .d0 = BOARD_SD_D0,
        .d1 = GPIO_NUM_NC,
        .d2 = GPIO_NUM_NC,
        .d3 = GPIO_NUM_NC,
        .d4 = GPIO_NUM_NC,
        .d5 = GPIO_NUM_NC,
        .d6 = GPIO_NUM_NC,
        .d7 = GPIO_NUM_NC,
        .cd = SDMMC_SLOT_NO_CD,
        .wp = SDMMC_SLOT_NO_WP,
        .width = 1,
        .flags = 0,
    };

    app_sd_sem = xSemaphoreCreateMutex();
	if (app_sd_sem != NULL) {
        esp_err_t err = esp_vfs_fat_sdmmc_mount(APP_SD_MOUNT_POINT, &host, &slot_config, &mount_config, &app_sdcard);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "SD card initialized");
            sdmmc_card_print_info(stdout, app_sdcard);
            app_sd_initialized = true;
        }
    }

}

void app_sd_deletefile(char * file)
{
	esp_err_t ret;

	if (!app_sd_initialized) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	if (!app_sd_sem) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

    assert(file != NULL);

	/* LOCK */
	_sd_lock();

	ret = unlink(_sd_path(file));
	if(ret != ESP_OK)
	{
		ESP_LOGE(TAG, "Error when removing file from SD card: 0x%x", ret);
	}

	/* UNLOCK */
	_sd_unlock();
}

void app_sd_writefile(char * file, char * data, uint32_t data_len)
{
	int written = 0;
	int f = -1;

	if (!app_sd_initialized) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	if (!app_sd_sem) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	assert(file != NULL);
	assert(data != NULL);

	/* LOCK */
	_sd_lock();

    if(!_is_file_exists(file))
    {
    	/* Create new file and open it */
    	f = creat(_sd_path(file), 0x777);
    	if(f <= 0)
    	{
		    ESP_LOGE(TAG, "Error when creating new file.");
    		goto ERROR;
    	}
    }
    else
    {
		/* Open file */
		f = open(_sd_path(file), O_WRONLY);
		if(f <= 0)
		{
		    ESP_LOGE(TAG, "Error when opening file.");
			goto ERROR;
		}
    }

	/* Write into file */
	written = write(f, data, data_len);
	if(written <= 0)
	{
        ESP_LOGE(TAG, "Error when writing file.");
		goto ERROR;
	}

    assert(written == data_len);

ERROR:
	if(f > 0)
		close(f);

	/* UNLOCK */
	_sd_unlock();
}

void app_sd_appendfile(char * file, char * data, uint32_t data_len)
{
	int written = 0;
	int f = -1;

	if (!app_sd_initialized) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	if (!app_sd_sem) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	assert(file != NULL);
	assert(data != NULL);

	/* LOCK */
	_sd_lock();

    if(!_is_file_exists(file))
    {
    	/* Create new file and open it */
    	f = creat(_sd_path(file), 0x777);
    	if(f <= 0)
    	{
		    ESP_LOGE(TAG, "Error when creating new file \"%s\".", _sd_path(file));
    		goto ERROR;
    	}
    }
    else
    {
		/* Open file */
		f = open(_sd_path(file), O_WRONLY);
		if(f <= 0)
		{
		    ESP_LOGE(TAG, "Error when opening file.");
			goto ERROR;
		}
    }

	/* Move file desc to end */
	lseek(f, 0, SEEK_END);

	/* Write into file */
	written = write(f, data, data_len);
	if(written < 0)
	{
        ESP_LOGE(TAG, "Error when writing file.");
		goto ERROR;
	}

    assert(written == data_len);

ERROR:
	if(f > 0)
		close(f);

	/* UNLOCK */
	_sd_unlock();
}

void app_sd_readfile(char * file, char * data, uint32_t * outlen, uint32_t maxlen)
{
	int readsize = 0;

	if (!app_sd_initialized) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	if (!app_sd_sem) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return;
    }

	assert(file != NULL);
	assert(data != NULL);
	assert(outlen != NULL);

	*outlen = 0;

	/* LOCK */
	_sd_lock();

	/* Open file */
	int f = open(_sd_path(file), O_RDONLY);
	if(f <= 0)
	{
        ESP_LOGE(TAG, "Error when opening file.");
		goto ERROR;
	}

	/* Read file */
    readsize = read(f, data, maxlen);
    if(readsize < 0)
    {
        ESP_LOGE(TAG, "Error when reading file.");
    	goto ERROR;
    }

    *outlen = readsize;

ERROR:
	if(f > 0)
		close(f);

	/* UNLOCK */
	_sd_unlock();
}

uint32_t app_sd_get_file_size(char *path)
{
	uint32_t size = 0;

	if (!app_sd_initialized) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return 0;
    }

	if (!app_sd_sem) {
        ESP_LOGW(TAG, "SD card not initialized!");
		return 0;
    }

	assert(path != NULL);

	/* LOCK */
	_sd_lock();

	if(!_is_file_exists(path))
	{
		goto ERROR;
	}

	size = _sd_get_file_size(path);

ERROR:
	/* UNLOCK */
	_sd_unlock();

    return size;
}

