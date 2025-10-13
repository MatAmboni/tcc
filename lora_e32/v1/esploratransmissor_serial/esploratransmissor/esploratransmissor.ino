#include <SoftwareSerial.h>

SoftwareSerial loraSerial(4, 5); // TX, RX
unsigned long packetCount = 0;

void setup() {
  Serial.begin(9600);
  loraSerial.begin(9600);
}

void loop() {
  packetCount++;
  Serial.println("Inicio");
  unsigned long sendTime = millis(); // tempo de envio
  // Monta a mensagem com o número do pacote e o timestamp
  String message = "Pacote:" + String(packetCount) + ",Time:" + String(sendTime);
  
  loraSerial.println(message);

  delay(500); // intervalo entre transmissões
}
