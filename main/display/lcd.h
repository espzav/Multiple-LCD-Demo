/* LCD displays control header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

typedef enum
{
    LCD_CONN_TYPE_I2C,
    LCD_CONN_TYPE_SPI,
    LCD_CONN_TYPE_PARALLEL8080,
    LCD_CONN_TYPE_PARALLEL_RGB,
} lcd_conn_type_t;

typedef enum
{
    LCD_DRIVER_GC9A01 = 1,
    LCD_DRIVER_RA8875,
    LCD_DRIVER_RM68120,
    LCD_DRIVER_SH1107,
    LCD_DRIVER_DLPC2607,
    LCD_DRIVER_NT35510,
    LCD_DRIVER_ST7789,
} lcd_driver_t;

typedef struct lcd_disp_s
{
    void * handle;
    void * io;
    lcd_driver_t driver;
    lcd_conn_type_t conn_type;
    void * user_data;
    struct
    {
        int port;
        uint8_t address;
    } i2c;
} lcd_disp_t;

typedef void (*flush_ready_cb_t)(lcd_disp_t * disp);

typedef struct lcd_cfg_s
{
    lcd_driver_t driver;
    flush_ready_cb_t flush_ready_cb;
    struct
    {
        int port;
    } i2c;
} lcd_cfg_t;

typedef struct lcd_cmdset_s
{
	uint16_t cmd;
   	uint8_t	data[16];
	uint8_t	length;
} lcd_cmdset_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init SPI LCD display
 *
 * @param config    -display configuration
 * @return
 *          - pointer to display handle structure or NULL when error
 */
lcd_disp_t * lcd_spi_init(lcd_cfg_t * config);

/**
 * @brief Draw color on SPI LCD display
 *
 * @param handle-pointer to display handle structure
 * @param x1    -X1 offset
 * @param y1    -Y1 offset
 * @param x2    -X2 offset
 * @param y2    -Y2 offset
 * @param color -color buffer
 */
void lcd_spi_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color);

/**
 * @brief Set brightness on SPI display
 *
 * @param disp      -pointer to display handle structure
 * @param percent   -brightness in percent
 *
 */
void lcd_spi_set_brightness(lcd_disp_t * disp, uint8_t percent);

/**
 * @brief Init Parallel LCD display
 *
 * @param config    -display configuration
 * @return
 *          - pointer to display handle structure or NULL when error
 */
lcd_disp_t * lcd_parallel8080_init(lcd_cfg_t * config);

/**
 * @brief Draw color on parallel LCD display
 *
 * @param disp  -pointer to display handle structure
 * @param x1    -X1 offset
 * @param y1    -Y1 offset
 * @param x2    -X2 offset
 * @param y2    -Y2 offset
 * @param color -color buffer
 */
void lcd_parallel8080_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color);

/**
 * @brief Set brightness on parallel display
 *
 * @param disp      -pointer to display handle structure
 * @param percent   -brightness in percent
 *
 */
void lcd_parallel8080_set_brightness(lcd_disp_t * disp, uint8_t percent);

/**
 * @brief Init I2C LCD display
 *
 * @param config    -display configuration
 * @return
 *          - pointer to display handle structure or NULL when error
 */
lcd_disp_t * lcd_i2c_init(lcd_cfg_t * config);

/**
 * @brief Draw color on I2C LCD display
 *
 * @param disp  -pointer to display handle structure
 * @param x1    -X1 offset
 * @param y1    -Y1 offset
 * @param x2    -X2 offset
 * @param y2    -Y2 offset
 * @param color -color buffer
 */
void lcd_i2c_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color);

#if SOC_LCD_RGB_SUPPORTED
/**
 * @brief Init RGB LCD display
 *
 * @param config    -display configuration
 * @return
 *          - pointer to display handle structure or NULL when error
 */
lcd_disp_t * lcd_rgb_init(lcd_cfg_t * config);

/**
 * @brief Draw color on RGB LCD display
 *
 * @param disp  -pointer to display handle structure
 * @param x1    -X1 offset
 * @param y1    -Y1 offset
 * @param x2    -X2 offset
 * @param y2    -Y2 offset
 * @param color -color buffer
 */
void lcd_rgb_draw(lcd_disp_t * disp, int x1, int y1, int x2, int y2, void * color);

#endif //SOC_LCD_RGB_SUPPORTED

#ifdef __cplusplus
}
#endif
