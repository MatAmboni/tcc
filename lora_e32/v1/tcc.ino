#include <WiFi.h>
#include <SoftwareSerial.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define WIFI_SSID "eduroam"
#define WIFI_PASSWORD "TeteuAmboni985"
#define API_KEY "AIzaSyBRXV43cCj_6WLfZM4HuVWx8VFfxAIOqmw"
#define DATABASE_URL "https://tcclora-f6c9d-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

SoftwareSerial loraSerial(4, 5);

void setup() {

  Serial.begin(9600);
  loraSerial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao Wi-Fi...");
  while (WiFi.waitForConnectResult() != WL_CONNECTED);
  Serial.println(" [OK]");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("firebase ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  #define RELE_PIN 14 // ou a porta que você estiver usando
  pinMode(RELE_PIN, OUTPUT);
  digitalWrite(RELE_PIN, LOW); // começa desligado

}

void loop() {
  if(loraSerial.available()) {
    char tempChar = loraSerial.read();
    String tempData = String(tempChar);

    if(tempData == "0" || tempData == "1") {
      String loraData = tempData;
      Serial.println(loraData);

      if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
        sendDataPrevMillis = millis();
        if(loraData == "0"){
          Firebase.RTDB.setBool(&fbdo, "temagua", false);
        }
        if(loraData == "1"){
          Firebase.RTDB.setBool(&fbdo, "temagua", true);
        }
        if (Firebase.RTDB.getBool(&fbdo, "comporta")) {
          if (fbdo.dataType() == "boolean") {
            estadoComporta = fbdo.boolData();
            digitalWrite(RELE_PIN, estadoComporta);
            Serial.println(estadoComporta ? "Comporta ABERTA" : "Comporta FECHADA");
          }
        }
      }
    }
  }
}
