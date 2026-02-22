#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"

WiFiUDP udp;
const char * broadcastIP = "192.168.4.255";
const uint16_t port = 1234;
uint8_t dummyData[1024]; // Large packet to keep radio busy

void setup() {
  setCpuFrequencyMhz(240);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("PowerFlood_S3", "password123");
  
  esp_wifi_set_max_tx_power(80); // 20dBm
  esp_wifi_set_ps(WIFI_PS_NONE); // Disable sleep

  memset(dummyData, 0xAA, sizeof(dummyData));

  // Stress Core 0
  xTaskCreatePinnedToCore([](void * p){
    while(1) { volatile double d = sqrt(123.456); }
  }, "stress", 2048, NULL, 1, NULL, 0);
}

void loop() {
  // Flood the radio with UDP packets to force TX current
  udp.beginPacket(broadcastIP, port);
  udp.write(dummyData, sizeof(dummyData));
  udp.endPacket();
  
  // No delay - keep the CPU and Radio at 100%
}
