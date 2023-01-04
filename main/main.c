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
#include "lvgl.h"
#include "app_network.h"
#include "app_storage.h"
#include "app_battery.h"
#include "app_sd.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "board.h"
#if (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_GT911)
#include "esp_lcd_touch_gt911.h"
#elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_TT21100)
#include "esp_lcd_touch_tt21100.h"
#elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_FT5X06)
#include "esp_lcd_touch_ft5x06.h"
#endif
#include "disp_lvgl.h"
#include "esp_timer.h"
#include "esp_lvgl_port.h"

static const char *TAG = "DEMO";

static void app_i2c_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BOARD_I2C_SDA,
        .scl_io_num = BOARD_I2C_SCL,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };
    ESP_ERROR_CHECK(i2c_param_config(CONFIG_I2C_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(CONFIG_I2C_NUM, conf.mode, 0, 0, 0));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Board type: %d, Parallel controller: %d", BOARD_TYPE, BOARD_DISP_PARALLEL_CONTROLLER);

    /* Initialize I2C */
    app_i2c_init();

    /* Initialize storage */
    app_storage_init();

    /* Initialize SD */
    app_sd_init();

    /* Initialize battery measurement */
    app_battery_init();

    /* Initialize time */
    app_net_wifi_time_init();

    /* Initialize WiFi */
    app_net_wifi_init();

    /* Initialize weather */
    app_net_weather_init();

#if (BOARD_DISP_PARALLEL_CONTROLLER > 0)
    /* Initialize Parallel Display */
    lcd_cfg_t lcd_parallel8080_cfg =
    {
        .driver = BOARD_DISP_PARALLEL_CONTROLLER/*LCD_DRIVER_RM68120*/,//LCD_DRIVER_RA8875,
    };
    lcd_disp_t * lcd_8080 = lcd_parallel8080_init(&lcd_parallel8080_cfg);
#endif

#if (BOARD_DISP_SPI_CONTROLLER > 0)
    /* Initialize SPI Display */
    lcd_cfg_t lcd_spi_cfg =
    {
        .driver = BOARD_DISP_SPI_CONTROLLER,
    };
    lcd_disp_t * lcd_spi = lcd_spi_init(&lcd_spi_cfg);
#endif

#if (BOARD_DISP_I2C_CONTROLLER > 0)
    /* Initialize I2C Display */
    lcd_cfg_t lcd_i2c_cfg =
    {
        .driver = LCD_DRIVER_SH1107,
        .i2c = {
            .port = CONFIG_I2C_NUM,
        }
    };
    lcd_disp_t * lcd_i2c = lcd_i2c_init(&lcd_i2c_cfg);
#endif

#if (BOARD_DISP_TOUCH_CONTROLLER > 0)
    esp_lcd_panel_io_handle_t io_handle = NULL;
#if (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_GT911)
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
#elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_TT21100)
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_TT21100_CONFIG();
#elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_FT5X06)
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
#endif
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)CONFIG_I2C_NUM, &io_config, &io_handle));

    /* Initialize touch */
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = BOARD_DISP_TOUCH_HRES,
        .y_max = BOARD_DISP_TOUCH_VRES,
        .rst_gpio_num = -1,
        .int_gpio_num = -1,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
    };
    esp_lcd_touch_handle_t tp;
    #if (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_GT911)
        tp_cfg.flags.swap_xy = 0;
        tp_cfg.flags.mirror_x = 0;
        tp_cfg.flags.mirror_y = 0;
        esp_lcd_touch_new_i2c_gt911(io_handle, &tp_cfg, &tp);
    #elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_TT21100)
        tp_cfg.flags.swap_xy = 0;
        tp_cfg.flags.mirror_x = 1;
        tp_cfg.flags.mirror_y = 0;
        esp_lcd_touch_new_i2c_tt21100(io_handle, &tp_cfg, &tp);
    #elif (BOARD_DISP_TOUCH_CONTROLLER == BOARD_DISP_TOUCH_FT5X06)
        tp_cfg.flags.swap_xy = 0;
        tp_cfg.flags.mirror_x = 0;
        tp_cfg.flags.mirror_y = 0;
        esp_lcd_touch_new_i2c_ft5x06(io_handle, &tp_cfg, &tp);
    #endif
#endif

    /* Initialize LVGL */
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    lv_disp_t * disp_parallel = NULL;
#if (BOARD_DISP_PARALLEL_CONTROLLER > 0)
    /* Add parallel display to LVGL */
    if (lcd_8080) {
        const lvgl_port_display_cfg_t disp_cfg = {
            .io_handle = lcd_8080->io,
            .panel_handle = lcd_8080->handle,
            .buffer_size = BOARD_DISP_PARALLEL_HRES*70,
            .double_buffer = false,
            .hres = BOARD_DISP_PARALLEL_HRES,
            .vres = BOARD_DISP_PARALLEL_VRES,
            .monochrome = false,
            .rotation = {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
            .flags = {
                .buff_dma = true,
            }
        };
        disp_parallel = lvgl_port_add_disp(&disp_cfg);
        assert(disp_parallel);
        lcd_8080->user_data = disp_parallel;
        /* Default theme */
        lv_theme_t * th = lv_theme_default_init(disp_parallel, lv_color_make(0x0F, 0x13, 0xFF), lv_color_make(0x51, 0x51, 0x51), true, &lv_font_montserrat_20);
        /*Assign the theme to the display*/
        lv_disp_set_theme(disp_parallel, th);
    }
#endif

    lv_disp_t * disp_spi = NULL;
#if (BOARD_DISP_SPI_CONTROLLER > 0)
    /* Add SPI display to LVGL */
    if (lcd_spi) {
        const lvgl_port_display_cfg_t disp_cfg = {
            .io_handle = lcd_spi->io,
            .panel_handle = lcd_spi->handle,
            .buffer_size = BOARD_DISP_SPI_HRES*70,
            .double_buffer = false,
            .hres = BOARD_DISP_SPI_HRES,
            .vres = BOARD_DISP_SPI_VRES,
            .monochrome = false,
            .rotation = {
                .swap_xy = true,
                .mirror_x = false,
                .mirror_y = true,
            },
            .flags = {
                .buff_dma = true,
            }
        };
        disp_spi = lvgl_port_add_disp(&disp_cfg);
        assert(disp_spi);
        lcd_spi->user_data = disp_spi;
        /* Default theme */
        lv_theme_t * th = lv_theme_default_init(disp_spi, lv_color_make(0x0F, 0x13, 0xFF), lv_color_make(0x51, 0x51, 0x51), true, &lv_font_montserrat_20);
        /*Assign the theme to the display*/
        lv_disp_set_theme(disp_spi, th);
    } else {
        ESP_LOGW(TAG, "SPI display is not present!");
    }
#endif

#if (BOARD_DISP_I2C_CONTROLLER > 0)
    /* Add I2C display to LVGL */
    lv_disp_t * disp_i2c = NULL;
    if (lcd_i2c) {
        const lvgl_port_display_cfg_t disp_cfg = {
            .io_handle = lcd_i2c->io,
            .panel_handle = lcd_i2c->handle,
            .buffer_size = BOARD_DISP_I2C_HRES*BOARD_DISP_I2C_VRES,
            .double_buffer = true,
            .hres = BOARD_DISP_I2C_VRES,
            .vres = BOARD_DISP_I2C_HRES,
            .monochrome = true,
            .rotation = {
                .swap_xy = false,
                .mirror_x = false,
                .mirror_y = false,
            },
            .flags = {
                .buff_dma = true,
            }
        };
        disp_i2c = lvgl_port_add_disp(&disp_cfg);
        assert(disp_i2c);
        lcd_i2c->user_data = disp_i2c;
        /* Default theme */
        lv_theme_t * th = lv_theme_default_init(disp_i2c, lv_color_make(0x0F, 0x13, 0xFF), lv_color_make(0x51, 0x51, 0x51), true, &lv_font_montserrat_20);
        /*Assign the theme to the display*/
        lv_disp_set_theme(disp_i2c, th);
    } else {
        ESP_LOGW(TAG, "I2C display is not present!");
    }
#endif

#if ((BOARD_DISP_PARALLEL_CONTROLLER > 0 || BOARD_DISP_SPI_CONTROLLER > 0) && BOARD_DISP_TOUCH_CONTROLLER > 0)
    /* Add touch controller to display */
    if (disp_parallel && tp) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = disp_parallel,
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
    } else if (disp_spi && tp) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = disp_spi,
            .handle = tp,
        };
        lvgl_port_add_touch(&touch_cfg);
    }
#endif

#if (BOARD_DISP_SPI_CONTROLLER > 0)
    /* Display animated logo and time on SPI rounded display */
    if (disp_parallel && disp_spi) {
        disp_lvgl_draw_rounded_display(disp_spi);
    }
#endif

#if (BOARD_DISP_I2C_CONTROLLER > 0)
    /* Display something on I2C rounded display */
    if (disp_i2c) {
        disp_lvgl_draw_monochrome_display(disp_i2c);
        lv_disp_set_rotation(disp_i2c, LV_DISP_ROT_270);
    }
#endif

#if (BOARD_DISP_PARALLEL_CONTROLLER > 0 || BOARD_DISP_SPI_CONTROLLER > 0)
    /* Display animated logo and other objects on Parallel display */
    if (disp_parallel) {
        disp_lvgl_draw_big_display(disp_parallel);
    } else if (disp_spi) {
        disp_lvgl_draw_big_display(disp_spi);
    }
#endif


    /* Connect to WiFi */
    app_net_wifi_connect_default();
    /* Update battery status */
    disp_lvgl_update_battery(app_battery_get());

    uint64_t last_battery_update = 0;
    uint64_t now = esp_timer_get_time();
	while (1)
	{
        now = esp_timer_get_time();

        /* Update only each 5s */
        if ((now - last_battery_update) > 2000000 ){
            /* Update battery status */
            disp_lvgl_update_battery(app_battery_get());
            last_battery_update = esp_timer_get_time();
        }

        /* Update time */
        disp_lvgl_update_time_sec();

        // raise the task priority of LVGL and/or reduce the handler period can improve the performance
        vTaskDelay(pdMS_TO_TICKS(100));
   }
}
