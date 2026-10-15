/* 
  Title: Timer Example
  Description: Ejercicio simple de uso de timer.
               Enciende y apaga led cada 100 ms
*/


#include "freertos/FreeRTOS.h" // Uso timer y macros
#include "driver/gpio.h"       // Pines digitales
#include "esp_log.h"           // Mensaje esp_log
#include "freertos/timers.h"   // Timers

#define led1 2                 // GPIO2

uint8_t led_level = 0;         // Estado actual led
static const char* TAG= "Main"; // TAG creado para usar esplog.
//docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html

uint8_t count = 0;
int interval = 100;        // periodo en ms
int timerId = 1;           // ID del timer
TimerHandle_t xTimers;     // Handler

// Prototipos anunciados para el compilador
esp_err_t init_led(void);
esp_err_t blink_led(void);
esp_err_t set_timer(void); // funcion de seteo timer

void vTimerCallback( TimerHandle_t pxTimer ){
    /* FreeRTOS ejecuta esta funcion cada 50 ms desde
       tarea interna de timers
    */
    ESP_LOGI(TAG, "Event was called from timer");
    blink_led();
}

void app_main(void)
{   // Llama los inicializadores y termina
    init_led();
    set_timer();

} // end app_main


esp_err_t init_led(void){

    gpio_reset_pin(led1);                       // deja en estado por defecto
    gpio_set_direction(led1, GPIO_MODE_OUTPUT); // configura como salida
    return ESP_OK;                              // Devuelve codigo OK
}

esp_err_t blink_led(void){

    led_level = !led_level;                    // Invierte estado
    gpio_set_level(led1, led_level);           // Escrite el estado en PIN
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