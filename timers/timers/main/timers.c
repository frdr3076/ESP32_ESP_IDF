/* Timers Example*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include "freertos/timers.h"


#define led1 2

uint8_t led_level = 0;
static const char* TAG= "Main"; // TAG creado para usar esplog.
//docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html

uint8_t count = 0;
int interval = 100;
int timerId = 1;
TimerHandle_t xTimers;

esp_err_t init_led(void);
esp_err_t blink_led(void);
esp_err_t set_timer(void); // funcion de seteo timer

void vTimerCallback( TimerHandle_t pxTimer ){
    // Executes when timer finishes.
    ESP_LOGI(TAG, "Event was called from timer");
    blink_led();


}


void app_main(void)
{

    init_led();
    set_timer();



} // end app_main


esp_err_t init_led(void){

    gpio_reset_pin(led1);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

esp_err_t blink_led(void){

    led_level = !led_level;
    gpio_set_level(led1, led_level);
    return ESP_OK;

}

esp_err_t set_timer(void){

    ESP_LOGI(TAG, "Timer init configuration");
    xTimers = xTimerCreate(    "Timer",                             // Just a text name, not used by the kernel.
                                    ( pdMS_TO_TICKS(interval)),    // The timer period in ticks.
                                    pdTRUE,                       // The timers will auto-reload themselves when they expire.
                                    ( void * ) timerId,          // Assign each timer a unique id equal to its array index.
                                    vTimerCallback              // Each timer calls the same callback when it expires.
                                    );
 
    if( xTimers == NULL )
    {          
        ESP_LOGE(TAG, "The timer was not created.");// The timer was not created.
    }
    else
    {
        // Start the timer.  No block time is specified, and even if one was
        // it would be ignored because the scheduler has not yet been
        // started.
        if( xTimerStart( xTimers, 0 ) != pdPASS )
        {
            ESP_LOGE(TAG, "The timer could not be set into Active state.");
            // The timer could not be set into the Active state.
        }
    }

    return ESP_OK;
}