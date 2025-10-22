#include <WiFi.h>
#include <SoftwareSerial.h>
#include <Firebase_ESP_Client.h>

#define WIFI_SSID "iPhone de Matheus"
#define WIFI_PASSWORD "amboni123"
#define API_KEY "AIzaSyCpHO-Ia8VsKD_Kbo4vKttMhnnqFFRcOek"
#define DATABASE_URL "https://tccamboni-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

SoftwareSerial loraSerial(4, 5); // TX, RX

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;
bool isGateOpen = false; // Variável para controlar o estado da comporta

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
}

void loop() {
  if (loraSerial.available() > 1) {
    String input = loraSerial.readString();
    input.trim();

    // Extrair valores recebidos (formato JSON simples)
    float ph = getValue(input, "ph").toFloat();
    float flowRate = getValue(input, "flowRate").toFloat();
    unsigned long totalMilliLitres = getValue(input, "totalMilliLitres").toInt();
    float analogVoltage = getValue(input, "analogVoltage").toFloat();
    String gateState = getValue(input, "gateState");

    // Atualizar o banco de dados com os valores recebidos
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0)) {
      sendDataPrevMillis = millis();

      Firebase.RTDB.setFloat(&fbdo, "sensors/ph", ph);
      Firebase.RTDB.setFloat(&fbdo, "sensors/flowRate", flowRate);
      Firebase.RTDB.setInt(&fbdo, "sensors/totalMilliLitres", totalMilliLitres);
      Firebase.RTDB.setFloat(&fbdo, "sensors/analogVoltage", analogVoltage);
      Firebase.RTDB.setString(&fbdo, "sensors/gateState", gateState);

      Serial.println("Firebase atualizado com dados dos sensores.");
    }
  }

  // Enviar comando para abrir ou fechar a comporta com base na variável isGateOpen
  String command = isGateOpen ? "OPEN_GATE" : "CLOSE_GATE";
  loraSerial.println(command);

  // Atualizar o banco de dados com o estado da comporta
  if (Firebase.ready() && signupOK) {
    Firebase.RTDB.setBool(&fbdo, "control/isGateOpen", isGateOpen);
  }

  delay(1000);
}

/* Função para extrair valores do JSON simples */
String getValue(String data, String key) {
  int keyIndex = data.indexOf("\"" + key + "\":");
  if (keyIndex == -1) return "";
  int start = data.indexOf(":", keyIndex) + 1;
  int end = data.indexOf(",", start);
  if (end == -1) end = data.indexOf("}", start);
  return data.substring(start, end).trim();
}
