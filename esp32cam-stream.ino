#include "esp_camera.h"
#include <WiFi.h>

const char* ssid = "Luna 2.4";
const char* password = "Grecia2607";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conectado a WiFi");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL;
  config.ledc_timer = LEDC_TIMER;
  config.pin_d0 = 0;
  config.pin_d1 = 1;
  config.pin_d2 = 2;
  config.pin_d3 = 3;
  config.pin_d4 = 4;
  config.pin_d5 = 5;
  config.pin_d6 = 6;
  config.pin_d7 = 7;
  config.pin_xclk = 8;
  config.pin_pclk = 9;
  config.pin_vsync = 10;
  config.pin_href = 11;
  config.pin_sscb_sda = 12;
  config.pin_sscb_scl = 13;
  config.pin_pwdn = 14;
  config.pin_reset = 15;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Inicializar la cámara
  esp_camera_init(&config);

  // Iniciar el servidor web para el flujo MJPEG
  server.on("/video", HTTP_GET, []() {
    WiFiClient client = server.client();
    camera_fb_t *fb = NULL;

    while (client.connected()) {
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Error al capturar imagen");
        return;
      }
      client.write(fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
