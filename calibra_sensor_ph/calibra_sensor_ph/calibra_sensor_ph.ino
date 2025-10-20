#include <Arduino.h>

const int ph_pin = 32;       // Pino analógico onde o sensor está conectado
float calibration_value = 11.5; // Ajuste conforme sua calibração
int buffer_arr[10];
int temp;
unsigned long int avgval;
float ph_act;

void setup() {
  Serial.begin(115200);
  Serial.println("::::: Monitor de pH (ESP32) ::::");
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

  delay(1000);
}
