#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>

const int Btn1 = 4;
const int Btn2 = 13;

const int JoystickBtn = -1;
const int JoystickX = 32;
const int JoystickY = 33;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct controller_package {
  uint8_t joyX;
  uint8_t joyY;
  bool btn_1;
  bool btn_2;
} controller_package;


controller_package package;

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  delay(100);

  analogSetAttenuation(ADC_11db);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("MAC ADDRESS: ");
  Serial.println(WiFi.macAddress());
  Serial.println("");

  pinMode(JoystickX, INPUT);
  pinMode(JoystickY, INPUT);
  pinMode(Btn1, INPUT_PULLUP);
  pinMode(Btn2, INPUT_PULLUP);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  Serial.println("Inicialization OK");
  
}

void handleJoystickAxis(uint16_t inputX, uint16_t inputY){
  //input - 4095
  //x - 255

  // uint8_t rX = inputX*255/4095;
  // uint8_t rY = inputY*255/4095;
  uint8_t rX = map(inputX, 0, 4150, 0, 255);
  uint8_t rY = map(inputY, 0, 4150, 0, 255);
  if(rX > 115 && rX < 130){
    rX = 127;
  }
  if(rY > 115 && rY < 130){
    rY = 127;
  }
  package.joyX = rX;
  package.joyY = rY;
  Serial.print("Brute X: ");
  Serial.println(inputX);
  Serial.print("Brute Y: ");
  Serial.println(inputY);
  Serial.print("Joy X sent: ");
  Serial.println(rX);
  Serial.print("Joy Y sent: ");
  Serial.println(rY);
}
void loop() {
  handleJoystickAxis(analogRead(JoystickX), analogRead(JoystickY));
  package.btn_1 = digitalRead(Btn1);
  package.btn_2 = digitalRead(Btn2);
  Serial.print("BTN1 sent: ");
  Serial.println(digitalRead(Btn1));
  Serial.print("BTN2 sent: ");
  Serial.println(digitalRead(Btn2));
  Serial.println("");

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &package, sizeof(package));
  if (result == ESP_OK) {
    //
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(100);
}