#include <SoftwareSerial.h>

SoftwareSerial loraSerial(2, 3); // TX, RX
 
void setup() {
  Serial.begin(9600);
  loraSerial.begin(9600);
}
 
void loop() {
  loraSerial.print("Mensagem");
  delay(2000);
}