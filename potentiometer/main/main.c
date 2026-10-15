#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"     // driver de lectura del ADC
#include "esp_adc/adc_cali.h"        // API genérica de calibración
#include "esp_adc/adc_cali_scheme.h" // Esquema concreto "line fitting"

/*
    In VSCode: Ctrl+Shift+P: ESP-IDF: Set Espressif Device Target

*/


#define POT_UNIT    ADC_UNIT_1       // Utilizamos unidad 1 de ADC, la 2 comparte con WiFi
#define POT_CHANNEL ADC_CHANNEL_6    // GPIO34 en ESP32 classic
#define POT_ATTEN   ADC_ATTEN_DB_12  // Atenuación. Rango de entrada aprox. 0 - 3.1 V. DB_12
// mayor ATTEN implica mayor rango pero menor resolución

static const char *TAG = "POT"; // Etiqueta de texto que se verá al imprimir los valores.


void app_main(void)
{
    // 1) Inicializar la unidad ADC1

    adc_oneshot_unit_handle_t adc; // puntero al objeto interno del driver
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = POT_UNIT }; // estructura
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc));

    // 2) Configurar el canal

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = POT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT, // Resolucion máxima 12 bits (0 a 4095) GPIO

    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, POT_CHANNEL, &chan_cfg));

    /* 3) Calibracion (por "line fitting")
       El ADC del ESP32 no es lineal y varia de un chip a otro, usa valores medidos en fabricación.
       Ejecutará el esquema "line fitting" si lo soporta.
    */
    
    adc_cali_handle_t cali = NULL; 
    bool calibrado = false;

    #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_line_fitting_config_t cali_cfg = {
            .unit_id = POT_UNIT,
            .atten = POT_ATTEN,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        calibrado = (adc_cali_create_scheme_line_fitting(&cali_cfg, &cali) == ESP_OK); // guarda ESP_OK
    #endif

    int crudo = 0, mv = 0;

    while(1){

        ESP_ERROR_CHECK(adc_oneshot_read(adc, POT_CHANNEL, &crudo)); // dispara lectura y conversión
        if(calibrado){
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, crudo, &mv));
            ESP_LOGI(TAG, "Crudo: %4d | Voltaje: %4d mV",crudo,mv);
        } else{
            ESP_LOGI(TAG, "Crudo: %4d", crudo);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }



} // end app_main
