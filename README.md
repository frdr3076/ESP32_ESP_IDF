# ESP32 ESP-IDF

Proyectos realizados en ESP32 en ESP-IDF con VSCode.<br>
Algunos ejemplos están realizados en Firebase con archivos .ino <br>

## Pinout ESP32
<img src="images/esp32-doit-devkit-v1.svg" alt="ESP32_Pinout" width="70%">

## Configuracion de proyectos
En VSCode utilizando Ctrl+Shift+P o colocando ">" en linea de comandos superior iniciar un proyecto nuevo con
ESP-IDF:New Proyect

<img src="images/esp-idf_new_project1.jpg" alt="new_project1" width="100%">

<br>
Indicar la version de ESP-IDF a utilizar:<br>
<img src="images/esp-idf_new_project2.jpg" alt="new_project2" width="70%">

<br>
Indicar ubicacion de carpeta y puerto COM:<br>
<img src="images/esp-idf_new_project3.jpg" alt="new_project3" width="70%">
<br>

Al armar un nuevo proyecto si no reconoce los includes (.h) es posible que el proyecto requiera reestructuración de CMake.<br>
En el CMake colocar lo siguiente:<br>

`idf_component_register(SRCS "<proyecto>.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_ledc esp_driver_gpio)`
<br>

## PWM (Pulse Width Modulation)

Es necesario utilizar `#include "driver/ledc.h"` para incorporar las estructuras que hacen funcionar el PWM.<br>
Se utiliza `vTimerCallback` para hacer una acción luego de que finalice el timer.<br>
Funcion `set_pwm` modifica parametros de estructura `ledc_channel_config_t` para configurar el canal y las del timer por medio de `ledc_timer_config_t`. <br>
Función `set_pwm_duty` utiliza funciones de la libreria `ledc.h` para setear el duty cycle con `ledc_set_duty` y actualizarlo con `ledc_update_duty`.
<br>



<br>


## Firebase
<br>
Este ejercicio no fue realizado con ESP-IDF sino con código .ino para enviar lectura de un potenciometro a base de datos Firebase.
<img src="images/ejercicio_potenciometro_firebase.jpg" alt="new_project3" width="70%">
<br>
