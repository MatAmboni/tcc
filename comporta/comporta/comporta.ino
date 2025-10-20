#define RELAY1 19
#define RELAY2 21
#define FIM_ABRE 22
#define FIM_FECHA 23

bool abrir = false;  // variável booleana que decide abrir ou fechar

void setup() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(FIM_ABRE, INPUT_PULLUP);
  pinMode(FIM_FECHA, INPUT_PULLUP);

  digitalWrite(RELAY1, LOW);
  digitalWrite(RELAY2, LOW);

  Serial.begin(115200);
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

void loop() {  
  if (abrir) {
    abrirComporta();
    abrir = false; // reseta o comando
  } else {
    fecharComporta();
    abrir = true; // alterna para teste
  }

  delay(5000); // apenas exemplo de teste
}
