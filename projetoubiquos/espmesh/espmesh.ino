#include "painlessMesh.h"
#include "WiFi.h"
#include "ArduinoJson.h" 

#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

uint32_t startTime = 0;
uint32_t rtt = 0;


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  // Criar um objeto JSON
  DynamicJsonDocument jsonDoc(256);  // Ajuste o tamanho conforme necessário

  // Adicionar dados ao objeto JSON
  jsonDoc["message"] = "Hi from node1";
  jsonDoc["nodeId"] = mesh.getNodeId();

  // Converter o objeto JSON em uma string
  String jsonString;
  serializeJson(jsonDoc, jsonString);

  // Enviar a mensagem pela malha
  startTime = millis();
  mesh.sendBroadcast(jsonString);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {

  // Converter a string recebida de volta para o objeto JSON
  DynamicJsonDocument jsonDoc(256);  // Ajuste o tamanho conforme necessário
  DeserializationError error = deserializeJson(jsonDoc, msg);

  // Verificar erros na desserialização
  if (error) {
    Serial.print("deserializeJson() falhou: ");
    Serial.println(error.c_str());
    return;
  }

  // Extrair dados do objeto JSON
  String receivedMessage = jsonDoc["message"];
  uint32_t nodeId = jsonDoc["nodeId"];

  Serial.print(nodeId);
  Serial.print(": ");
  Serial.println(receivedMessage);

  uint32_t endTime = millis();
  rtt = endTime - startTime;
  Serial.println("WiFi signal: " + String(WiFi.RSSI()) + " db | " + "RTT = " + rtt);
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  WiFi.setTxPower(WIFI_POWER_19dBm);
}

void loop() {
  mesh.update();
}