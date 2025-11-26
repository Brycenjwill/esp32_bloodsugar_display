idf_component_register(SRCS "display.c" "wifi.c" "main.c" "api.c" "display.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_event esp_netif nvs_flash mbedtls esp_http_client json driver)