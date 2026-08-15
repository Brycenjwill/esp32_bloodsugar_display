#include "timer.h"
#include "freertos/FreeRTOS.h"
#include <math.h>
#include "esp_log.h"

static int reset = 0; // Set to 1 to reset 
static int minutes_since = 0;
static int64_t start_time = 0;
static int time_initialized = 0;

int get_minutes_since() {
    return minutes_since;
}

void reset_timer(int64_t start_t) {
    start_time = start_t;
    minutes_since = 0;
    reset = 1;
}

void set_time_initialized() {
    time_initialized = 1;
}

void do_timer(void *pvParameters) {
    while (true) {
        while (time_initialized == 0) {
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }     
        while (reset == 0) {
            vTaskDelay(pdMS_TO_TICKS(100)); 
        }        
        reset = 0;
        ESP_LOGI("TIMER", "New start time: %lld", start_time);

        time_t now;
        time(&now); // System time is set when wifi is setup, so system time should be set before the start_time is set.
        int64_t time_in_ms = (int64_t)now * 1000LL; // Current time (rounded to seconds, since we are dealing with time in minutes anyways)
        ESP_LOGI("TIMER", "New time: %lld", time_in_ms);
        minutes_since = (int)((time_in_ms - start_time) / 60000); // Set minutes since
        int ms_since = (int)(time_in_ms - start_time - (minutes_since * 60000));
        ESP_LOGI("TIMER", "New minutes since: %d", minutes_since);

        // Loop and set minutes_since every time we cross 1 more minute since start_time
        for (int i = minutes_since; i < 5; i ++) {
            vTaskDelay(pdMS_TO_TICKS(60000 -  ms_since));
            ms_since = 0;
            minutes_since += 1;

            if (reset == 1) {
                // Get out if reset signal is set
                break;
            }
            ESP_LOGI("TIMER", "New minutes since: %d", minutes_since);
        }
    }
}
void start_timer() {
    xTaskCreate(
        do_timer,
        "timer_task",
        3072,
        NULL,
        5,
        NULL
    );
}