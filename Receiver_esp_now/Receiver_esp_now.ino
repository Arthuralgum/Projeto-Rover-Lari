#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>


const uint8_t m1frente = 18; // RX2 Pin
const uint8_t m1tras = 17; // TX2 Pin
const uint8_t m2tras = 21; // D18 Pin
const uint8_t m2frente = 19; // D19 Pin

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct controller_package {
  uint8_t joyX;
  uint8_t joyY;
  bool btn_1;
  bool btn_2;
} controller_package;

controller_package package;

esp_now_peer_info_t peerInfo;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&package, incomingData, sizeof(package));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("X: ");
  Serial.println(package.joyX);
  Serial.print("Y: ");
  Serial.println(package.joyY);
  Serial.print("btn_1: ");
  Serial.println(package.btn_1);
  Serial.print("btn_2: ");
  Serial.println(package.btn_2);
  if (package.joyX < 127 && package.joyY > 127){ //frente esquerda
      analogWrite(m1frente, package.joyX);
      analogWrite(m2tras, package.joyY);
      analogWrite(m1tras, 0);
      analogWrite(m2frente, 0);

  }else if (package.joyX > 127 && package.joyY > 127){ //frente direita 
      analogWrite(m1frente, 0);
      analogWrite(m2tras, 0);
      analogWrite(m1tras, 255-package.joyX);
      analogWrite(m2frente, 225-package.joyY);

  }else if (package.joyX < 127 && package.joyY < 127){ //tras esquerda
      analogWrite(m1frente, 0);
      analogWrite(m2tras, 0);
      analogWrite(m1tras, 255-package.joyX);
      analogWrite(m2frente, 225-package.joyY);

  }else if (package.joyX < 127 && package.joyY < 127){ //tras direita
      analogWrite(m1frente, 0);
      analogWrite(m2tras, 0);
      analogWrite(m1tras, 255-package.joyX);
      analogWrite(m2frente, 225-package.joyY);

  }
  else {
      analogWrite(m1frente, 0);
      analogWrite(m2tras, 0);
      analogWrite(m1tras, 0);
      analogWrite(m2frente, 0);
    
  }
  delay(100);
}

void setup() {
  Serial.begin(115200);
  pinMode(m1frente, OUTPUT);
  pinMode(m1tras, OUTPUT);
  pinMode(m2tras, OUTPUT);
  pinMode(m2frente, OUTPUT);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {}
