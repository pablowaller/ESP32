#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// 🔥 Credenciales de Firebase
#define FIREBASE_HOST "https://sense-bell-default-rtdb.firebaseio.com/"  // URL de la base de datos
#define FIREBASE_AUTH "lZ5hOsyDNVMex6IibzuiLZEToIsFeOC70ths5los"  // 🔑 Clave de autenticación de Firebase

// 📡 Credenciales WiFi
#define WIFI_SSID "Luna 2.4"
#define WIFI_PASSWORD "Grecia2607"

// 🔥 Objetos de Firebase
FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);

  // Conectar a WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado a WiFi");

  // Configurar Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Suscribirse a cambios en "/notifications"
  if (Firebase.beginStream(firebaseData, "/notifications")) {
    Serial.println("✅ Suscripción exitosa a /notifications");
    Firebase.setStreamCallback(firebaseData, onNotification, onStreamTimeout);
  } else {
    Serial.println("❌ Error al suscribirse a /notifications");
    Serial.println(firebaseData.errorReason());
  }
}

void loop() {
  // Mantener la conexión con Firebase activa
}

// 🔔 Esta función se ejecuta cuando hay un cambio en "/notifications"
void onNotification(StreamData data) {
  Serial.println("🔔 Notificación recibida:");
  Serial.println(data.stringData());  // Imprime el mensaje en el Serial Monitor

  // Aquí puedes agregar código para activar un LED, un relay o lo que necesites
}

// 🔄 En caso de que la conexión se pierda, intenta reconectarse
void onStreamTimeout(bool timeout) {
  if (timeout) {
    Serial.println("⏳ Stream de Firebase perdido, reconectando...");
    Firebase.beginStream(firebaseData, "/notifications");
  }
}
