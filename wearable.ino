#include <WiFi.h>

// Configuración de red WiFi
const char* ssid = "Luna 2.4";       // Cambia por el nombre de tu red WiFi
const char* password = "Grecia2607";  // Cambia por la contraseña de tu red WiFi

// Pin donde se conecta el motor vibratorio
const int motorPin = 4; // Cambia según tu configuración

void setup() {
  // Configuración de hardware
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW); // Motor apagado inicialmente

  // Configuración de conexión WiFi
  Serial.begin(115200);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Activar motor si se recibe un comando por Serial
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Elimina espacios en blanco

    if (command == "activar") {
      digitalWrite(motorPin, HIGH); // Enciende el motor vibratorio
      delay(500);                   // Mantén la vibración por 500 ms
      digitalWrite(motorPin, LOW);  // Apaga el motor
      Serial.println("Motor activado.");
    }
  }
}
