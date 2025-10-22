#include <Arduino.h>
#include <SoftwareSerial.h>

// Definições de pinos e constantes
#define pinAnalog 35
#define RELAY1 19
#define RELAY2 21
#define FIM_ABRE 22
#define FIM_FECHA 23
#define SENSOR 25
#define ph_pin 32

const float calibrationFactor = 4.5;
const float calibration_value = 11.5;
const int interval = 1000;

volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
long currentMillis = 0;
long previousMillis = 0;

float AnalogOutput = 0;
float ph_act;
bool abrir = false;

SoftwareSerial loraSerial(4, 5); // TX, RX

// Prototipação de funções
void abrirComporta();
void fecharComporta();
void pararMotor();
void processMessage(String msg);
void IRAM_ATTR pulseCounter();

void setup() {
  Serial.begin(115200);
  loraSerial.begin(9600);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(FIM_ABRE, INPUT_PULLUP);
  pinMode(FIM_FECHA, INPUT_PULLUP);
  pinMode(SENSOR, INPUT_PULLUP);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;

  delay(1000);
}

void loop() {
  avgval = 0;
  float voltage = 0.0;

  // Ler 10 amostras e guardar no buffer
  for (int i = 0; i < 10; i++) {
    buffer_arr[i] = analogRead(ph_pin);
    delay(30);
  }

  // Ordenar os valores (para reduzir ruído)
  for (int i = 0; i < 9; i++) {
    for (int j = i + 1; j < 10; j++) {
      if (buffer_arr[i] > buffer_arr[j]) {
        temp = buffer_arr[i];
        buffer_arr[i] = buffer_arr[j];
        buffer_arr[j] = temp;
      }
    }
  }

  // Média dos valores centrais (para reduzir outliers)
  for (int i = 2; i < 8; i++)
    avgval += buffer_arr[i];

  // Converter média em tensão
  float volts0 = (float)avgval / 6.0 * (3.3 / 4095.0);

  // Calcular o pH
  ph_act = -5.70 * volts0 + calibration_value;

  // Calcular fluxo
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    pulse1Sec = pulseCount;
    pulseCount = 0;
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();
    flowMilliLitres = (flowRate / 60) * 1000;
    totalMilliLitres += flowMilliLitres;
  }

  // Ler saída analógica
  AnalogOutput = analogRead(pinAnalog);
  float analogVoltage = AnalogOutput * (5.0 / 1023.0);

  // Enviar dados via LoRa
  String loraMessage = String("{\"ph\":") + ph_act +
                       ",\"flowRate\":" + flowRate +
                       ",\"totalMilliLitres\":" + totalMilliLitres +
                       ",\"analogVoltage\":" + analogVoltage +
                       ",\"gateState\":\"" + (abrir ? "open" : "closed") + "\"}";
  loraSerial.println(loraMessage);

  // Receber comandos via LoRa
  if (loraSerial.available()) {
    String message = loraSerial.readStringUntil('\n');
    message.trim();
    processMessage(message);
  }

  delay(1000);
}

// Função para abrir a comporta
void abrirComporta() {
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, LOW);
  while (digitalRead(FIM_ABRE) == HIGH) {
    delay(50);
  }
  pararMotor();
  abrir = true;
}

// Função para fechar a comporta
void fecharComporta() {
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, HIGH);
  while (digitalRead(FIM_FECHA) == HIGH) {
    delay(50);
  }
  pararMotor();
  abrir = false;
}

void pararMotor() {
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);
}

void processMessage(String msg) {
  if (msg == "OPEN_GATE") {
    abrirComporta();
  } else if (msg == "CLOSE_GATE") {
    fecharComporta();
  }
}

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}