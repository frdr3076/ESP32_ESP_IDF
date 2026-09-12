/* Blink Example

*/
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"


#define LED_GPIO_NUM    GPIO_NUM_2

void app_main(void){

    // Definir pin como salida.
    gpio_set_direction(LED_GPIO_NUM,GPIO_MODE_OUTPUT); 
    printf("\nPrimer ejemplo con ESP-IDF\n\r");


    for(;;){

        gpio_set_level(LED_GPIO_NUM,1); // 1 enciende

        vTaskDelay(500 / portTICK_PERIOD_MS);

        gpio_set_level(LED_GPIO_NUM,0); // 0 apaga

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

}

