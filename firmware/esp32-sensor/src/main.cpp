#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

const int POD_ID = 3;           // CHANGE ME
uint8_t receiverAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

const int pirPin  = 5;
const int trigPin = 3;
const int echoPin = 4;

const unsigned long PIR_CONFIRM_TIME = 100; // 100ms to ignore Wi-Fi noise
const unsigned long PIR_ALERT_TIME = 300;   // 0.3 seconds alert duration

typedef struct struct_message {
  int podID, speedKmh, distanceCm, humanDetected;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

unsigned long pirStartTime = 0, alertStartTime = 0, lastSendTime = 0, lastPirPrintTime = 0;
bool pirTriggered = false, alertActive = false;

int readDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 30000);
  return (duration == 0) ? 400 : duration * 0.034 / 2;
}

void setup() {
  Serial.begin(115200); delay(1000);
  Serial.println("ESP32 POD SYSTEM (OPTIMIZED)");

  pinMode(pirPin, INPUT_PULLDOWN);  // Critical: PIR rests at 0V
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE); // Lock to Channel 1

  if (esp_now_init() != ESP_OK) return;

  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0; peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("READY. PIR Alert: 0.3s.");
}

void loop() {
  unsigned long currentTime = millis();

  int pirState = digitalRead(pirPin);

  // Debug print every 500ms
  if (currentTime - lastPirPrintTime >= 500) {
    Serial.print("PIR: "); Serial.println(pirState == HIGH ? "HIGH" : "LOW");
    lastPirPrintTime = currentTime;
  }

  // Detect real motion (Hold HIGH for 100ms)
  if (pirState == HIGH) {
    if (pirStartTime == 0) pirStartTime = currentTime;
    if (!pirTriggered && currentTime - pirStartTime >= PIR_CONFIRM_TIME) {
      pirTriggered = true;
      alertActive = true;
      alertStartTime = currentTime;
      Serial.println(">>> MOTION DETECTED <<<");
    }
  } else {
    pirStartTime = 0;           // Reset timer if signal drops
    pirTriggered = false;       // Unlock for next trigger
  }

  // Auto-clear alert after 0.3s
  if (alertActive && currentTime - alertStartTime >= PIR_ALERT_TIME) {
    alertActive = false;
    Serial.println(">>> ALERT CLEARED <<<");
  }

  myData.podID = POD_ID;
  myData.speedKmh = 0;
  myData.distanceCm = readDistance();
  myData.humanDetected = alertActive ? 1 : 0;

  // Send every 250ms
  if (currentTime - lastSendTime >= 250) {
    esp_now_send(receiverAddress, (uint8_t *)&myData, sizeof(myData));
    lastSendTime = currentTime;
  }
}