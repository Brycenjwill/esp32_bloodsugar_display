#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "secrets.h"
#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"
#include "timer.h"

static const char *TAG = "wifi";

void time_sync_notification_cb(struct timeval *tv)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];

    time(&now);
    localtime_r(&now, &timeinfo);

    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    
    ESP_LOGI(TAG, "NTP Sync Complete! Got Time: %s", strftime_buf);
    set_time_initialized();
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Disconnected: reason=%d", disc->reason);
        esp_wifi_connect();
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        sntp_set_time_sync_notification_cb(time_sync_notification_cb);
        esp_sntp_setservername(0, "pool.ntp.org");
        esp_sntp_init();
        setenv("TZ", "UTC0", 1);
        tzset();
        ESP_LOGI(TAG, "SNTP initialized, waiting for callback...");
    }
}

void wifi_init_sta(void)
{
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}
