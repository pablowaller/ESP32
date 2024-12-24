#include <HTTPClient.h>
#include <WebServer.h>
#include <WiFi.h>
#include <TimeLib.h>
#include <esp32cam.h>
#define TOUCH_PIN 13

const char* WIFI_SSID = "Luna 2.4";
const char* WIFI_PASS = "Grecia2607";

const char* serverUrl = "http://192.168.0.145:5000/upload";

WebServer server(80);

static auto hiRes = esp32cam::Resolution::find(800, 600);

unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 500;

void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}

void handleJpg() {
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

void sendImage() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("Captura fallida.");
    return;
  }

  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "image/jpeg");

  int httpResponseCode = http.POST((uint8_t*)frame->data(), frame->size());
  
  if (httpResponseCode > 0) {
    Serial.printf("Respuesta del servidor: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error al enviar la imagen: %s\n", http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT_PULLDOWN); 
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam.jpg");

  server.on("/cam.jpg", handleJpg);
  
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.google.com");

  server.begin();
}

void loop() {
  server.handleClient();
  int touchState = digitalRead(TOUCH_PIN);

  if (touchState == HIGH && (millis() - lastTouchTime) > debounceDelay) {
    Serial.println("Sensor activado. Enviando imagen al servidor...");
    sendImage();  // Enviar imagen al servidor remoto
    lastTouchTime = millis();  // Actualizar el tiempo del último toque
  } else if (touchState == LOW && (millis() - lastTouchTime) > debounceDelay) {
    Serial.println("Sensor inactivo.");
    lastTouchTime = millis();  // Actualizar el tiempo cuando el sensor vuelve a inactivarse
  }

  delay(200);
}