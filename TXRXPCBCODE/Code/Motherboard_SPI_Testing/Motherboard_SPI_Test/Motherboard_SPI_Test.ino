#include <Arduino.h>
#include "driver/spi_slave.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include "esp_wifi.h"
#include <LittleFS.h>

volatile float magnitude = 0.000;
volatile int altitude_ang = 0;
volatile int azimuth_ang = 0;

int targetFreq = 1000; // Single integer frequency

AsyncWebServer server(80);

static const char* TAG = "SPI_SLAVE";

#define NUM_BYTES    8

// SPI2 IOMUX pins for ESP32-S3
#define PIN_MOSI     11
#define PIN_MISO     13
#define PIN_SCLK     12
#define PIN_CS       10

// DMA-capable, 32-bit aligned buffers
static uint8_t txbuf[NUM_BYTES] __attribute__((aligned(4)));
static uint8_t rxbuf[NUM_BYTES] __attribute__((aligned(4)));

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    delay(500);

    if(!LittleFS.begin(true)){
      Serial.println("An Error has occurred while mounting LittleFS");
      return;
    }

    ESP_LOGI(TAG, "Initializing SPI2 slave...");

    memset(rxbuf, 0, NUM_BYTES);
    memset(txbuf, 0, NUM_BYTES);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num   = PIN_MOSI;
    buscfg.miso_io_num   = PIN_MISO;
    buscfg.sclk_io_num   = PIN_SCLK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.flags         = SPICOMMON_BUSFLAG_IOMUX_PINS;  // bypass GPIO matrix
    buscfg.max_transfer_sz = NUM_BYTES;

    spi_slave_interface_config_t slvcfg = {};
    slvcfg.mode          = 0;                // CPOL=0, CPHA=0 — must match Pi
    slvcfg.spics_io_num  = PIN_CS;
    slvcfg.queue_size    = 1;
    slvcfg.flags         = 0;
    slvcfg.post_setup_cb = NULL;
    slvcfg.post_trans_cb = NULL;

    esp_err_t ret = spi_slave_initialize(SPI2_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "spi_slave_initialize failed: %s", esp_err_to_name(ret));
        while (1) delay(100);
    }

    ESP_LOGI(TAG, "SPI2 slave ready — waiting for master");

    WiFi.softAP("JAXIS", "Coilcapstone");

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *r){
    String j = "{";
      j += "\"mag\":" + String(magnitude, 3) + ",";
      j += "\"alt\":" + String((int)altitude_ang) + ","; // Cast to int
      j += "\"azi\":" + String((int)azimuth_ang);       // Cast to int
      j += "}";
      r->send(200, "application/json", j);
    });
    server.on("/setFreq", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("val")) {
        targetFreq = request->getParam("val")->value().toInt();
    }
    request->send(200, "text/plain", "Frequency Updated");
    });

    server.begin();
}

void loop() {
  memset(txbuf, 0, NUM_BYTES);
  memset(rxbuf, 0, NUM_BYTES);

  txbuf[0] = (uint8_t) ((targetFreq / 1000) % 10) * 1; // Thousands place (1)
  txbuf[1] = (uint8_t) ((targetFreq / 100) % 10) * 1;  // Hundreds place (2)
  txbuf[2] = (uint8_t) ((targetFreq / 10) % 10) * 1;   // Tens place (3)
  txbuf[3] = (uint8_t) (targetFreq % 10) * 1;          // Ones place (4)*/

  spi_slave_transaction_t t = {};
  t.length    = NUM_BYTES * 8;   // in bits
  t.tx_buffer = txbuf;
  t.rx_buffer = rxbuf;

  esp_err_t ret = spi_slave_transmit(SPI2_HOST, &t, portMAX_DELAY);

  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "transmit error: %s", esp_err_to_name(ret));
    return;
  }

  // t.trans_len tells you how many bits the master actually clocked
  Serial.printf("trans_len: %d bits (expected %d)\n", t.trans_len, NUM_BYTES * 8);

  Serial.print("RX: ");
  for (int i = 0; i < NUM_BYTES; i++) {
    Serial.printf("%02X ", rxbuf[i]);
  }
  Serial.println();
    
  int mag_tensones = rxbuf[0];
  int mag_tensplace = rxbuf[1];
  int mag_hundreths = rxbuf[2];
  int mag_thousands = rxbuf[3];

  int azimuth_one = rxbuf[4];
  int azimuth_two = rxbuf[5];

  int altitude_one = rxbuf[6];
  int altitude_two = rxbuf[7];

  magnitude = mag_tensones + (mag_tensplace / 10.0) + (mag_hundreths / 100.0) + (mag_thousands / 1000.0);
  azimuth_ang = azimuth_one + azimuth_two;
  altitude_ang  =  altitude_one + altitude_two;

  Serial.print("TX was: ");
  for (int i = 0; i < NUM_BYTES; i++) {
      Serial.printf("%02X ", txbuf[i]);
  }
  Serial.println();
}
