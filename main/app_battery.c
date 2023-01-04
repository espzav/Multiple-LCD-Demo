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
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"


#define APP_ADC_BAT_R1	 3300
#define APP_ADC_BAT_R2	 3300
#define APP_ADC_BAT_CNST (2.0)	// = (R1+R2)/R2

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Function definitions
*******************************************************************************/
static bool _adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "BAT";

adc_oneshot_unit_handle_t adc_handle;
adc_cali_handle_t adc_cali_handle;

/*******************************************************************************
* Public API functions
*******************************************************************************/

void app_battery_init(void)
{
    const adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_2,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    const adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_3, &config));

	_adc_calibration_init(ADC_UNIT_2, ADC_ATTEN_DB_11, &adc_cali_handle);
}

uint32_t app_battery_get(void)
{
	esp_err_t ret;
	int adc_raw;
	int voltage_mv = 0;

	ret = adc_oneshot_read(adc_handle, ADC_CHANNEL_3, &adc_raw);
	if(ret != ESP_OK)
		return 0;

	ret = adc_cali_raw_to_voltage(adc_cali_handle, adc_raw, &voltage_mv);
	if(ret != ESP_OK)
		return 0;

    return (voltage_mv*APP_ADC_BAT_CNST);
}

/*******************************************************************************
* Private functions
*******************************************************************************/

static bool _adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGD(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGD(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}
