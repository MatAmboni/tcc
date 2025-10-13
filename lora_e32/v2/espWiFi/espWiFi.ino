
/***************************
 * 
 * 
 * 
 *************************/ 



#include <WiFi.h>
#include <SoftwareSerial.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "iPhone de Matheus"
#define WIFI_PASSWORD "amboni123"
#define API_KEY "AIzaSyCpHO-Ia8VsKD_Kbo4vKttMhnnqFFRcOek"
#define DATABASE_URL "https://tccamboni-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

SoftwareSerial loraSerial(4, 5); // TX, RX

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

/*************************** 
 *  COMENTARIOS CODIGO
 *************************/

void setup() {
  Serial.begin(9600);
  loraSerial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi...");
  while (WiFi.waitForConnectResult() != WL_CONNECTED);
  Serial.println(" [OK]");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase OK");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Inicia stream para observar "isGateOpen"
  if (!Firebase.RTDB.beginStream(&stream, "markers/marker_0/isGateOpen")) {
    Serial.printf("[master] Erro no stream: %s\n", stream.errorReason().c_str());
  } else {
    Serial.println("[master] Stream iniciado em marker_0/isGateOpen");
  }
}

void loop() {
  // --- RECEBER DADOS DO LoRa E ENVIAR PARA FIREBASE ---
  if (loraSerial.available()) {
    String input = loraSerial.readStringUntil('\n');
    input.trim();
    Serial.println("Recebido LoRa: " + input);

    // Extrair valores (formato JSON simples)
    int humidity = getValue(input, "humidity").toInt();
    int waterLevel = getValue(input, "waterLevel").toInt();
    bool isGateOpen = getValue(input, "isGateOpen") == "1";

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

      Firebase.RTDB.setInt(&fbdo, "markers/marker_0/humidity", humidity);
      Firebase.RTDB.setInt(&fbdo, "markers/marker_0/waterlevel", waterLevel);
      Firebase.RTDB.setBool(&fbdo, "markers/marker_0/isGateOpen", isGateOpen);

      Serial.println("Firebase atualizado");
    }
  }

  // --- ESCUTAR ALTERAÇÕES NO BANCO ---
  if (!Firebase.RTDB.readStream(&stream)) {
    Serial.printf("Stream falhou: %s\n", stream.errorReason().c_str());
  }

  if (stream.streamAvailable()) {
    if (stream.dataTypeEnum() == fb_esp_rtdb_data_type_boolean) {
      bool gateState = stream.boolData();
      Serial.printf("isGateOpen mudou: %s\n", gateState ? "true" : "false");

      if (gateState) {
        loraSerial.println("OPEN_GATE");  // manda comando para abrir
      } else {
        loraSerial.println("CLOSE_GATE"); // manda comando para fechar
      }
    }
  }
}

/* Função para extrair valores do JSON simples */
String getValue(String data, String key) {
  int keyIndex = data.indexOf("\"" + key + "\":");
  if (keyIndex == -1) return "";
  int start = data.indexOf(":", keyIndex) + 1;
  int end = data.indexOf(",", start);
  if (end == -1) end = data.indexOf("}", start);
  return data.substring(start, end);
}
