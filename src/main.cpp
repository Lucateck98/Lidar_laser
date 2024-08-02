#include <Arduino.h>
#include <HardwareSerial.h>
#include <StreamDebugger.h>
#include <iostream>
StreamDebugger debugger(Serial1,Serial);
byte start_hex[8] = {0x54, 0xA0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x5E};
byte stop_hex[8] = {0x54, 0xA1, 0x04, 0x00, 0x00, 0x00, 0x01, 0x5E};
byte changeSpeed_hex[8] = {0x54, 0xA2, 0x04, 0xA0, 0x05, 0x00, 0x00, 0x16}; // 0x16 oppure 0xA1 --- SPeed setted to 05A0 = 1440 = 4 rotazioni al secondo
byte speed_hex[8] = {0x54, 0xA3, 0x04, 0x00, 0x00, 0x00, 0x01, 0x62};
byte read_lidar_command[8];
byte readConnection[1];
uint8_t timeoutTime = 10;
const int SPEEDSTARTLEN = 12;



int timer;
uint16_t tempoCambio = 3000;
bool elevatorStop = false;
String readInputFromPC = "0";
uint8_t minVoltage = 200; // Soglia da settare in modo diverso (3,3v)


void pinSetUp() {
  pinMode(14,INPUT); // Per ricevere il segnale che l'ascensore è FERMO -> 1, in MUOVIMENTO -> 0
  pinMode(15,OUTPUT); // Per dare il consenso alle port di aprirsi 1 - 0
}
void communicationSetUp() {
  Serial.setTimeout(timeoutTime);
  Serial.begin(115200); // for computer
  Serial.println("Starting Comunication PC : 115200..");
  Serial1.begin(230400,SERIAL_8N1,RX1,TX1);
  Serial.println("Starting Comunication LIDAR : 230400..");
}
//----------------------------------------------------------------------------------------
void setup()
{
  pinSetUp();
  communicationSetUp();
}
void printLidarCommand() {
  Serial.println();
  for (int i = 0;i<sizeof(read_lidar_command);i++) {
    Serial.printf("%X",read_lidar_command[i]);
    Serial.print(" ");
  }
}
void changeElevatorStatus() {
  //Lettura del voltaggio dell'ascensore 
  if (analogRead(14) >= minVoltage) {
    elevatorStop = 1;
  }else{
    elevatorStop = 0;
  }
}
void debugInput() {
  //Cambia una sola volta , senza avere problemi di altri input
  if (readInputFromPC == "1") { elevatorStop = 1 ; } else if (readInputFromPC == "0") { elevatorStop = 0 ; }
}

void changeSpeed(int value) {
  byte MSB = value >> 8;
  byte LSB = value & 0xFF;
  changeSpeed_hex[3] = LSB;
  changeSpeed_hex[4] = MSB;
  Serial1.write(changeSpeed_hex,sizeof(changeSpeed_hex));
}
bool checkNumber() {
  int value = (readInputFromPC.substring(SPEEDSTARTLEN,readInputFromPC.length())).toInt();
  if (value >=(360*2) && value <= (360*8)) {
    changeSpeed(value);
    return true;
  } else {
    return false;
  }
}
int extractSpeed() {
  Serial1.write(speed_hex,sizeof(speed_hex));
  Serial1.readBytes(read_lidar_command,8);
  byte MSB = read_lidar_command[4];
  byte LSB = read_lidar_command[3];
  int speed = LSB | MSB << 8;
  return speed;
}
void startLidar() {
  Serial1.write(start_hex,sizeof(start_hex));
}
void stopLidar() {
  Serial1.write(stop_hex,sizeof(stop_hex));
}
void debugOutput() {
  if (readInputFromPC == "lidar-start") {
    Serial.println("Starting Lidar");
    startLidar();
    printLidarCommand();
      
  }else if (readInputFromPC == "lidar-stop") {
    Serial.println("Stopping Lidar");
    stopLidar();
    printLidarCommand();

  }else if (readInputFromPC.substring(0,11) =="lidar-speed") {
    if (checkNumber()) {
      Serial.println("Speed Changed");
      printLidarCommand();
    }else{
      Serial.println("Wrong syntax or number");
    }
  }else if (readInputFromPC == "lidar-read") {
    Serial.print("Lidar Speed : ");
    Serial.println(extractSpeed());
  }
}
void showCommandOption() {
  if (readInputFromPC == "help") {
    Serial.println("Elevator Stop = 1");
    Serial.println("Elevator Running = 0");
    Serial.println("");
    Serial.println("Start = lidar-start");
    Serial.println("Stop = lidar-stop");
    Serial.println("Change Speed ( from 720 to 2880 ) = lidar-speed XXXX");
    Serial.println("Check current speed = lidar-read");
  }
}
void slowCycle() {
  if((millis()-timer)>tempoCambio) {
    //changeElevatorStatus(); // controllo per cambiare
    if (elevatorStop) { Serial.println("FERMO"); } else {Serial.println("MUOVIMENTO");}
    timer = millis();
  }
}
void resetUserInput() {
  if ( elevatorStop == 1 ) {
    readInputFromPC = "1";
  } else if (elevatorStop == 0) {
    readInputFromPC = "0";
  }
}
void choosePcInputAction() {
  readInputFromPC = Serial.readString();
  
  showCommandOption();
  debugInput(); // testing da pc ( SIMULAZIONE Voltaggio Ascensore )
  debugOutput(); // testing  da pc -> Laser ( Comandi start stop e velocità )
  //slowCycle(); // ciclo che itera 1 volta ogni x secondi
  Serial.println("Dopo comandi");
  Serial.println(readInputFromPC);
  resetUserInput();
  Serial.println("Dopo reset");
  Serial.println(readInputFromPC);
}
//----------------------------------------------------------------------------------------
void loop(){ 
  choosePcInputAction();
  //changeElevatorStatus();
}






