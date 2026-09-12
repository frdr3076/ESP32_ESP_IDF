/*

Ejemplo ESP32 LOG

*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

#define led1 2

uint8_t led_level = 0;
static const char* TAG= "Main"; // TAG creado para usar esplog.
//docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html

uint8_t count = 0;

esp_err_t init_led(void);
esp_err_t blink_led(void);


void app_main(void)
{

    init_led();

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        blink_led();
        //printf("Led level: %u\n", led_level); // texto de iluminacion led.

        count += 1;
        
        if(count > 30){
            count = 0;
        }
        if(count < 10){
            ESP_LOGI(TAG,"Value: %u.",count);
        }
        if( (count >= 10) && (count < 20) ){
            ESP_LOGW(TAG,"Value: %u.",count);
        }
        if( (count >= 20) && (count <= 30) ){
            ESP_LOGE(TAG,"Value: %u.",count);
        }


    } // end while


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