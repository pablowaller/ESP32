#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <FirebaseESP32.h>

// Configura tu red WiFi
const char* ssid = "Luna 2.4";
const char* password = "Grecia2607";

// Inicializa el servidor web
WebServer server(80);

#define FLASH_LED_PIN 4  // El LED del flash está en el GPIO 4

#define API_KEY "lZ5hOsyDNVMex6IibzuiLZEToIsFeOC70ths5los" 
#define DATABASE_URL "https://sense-bell-default-rtdb.firebaseio.com/"  

// Variables para Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void checkFlashStatus();

// Configura los pines de la ESP32-CAM (Ai-Thinker)
void configCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1; // No se usa reset por defecto
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Calidad de imagen y resolución
  config.frame_size = FRAMESIZE_QVGA; // Cambia a FRAMESIZE_UXGA para 1600x1200
  config.jpeg_quality = 12;          // Rango: 0-63 (0 = mayor calidad)
  config.fb_count = 1;

  // Inicializa la cámara
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Error al inicializar la cámara");
    while (true);
  }
}

// Flujo de video MJPEG con LED flash activado
void handleStream() {
  WiFiClient client = server.client();
  camera_fb_t *fb = NULL;
  String boundary = "frame";
  String response = "HTTP/1.1 200 OK\r\n" +
                    String("Content-Type: multipart/x-mixed-replace; boundary=") + boundary + "\r\n\r\n";
  client.write(response.c_str(), response.length());

  while (client.connected()) {
    // 🔦 ENCIENDE EL FLASH ANTES DE CAPTURAR
    digitalWrite(FLASH_LED_PIN, HIGH);
    delay(100);  // Aumentado a 100ms para mejor iluminación antes de capturar

    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error al capturar imagen");
      continue;
    }

    // 🔦 APAGA EL FLASH DESPUÉS DE CAPTURAR
    digitalWrite(FLASH_LED_PIN, LOW);

    // Encabezado para cada frame
    String part = "--" + boundary + "\r\n" +
                  "Content-Type: image/jpeg\r\n" +
                  "Content-Length: " + String(fb->len) + "\r\n\r\n";
    client.write(part.c_str(), part.length());
    client.write(fb->buf, fb->len);
    client.write("\r\n");

    esp_camera_fb_return(fb);
  }
}

// Configuración inicial
void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(240);  // Aumentado a 240 MHz para mejor rendimiento

  // Configura el LED del flash como salida
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW); // Asegurar que inicie apagado

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  // Configura la cámara
  configCamera();

  // Ruta del flujo de video
  server.on("/stream", HTTP_GET, handleStream);

  // Inicia el servidor
  server.begin();
  Serial.println("Servidor iniciado");

  // Configurar Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, NULL);
  Firebase.reconnectWiFi(true);

  // Escuchar cambios en "/flash"
  Firebase.setStreamCallback(fbdo, streamCallback, streamTimeoutCallback);
  if (!Firebase.beginStream(fbdo, "/flash")) {
    Serial.println("Error al iniciar stream: " + fbdo.errorReason());
  }
}

void streamCallback(StreamData data) {
  if (data.dataType() == "boolean") {  // Solo procesar si es un booleano
    bool flashState = data.boolData();
    digitalWrite(FLASH_LED_PIN, flashState ? HIGH : LOW);
    Serial.println(flashState ? "🔦 Flash ON" : "💡 Flash OFF");
  }
}

// 🔹 CALLBACK: Manejo de errores de Firebase
void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Firebase Timeout, reconectando...");
    Firebase.beginStream(fbdo, "/flash");
  }
}

void loop() {
  server.handleClient();
  
  // Reintentar conexión WiFi en caso de desconexión
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi perdido, reconectando...");
    WiFi.disconnect();
    WiFi.reconnect();
    delay(5000); // Espera antes de reintentar para evitar bucles rápidos
  }

  if (Firebase.ready()) {
    Firebase.readStream(fbdo);
  }
}
