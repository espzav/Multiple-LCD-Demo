/* LVGL I2C LCD display example

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
#include "esp_lcd_sh1107.h"
#include "lvgl.h"
#include "lcd.h"
#include "board.h"

#define LCD_SH1107_I2C_RST_ON  0
#define LCD_SH1107_I2C_RST_OFF 1

#define LCD_I2C_TIMEOUT_MS 20

#define LCD_SH1107_I2C_ADDRESS 0x3C //0x3D
#define LCD_SH1107_I2C_CMD  0X00
#define LCD_SH1107_I2C_RAM  0X40


/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "LCDI2C";

static lcd_disp_t lcd_display = {0};
static flush_ready_cb_t lcd_flush_ready_cb = NULL;

/*******************************************************************************
* Private functions
*******************************************************************************/

static bool _lcd_i2c_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (lcd_flush_ready_cb)
        lcd_flush_ready_cb(user_ctx);

    return false;
}

/*******************************************************************************
* Public API functions
*******************************************************************************/

/* Initialize LCD LVGL example */
lcd_disp_t * lcd_i2c_init(lcd_cfg_t * config)
{
    lcd_disp_t * disp = &lcd_display;

    assert(config != NULL);

    disp->driver = config->driver;
    disp->conn_type = LCD_CONN_TYPE_I2C;
    disp->user_data = NULL;

    /* Save flush ready callback */
    lcd_flush_ready_cb = config->flush_ready_cb;

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = ESP_LCD_IO_I2C_SH1107_CONFIG();
    if(lcd_flush_ready_cb) {
        io_config.on_color_trans_done = _lcd_i2c_flush_ready;
    }
    io_config.user_ctx = disp;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)config->i2c.port, &io_config, &io_handle));
    esp_lcd_panel_handle_t lcd_panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = BOARD_DISP_I2C_RST,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &lcd_panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_panel_handle, false));
    //esp_lcd_panel_swap_xy(lcd_panel_handle, true);
    //esp_lcd_panel_mirror(lcd_panel_handle, false, false);
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, true));

    disp->io = io_handle;
    disp->handle = lcd_panel_handle;

   	ESP_LOGI(TAG, "I2C LCD initialized");

	vTaskDelay(pdMS_TO_TICKS(10));

    return disp;
}

void lcd_i2c_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color)
{
    assert(disp != NULL);
    esp_lcd_panel_handle_t lcd_panel_handle = (esp_lcd_panel_handle_t)(disp->handle);

    assert(lcd_panel_handle != NULL);

    esp_lcd_panel_draw_bitmap(lcd_panel_handle, x1, y1, x2, y2, color);

 #if 0
    assert(disp != NULL);

    uint8_t columnLow = x1 & 0x0F;
	uint8_t columnHigh = (x1 >> 4) & 0x0F;
    uint8_t row1 = 0, row2 = 0;
    uint32_t size = 0;
    lv_color_t *ptr;

    /* For landscape use these lines */
    row1 = x1>>3;
    row2 = x2>>3;

    /* For portrait use these lines (Must be right changed flush, pixel callback and display size swapped) */
    //row1 = y1>>3;
    //row2 = y2>>3;

    for(int i = row1; i < row2+1; i++){
        lcd_i2c_write_byte(disp, LCD_SH1107_I2C_CMD, 0x10 | columnHigh);
        lcd_i2c_write_byte(disp, LCD_SH1107_I2C_CMD, 0x00 | columnLow);
        lcd_i2c_write_byte(disp, LCD_SH1107_I2C_CMD, 0xB0 | i);
	    size = y2 - y1 + 1;

        /* For landscape use this line */
        ptr = color + i * 128;

        /* For portrait use these lines (Must be right changed flush, pixel callback and display size swapped) */
        //ptr = color + i * 64;

        if (i != row2) {
            lcd_i2c_write_bytes(disp, LCD_SH1107_I2C_RAM, (uint8_t*)ptr, size);
        }
    }

    if (lcd_flush_ready_cb)
        lcd_flush_ready_cb(disp);
#endif
}
