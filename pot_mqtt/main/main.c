#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"     // driver de lectura del ADC
#include "esp_adc/adc_cali.h"        // API genérica de calibración
#include "esp_adc/adc_cali_scheme.h" // Esquema concreto "line fitting"

#include "mqtt_client.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"


/*
    In VSCode: Ctrl+Shift+P: ESP-IDF: Set Espressif Device Target

*/

#define POT_UNIT    ADC_UNIT_1       // Utilizamos unidad 1 de ADC, la 2 comparte con WiFi
#define POT_CHANNEL ADC_CHANNEL_6    // GPIO34 en ESP32 classic
#define POT_ATTEN   ADC_ATTEN_DB_12  // Atenuación. Rango de entrada aprox. 0 - 3.1 V. DB_12
// mayor ATTEN implica mayor rango pero menor resolución
#define WIFI_SSID  "MY_NETWORKNAME"
#define WIFI_PASS  "MY_PASS"
#define BROKER_URI "mqtt://MY_IP" //IP de PC (ipconfig)
#define TOPIC      "esp32/pot"


static const char *TAG = "POT"; // Etiqueta de texto que se verá al imprimir los valores.

// WIFI
static EventGroupHandle_t wifi_events;
#define WIFI_OK BIT0

static void wifi_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "WiFi desconectado, reintentando...");
        xEventGroupClearBits(wifi_events, WIFI_OK);
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "IP obtenida: " IPSTR, IP2STR(&e->ip_info.ip));
        xEventGroupSetBits(wifi_events, WIFI_OK);
    }
}

static void wifi_init(void)
{
    esp_err_t r = nvs_flash_init();
    if (r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        r = nvs_flash_init();
    }
    ESP_ERROR_CHECK(r);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_events = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_handler, NULL));

    wifi_config_t wc = { 0 };
    strncpy((char *)wc.sta.ssid, WIFI_SSID, sizeof(wc.sta.ssid));
    strncpy((char *)wc.sta.password, WIFI_PASS, sizeof(wc.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Conectando a %s...", WIFI_SSID);
    xEventGroupWaitBits(wifi_events, WIFI_OK, pdFALSE, pdTRUE, portMAX_DELAY);
}

static void mqtt_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (id == MQTT_EVENT_CONNECTED)         ESP_LOGI(TAG, "MQTT conectado");
    else if (id == MQTT_EVENT_DISCONNECTED) ESP_LOGW(TAG, "MQTT desconectado");
}



// APP_MAIN
void app_main(void)
{
    // WIFI INIT

    wifi_init();

    esp_mqtt_client_config_t mcfg = { .broker.address.uri = BROKER_URI };
    esp_mqtt_client_handle_t mqtt = esp_mqtt_client_init(&mcfg);
    esp_mqtt_client_register_event(mqtt, ESP_EVENT_ANY_ID, mqtt_handler, NULL);
    esp_mqtt_client_start(mqtt);



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

    int raw = 0, mv = 0;
    char json[48];         // WIFI

    while(1){

        ESP_ERROR_CHECK(adc_oneshot_read(adc, POT_CHANNEL, &raw)); // dispara lectura y conversión
        if(calibrado){
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, raw, &mv));
            ESP_LOGI(TAG, "raw: %4d | Voltaje: %4d mV",raw,mv);
        } else{
            ESP_LOGI(TAG, "raw: %4d", raw);
        }

        // dentro del while, después de leer:
        
        snprintf(json, sizeof json, "{\"raw\":%d,\"mv\":%d}", raw, mv);
        esp_mqtt_client_publish(mqtt, TOPIC, json, 0, 0, 0);
        ESP_LOGI(TAG, "Publicado: %s", json);




        vTaskDelay(pdMS_TO_TICKS(500));
    }



} // end app_main
