#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// 🔥 Credenciales de Firebase
#define FIREBASE_HOST "https://sense-bell-default-rtdb.firebaseio.com/"  // URL de la base de datos
#define FIREBASE_AUTH "AIzaSyDQJ-amic1aPwLp1B-XyctBgcMRd6ogYwM"  // 🔑 Clave de autenticación de Firebase

// 📡 Credenciales WiFi
#define WIFI_SSID "Luna 2.4"
#define WIFI_PASSWORD "Grecia2607"

// 🔥 Objetos de Firebase
FirebaseConfig config;
FirebaseAuth auth;
FirebaseData firebaseData;

// ⚡ Configuración del motor
#define MOTOR_PIN 14  // D5 en la NodeMCU

void setup() {
    Serial.begin(115200);
    pinMode(MOTOR_PIN, OUTPUT); // Configurar el pin como salida

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

void onNotification(StreamData data) {
    Serial.println("🔔 Notificación recibida:");
    Serial.println(data.jsonString());  // Imprime el JSON completo en Serial Monitor

    // Crear un objeto FirebaseJsonData para almacenar el valor
    FirebaseJsonData jsonData;
    
    // Obtener el JSON del stream
    FirebaseJson &json = data.jsonObject();

    // Buscar la clave "message"
    if (json.get(jsonData, "message")) {  
        if (jsonData.type == "string") {  // Asegurar que el dato es un string
            String message = jsonData.stringValue;
            Serial.println("📩 Mensaje: " + message);
            
            // Si hay un mensaje, activa el motor
            vibrarMotor();
        }
    }
}
// 🔄 En caso de que la conexión se pierda, intenta reconectarse
void onStreamTimeout(bool timeout) {
    if (timeout) {
        Serial.println("⏳ Stream de Firebase perdido, reconectando...");
        Firebase.beginStream(firebaseData, "/notifications");
    }
}

// ⚡ Función para hacer vibrar el motor
void vibrarMotor() {
    digitalWrite(MOTOR_PIN, HIGH);  // Encender motor
    delay(1000);                    // Esperar 1 segundo
    digitalWrite(MOTOR_PIN, LOW);   // Apagar motor
    delay(500);                     // Esperar 500ms
}
