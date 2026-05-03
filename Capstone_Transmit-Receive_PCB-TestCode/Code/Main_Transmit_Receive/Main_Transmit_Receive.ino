#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include "esp_wifi.h"

const uint16_t ADDR_X = 0b0100000000101010; // Example: Read Command + Address
const uint16_t ADDR_Y = 0b0100000000101110; 
const uint16_t ADDR_Z = 0b0100000000110010;

#define SS 10
#define MOSI 11
#define MISO 13
#define SCK 12
#define SAMPLING_FREQ 10000
#define SAMPLE_WINDOW 1000 

// Volatile variables for thread safety
volatile uint32_t sample_buffer[SAMPLE_WINDOW];
volatile int sample_idx = 0;
volatile bool bufferReady = false;

// Results to display
float detected_freq = 0;
float amplitude_v = 0;
float offset_v = 0;

AsyncWebServer server(80);
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// Convert raw ADC (24-bit) to Voltage (assuming 5V ref)
float rawToVolt(uint32_t raw) {
    return (raw * 5.0) / 16777216.0;
}

void analyzeSignal() {
    uint32_t s_min = 0xFFFFFF;
    uint32_t s_max = 0;
    unsigned long long sum = 0;

    for (int i = 0; i < SAMPLE_WINDOW; i++) {
        uint32_t val = sample_buffer[i];
        if (val < s_min) s_min = val;
        if (val > s_max) s_max = val;
        sum += val;
    }

    uint32_t avg_raw = sum / SAMPLE_WINDOW;
    offset_v = rawToVolt(avg_raw);
    amplitude_v = (rawToVolt(s_max) - rawToVolt(s_min)) / 2.0;

    // Frequency Detection using Time-over-Crossings
    int crossings = 0;
    int firstCrossingIdx = -1;
    int lastCrossingIdx = -1;

    for (int i = 1; i < SAMPLE_WINDOW; i++) {
        // Detect a positive-going crossing for cleaner cycle counting
        if (sample_buffer[i-1] < avg_raw && sample_buffer[i] >= avg_raw) {
            crossings++;
            if (firstCrossingIdx == -1) firstCrossingIdx = i;
            lastCrossingIdx = i;
        }
    }

    if (crossings > 1) {
        // Calculate the time elapsed between the first and last crossing
        // (1.0 / SAMPLING_FREQ) is the time per sample in seconds
        float timeBetweenCrossings = (lastCrossingIdx - firstCrossingIdx) * (1.0 / (float)SAMPLING_FREQ);
        
        // Frequency = (Number of full cycles) / (Time elapsed)
        // Since we only counted positive-going crossings, each crossing is 1 full cycle
        detected_freq = (crossings - 1) / timeBetweenCrossings;
    } else {
        detected_freq = 0;
    }
}


void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000000101110); // CH_Y
    uint32_t val = (uint32_t)SPI.transfer(0x00) << 16;
    val |= (uint32_t)SPI.transfer(0x00) << 8;
    val |= (uint32_t)SPI.transfer(0x00);
    digitalWrite(SS, HIGH);
    SPI.endTransaction(); 

    if (!bufferReady) {
        sample_buffer[sample_idx++] = val;
        if (sample_idx >= SAMPLE_WINDOW) {
            bufferReady = true;
        }
    }
    portEXIT_CRITICAL_ISR(&timerMux);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Signal Telemetry</title>
  <style>
    body { font-family: sans-serif; background: #121212; color: #00ff88; text-align: center; padding-top: 50px; }
    .card { background: #1e1e1e; padding: 20px; border-radius: 15px; display: inline-block; min-width: 300px; }
    .label { color: #888; font-size: 0.8em; text-transform: uppercase; }
    .val { font-size: 2.5em; font-weight: bold; margin-bottom: 20px; }
  </style>
</head>
<body>
  <div class="card">
    <div class="label">Frequency</div><div class="val"><span id="f">0</span> Hz</div>
    <div class="label">Amplitude (Pk-Pk/2)</div><div class="val"><span id="a">0</span> V</div>
    <div class="label">DC Offset</div><div class="val"><span id="o">0</span> V</div>
  </div>
<script>
  setInterval(() => {
    fetch('/data').then(res => res.json()).then(d => {
      document.getElementById('f').innerText = d.f.toFixed(1);
      document.getElementById('a').innerText = d.a.toFixed(4);
      document.getElementById('o').innerText = d.o.toFixed(4);
    });
  }, 50);
</script>
</body></html>)rawliteral";

void setup() {
    delay(5000);
    
    Serial.begin(115200);
    //SPI.begin(SCK, MISO, MOSI, SS);
    SPI.begin();
    pinMode(SS, OUTPUT);

    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    delay(10);
    
    // CH MAP 0
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
    
    WiFi.softAP("JAXIS", "Coilcapstone");
    
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ r->send_P(200, "text/html", index_html); });
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *r){
        String j = "{\"f\":"+String(detected_freq, 2)+",\"a\":"+String(amplitude_v,4)+",\"o\":"+String(offset_v,4)+"}";
        r->send(200, "application/json", j);
    });
    server.begin();

    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 1000000 / SAMPLING_FREQ, true, 0);

  
}

void loop() {
    if (bufferReady) {
        analyzeSignal();
        // Allow the timer to start filling the buffer again
        portENTER_CRITICAL(&timerMux);
        sample_idx = 0;
        bufferReady = false;
        portEXIT_CRITICAL(&timerMux);
    }
}
