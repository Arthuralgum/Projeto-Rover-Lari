#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>


const uint8_t motor1e = 18; // RX2 Pin
const uint8_t motor1d = 17; // TX2 Pin
const uint8_t motor2e = 21; // D18 Pin
const uint8_t motor2d = 19; // D19 Pin

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
  if (package.joyX > 127 && package.joyY > 127){
      analogWrite(motor1e, package.joyX);
      analogWrite(motor2e, package.joyY);
      analogWrite(motor1d, 0);
      analogWrite(motor2d, 0);

  }else 
  if (package.joyX < 127 && package.joyY < 127){
      analogWrite(motor1e, 0);
      analogWrite(motor2e, 0);
      analogWrite(motor1d, 255-package.joyX);
      analogWrite(motor2d, 225-package.joyY);

  }
  else {
      analogWrite(motor1e, 0);
      analogWrite(motor2e, 0);
      analogWrite(motor1d, 0);
      analogWrite(motor2d, 0);
    
  }
  delay(100);
}

void setup() {
  Serial.begin(115200);
  pinMode(motor1e, OUTPUT);
  pinMode(motor1d, OUTPUT);
  pinMode(motor2e, OUTPUT);
  pinMode(motor2d, OUTPUT);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {}
