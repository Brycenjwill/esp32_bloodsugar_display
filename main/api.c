#include "api.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "mbedtls/md.h"
#include <string.h>
#include "secrets.h"
#include "cJson.h"
#include "display.h"
static const char *TAG = "api";

static void sha1_hex(const char *input, char *output_hex)
{
    const mbedtls_md_info_t *md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
    unsigned char hash[20];

    mbedtls_md(md_info, (const unsigned char *)input, strlen(input), hash);

    for (int i = 0; i < 20; i++) {
        sprintf(output_hex + (i * 2), "%02x", hash[i]);
    }
    output_hex[40] = '\0';
}

void api_send_request(const char *host, const char *endpoint, const char *secret)
{
    char url[256];
    snprintf(url, sizeof(url), "%s%s", host, endpoint);

    ESP_LOGI(TAG, "Requesting URL: %s", url);

    char hashedSecret[41];
    sha1_hex(secret, hashedSecret);

    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to create HTTP client");
        return;
    }

    esp_http_client_set_header(client, "api-secret", hashedSecret);
    esp_http_client_set_header(client, "Accept-Encoding", "identity;q=1, *;q=0");

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP open failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    int headers_len = esp_http_client_fetch_headers(client);
    ESP_LOGI(TAG, "Headers length: %d", headers_len);

    char buffer[512];
    int total = 0;

    while (1) {
        int r = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
        if (r <= 0) break;
        buffer[r] = '\0';
        ESP_LOGI(TAG, "%s", buffer);
        total += r;
    }

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        ESP_LOGE(TAG, "JSON parse failed");
        return;
    }

    if (!cJSON_IsArray(root)) {
        ESP_LOGE(TAG, "Expected array");
        cJSON_Delete(root);
        return;
    }

    cJSON *item = cJSON_GetArrayItem(root, 0);
    if (!cJSON_IsObject(item)) {
        ESP_LOGE(TAG, "Expected object inside array");
        cJSON_Delete(root);
        return;
    }

    cJSON *sgv = cJSON_GetObjectItem(item, "sgv");
    if (!cJSON_IsNumber(sgv)) {
        ESP_LOGE(TAG, "sgv missing or not number");
        cJSON_Delete(root);
        return;
    }

    ESP_LOGI(TAG, "sgv: %d", sgv->valueint);

    cJSON *direction = cJSON_GetObjectItem(item, "direction");
    if (!cJSON_IsString(direction)) {
        ESP_LOGE(TAG, "direction missing");
        cJSON_Delete(root);
        return;
    }

    ESP_LOGI(TAG, "direction: %s", direction->valuestring);

    update_display(sgv->valueint, direction->valuestring);

    cJSON_Delete(root);

    ESP_LOGI(TAG, "Total bytes read: %d", total);

    esp_http_client_cleanup(client);
}



void api_task(void)
{
    while (1) {
        api_send_request(API_HOST, API_ENDPOINT, API_SECRET);

        // Sleep for 60 seconds
        vTaskDelay(pdMS_TO_TICKS(90000));
    }
}
