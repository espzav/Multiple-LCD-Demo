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
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#include "cJSON.h"
#include "lcd.h"
#include "esp_lcd_touch.h"
#include "lvgl.h"
#include "disp_lvgl.h"
#include "app_network.h"
#include "app_storage.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

#define DEFAULT_SCAN_LIST_SIZE 10

#define EXAMPLE_WEATHER_SERVER  "api.openweathermap.org"
/* This path can be used, when we have data from GPS */
//#define EXAMPLE_WEATHER_PATH    "/data/2.5/weather?lat=" CONFIG_WEATHER_LAT "&lon=" CONFIG_WEATHER_LON "&units=metric&appid=" CONFIG_WEATHER_API_KEY
#define EXAMPLE_WEATHER_PATH    "/data/2.5/find?q=%s&units=metric&appid=" CONFIG_WEATHER_API_KEY
#define EXAMPLE_WEATHER_PORT    "80"

#define EXAMPLE_STORE_TIMEZONE  "timezone"
#define EXAMPLE_STORE_WIFI_SSID  "wifi_ssid"
#define EXAMPLE_STORE_WIFI_PASS  "wifi_pass"
#define EXAMPLE_STORE_WEATHER_PLACE  "weather_place"
#define EXAMPLE_STORE_WEATHER_UPDATE  "weather_update"

/*******************************************************************************
* Types definitions
*******************************************************************************/

/*******************************************************************************
* Function definitions
*******************************************************************************/
static void http_download_weather(void *pvParameters);

/*******************************************************************************
* Local variables
*******************************************************************************/
static const char *TAG = "WIFI";

static const char *WEATHER_REQUEST = "GET " EXAMPLE_WEATHER_PATH " HTTP/1.0\r\n"
    "Host: "EXAMPLE_WEATHER_SERVER":"EXAMPLE_WEATHER_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

/* Example time zones */
static const char TIME_ZONES_NAMES[] = "Europe/Prague\nEurope/London\nAsia/Shanghai\nAsia/Singapore\nAmerica/New York";
static const char *TIME_ZONES[] = {"CET-1CEST,M3.5.0,M10.5.0/3", "GMT0BST,M3.5.0/1,M10.5.0", "CST-8", "<+08>-8", "EST5EDT,M3.2.0,M11.1.0"};

static bool example_sntp_running = false;
static char example_weather_place[APP_NET_WEATHER_PLACE_MAX] = "BRNO";
static uint16_t example_weather_update_min = CONFIG_WEATHER_UPDATE_MIN;
static esp_timer_handle_t weather_timer = NULL;

static bool example_wifi_connected = false;
static wifi_config_t example_wifi_config;
static esp_netif_t * example_netif = NULL;
/*******************************************************************************
* Private functions
*******************************************************************************/

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);

/*
 * If 'NTP over DHCP' is enabled, we set dynamic pool address
 * as a 'secondary' server. It will act as a fallback server in case that address
 * provided via NTP over DHCP is not accessible
 */
#if LWIP_DHCP_GET_NTP_SRV && SNTP_MAX_SERVERS > 1
    sntp_setservername(1, "pool.ntp.org");

#if LWIP_IPV6 && SNTP_MAX_SERVERS > 2          // statically assigned IPv6 address is also possible
    ip_addr_t ip6;
    if (ipaddr_aton("2a01:3f7::1", &ip6)) {    // ipv6 ntp source "ntp.netnod.se"
        sntp_setserver(2, &ip6);
    }
#endif  /* LWIP_IPV6 */

#else   /* LWIP_DHCP_GET_NTP_SRV && (SNTP_MAX_SERVERS > 1) */
    // otherwise, use DNS address from a pool
    sntp_setservername(0, CONFIG_SNTP_TIME_SERVER);
#endif

#ifdef CONFIG_SNTP_TIME_SYNC_METHOD_SMOOTH
    sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
#endif
    sntp_init();

    ESP_LOGI(TAG, "List of configured NTP servers:");

    for (uint8_t i = 0; i < SNTP_MAX_SERVERS; ++i) {
        if (sntp_getservername(i)){
            ESP_LOGI(TAG, "server %d: %s", i, sntp_getservername(i));
        } else {
            // we have either IPv4 or IPv6 address, let's print it
            char buff[INET6_ADDRSTRLEN];
            ip_addr_t const *ip = sntp_getserver(i);
            if (ipaddr_ntoa_r(ip, buff, INET6_ADDRSTRLEN) != NULL)
                ESP_LOGI(TAG, "server %d: %s", i, buff);
        }
    }

    example_sntp_running = true;
}

static void weather_parse_json(const char * json_str)
{
    disp_lvgl_weather_t weather;
	cJSON *root, *root_weather, *json_list, *item,  *json_weather, *json_main, *json_name, *json_sys;
	int arr_items_cnt = 0;

	assert(json_str != NULL);

    /* Parse JSON */
	root = cJSON_Parse(json_str);
	if (root == NULL) {
        ESP_LOGE(TAG, "JSON parsing error!");
        return;
    }

    root_weather = root;

    /* Get "list" item */
    json_list = cJSON_GetObjectItem(root, "list");
    if (json_list)
    {
        arr_items_cnt = cJSON_GetArraySize(json_list);
        if (arr_items_cnt > 0) {
            /* Get first array item from "list" */
            item = cJSON_GetArrayItem(json_list, 0);
            if(item != NULL)
                root_weather = item;
        }
    }

    ESP_LOGI(TAG, "Weather:");

	/* Get "weather" array */
	json_weather = cJSON_GetObjectItem(root_weather, "weather");
	if (json_weather != NULL) {
        arr_items_cnt = cJSON_GetArraySize(json_weather);
        if (arr_items_cnt > 0) {
            /* Get first array item from "weather" */
            item = cJSON_GetArrayItem(json_weather, 0);

            if(item)
            {
                cJSON *item_main = cJSON_GetObjectItem(item,"main");
                cJSON *item_description = cJSON_GetObjectItem(item,"description");
                cJSON *item_icon = cJSON_GetObjectItem(item,"icon");

                if (item_main) {
                    weather.main = item_main->valuestring;
                    ESP_LOGI(TAG, "     -%s", item_main->valuestring);
                }
                if (item_description) {
                    weather.description = item_description->valuestring;
                    ESP_LOGI(TAG, "     -%s", item_description->valuestring);
                }
                if (item_icon) {
                    weather.icon = item_icon->valuestring;
                    ESP_LOGI(TAG, "     -%s", item_icon->valuestring);
                }
            }

        }
    } else {
        ESP_LOGE(TAG, "Node \"weather\" not found in JSON!");
        goto ERROR;
    }

	/* Get "main" object */
	json_main = cJSON_GetObjectItem(root_weather, "main");
    if (json_main != NULL) {
        cJSON *item_temp = cJSON_GetObjectItem(json_main,"temp");
        cJSON *item_feels_like = cJSON_GetObjectItem(json_main,"feels_like");
        cJSON *item_temp_min = cJSON_GetObjectItem(json_main,"temp_min");
        cJSON *item_temp_max = cJSON_GetObjectItem(json_main,"temp_max");
        cJSON *item_pressure = cJSON_GetObjectItem(json_main,"pressure");
        cJSON *item_humidity = cJSON_GetObjectItem(json_main,"humidity");

        if (item_temp) {
            weather.temp = item_temp->valuedouble;
            ESP_LOGI(TAG, "     -Temp: %0.2f °C", item_temp->valuedouble);
        }
        if (item_feels_like) {
            weather.temp_feels = item_feels_like->valuedouble;
            ESP_LOGI(TAG, "     -Feels: %0.2f °C", item_feels_like->valuedouble);
        }
        if (item_temp_min) {
            weather.temp_min = item_temp_min->valuedouble;
            ESP_LOGI(TAG, "     -Temp min: %0.2f °C", item_temp_min->valuedouble);
        }
        if (item_temp_max) {
            weather.temp_max = item_temp_max->valuedouble;
            ESP_LOGI(TAG, "     -Temp max: %0.2f °C", item_temp_max->valuedouble);
        }
        if (item_pressure) {
            weather.pressure = item_pressure->valueint;
            ESP_LOGI(TAG, "     -Pressure: %d", item_pressure->valueint);
        }
        if (item_humidity) {
            weather.humidity = item_humidity->valueint;
            ESP_LOGI(TAG, "     -Humidity: %d", item_humidity->valueint);
        }
    } else {
        ESP_LOGE(TAG, "Node \"main\" not found in JSON!");
        goto ERROR;
    }

	/* Get "name" object */
	json_name = cJSON_GetObjectItem(root_weather, "name");
    if (json_name != NULL) {
        weather.place = json_name->valuestring;
        ESP_LOGI(TAG, "     -Place: %s", json_name->valuestring);
    } else {
        ESP_LOGE(TAG, "Node \"name\" not found in JSON!");
        goto ERROR;
    }

	/* Get "name" object */
	json_sys = cJSON_GetObjectItem(root_weather, "sys");
    if (json_sys != NULL) {
        cJSON *item_country = cJSON_GetObjectItem(json_sys,"country");
        if (item_country) {
            weather.country = item_country->valuestring;
            ESP_LOGI(TAG, "     -Country: %s", item_country->valuestring);
        }
    } else {
        ESP_LOGE(TAG, "Node \"sys\" not found in JSON!");
        goto ERROR;
    }

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    /* Update time */
    weather.last_update.hour = timeinfo.tm_hour;
    weather.last_update.min = timeinfo.tm_min;

    /* Update weather on display */
    disp_lvgl_update_weather(&weather);

ERROR:

	if(root)
		cJSON_Delete(root);

}

static void weather_timer_callback(void* arg)
{
    if(weather_timer != NULL)
        esp_timer_delete(weather_timer);

    weather_timer = NULL;

    /* Download current weather from web */
    xTaskCreate(&http_download_weather, "http_download_weather", 4096, NULL, 5, NULL);
}

static void http_download_weather(void *pvParameters)
{
    struct addrinfo *res = NULL;
    int s = -1, r;
    char send_recv_buf[1500];

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    /* Weather API key check */
    if (strlen(CONFIG_WEATHER_API_KEY) <= 0) {
        ESP_LOGE(TAG, "Please add weather API key in menuconfig.");
    }

    /* Get IP address from DNS */
    int err = getaddrinfo(EXAMPLE_WEATHER_SERVER, EXAMPLE_WEATHER_PORT, &hints, &res);

    if(err != 0 || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
        goto ERROR;
    }

    /* Create socket */
    s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        ESP_LOGE(TAG, "Socket creating error.");
        goto ERROR;
    }

    /* Connect to server */
    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_LOGE(TAG, "Socket connect failed errno=%d", errno);
        goto ERROR;
    }
    ESP_LOGI(TAG, "Connected to %s", EXAMPLE_WEATHER_SERVER);
    freeaddrinfo(res);
    res = NULL;

    /* Prepare HTTP request */
    snprintf(send_recv_buf, sizeof(send_recv_buf), WEATHER_REQUEST, example_weather_place);

    /* Write HTTP request */
    if (write(s, send_recv_buf, strlen(send_recv_buf)) < 0) {
        ESP_LOGE(TAG, "Socket send failed");
        goto ERROR;
    }
    ESP_LOGI(TAG, "HTTP request send");

    /* Socket timeout */
    struct timeval receiving_timeout;
    receiving_timeout.tv_sec = 5;
    receiving_timeout.tv_usec = 0;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
            sizeof(receiving_timeout)) < 0) {
        ESP_LOGE(TAG, "Failed to set socket receiving timeout");
        goto ERROR;
    }

    int received = 0;
    bzero(send_recv_buf, sizeof(send_recv_buf));

    /* Read HTTP response */
    do {
        r = read(s, &send_recv_buf[received], sizeof(send_recv_buf) - received - 1);
        received += r;
    } while(r > 0);

    /* Termination */
    send_recv_buf[received] = 0;

    /* Looking for star HTTP body */
    int json_start = 0;
    for (int i = 4; i < received; i++) {
        if(send_recv_buf[i] == '\n' && send_recv_buf[i-1] == '\r' && send_recv_buf[i-2] == '\n' && send_recv_buf[i-3] == '\r') {
            json_start = i+1;
            break;
        }
    }

    ESP_LOGI(TAG, "Read weather info done.");

    ESP_LOGI(TAG, "%s", &send_recv_buf[json_start]);

    /* Parse received JSON and save data */
    weather_parse_json(&send_recv_buf[json_start]);

    /* Call again after X minutes */
    const esp_timer_create_args_t timer_args = {
            .callback = &weather_timer_callback,
            .name = "weather"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &weather_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(weather_timer, example_weather_update_min*60000000));

ERROR:
    if(s >= 0)
        close(s);

    if(res != NULL)
        freeaddrinfo(res);

    /* Close task */
    vTaskDelete( NULL );

}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "connecting...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        example_wifi_connected = false;
        //esp_wifi_connect();
        disp_lvgl_update_wifi_ssid(app_net_wifi_get_connected());
        ESP_LOGI(TAG, "retry to connect to the AP");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        example_wifi_connected = true;
        if (!example_sntp_running)
            initialize_sntp();

        disp_lvgl_update_wifi_ssid(app_net_wifi_get_connected());

        /* Download current weather from web */
        xTaskCreate(&http_download_weather, "http_download_weather", 4096, NULL, 5, NULL);
    }
}

static void wifi_scan(void *pvParameters)
{
    char ssid_list[33*DEFAULT_SCAN_LIST_SIZE] = "";
    int connected_id = 0;
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    if (!example_wifi_connected) {
        memset(&example_wifi_config, 0, sizeof(example_wifi_config));
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        example_wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &example_wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    /* Start scanning */
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
    ESP_LOGI(TAG, "WiFi scanning...");

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);

    /* Loop results */
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        ESP_LOGI(TAG, "SSID \t\t%s (RSSI %d)", ap_info[i].ssid, ap_info[i].rssi);

        /* Connected WiFi now */
        if (example_wifi_connected && strcmp(app_net_wifi_get_connected(), (char*)ap_info[i].ssid) == 0)
            connected_id = i;

        snprintf(ssid_list+strlen(ssid_list), sizeof(ssid_list)-strlen(ssid_list), "%s\n", ap_info[i].ssid);
    }

    /* Put results to LVGL display */
    disp_lvgl_update_wifi_scanned_ssid(ssid_list, connected_id);

    /* Close task */
    vTaskDelete( NULL );
}

/*******************************************************************************
* Public API functions
*******************************************************************************/

void app_net_wifi_time_init(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        // update 'now' variable with current time
        time(&now);
    }

    /* Load timezone from storage */
    char timezone_buf[100];
    int readsize = 0;
    readsize = app_storage_read(EXAMPLE_STORE_TIMEZONE, (uint8_t*)timezone_buf, sizeof(timezone_buf));
    if (readsize > 0) {
        /* Set timezone */
        setenv("TZ", timezone_buf, 1);
    } else {
        /* Set default timezone */
        setenv("TZ", CONFIG_TIME_ZONE, 1);
    }

    tzset();
    localtime_r(&now, &timeinfo);
}

void app_net_wifi_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    example_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    /**
     * NTP server address could be aquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp aquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
#endif

    ESP_LOGI(TAG, "WiFi initialized.");
}

void app_net_weather_init(void)
{
    int readsize = 0;
    uint16_t weather_update = 0;
    char weather_place[APP_NET_WEATHER_PLACE_MAX];

    /* Load Weather place from storage */
    readsize = app_storage_read(EXAMPLE_STORE_WEATHER_PLACE, (uint8_t*)weather_place, sizeof(weather_place));
    if (readsize > 0)
        strcpy(example_weather_place, weather_place);

    /* Load Weather update interval from storage */
    readsize = app_storage_read(EXAMPLE_STORE_WEATHER_UPDATE, (uint8_t*)&weather_update, sizeof(uint16_t));
    if (readsize > 0)
        example_weather_update_min = weather_update;

}

void app_net_wifi_connect_default(void)
{
    int readsize = 0;
    char wifi_ssid[33];
    char wifi_pass[33];

    /* Load WiFi SSID from storage */
    readsize = app_storage_read(EXAMPLE_STORE_WIFI_SSID, (uint8_t*)wifi_ssid, sizeof(wifi_ssid));

    if (readsize > 0) {
        /* Load WiFi PASS from storage */
        readsize = app_storage_read(EXAMPLE_STORE_WIFI_PASS, (uint8_t*)wifi_pass, sizeof(wifi_pass));

        /* Try to connect to saved WiFi network */
        app_net_wifi_connect(wifi_ssid, (readsize > 0 ? wifi_pass : NULL), false);
    }
}

void app_net_wifi_connect(const char * ssid, const char * password, bool save)
{
    memset(&example_wifi_config, 0, sizeof(example_wifi_config));

    assert(ssid != NULL);

    /* Prepared settings for scan when connected */
	example_wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;

    /* Copy SSID */
    strcpy((char*)example_wifi_config.sta.ssid, ssid);

    /* Copy password if selected */
    if (password != NULL) {
        strcpy((char*)example_wifi_config.sta.password, password);
        /* Setting a password implies station will connect to all security modes including WEP/WPA.
            * However these modes are deprecated and not advisable to be used. Incase your Access point
            * doesn't support WPA2, these mode can be enabled by commenting below line */
        example_wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }

    example_wifi_config.sta.pmf_cfg.capable = true;
    example_wifi_config.sta.pmf_cfg.required = false;

    if (save) {
        /* Save WiFi SSID to storage */
        app_storage_write(EXAMPLE_STORE_WIFI_SSID, (const uint8_t*)ssid, strlen(ssid)+1);
        /* Save WiFi password to storage */
        if (password)
            app_storage_write(EXAMPLE_STORE_WIFI_PASS, (const uint8_t*)password, strlen(password)+1);
    }

    /* Disconnect when connected */
    if (example_wifi_connected) {
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        /* Wait for disconnect */
        while (example_wifi_connected != false);
    }
    ESP_ERROR_CHECK(esp_wifi_stop());

    ESP_LOGI(TAG, "Trying to connect to WiFi SSID: %s", (char*)example_wifi_config.sta.ssid);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &example_wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
}

char * app_net_wifi_get_connected(void)
{
    if(example_wifi_connected)
        return (char*)example_wifi_config.sta.ssid;

    return NULL;
}

void app_net_wifi_scan(void)
{
    /* Start scanning in task */
    xTaskCreate(&wifi_scan, "wifi_scan", 4096, NULL, 5, NULL);
}

const char * app_net_time_get_zone_names(void)
{
    return TIME_ZONES_NAMES;
}

void app_net_time_set_zone(uint8_t n)
{
    /* Save timezone to storage */
    app_storage_write(EXAMPLE_STORE_TIMEZONE, (const uint8_t*)TIME_ZONES[n], strlen(TIME_ZONES[n])+1);

    // Set timezone to Central Europe Time and print local time
    setenv("TZ", TIME_ZONES[n], 1);
    tzset();
}

char * app_net_weather_get_place(void)
{
    return example_weather_place;
}

esp_err_t app_net_weather_set_place(const char * place, bool update)
{
    assert(place != NULL);

    /* Check maximum size of string */
    if (strlen(place) > sizeof(example_weather_place)) {
        return ESP_ERR_INVALID_SIZE;
    }

    /* Save weather place to storage */
    app_storage_write(EXAMPLE_STORE_WEATHER_PLACE, (const uint8_t*)place, strlen(place)+1);
    ESP_LOGI(TAG, "Stored weather_place (%d): %s", strlen(place)+1, place);

    /* Copy place */
    strcpy(example_weather_place, place);

    if (update && example_wifi_connected) {
        /* Get weather with new place */
        weather_timer_callback(NULL);
    }

    return ESP_OK;
}

void app_net_weather_set_update_interval(uint16_t minutes, bool update)
{
    example_weather_update_min = minutes;

    /* Save weather place to storage */
    app_storage_write(EXAMPLE_STORE_WEATHER_UPDATE, (const uint8_t*)&minutes, sizeof(uint16_t));

    if (update && example_wifi_connected) {
        /* Get weather with new place */
        weather_timer_callback(NULL);
    }
}

uint16_t app_net_weather_get_update_interval(void)
{
    return example_weather_update_min;
}

void app_net_wifi_get_str_ip(char * buf, uint32_t bufsize)
{
    esp_netif_ip_info_t ipInfo;

    assert(buf != NULL);

    if (example_netif) {
        /* Get IP address info */
        esp_netif_get_ip_info(example_netif, &ipInfo);

        /* Print IP address to buffer */
        snprintf(buf, bufsize, IPSTR, IP2STR(&ipInfo.ip));
    }
}

void app_net_wifi_get_str_mac(char * buf, uint32_t bufsize)
{
    uint8_t mac[6];

    assert(buf != NULL);

    if (example_netif) {
        /* Get MAC address info */
        esp_netif_get_mac(example_netif, mac);

        /* Print MAC address to buffer */
        snprintf(buf, bufsize, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}
