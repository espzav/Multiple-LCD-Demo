file(GLOB_RECURSE IMAGE_SOURCES images/*.c)
file(GLOB_RECURSE IMAGE_WEATHER_SOURCES images/weather/*.c)
file(GLOB_RECURSE DISPLAY_SOURCES display/*.c)
file(GLOB_RECURSE FONTS_SOURCES fonts/*.c)

idf_component_register(
    SRCS "app_storage.c" "app_network.c" "main.c" "disp_lvgl.c" "app_storage.c" "app_battery.c" "app_sd.c" "lv_100ask_2048/lv_100ask_2048.c" ${DISPLAY_SOURCES} ${IMAGE_SOURCES} ${IMAGE_WEATHER_SOURCES} ${FONTS_SOURCES}
    INCLUDE_DIRS "" "board" "display" "lv_100ask_2048"
    REQUIRES "driver" "fatfs" "esp_timer" "nvs_flash" "json" "esp_lcd" "lvgl" "esp_event" "esp_wifi" "esp_adc" "esp_lcd_sh1107" "list" "esp_lvgl_port"
)
