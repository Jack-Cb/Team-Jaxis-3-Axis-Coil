#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include <SPI.h>

const char* ssid = "JAXIS";
const char* password = "Coilcapstone"; // Must be at least 8 characters

#define SS 10
#define MOSI 11
#define MISO 13
#define SCK 12

uint32_t spi_stream_a = 0;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  SPI.begin(SCK, MISO, MOSI, SS); 
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);

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
    server.send(200, "text/plain", "Raw Data: ");
    server.send(200, "text/plain", spi_stream_a);
  });

  server.begin();
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  
  digitalWrite(SS, LOW);    // Select device
  SPI.transfer(0x00);       // Trigger SCLK for 8 clock cycles
  digitalWrite(SS, HIGH);   // Deselect device
  
  SPI.endTransaction(); 
  spi_stream_a |= (uint32_t)SPI.transfer(0x00) << 16;
  spi_stream_a |= (uint32_t)SPI.transfer(0x00) << 8;
  spi_stream_a |= (uint32_t)SPI.transfer(0x00);
  server.handleClient();
}
