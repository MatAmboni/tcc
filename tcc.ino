#include <Arduino.h>
#include <SoftwareSerial.h>
#define pinAnalog 35

float AnalogOutput = 0;
float voltage = 0;


#define RELAY1 19
#define RELAY2 21
#define FIM_ABRE 22
#define FIM_FECHA 23
#define SENSOR  25
long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

SoftwareSerial loraSerial(4, 5); // TX, RX

const int ph_pin = 32;       // Pino analógico onde o sensor está conectado
float calibration_value = 11.5; // Ajuste conforme sua calibração
int buffer_arr[10];
int temp;
unsigned long int avgval;
float ph_act;
bool abrir = false;  // variável booleana que decide abrir ou fechar

void setup() {
  Serial.begin(115200);
  loraSerial.begin(9600);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(FIM_ABRE, INPUT_PULLUP);
  pinMode(FIM_FECHA, INPUT_PULLUP);
  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

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

  // Exibir os resultados
  Serial.print("Tensão: ");
  Serial.print(volts0, 3);
  Serial.print(" V\t");

  Serial.print("pH: ");
  Serial.println(ph_act, 2);

  if (abrir) {
    abrirComporta();
    abrir = false; // reseta o comando
  } else {
    fecharComporta();
    abrir = true; // alterna para teste
  }

  if (loraSerial.available()) {
    String message = loraSerial.readStringUntil('\n');
    message.trim();
    Serial.println("Recebido: " + message);

    // Processa a mensagem
    processMessage(message);
    // readSensors(); //precisa implementar
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > interval) {
    
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;
    previousMillis = millis();

    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
  }

  AnalogOutput = analogRead(pinAnalog);
  float voltage = AnalogOutput * (5.0 / 1023.0);
  Serial.println("Saida analog");
  Serial.println(voltage); 

  delay(1000);
}

// Função para abrir a comporta
void abrirComporta() {
    Serial.println("Abrindo comporta...");
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, LOW);
  
    // Espera até bater o fim de curso de abertura
    while (digitalRead(FIM_ABRE) == HIGH) {
      delay(50);
    }
  
    pararMotor();
    Serial.println("Comporta aberta.");
  }
  
  // Função para fechar a comporta
  void fecharComporta() {
    Serial.println("Fechando comporta...");
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, HIGH);
  
    // Espera até bater o fim de curso de fechamento
    while (digitalRead(FIM_FECHA) == HIGH) {
      delay(50);
    }
  
    pararMotor();
    Serial.println("Comporta fechada.");
  }
  
  void pararMotor() {
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
  }
  
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

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}