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
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_gc9a01.h"
#include "lvgl.h"
#include "lcd.h"
#include "board.h"

#define EXAMPLE_LCD_BL_ON  1
#define EXAMPLE_LCD_BL_OFF 0

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "LCDSPI";

static esp_lcd_panel_handle_t lcd_panel_handle = NULL; //LCD handle

static lcd_disp_t lcd_display = {0};
static flush_ready_cb_t lcd_flush_ready_cb = NULL;

/*******************************************************************************
* Private functions
*******************************************************************************/

static bool lcd_spi_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    if (lcd_flush_ready_cb)
        lcd_flush_ready_cb(user_ctx);

    return false;
}

/*******************************************************************************
* Public API functions
*******************************************************************************/

/* Initialize LCD LVGL example */
lcd_disp_t * lcd_spi_init(lcd_cfg_t * config)
{
    const lcd_cmdset_t * lcd_init = NULL;
    lcd_disp_t * disp = &lcd_display;

    assert(config != NULL);

    disp->driver = config->driver;
    disp->conn_type = LCD_CONN_TYPE_SPI;
    disp->user_data = NULL;

    /* Save flush ready callback */
    lcd_flush_ready_cb = config->flush_ready_cb;

#if (BOARD_DISP_SPI_BL >= 0)
   	gpio_config_t bk_gpio_config =
   	{
      .mode = GPIO_MODE_OUTPUT,
      .pin_bit_mask = 1ULL << BOARD_DISP_SPI_BL
   	};
	ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
	gpio_set_level(BOARD_DISP_SPI_BL, EXAMPLE_LCD_BL_OFF);
#endif

	/* SPI initialization */
	spi_bus_config_t buscfg =
	{
		.sclk_io_num = BOARD_DISP_SPI_SCLK,
		.mosi_io_num = BOARD_DISP_SPI_MOSI,
		.miso_io_num = -1,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = BOARD_DISP_SPI_HRES * BOARD_DISP_SPI_VRES * sizeof(lv_color_t)
	};
	ESP_ERROR_CHECK(spi_bus_initialize(1, &buscfg, SPI_DMA_CH_AUTO));

   	ESP_LOGI(TAG, "SPI initialized");

	/* LCD component initialization */
	esp_lcd_panel_io_spi_config_t io_config =
	{
		.dc_gpio_num = BOARD_DISP_SPI_DC,
		.cs_gpio_num = BOARD_DISP_SPI_CS,
		.pclk_hz = (40 * 1000 * 1000),
		.spi_mode = 0,
		.trans_queue_depth = 10,
		.lcd_cmd_bits = 8,
		.lcd_param_bits = 8,
		.user_ctx = disp
	};

    if(lcd_flush_ready_cb) {
        io_config.on_color_trans_done = lcd_spi_flush_ready;
    }

	esp_lcd_panel_io_handle_t io_handle = NULL;
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) 1, &io_config, &io_handle));

	ESP_LOGI(TAG, "Install LCD driver of st7789");
	esp_lcd_panel_dev_config_t panel_config =
	{
		.reset_gpio_num = BOARD_DISP_SPI_RST,
		.color_space = ESP_LCD_COLOR_SPACE_BGR,
		.bits_per_pixel = 16,
#if (BOARD_TYPE != BOARD_TYPE_ESPBOX)
		.flags.reset_active_high = 0
#endif
	};

    if (config->driver == LCD_DRIVER_GC9A01) {
	    ESP_ERROR_CHECK(esp_lcd_new_panel_gc9a01(io_handle, &panel_config, &lcd_panel_handle));
    } else if (config->driver == LCD_DRIVER_ST7789) {
	    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel_handle));
    } else if (config->driver == LCD_DRIVER_NT35510) {
	    ESP_ERROR_CHECK(esp_lcd_new_panel_nt35510(io_handle, &panel_config, &lcd_panel_handle));
    } else {
        ESP_LOGE(TAG, "Not supported LCD driver!");
        return NULL;
    }

	ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));

   	/* Initialize display */
	for (int i = 0; lcd_init && lcd_init[i].length != 0xFF; i++) {
		esp_lcd_panel_io_tx_param(io_handle, lcd_init[i].cmd, lcd_init[i].data, lcd_init[i].length);
	}

    esp_lcd_panel_reset(lcd_panel_handle);
    esp_lcd_panel_init(lcd_panel_handle);

#if (BOARD_TYPE == BOARD_TYPE_ESPBOX)
    esp_lcd_panel_invert_color(lcd_panel_handle, false);
    esp_lcd_panel_set_gap(lcd_panel_handle, 0, 0);
    esp_lcd_panel_swap_xy(lcd_panel_handle, 0);
    esp_lcd_panel_mirror(lcd_panel_handle, true, true);
#else
	esp_lcd_panel_invert_color(lcd_panel_handle, true);
	esp_lcd_panel_mirror(lcd_panel_handle, false, true);
#endif

	esp_lcd_panel_disp_on_off(lcd_panel_handle, true);

#if (BOARD_DISP_SPI_BL >= 0)
    /* Turn on backlight */
    ESP_LOGI(TAG, "Turn on LCD backlight");
    gpio_set_level(BOARD_DISP_SPI_BL, EXAMPLE_LCD_BL_ON);
#endif

   	ESP_LOGI(TAG, "SPI LCD initialized");

    disp->io = io_handle;
    disp->handle = lcd_panel_handle;

    return disp;
}

void lcd_spi_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color)
{
    assert(disp != NULL);
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)(disp->handle);

    esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2, y2, color);
}
