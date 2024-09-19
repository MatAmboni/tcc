#include <WiFi.h>

const char *ssid = "ESP32-1";  // Mesmo nome da rede P2P
const char *password = "password";
const char *serverIP = "192.168.4.1";  // IP do servidor (ESP32_1)
const int serverPort = 8888;

WiFiClient client;

void setup() {
  Serial.begin(115200);

  // Conecta-se à rede P2P
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando ao WiFi...");
  }

  Serial.println("Conectado ao WiFi");

  // Conecta-se ao servidor
  client.connect(serverIP, serverPort);
}

void loop() {
  if (client.connected()) {
    // Armazena o tempo de recebimento da mensagem de handshake
    unsigned long startTime = millis();

    // Aguarda a mensagem de handshake do servidor
    while (client.available() == 0) {
      delay(10);
    }

    String handshake = client.readStringUntil('\r');
    Serial.println("Mensagem do Servidor: " + handshake);

    // Responde ao handshake
    client.print("Resposta ao Handshake");

    Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db");

    delay(5000);  // Aguarda 5 segundos antes de enviar o próximo handshake
  } else {
    Serial.println("Conexão perdida. Reconectando...");
    client.connect(serverIP, serverPort);
  }
}
