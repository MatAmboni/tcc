#include <WiFi.h>

const char *ssid = "ESP32-1";  // Nome da rede P2P
const char *password = "password";
WiFiServer server(8888);

uint32_t startTime = 0;
uint32_t rtt = 0;

void setup() {
  Serial.begin(115200);

  // Configuração do modo P2P
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println("Servidor iniciado");

  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("Cliente conectado");

    // Envia mensagem de handshake
    client.print("Handshake");

    // Armazena o tempo de envio
    startTime = millis();

    // Aguarda a resposta do cliente
    while (client.connected()) {
      if (client.available()) {
        String response = client.readStringUntil('\r');
        Serial.println("Resposta do Cliente: " + response);

        // Calcula o RTT
        uint32_t endTime = millis();
        rtt = endTime - startTime;
        Serial.println("Round Trip Time = " + rtt);

        break;
      }
      delay(10);
    }

    client.stop();
    Serial.println("Cliente desconectado");
  }

  delay(1000);
}
