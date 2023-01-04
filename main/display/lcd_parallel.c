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
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ra8875.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "board.h"

#include "lcd.h"

#define EXAMPLE_LCD_RST_ON  0
#define EXAMPLE_LCD_RST_OFF 1

#define EXAMPLE_TCA9554_ADDR 0x20
#define EXAMPLE_CAT5171_ADDR (0x2d)

#define EXAMPLE_I2C_MASTER_TIMEOUT_MS 1000

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "LCD8080";

static lcd_disp_t lcd_display = {0};
static flush_ready_cb_t lcd_flush_ready_cb = NULL;

/*******************************************************************************
* Private functions
*******************************************************************************/

static bool _lcd_parallel8080_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (lcd_flush_ready_cb)
        lcd_flush_ready_cb(user_ctx);

    return false;
}

/*******************************************************************************
* Public API functions
*******************************************************************************/

lcd_disp_t * lcd_parallel8080_init(lcd_cfg_t * config)
{
    lcd_disp_t * disp = &lcd_display;
    esp_lcd_panel_handle_t lcd_panel_handle = NULL;

    assert(config != NULL);

    disp->driver = config->driver;
    disp->conn_type = LCD_CONN_TYPE_PARALLEL8080;
    disp->user_data = NULL;

    /* Save flush ready callback */
    lcd_flush_ready_cb = config->flush_ready_cb;

    if (config->driver == LCD_DRIVER_RM68120) {
        uint8_t write_buf[2] = {0x03, 0x05};
        i2c_master_write_to_device(CONFIG_I2C_NUM, EXAMPLE_TCA9554_ADDR, write_buf, sizeof(write_buf), EXAMPLE_I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
        write_buf[0] = 0x01;
        write_buf[1] = 0xCF;
        i2c_master_write_to_device(CONFIG_I2C_NUM, EXAMPLE_TCA9554_ADDR, write_buf, sizeof(write_buf), EXAMPLE_I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
    }

    /* Init Intel 8080 bus */
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_PLL160M,
        .dc_gpio_num = BOARD_DISP_PARALLEL_DC,
        .wr_gpio_num = BOARD_DISP_PARALLEL_WR,
        .data_gpio_nums = {
            BOARD_DISP_PARALLEL_DB0,
            BOARD_DISP_PARALLEL_DB1,
            BOARD_DISP_PARALLEL_DB2,
            BOARD_DISP_PARALLEL_DB3,
            BOARD_DISP_PARALLEL_DB4,
            BOARD_DISP_PARALLEL_DB5,
            BOARD_DISP_PARALLEL_DB6,
            BOARD_DISP_PARALLEL_DB7,
            BOARD_DISP_PARALLEL_DB8,
            BOARD_DISP_PARALLEL_DB9,
            BOARD_DISP_PARALLEL_DB10,
            BOARD_DISP_PARALLEL_DB11,
            BOARD_DISP_PARALLEL_DB12,
            BOARD_DISP_PARALLEL_DB13,
            BOARD_DISP_PARALLEL_DB14,
            BOARD_DISP_PARALLEL_DB15,
        },
        .bus_width = BOARD_DISP_PARALLEL_WIDTH,
        .max_transfer_bytes = (BOARD_DISP_PARALLEL_HRES) * 80 * sizeof(uint16_t),
        .psram_trans_align = 64,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = BOARD_DISP_PARALLEL_CS,
        .pclk_hz = 10 * 1000 * 1000,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 1,
            .dc_dummy_level = 0,
            .dc_data_level = 0,
        },
        .user_ctx = disp,
        .lcd_cmd_bits = 16,
        .lcd_param_bits = 8,
    };

    if(lcd_flush_ready_cb) {
        io_config.on_color_trans_done = _lcd_parallel8080_flush_ready;
    }


    /* Intialize selected controller */
    if (config->driver == LCD_DRIVER_RM68120) {
        io_config.dc_levels.dc_cmd_level = 0;
        io_config.dc_levels.dc_data_level = 1;
        io_config.lcd_cmd_bits = 16;
        io_config.pclk_hz = 40 * 1000 * 1000;

        #ifdef CONFIG_LV_COLOR_16_SWAP
        ESP_LOGW(TAG, "CONFIG_LV_COLOR_16_SWAP should be disabled!");
        #endif

    } else if (config->driver == LCD_DRIVER_RA8875) {
        io_config.dc_levels.dc_cmd_level = 1;
        io_config.dc_levels.dc_data_level = 0;
        io_config.flags.swap_color_bytes = 1;
        io_config.flags.pclk_idle_low = 0;
        io_config.pclk_hz = 20 * 1000 * 1000; //TODO: can be 40 MHz, when connected on board (not via wires)
    } else {
        ESP_LOGE(TAG, "Not supported LCD driver!");
        return NULL;
    }
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    const esp_lcd_panel_ra8875_config_t vendor_config = {
        .wait_gpio_num = BOARD_DISP_PARALLEL_WAIT,
        .lcd_width = BOARD_DISP_PARALLEL_HRES,
        .lcd_height = BOARD_DISP_PARALLEL_VRES,
        .mcu_bit_interface = BOARD_DISP_PARALLEL_WIDTH,
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BOARD_DISP_PARALLEL_RST,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
        .vendor_config = (void*)&vendor_config,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ra8875(io_handle, &panel_config, &lcd_panel_handle));

	ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, true));

    disp->io = io_handle;
    disp->handle = lcd_panel_handle;

    ESP_LOGI(TAG, "Initialized Intel 8080 bus");

	vTaskDelay(pdMS_TO_TICKS(10));

    return disp;
}

void lcd_parallel8080_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color)
{
    assert(disp != NULL);
    esp_lcd_panel_handle_t lcd_panel_handle = (esp_lcd_panel_handle_t)(disp->handle);

    assert(lcd_panel_handle != NULL);

    esp_lcd_panel_draw_bitmap(lcd_panel_handle, x1, y1, x2, y2, color);
}

void lcd_parallel8080_set_brightness(lcd_disp_t * disp, uint8_t percent)
{
}
