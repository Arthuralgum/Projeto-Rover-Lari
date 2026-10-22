#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>


const uint8_t motor1 = 1;
const uint8_t motor2 = 2;
const uint8_t motor3 = 3;
const uint8_t motor4 = 4;

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
  delay(100);
}

void setup() {
  Serial.begin(115200);
  pinMode(motor2, OUTPUT);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {}