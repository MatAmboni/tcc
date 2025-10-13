/*
  Trabalho de Conclusão de Curso
  Autor: Matheus Costa Amboni
  Universidade Federal de Santa Catarina (UFSC)

  Título: Sistema de gerenciamento hídrico de lavouras de arroz utilizando LoRa

  Descrição geral:
  Este código corresponde ao nó *slave* do sistema, instalado na lavoura. 
  Ele realiza a leitura de sensores (fluxo de água, nível de água e umidade do solo),
  formata os dados em uma string no estilo JSON e transmite via módulo LoRa para o nó *master*,
  responsável pela centralização e tomada de decisão.

  Principais funcionalidades:
  - Medição de vazão de água através do sensor de fluxo YF-S201 (pulsos digitais).
  - Leitura do nível de água por meio de uma boia com contato elétrico (interruptor).
  - Estrutura para futura integração de sensores de umidade do solo e acionamento de relé.
  - Comunicação via LoRa utilizando SoftwareSerial.
*/

// --- Bibliotecas ---
#include <SoftwareSerial.h>  // Biblioteca para comunicação serial em pinos digitais arbitrários

// --- Configuração do LoRa (comunicação serial) ---
SoftwareSerial loraSerial(4, 5); // RX = 4, TX = 5

// --- Definição dos sensores conectados --- 
const int pinFlow  = 18;   // Sensor de fluxo YF-S201 → saída digital de pulsos
const int pinLevel = 2;    // Sensor de nível (boia horizontal tipo interruptor)
//const int pinSoil = A0;  // Sensor de umidade PH-4502C (entrada analógica) [opcional futuro]
//const int pinRelay = 7;  // Relé para acionamento de motor/bomba [opcional futuro]

// --- Variáveis do sensor de fluxo ---
volatile int pulseCount = 0;      // Contador de pulsos (interrupção)
unsigned long oldTime = 0;        // Marca de tempo para cálculo da taxa de fluxo
float flowRate = 0.0;             // Vazão calculada em L/min

// --- Função de interrupção para contar pulsos do sensor de fluxo ---
// Cada pulso do YF-S201 corresponde a uma fração da vazão total
void pulseCounter() {
  pulseCount++;
}

void setup() {
  // Inicializa comunicação serial (debug via USB)
  Serial.begin(9600);

  // Inicializa comunicação LoRa via SoftwareSerial
  loraSerial.begin(9600);

  // Configuração do sensor de nível (boia)
  // INPUT_PULLUP: utiliza resistor de pull-up interno
  // Lógica: HIGH = circuito aberto, LOW = circuito fechado (boia acionada)
  pinMode(pinLevel, INPUT_PULLUP);

  // Configuração do sensor de fluxo
  pinMode(pinFlow, INPUT_PULLUP);
  // Associa função de interrupção ao pino do sensor de fluxo
  attachInterrupt(digitalPinToInterrupt(pinFlow), pulseCounter, RISING);

  // Configuração opcional futura:
  // pinMode(pinSoil, INPUT);
  // pinMode(pinRelay, OUTPUT);
  // digitalWrite(pinRelay, LOW); // motor desligado no início

  Serial.println("Slave iniciado e aguardando medições...");
}

void loop() {
  // --- Cálculo da vazão de água (YF-S201) ---
  // A cada 1 segundo, calcula a vazão com base nos pulsos acumulados
  if (millis() - oldTime > 1000) {
    // Desabilita a interrupção para evitar conflitos durante o cálculo
    detachInterrupt(digitalPinToInterrupt(pinFlow));

    // Fórmula baseada no datasheet:
    // Vazão (L/min) = Pulsos por segundo / 7.5
    flowRate = (pulseCount / 7.5);

    // Reinicia contador de pulsos
    pulseCount = 0;

    // Atualiza o tempo de referência
    oldTime = millis();

    // Reativa a interrupção do sensor de fluxo
    attachInterrupt(digitalPinToInterrupt(pinFlow), pulseCounter, RISING);
  }

  // --- Leitura dos sensores ---
  int waterLevel = digitalRead(pinLevel);  
  // waterLevel = 0 → boia fechada (nível atingido)
  // waterLevel = 1 → boia aberta (nível baixo)

  // Exemplo de leitura futura do sensor de umidade do solo:
  // int soilRaw = analogRead(pinSoil); // 0–1023
  // int humidity = map(soilRaw, 1023, 0, 0, 100); // conversão aproximada para %

  // Exemplo de controle de relé futuro:
  // if (humidity < 30) {
  //   digitalWrite(pinRelay, HIGH); // liga bomba
  // } else {
  //   digitalWrite(pinRelay, LOW);  // desliga bomba
  // }

  // --- Formatação dos dados em JSON ---
  // O formato facilita o envio estruturado e posterior interpretação no nó master
  String message = "{";
  message += "\"flowRate\":" + String(flowRate) + ",";   // Vazão em L/min
  message += "\"waterLevel\":" + String(waterLevel);     // Estado da boia
  // message += ",\"humidity\":" + String(humidity);     // Futuro: Umidade do solo
  message += "}";

  // --- Envio via LoRa ---
  loraSerial.println(message);                     // Transmissão para o master
  Serial.println("Enviado para master: " + message); // Log local (debug)

  // Intervalo de 2 segundos entre transmissões
  delay(2000);
}
