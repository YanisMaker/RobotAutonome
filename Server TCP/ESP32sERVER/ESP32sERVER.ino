#include <WiFi.h>

// ===== WIFI =====
const char* WIFI_SSID = "Freebox-61B0E1";
const char* WIFI_PASS = "rhf2zq23hxqkrfmsqb2sb5";

// ===== TCP =====
WiFiServer server(3333);
WiFiClient client;

// ===== UART vers Arduino =====
#define UART_TX_PIN 17
#define UART_RX_PIN 16
#define UART_BAUD   115200

String rxBuffer = "";

void setup() {
  Serial.begin(115200);
  delay(1000);

  // UART Arduino
  Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  // Connexion WiFi
  Serial.println("Connexion WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connecté !");
  Serial.print("IP ESP32 : ");
  Serial.println(WiFi.localIP());

  // Serveur TCP
  server.begin();
  server.setNoDelay(true);
  Serial.println("Serveur TCP démarré (port 3333)");
}

void loop() {
  // Connexion client
  if (!client || !client.connected()) {
    client = server.available();
    if (client) {
      Serial.println("Client TCP connecté");
      rxBuffer = "";
    }
  }

  // Réception données
  if (client && client.connected()) {
    while (client.available()) {
      char c = client.read();

      if (c == '\n') {
        rxBuffer.trim();
        if (rxBuffer.length() > 0) {
          Serial.print("TCP reçu : ");
          Serial.println(rxBuffer);

          // Envoi vers Arduino
          Serial1.println(rxBuffer);
        }
        rxBuffer = "";
      } else {
        rxBuffer += c;
      }
    }
  }
}
