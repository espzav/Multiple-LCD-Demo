/* Multiple LCD displays Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "lcd.h"
#include "esp_lcd_touch_tt21100.h"
#include "lvgl.h"
#include "disp_lvgl.h"
#include "app_network.h"
#include "app_storage.h"
#include "driver/gpio.h"
#include "driver/i2c.h"

static const char *TAG = "DEMO";

static void lcd_spi_flush_ready_cb(lcd_disp_t * disp)
{
   disp_lvgl_flush_ready(disp);
}

static void lcd_8080_flush_ready_cb(lcd_disp_t * disp)
{
   disp_lvgl_flush_ready(disp);
}

static void lcd_i2c_flush_ready_cb(lcd_disp_t * disp)
{
   disp_lvgl_flush_ready(disp);
}

static void app_i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_I2C_SDA,
        .scl_io_num = CONFIG_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(CONFIG_I2C_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(CONFIG_I2C_NUM, conf.mode, 0, 0, 0));
}

void app_main(void)
{
    /* Initialize I2C */
    app_i2c_init();

    /* Initialize storage */
    app_storage_init();

    /* Initialize time */
    app_net_wifi_time_init();

    /* Initialize WiFi */
    app_net_wifi_init();

    /* Initialize weather */
    app_net_weather_init();

    /* Initialize Parallel Display */
    lcd_cfg_t lcd_parallel8080_cfg =
    {
        .driver = LCD_DRIVER_RM68120/*LCD_DRIVER_RM68120*/,//LCD_DRIVER_RA8875,
        .flush_ready_cb = lcd_8080_flush_ready_cb,
    };
    lcd_disp_t * lcd_8080 = NULL;//lcd_parallel8080_init(&lcd_parallel8080_cfg);

    /* Initialize SPI Display */
    lcd_cfg_t lcd_spi_cfg =
    {
        .driver = LCD_DRIVER_ST7789,
        .flush_ready_cb = lcd_spi_flush_ready_cb,
    };
    lcd_disp_t * lcd_spi = lcd_spi_init(&lcd_spi_cfg);

    /* Initialize I2C Display */
    lcd_cfg_t lcd_i2c_cfg =
    {
        .driver = LCD_DRIVER_SH1107,
        .flush_ready_cb = lcd_i2c_flush_ready_cb,
        .i2c = {
            .port = CONFIG_I2C_NUM,
        }
    };
    lcd_disp_t * lcd_i2c = NULL;//lcd_i2c_init(&lcd_i2c_cfg);

    /* Initialize touch */
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = CONFIG_LCD_SPI_HRES,
        .y_max = CONFIG_LCD_SPI_VRES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 1,
            .mirror_x = 1,
            .mirror_y = 1,
        },
        .device = {
            .i2c = {
                .port = CONFIG_I2C_NUM,
            }
        }
    };
    esp_lcd_touch_handle_t tp = esp_lcd_touch_new_i2c_tt21100(&tp_cfg);

    /* Initialize LVGL */
    disp_lvgl_init();

    /* Add parallel display to LVGL */
    lv_disp_t * disp_parallel = NULL;
    if (lcd_8080) {
        disp_parallel = disp_lvgl_add(lcd_8080, CONFIG_LCD_PARALLEL_VRES, CONFIG_LCD_PARALLEL_HRES, false);
    } else {
        ESP_LOGW(TAG, "Parallel display is not present!");
    }

    /* Add SPI display to LVGL */
    lv_disp_t * disp_spi = NULL;
    if (lcd_spi) {
        disp_spi = disp_lvgl_add(lcd_spi, CONFIG_LCD_SPI_VRES, CONFIG_LCD_SPI_HRES, false);
    } else {
        ESP_LOGW(TAG, "SPI display is not present!");
    }

    /* Add I2C display to LVGL */
    lv_disp_t * disp_i2c = NULL;
    if (lcd_i2c) {
        disp_i2c = disp_lvgl_add(lcd_i2c, CONFIG_LCD_I2C_VRES, CONFIG_LCD_I2C_HRES, true);
    } else {
        ESP_LOGW(TAG, "I2C display is not present!");
    }

    /* Add touch controller to display */
    if (disp_spi && tp) {
        disp_lvgl_add_touch(disp_spi, tp);
    }

    /* Display animated logo and time on SPI rounded display */
   /* if (disp_spi) {
        disp_lvgl_draw_rounded_display(disp_spi);
    }*/

    /* Display something on I2C rounded display */
    if (disp_i2c) {
        disp_lvgl_draw_monochrome_display(disp_i2c);
    }

    /* Display animated logo and other objects on Parallel display */
    if (disp_spi) {
        disp_lvgl_draw_big_display(disp_spi);
    }

    /* Connect to WiFi */
    app_net_wifi_connect_default();

    uint32_t delay = 10;
	while (1)
	{
        /* Poll LVGL LCD */
        delay = disp_lvgl_poll();
        if (delay < 10)
            delay = 10;
        else if (delay > 100)
            delay = 100;

        /* Update time */
        disp_lvgl_update_time_sec();

        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(delay));
   }
}
