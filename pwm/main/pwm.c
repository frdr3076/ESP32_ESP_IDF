/* 
    Title: PWM Example 
    Description:  Cambia brillo gradualmente de 3 LEDs
                  (ver ejercicio timers.c)
    Diagram:
                GPIO33 ── 220–330 Ω ── R ┐
                GPIO25 ── 220–330 Ω ── G ├── cátodo común ── GND
                GPIO26 ── 220–330 Ω ── B ┘
*/

#include "freertos/FreeRTOS.h"  // Utilizaremos timer y macros de FreeRTOS
#include "driver/gpio.h"        // Pines digitales (gpio_set_level)
#include "esp_log.h"            // Mensajes de esp_log
#include "freertos/timers.h"    // Timers
#include "driver/ledc.h"        // periferico PWM para ESP32

#define led1 2                  // GPIO2

uint8_t led_level = 0;          // Estado actual del led
static const char* TAG= "Main"; // TAG creado para usar con esplog I() Main:...
//docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html

uint8_t count = 0;
int interval = 50;      // período del timer en ms
int timerId = 1;        // ID del timer (util si se usan varios con el Callback)
int dutyR = 0;          // Duty Cycle de cada color
int dutyG = 300;
int dutyB = 600;

TimerHandle_t xTimers;  // Handler para los timers

// Prototipos anunciados para el compilador
esp_err_t init_led(void);
esp_err_t blink_led(void);
esp_err_t set_timer(void);
esp_err_t set_pwm(void);
esp_err_t set_pwm_duty(void);


void vTimerCallback( TimerHandle_t pxTimer ){
    // FreeRTOS ejecuta esta funcion cada 50 ms desde tarea interna de timers
    
    // sube los duty (diente de cierra), supera 1023 y se apaga:
    // Cuenta: (1023/10)*0.05 = 5 seg

    dutyR += 10;   
    if(dutyR > 1023)
        dutyR=0;

    dutyG += 10;
    if(dutyG > 1023)
        dutyG=0;

    dutyB += 10;
    if(dutyB > 1023)
        dutyB=0;


    blink_led();     // cambia el estado del pin cuando se lo llama
    set_pwm_duty();

}


void app_main(void)
{   // Llama las tres inicializaciones y termina la tarea.

    init_led();
    set_pwm();
    set_timer();

} // end app_main


esp_err_t init_led(void){

    gpio_reset_pin(led1);                       // deja en estado por defecto
    gpio_set_direction(led1, GPIO_MODE_OUTPUT); // configura como salida
    return ESP_OK;                              // Devuelve codigo OK
}

esp_err_t blink_led(void){

    led_level = !led_level;                    // invierte estado
    gpio_set_level(led1, led_level);           // escribe el estado en PIN
    return ESP_OK;

}

esp_err_t set_timer(void){

    ESP_LOGI(TAG, "Timer init configuration");           // Imprime mensaje en monitor serie
    xTimers = xTimerCreate(    "Timer",                         // Nombre para debug
                                ( pdMS_TO_TICKS(interval)),    // periodo ms a ticks: pdMS_TO_TICKS(interval)
                                pdTRUE,                       // pdTRUE se repite indefinidamente
                                ( void * ) timerId,          // ID del Timer
                                vTimerCallback              // Calls the callback cuando expira
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

esp_err_t set_pwm(void){
    /* 
      Abrir libreria en pestaña en paralelo y traer las funciones.
      LEDC tiene 2 piezas: 
        - Timers que fijan frecuencia y resolución
        - Canales que conectan timer con pin y le dan duty. Aquí 1 timer alimenta 3 canales.
    */

    // Configuración de ledc.h

    // Configuracion de Canales: esp_err_t ledc_channel_config(const ledc_channel_config_t *ledc_conf);
    ledc_channel_config_t channelConfigR = {0};        // Estructura de configuración en 0 (valores no usados init en 0)
    channelConfigR.gpio_num = 33;                      // GPIO33
    channelConfigR.speed_mode = LEDC_HIGH_SPEED_MODE;  // Cambios de duty se aplican por hardware
    channelConfigR.channel = LEDC_CHANNEL_0;           // CH0 de los 8 disponibles
    channelConfigR.intr_type = LEDC_INTR_DISABLE;      // Sin interrupciones
    channelConfigR.timer_sel = LEDC_TIMER_0;           // Frecuencia tomada de TIMER0
    channelConfigR.duty = 0;                           // Init con duty 0

    ledc_channel_config_t channelConfigG = {0};
    channelConfigG.gpio_num = 25;                      // GPIO25
    channelConfigG.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfigG.channel = LEDC_CHANNEL_1;
    channelConfigG.intr_type = LEDC_INTR_DISABLE;
    channelConfigG.timer_sel = LEDC_TIMER_0;
    channelConfigG.duty = 0;

    ledc_channel_config_t channelConfigB = {0};
    channelConfigB.gpio_num = 26;                     // GPIO26
    channelConfigB.speed_mode = LEDC_HIGH_SPEED_MODE;
    channelConfigB.channel = LEDC_CHANNEL_2;
    channelConfigB.intr_type = LEDC_INTR_DISABLE;
    channelConfigB.timer_sel = LEDC_TIMER_0;
    channelConfigB.duty = 0;


    // Configuracion de Timer: esp_err_t ledc_timer_config
    
    ledc_timer_config_t timerConfig = {0};
    timerConfig.speed_mode = LEDC_HIGH_SPEED_MODE;      // Mismo modo que canales
    timerConfig.duty_resolution = LEDC_TIMER_10_BIT;   // 10bit 0 a 1023 niveles de brillo
    timerConfig.timer_num = LEDC_TIMER_0;             // TIMER0
    timerConfig.freq_hz = 20000;                     // 20 kHz

    ledc_timer_config(&timerConfig);             // Aplica configuracion de timer

    ledc_channel_config(&channelConfigR);       // Apica config. canalR
    ledc_channel_config(&channelConfigG);      // Apica config. canalG
    ledc_channel_config(&channelConfigB);      // Apica config. canalB

    return ESP_OK;
}

esp_err_t set_pwm_duty(void){

    /* esp_err_t ledc_set_duty(
                                ledc_mode_t speed_mode,
                                ledc_channel_t channel,
                                uint32_t duty);
    */
    
    // Prepara duty antes de aplicarlo
    ledc_set_duty(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_0,dutyR);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_1,dutyG);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE,LEDC_CHANNEL_2,dutyB);

    // Aplica valor preparado. Se hace de esta forma para cambiar varios parametros juntos.
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_2);

    return ESP_OK;
}












