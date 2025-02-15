#include <string.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define LED_GPIO 2
#define WIFI_SSID "ESP_PRADITA"
#define WIFI_PASS "12345678"

static const char *TAG = "softap";

// html_page
const char *html_page =
    "<html>"
    "<head>"
    "</head>"
    "<body>"
    "<h3>ESP Pradita</h3>"
    "</body>"
    "</html>";

esp_err_t webpage_handler(httpd_req_t *req)
{
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t led_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO, 1);
    httpd_resp_send(req, "LED ON", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t led_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO, 0);
    httpd_resp_send(req, "LED OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_handle_t start_webserver()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    httpd_start(&server, &config);

    httpd_uri_t uri_page = {.uri = "/", .method = HTTP_GET, .handler = webpage_handler};
    httpd_uri_t uri_on = {.uri = "/on", .method = HTTP_GET, .handler = led_on_handler};
    httpd_uri_t uri_off = {.uri = "/off", .method = HTTP_GET, .handler = led_off_handler};

    // register all routes
    httpd_register_uri_handler(server, &uri_page);
    httpd_register_uri_handler(server, &uri_on);
    httpd_register_uri_handler(server, &uri_off);

    return server;
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // wifi config
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .password = WIFI_PASS,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        }};

    // if the there is no wifi password set authmode to open
    if (strlen(WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Soft AP mode started. SSID:%s Password:%s", WIFI_SSID, WIFI_PASS);
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_softap();
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    start_webserver();
}