#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "Luna 2.4"
#define WIFI_PASSWORD "Grecia2607"

#define FIREBASE_HOST "https://sense-bell-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "lZ5hOsyDNVMex6IibzuiLZEToIsFeOC70ths5los"

// 🔹 Instancias de Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// 🔹 Configuración del módulo VC02
#define VC02_TX 14  // TX del VC02 al RX de la ESP32
#define VC02_RX 15  // RX del VC02 al TX de la ESP32

// 🔹 Servidor web
WebServer server(80);

// Configuración de la cámara ESP32-CAM (Ai-Thinker)
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
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    if (esp_camera_init(&config) != ESP_OK) {
        Serial.println("❌ Error al inicializar la cámara");
        while (true);
    }
    Serial.println("✅ Cámara iniciada");
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
            Serial.println("❌ Error al capturar imagen");
            continue;
        }

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
    
    // 🔹 Comunicación con VC02
    Serial1.begin(115200, SERIAL_8N1, VC02_TX, VC02_RX);

    // 🔹 Conexión WiFi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Conectando a WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\n✅ Conectado a WiFi!");
    Serial.print("📡 IP: ");
    Serial.println(WiFi.localIP());

    // 🔹 Inicializar Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // 🔹 Configurar cámara
    configCamera();

    // 🔹 Ruta del streaming
    server.on("/stream", HTTP_GET, handleStream);

    // 🔹 Iniciar servidor web
    server.begin();
    Serial.println("✅ Servidor iniciado en /stream");
}

void loop() {
    server.handleClient();

    // 🔹 Leer transcripción del VC02
    if (Serial1.available()) {
        String audioText = Serial1.readString();
        Serial.println("🎙 Texto transcrito: " + audioText);

        // 🔹 Enviar a Firebase
        if (Firebase.setString(firebaseData, "/transcripciones/audio", audioText)) {
            Serial.println("✅ Texto enviado a Firebase");
        } else {
            Serial.println("❌ Error al enviar: " + firebaseData.errorReason());
        }
    }
}
