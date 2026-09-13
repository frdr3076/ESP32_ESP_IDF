// Proyecto basado en: https://randomnerdtutorials.com/esp32-firebase-realtime-database/
// Adaptado por Mg.ing Edison V, www.editronikx.com.co, 2024

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Insertar credenciales de red
#define WIFI_SSID "WiFi_SSID"

#define WIFI_PASSWORD "Password"

// Proporcionar la informacion del proceso de generacion del token.
#include "addons/TokenHelper.h"

// Proporcionar la informacion de impresion de la carga util de RTDB y otras funciones de ayuda.
#include "addons/RTDBHelper.h"

// Definir el pin digital del LED integrado
#define LED 2

// Insertar la API Key del proyecto Firebase
#define API_KEY "API_KEY_here"

// Insertar la URL de la base de datos RTDB
#define DATABASE_URL "https://esp32-proyecto-basico-default-rtdb.firebaseio.com/"

// Definir objeto de datos de Firebase

FirebaseData fbdo;

// Definir autenticacion de Firebase
FirebaseAuth auth;

// Definir configuracion de Firebase
FirebaseConfig config;

// Variable millis para enviar/almacenar datos en la base de datos Firebase
unsigned long sendDataPrevMillis = 0;
const long timerDelay = 5000; // Enviar/almacenar datos en la base de datos cada 5 segundos

// Variable booleana para el estado de registro
bool signupOK = false;
const int Pinadc = 34;

void setup() {

  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(Pinadc, INPUT);

  // Conectar a la red WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("-------Conectando a wifi------");
  Serial.print("Conectando a: ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(LED, HIGH);
    delay(250);
    digitalWrite(LED, LOW);
    delay(250);
  }

  digitalWrite(LED, LOW);
  Serial.println();
 // Verificar si la conexión fue exitosa
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se pudo conectar al WiFi.");
  } else {
    Serial.println(" Conectado a WiFi. Dirección IP: ");
    Serial.println(WIFI_SSID);
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());  // Imprimir la dirección IP obtenida
      Serial.println("---------------");
    
    // Agrega esto en tu setup() después de conectarte al WiFi
    // gmtOffset_sec = -3*3600 -> hora de Argentina (UTC-3, sin horario de verano)
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    // Espera a que la hora se sincronice
    Serial.print("Sincronizando hora");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(500);
      Serial.print(".");
      now = time(nullptr);
    }
    Serial.println(" ¡Hora sincronizada!");


  }

  // Asignar la API Key (requerido)
  config.api_key = API_KEY;

  // Asignar la URL de la base de datos RTDB (requerido)

  config.database_url = DATABASE_URL;
  
  
  // Nota: se eliminó "Firebase.setPrerequine(&config)" y las líneas de
  // config.cert.* : esa función no existe en la librería Firebase-ESP-Client
  // (mobizt), probablemente venía de una sugerencia mal aplicada y podía
  // impedir la compilación o dejar el certificado en un estado inválido.
  fbdo.setResponseSize(4096);

  // Registrarse en Firebase
  Serial.println();
  Serial.println("--Registrandose ----");
  Serial.print("Nuevo usuario... ");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Serial.println("---------------");

  // Asignar la funcion de callback para el estado del token
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

}



void loop() {
  // Enviar datos a Firebase si esta listo, registrado y ha pasado el tiempo especificado
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    int bits = analogRead(Pinadc);  // Leer el valor analogico (0-4096)

    // Convertir el valor ADC (0-4096) a un valor de voltaje para enviar
    float voltaje = (bits / 4096.0) * 3.3;  // Asumiendo un voltaje de referencia de 3.3V

    Serial.println();
    Serial.println("-------Bits enteros:------");
    Serial.print("Bits enteros : ");
    Serial.println(bits);


    Serial.println();
    Serial.println("-------Voltaje:------");
    Serial.print("Voltaje : ");
    Serial.println(voltaje);

    // Obtener la hora actual (ya sincronizada por NTP en el setup)
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char horaStr[9];  // HH:MM:SS
    strftime(horaStr, sizeof(horaStr), "%H:%M:%S", &timeinfo);

    char fechaHoraStr[20];  // YYYY-MM-DD HH:MM:SS
    strftime(fechaHoraStr, sizeof(fechaHoraStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    Serial.println();
    Serial.println("-------Hora:------");
    Serial.println(fechaHoraStr);

    // Armar un unico payload (objeto JSON) con voltaje, bits y hora
    FirebaseJson json;
    json.set("voltaje", voltaje);
    json.set("bits_enteros", bits);
    json.set("epoch", (int)now);          // timestamp unix, util para ordenar/graficar
    json.set("fecha_hora", fechaHoraStr); // string legible

    // Almacenar datos en la base de datos Firebase
    Serial.println();
    Serial.println("---------------Almacenando Datos");
    digitalWrite(LED, HIGH);

    // pushJSON crea un nuevo nodo con ID unico en /lecturas por cada envio,
    // asi queda un historial completo en vez de sobrescribir el mismo valor
    if (Firebase.RTDB.pushJSON(&fbdo, "/lecturas", &json)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    digitalWrite(LED, LOW);
    Serial.println("---------------");
  }
}