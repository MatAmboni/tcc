#include <SoftwareSerial.h>

SoftwareSerial loraSerial(4, 5); // TX, RX

// Defina o pino do motor/servo/relé que vai controlar a comporta
//#define GATE_PIN 8  

void setup() {
  Serial.begin(9600);
  loraSerial.begin(9600);

//  pinMode(GATE_PIN, OUTPUT);
//  digitalWrite(GATE_PIN, LOW); // começa fechado
}

void loop() {
  Serial.println("Inicio");
  delay(5000);
  if (loraSerial.available()) {
    String message = loraSerial.readStringUntil('\n');
    message.trim();
    Serial.println("Recebido: " + message);

    // Processa a mensagem
    processMessage(message);
    // readSensors(); //precisa implementar
  }
}

// -------------------------
// Função para processar comandos
// -------------------------
void processMessage(String msg) {
  String key = "\"isGateOpen\":";
  int idx = msg.indexOf(key);
  if (idx != -1) {
    int start = idx + key.length();
    // pula espaços
    while (start < msg.length() && (msg.charAt(start) == ' ' || msg.charAt(start) == '\t')) start++;
    char c = msg.charAt(start);
    if (c == '0') openGate();    // ajusta conforme sua lógica
    else if (c == '1') closeGate();
    else Serial.println("Valor inválido para isGateOpen");
  } else {
    Serial.println("isGateOpen não encontrado (fallback).");
    // fallback para comandos planos:
    if (msg == "OPEN_GATE") openGate();
    else if (msg == "CLOSE_GATE") closeGate();
    else Serial.println("Mensagem desconhecida");
  }
}


// -------------------------
// Funções da comporta
// -------------------------
void openGate() {
  Serial.println(">> Abrindo comporta...");
//  digitalWrite(GATE_PIN, HIGH); // exemplo: ativa relé/motor
}

void closeGate() {
  Serial.println(">> Fechando comporta...");
//  digitalWrite(GATE_PIN, LOW);  // exemplo: desativa relé/motor
}

void readSensors() {
  //readsensor
  if (loraSerial.available()) {
    //Serial.println(humiditysensors);
    //Serial.println(temperaturesensors);
  }
}
