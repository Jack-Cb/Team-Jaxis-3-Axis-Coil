#include <Arduino.h>
#include "driver/spi_slave.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include "esp_wifi.h"

volatile float magnitude = 0.000;
volatile int altitude_ang = 0;
volatile int azimuth_ang = 0;

int targetFreq = 1000; // Single integer frequency

AsyncWebServer server(80);
//hw_timer_t * timer = NULL;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//#define WINDOW_FREQ 10000

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

/*void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    
    for (int i = 0; i < NUM_BYTES; i++) {
        txbuf[i] = 0;
    }
    txbuf[0] = (int)minfreq;
    txbuf[1] = (int)maxfreq;
    
    memset(rxbuf, 0, NUM_BYTES);

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

    Serial.print("TX was: ");
    for (int i = 0; i < NUM_BYTES; i++) {
        Serial.printf("%02X ", txbuf[i]);
    }
    Serial.println();

    portEXIT_CRITICAL_ISR(&timerMux);
}*/

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>8-Octant Telemetry</title>
<style>
  body{font-family:sans-serif;background:#121212;color:#0f8;text-align:center;padding:20px;margin:0;}
  .container {display:flex; justify-content:center; gap:20px; flex-wrap:wrap; align-items: stretch; margin-top:30px;}
  .card{background:#1e1e1e;padding:20px;border-radius:15px;min-width:280px;border:1px solid #333;display:flex;flex-direction:column;align-items:center;}
  .header{color:#fff;font-size:1.4em;border-bottom:1px solid #0f8;margin-bottom:15px;padding-bottom:5px;width:100%;text-transform:uppercase;height:30px;line-height:30px;}
  .l{color:#888;font-size:0.7em;text-transform:uppercase;margin-top:10px;}
  .v{font-size:2.2em;font-weight:bold;margin-bottom:5px;text-align:center;width:100%;}
  #vectorCanvas { background: #000; border-radius: 10px; border: 1px solid #555; box-shadow: 0 0 15px rgba(0,0,0,0.5);}
  input { background: #333; color: #0f8; border: 1px solid #444; padding: 10px; border-radius: 4px; width: 140px; text-align: center; }
  .btn { background: #0f8; color: #121212; border: none; padding: 12px 25px; border-radius: 4px; cursor: pointer; font-weight: bold; }
</style>
</head>
<body>
  <div class="container">
    <div class="card">
      <div class="header">3D Octant Visual</div>
      <canvas id="vectorCanvas" width="300" height="300"></canvas>
    </div>
    <div class="card">
      <div class="header">Telemetry Data</div>
      <div style="margin: auto 0;">
        <div class="l">Magnitude</div><div id="mag" class="v">0.000</div>
        <div class="l">Altitude</div><div id="alt" class="v">0&deg;</div>
        <div class="l">Azimuth</div><div id="azi" class="v">0&deg;</div>
      </div>
    </div>
  </div>
  <div class="container">
    <div class="card" style="min-width: 400px;">
      <div class="header">Frequency Control</div>
      <div class="l">Set Target Frequency (Hz)</div>
      <input type="number" id="freqIn" placeholder="Enter Hz">
      <button onclick="updateFreq()" class="btn" style="margin-top:10px;">Update Frequency</button>
    </div>
  </div>

<script>
  const canvas = document.getElementById('vectorCanvas');
  const ctx = canvas.getContext('2d');
  const CX = canvas.width / 2;
  const CY = canvas.height / 2;

  function project(x, y, z) {
    const isoX = (x - z) * Math.cos(0.4);
    const isoY = y + (x + z) * Math.sin(0.4);
    return { x: CX + isoX, y: CY + isoY };
  }

  function drawOctants(alt, azi, mag) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const S = 90; // Scale

    // 1. Draw Bright Main Axes
    ctx.shadowBlur = 0; // Reset shadow for grid
    ctx.lineWidth = 2;
    
    // X - Reddish
    ctx.strokeStyle = '#ff4444';
    let x1 = project(-S, 0, 0), x2 = project(S, 0, 0);
    ctx.beginPath(); ctx.moveTo(x1.x, x1.y); ctx.lineTo(x2.x, x2.y); ctx.stroke();
    
    // Y - Greenish (Altitude)
    ctx.strokeStyle = '#44ff44';
    let y1 = project(0, -S, 0), y2 = project(0, S, 0);
    ctx.beginPath(); ctx.moveTo(y1.x, y1.y); ctx.lineTo(y2.x, y2.y); ctx.stroke();
    
    // Z - Bluish (Depth)
    ctx.strokeStyle = '#4444ff';
    let z1 = project(0, 0, -S), z2 = project(0, 0, S);
    ctx.beginPath(); ctx.moveTo(z1.x, z1.y); ctx.lineTo(z2.x, z2.y); ctx.stroke();

    // 2. Draw Octant Boundary Box (Light Gray)
    ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)';
    ctx.lineWidth = 1;
    ctx.setLineDash([5, 5]);
    
    const corners = [S, -S];
    corners.forEach(x => {
      corners.forEach(y => {
        let p1 = project(x, y, -S), p2 = project(x, y, S);
        ctx.beginPath(); ctx.moveTo(p1.x, p1.y); ctx.lineTo(p2.x, p2.y); ctx.stroke();
        let p3 = project(x, -S, y), p4 = project(x, S, y);
        ctx.beginPath(); ctx.moveTo(p3.x, p3.y); ctx.lineTo(p4.x, p4.y); ctx.stroke();
        let p5 = project(-S, x, y), p6 = project(S, x, y);
        ctx.beginPath(); ctx.moveTo(p5.x, p5.y); ctx.lineTo(p6.x, p6.y); ctx.stroke();
      });
    });

    // 3. Calculate Vector
    let rAlt = alt * (Math.PI / 180);
    let rAzi = azi * (Math.PI / 180);
    let len = Math.min(S, 20 + mag * 5);

    let vx = len * Math.cos(rAlt) * Math.sin(rAzi);
    let vy = -len * Math.sin(rAlt);
    let vz = len * Math.cos(rAlt) * Math.cos(rAzi);
    let tip = project(vx, vy, vz);

    // 4. Draw Vector (Bright Cyan Glow)
    ctx.setLineDash([]);
    ctx.shadowBlur = 15;
    ctx.shadowColor = '#0f8';
    ctx.strokeStyle = '#0f8';
    ctx.lineWidth = 5;
    ctx.beginPath(); ctx.moveTo(CX, CY); ctx.lineTo(tip.x, tip.y); ctx.stroke();

    // 5. Tip & Labels
    ctx.fillStyle = '#fff';
    ctx.beginPath(); ctx.arc(tip.x, tip.y, 6, 0, Math.PI * 2); ctx.fill();

    ctx.shadowBlur = 0;
    ctx.font = 'bold 12px Arial';
    ctx.fillStyle = '#ff4444'; ctx.fillText('X', x2.x+5, x2.y);
    ctx.fillStyle = '#44ff44'; ctx.fillText('Y', y2.x, y2.y-5);
    ctx.fillStyle = '#4444ff'; ctx.fillText('Z', z2.x+5, z2.y+5);
  }

  setInterval(() => {
    fetch('/data').then(r => r.json()).then(d => {
      document.getElementById('mag').innerText = d.mag.toFixed(3);
      document.getElementById('alt').innerText = d.alt + "\u00B0";
      document.getElementById('azi').innerText = d.azi + "\u00B0";
      drawOctants(d.alt, d.azi, d.mag);
    }).catch(e => {});
  }, 100);

  function updateFreq() {
    let val = document.getElementById('freqIn').value;
    if(!val) return;
    fetch(`/setFreq?val=${val}`).then(r => r.text()).then(msg => alert(msg));
  }
</script>
</body></html>)rawliteral";

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    delay(500);

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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ r->send_P(200, "text/html", index_html); });
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

    //timer = timerBegin(1000000);
    //timerAttachInterrupt(timer, &onTimer);
    // Alarm fires 3x faster to maintain 10kHz per channel
    //timerAlarm(timer, 1000000 / (WINDOW_FREQ), true, 0);
}

void loop() {
  //portENTER_CRITICAL(&timerMux);
  memset(txbuf, 0, NUM_BYTES);
  memset(rxbuf, 0, NUM_BYTES);

  txbuf[0] = (uint8_t) ((targetFreq / 1000) % 10) * 1; // Thousands place (1)
  txbuf[1] = (uint8_t) ((targetFreq / 100) % 10) * 1;  // Hundreds place (2)
  txbuf[2] = (uint8_t) ((targetFreq / 10) % 10) * 1;   // Tens place (3)
  txbuf[3] = (uint8_t) (targetFreq % 10) * 1;          // Ones place (4)*/

  //txbuf[1] = 0x02;
  //txbuf[2] = 0x00;
  //txbuf[3] = 0x00;
  //txbuf[4] = 0x00;

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
  //portEXIT_CRITICAL(&timerMux);
}
