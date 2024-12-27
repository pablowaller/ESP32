#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Configura tu red WiFi
const char* ssid = "Luna 2.4";
const char* password = "Grecia2607";

// Inicializa el servidor web
WebServer server(80);

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

// Flujo de video MJPEG
void handleStream() {
  WiFiClient client = server.client();
  camera_fb_t *fb = NULL;
  String boundary = "frame";
  String response = "HTTP/1.1 200 OK\r\n" +
                    String("Content-Type: multipart/x-mixed-replace; boundary=") + boundary + "\r\n\r\n";
  client.write(response.c_str(), response.length());

  while (client.connected()) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error al capturar imagen");
      continue;
    }

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
}

void loop() {
  server.handleClient();
}
