#include <SoftwareSerial.h>
 
SoftwareSerial loraSerial(4, 5); // TX, RX
 
void setup() {
  Serial.begin(9600);
  loraSerial.begin(9600);  
}
 
void loop() { 
  Serial.print("Esperando mensagem");
    if(loraSerial.available() > 0){
//    Serial.print("Serial disponivel");
    String input = loraSerial.readString();
    Serial.println(input);  
    }
  delay(2000); 
}
