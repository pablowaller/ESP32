#include <ESP8266WiFi.h>

// Configuración de red WiFi
const char* ssid = "Luna 2.4";       // Nombre de tu red WiFi
const char* password = "Grecia2607"; // Contraseña de tu red WiFi

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    intentos++;
    if (intentos > 20) { // Si no se conecta después de 10 segundos
      Serial.println("\nNo se pudo conectar al WiFi. Reiniciando...");
      ESP.restart(); // Reinicia el ESP8266
    }
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // No es necesario hacer nada en el loop para esta prueba
}