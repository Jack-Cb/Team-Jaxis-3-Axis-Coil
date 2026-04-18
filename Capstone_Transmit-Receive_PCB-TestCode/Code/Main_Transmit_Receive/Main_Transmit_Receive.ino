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

uint32_t spi_stream_rx = 0;
float spi_stream_vx = 0.00000000;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
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
    String html = "<h1>ESP32-S3 Telemetry</h1>";
    html += "<p><b>Raw X:</b> " + String(spi_stream_rx) + "</p>";
    html += "<p><b>Volt X:</b> " + String(spi_stream_vx) + "V</p>";
  
    server.send(200, "text/html", html);
  });

  server.begin();
  Serial.print("AP IP Address: ");
  Serial.println(WiFi.softAPIP());

  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000010000011); //ADDRESS
  SPI.transfer16(0b0000000000000001); //DATA
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000011000011); // AFE0
  SPI.transfer16(0b0000000001101001);
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000100110101); // VOLTAGE BIAS REG
  SPI.transfer16(0b0000000000000000);
  digitalWrite(SS, HIGH);
  SPI.endTransaction(); 

}

void loop() {
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000000011); 
  uint8_t id = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(id, HEX);
  
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000101010); 
  spi_stream_rx = 0;
  spi_stream_rx |= (uint32_t)SPI.transfer(0x00) << 16;
  spi_stream_rx |= (uint32_t)SPI.transfer(0x00) << 8;
  spi_stream_rx |= (uint32_t)SPI.transfer(0x00);
  spi_stream_vx = (spi_stream_rx * 5) / 16777216.00000000;
  Serial.print(spi_stream_rx);
  Serial.print(", Voltage X: ");
  Serial.println(spi_stream_vx, 10);
  SPI.endTransaction(); 

  server.handleClient();
  
  digitalWrite(SS, HIGH);
}
