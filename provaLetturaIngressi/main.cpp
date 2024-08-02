#include <Arduino.h>
#include <WiFi.h>
TaskHandle_t thread_PC;
TaskHandle_t thread_LIDAR;
const char* ssid     = "MoneyGuardKMG";
const char* password = "soymarisco";
WiFiServer server(80);
byte data[45] ;
byte buffer[2];
bool ready_to_post = 0;
int data_holder_coordinates[720][2];
int sizeofdataholder = sizeof(data_holder_coordinates)/sizeof(data_holder_coordinates[0]);
int timeBefore;
long startTime;
int data_holder_counter;
const int N_POINTS = 12;
const byte START_HEX[8] = {0x54, 0xA0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5E};
const byte STOP_HEX[8] = {0x54, 0xA1, 0x04, 0x00, 0x00, 0x00, 0x00, 0x4A};

void communicationSetUp() {
  Serial.begin(115200); // for computer
  Serial.println("Starting Comunication PC : 115200..");
  Serial1.begin(230400,SERIAL_8N1,RX1,TX1);
  Serial.println("Starting Comunication LIDAR : 230400..");
}
void pinSetUp() {
  pinMode(14,PULLDOWN); 
  pinMode(15,OUTPUT); 
}

String buffering() {
  String str;
  for (int i = 1; i< sizeofdataholder;i++) {
    if (data_holder_coordinates[i][0] != 0) {
      str += data_holder_coordinates[i][0];str += " ";
      str += data_holder_coordinates[i][1];str += "\r\n";}}
  return str;
}

void internetConnectionPost() {
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {
                            // if you get a client,
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        if (c == '\n') {                    // if the byte is a newline character
          if (currentLine.length() == 0) {
            client.print("HTTP/1.1 200 OK"+ String("\r\n")+"Content-type:text/html"+String("\r\n")+String("\r\n")); // Intestazione necessaria
            vTaskSuspend(thread_LIDAR);
            client.print(buffering()+String("\r\n")); // Stampa           
            std::fill(&data_holder_coordinates[0][0], &data_holder_coordinates[0][0]+sizeofdataholder*2, 0);
            vTaskResume(thread_LIDAR);
            break;
          } else {currentLine = "";}
        } else if (c != '\r') {currentLine += c;}}}

        client.stop();}
}
void TaskPC(void *pvParameters) {
  while (1) {
    internetConnectionPost();
    delay(5);
  }
}
void insertToDataHolder(float angle_s, float angle_e, int values[12],int strenght[12]) {
  vTaskSuspend(thread_PC);
  float step;
  if (angle_s > angle_e) { step = (angle_e- 360+angle_s)/N_POINTS ;} else { step = (angle_e - angle_s)/ N_POINTS;}
  for (int i = 0; i< N_POINTS;i++) {
    if (strenght[i]> 100 && values[i] > 0 ) { // dato accettabile 
      if (angle_s + step*i > 360) {data_holder_coordinates[data_holder_counter][0] = (angle_s-360+step*i) *100;} // Se il dato supera i 360 gradi tra inizio e fine 
      else {data_holder_coordinates[data_holder_counter][0] = (angle_s+step*i )*100;}// Se non supera i 360°
      data_holder_coordinates[data_holder_counter][1] = values[i];
      data_holder_counter+=1;
      if (data_holder_counter >= sizeofdataholder) {data_holder_counter = 0;break;}
    }
  }
  vTaskResume(thread_PC);
}

void TaskLidar(void *pvParameters) {
  while(1) {
    Serial1.readBytes(buffer,sizeof(buffer));
    // COntrollo iniziale
    if (buffer[0] == 0x54 && buffer[1] == 0x2C) {
      // Leggo il resto dei dati
      Serial1.readBytes(data,sizeof(data));
      int time = data[42]|data[43]<<8;
      // Controllo doppioni
        if(time != timeBefore) {
            float startAngle = float(data[2]|data[3]<<8)/100;
              float endAngle = float(data[40]|data[41]<<8)/100;
              int distances[N_POINTS];
              int strenght[N_POINTS];
              for (int i = 1; i < N_POINTS+1;i++) {
                distances[i-1] = data[(i*3)+1]|data[i*3+2] <<8;
                strenght[i-1] = data[(i*3)+3];
              }
              insertToDataHolder(startAngle,endAngle,distances,strenght);
            timeBefore = time;
        }
    }
  }
}
void multyThreadSetup() {
  //-------------------------------------------------
  xTaskCreatePinnedToCore //-------------------- 0
  ( TaskPC,     // NOME FUNZIONE
    "task_PC",  // NOME TASK
    10000,      // TASK SIZE BYTES
    NULL,       // TASK PARAMETER POINTER
    1,          // TASK PRIORITY
    &thread_PC, // TASK CLASSE  task can be referenced
    1 );        // CPU CORE TO ATTACH
  xTaskCreatePinnedToCore //-------------------- 1
  ( TaskLidar,
    "task_Lidar",
    10000,
    NULL,
    1,
    &thread_LIDAR,
    0 );
//-------------------------------------------------
}
void wifiSetup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  server.begin();
  Serial.println(WiFi.localIP());
}
void setup() {
  pinSetUp();
  communicationSetUp();
  multyThreadSetup();
  wifiSetup();
  Serial1.write(START_HEX,sizeof(START_HEX));
}
void loop() {

}