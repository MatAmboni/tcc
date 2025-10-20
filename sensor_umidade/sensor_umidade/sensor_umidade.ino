#define pinAnalog 35

float AnalogOutput = 0;
float voltage = 0;

// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  
  AnalogOutput = analogRead(pinAnalog);
  float voltage = AnalogOutput * (5.0 / 1023.0);
  Serial.println("Saida analog");
  Serial.println(voltage);    
  delay(2000);
}
