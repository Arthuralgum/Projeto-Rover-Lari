#include <WiFi.h>
#include <Arduino.h>
#include <esp_now.h>


const uint8_t m1frente = 18; // RX2 Pin - int1 

const uint8_t m1tras = 17; // TX2 Pin - int2
const uint8_t m2tras = 21; // D18 Pin - int3
const uint8_t m2frente = 19; // D19 Pin - int4

const uint8_t pinoENA = 25; 
const uint8_t pinoENB = 26;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct controller_package {
  uint8_t joyX;
  uint8_t joyY;
  bool btn_1;
  bool btn_2;
} controller_package;

controller_package package;

esp_now_peer_info_t peerInfo;

void driveMotors(int velM1, int velM2) {
  // --- MOTOR 1 (Esquerdo) ---
  if (velM1 > 0) {
    analogWrite(m1frente, velM1);
    analogWrite(m1tras, 0);
  } else if (velM1 < 0) {
    analogWrite(m1frente, 0);
    analogWrite(m1tras, abs(velM1));
  } else {
    analogWrite(m1frente, 0);
    analogWrite(m1tras, 0);
  }

  // --- MOTOR 2 (Direito) ---
  if (velM2 > 0) {
    analogWrite(m2frente, velM2);
    analogWrite(m2tras, 0);
  } else if (velM2 < 0) {
    analogWrite(m2frente, 0);
    analogWrite(m2tras, abs(velM2));
  } else {
    analogWrite(m2frente, 0);
    analogWrite(m2tras, 0);
  }
}

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
  

    int inputY = map(package.joyY, 0, 255, -255, 255); // Forward (+) / Backward (-)
  int inputX = map(package.joyX, 0, 255, -255, 255); // Right (+) / Left (-)

  if (abs(inputY) < 20) inputY = 0;
  if (abs(inputX) < 20) inputX = 0;

  int motorEsquerdo = inputY + inputX; // Left motor combines drive and steer
  int motorDireito  = inputY - inputX; // Right motor opposes steer for differential turning

  motorEsquerdo = constrain(motorEsquerdo, -255, 255);
  motorDireito  = constrain(motorDireito, -255, 255);

  driveMotors(motorEsquerdo, motorDireito);

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


void loop() {

}