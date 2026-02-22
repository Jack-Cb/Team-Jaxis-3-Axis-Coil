#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"

const char* ssid = "BeavTester";
const char* password = "QueryOneOne"; // Must be at least 8 characters

WebServer server(80);

void setup() {
  Serial.begin(115200);

  // 1. Initialize Wi-Fi in Access Point Mode
  WiFi.mode(WIFI_AP);
  
  // 2. Start the Access Point
  // Use Channel 6 (common for HT40) and limit to 4 connections for stability
  if (WiFi.softAP(ssid, password, 6, 0, 4)) {
    Serial.println("SoftAP Started successfully");
  }

  // 3. Configure for High Throughput (HT40)
  // This allows the use of 40MHz channel width for higher PHY rates
  esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT40);

  // 4. Disable Power Save for maximum performance
  esp_wifi_set_ps(WIFI_PS_NONE);

  // 5. Standard WebServer route
  server.on("/", []() {
    server.send(200, "text/plain", "Connected to ESP32-S3 HT40 SoftAP");
  });

  server.begin();
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
