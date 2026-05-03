#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPI.h>
#include "esp_wifi.h"

#define SS 10
#define MOSI 11
#define MISO 13
#define SCK 12
#define SAMPLING_FREQ 10000 
#define SAMPLE_WINDOW 1000 

// Addresses - Update these as needed
const uint16_t ADDR_X = 0b0100000000101010; // Example: Read Command + Address
const uint16_t ADDR_Y = 0b0100000000101110; 
const uint16_t ADDR_Z = 0b0100000000110010;

struct Channel {
    volatile uint32_t buffer[SAMPLE_WINDOW];
    volatile int idx = 0;
    float freq = 0, amp = 0, off = 0;
};

Channel chX, chY, chZ;
volatile int currentCh = 0; 
volatile bool buffersReady = false;
volatile bool shouldResetADC = false;

AsyncWebServer server(80);
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

float rawToVolt(uint32_t raw) { return (raw * 5.0) / 16777216.0; }

void analyzeSignal(Channel &c) {
    uint32_t s_min = 0xFFFFFF, s_max = 0;
    unsigned long long sum = 0;
    for (int i = 0; i < SAMPLE_WINDOW; i++) {
        uint32_t val = c.buffer[i];
        if (val < s_min) s_min = val;
        if (val > s_max) s_max = val;
        sum += val;
    }
    uint32_t avg_raw = sum / SAMPLE_WINDOW;
    c.off = rawToVolt(avg_raw);
    c.amp = (rawToVolt(s_max) - rawToVolt(s_min)) / 2.0;

    int crossings = 0, first = -1, last = -1;
    for (int i = 1; i < SAMPLE_WINDOW; i++) {
        if (c.buffer[i-1] < avg_raw && c.buffer[i] >= avg_raw) {
            crossings++;
            if (first == -1) first = i;
            last = i;
        }
    }
    c.freq = (crossings > 1) ? (crossings - 1) / ((last - first) * (1.0 / SAMPLING_FREQ)) : 0;
}

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    
    uint16_t addr = (currentCh == 0) ? ADDR_X : (currentCh == 1 ? ADDR_Y : ADDR_Z);
    Channel *target = (currentCh == 0) ? &chX : (currentCh == 1 ? &chY : &chZ);

    SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    SPI.transfer16(addr);
    uint32_t val = (uint32_t)SPI.transfer(0x00) << 16 | (uint32_t)SPI.transfer(0x00) << 8 | (uint32_t)SPI.transfer(0x00);
    digitalWrite(SS, HIGH);
    SPI.endTransaction();

    if (!buffersReady) {
        target->buffer[target->idx++] = val;
        if (chX.idx >= SAMPLE_WINDOW && chY.idx >= SAMPLE_WINDOW && chZ.idx >= SAMPLE_WINDOW) {
            buffersReady = true;
        }
    }
    currentCh = (currentCh + 1) % 3;
    portEXIT_CRITICAL_ISR(&timerMux);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head><title>XYZ Telemetry</title><style>
body{font-family:sans-serif;background:#121212;color:#0f8;text-align:center;display:flex;justify-content:center;gap:15px;padding-top:30px;flex-wrap:wrap;}
.card{background:#1e1e1e;padding:15px;border-radius:12px;min-width:220px;border:1px solid #333;}
.axis{color:#fff;font-size:1.2em;border-bottom:1px solid #0f8;margin-bottom:10px;}
.l{color:#888;font-size:0.7em;text-transform:uppercase;}
.v{font-size:1.6em;font-weight:bold;margin-bottom:10px;}
</style></head><body>
<div class="card"><div class="axis">X-AXIS</div><div class="l">Hz</div><div id="xf" class="v">0</div><div class="l">Amp</div><div id="xa" class="v">0</div><div class="l">Off</div><div id="xo" class="v">0</div></div>
<div class="card"><div class="axis">Y-AXIS</div><div class="l">Hz</div><div id="yf" class="v">0</div><div class="l">Amp</div><div id="ya" class="v">0</div><div class="l">Off</div><div id="yo" class="v">0</div></div>
<div class="card"><div class="axis">Z-AXIS</div><div class="l">Hz</div><div id="zf" class="v">0</div><div class="l">Amp</div><div id="za" class="v">0</div><div class="l">Off</div><div id="zo" class="v">0</div></div>
<script>
setInterval(()=>{fetch('/data').then(r=>r.json()).then(d=>{
  document.getElementById('xf').innerText=d.x.f.toFixed(1);document.getElementById('xa').innerText=d.x.a.toFixed(4);document.getElementById('xo').innerText=d.x.o.toFixed(4);
  document.getElementById('yf').innerText=d.y.f.toFixed(1);document.getElementById('ya').innerText=d.y.a.toFixed(4);document.getElementById('yo').innerText=d.y.o.toFixed(4);
  document.getElementById('zf').innerText=d.z.f.toFixed(1);document.getElementById('za').innerText=d.z.a.toFixed(4);document.getElementById('zo').innerText=d.z.o.toFixed(4);
});},100);

</script>
<button class="btn" onclick="callFunction()">Reset ADC</button>

<style>
  .btn {
    background: #00ff88;
    color: #121212;
    padding: 10px 20px;
    border: none;
    border-radius: 5px;
    cursor: pointer;
    font-weight: bold;
    margin-top: 20px;
  }
  .btn:active { background: #00cc6e; }
</style>

<script>
  function callFunction() {
    fetch('/trigger')
      .then(response => response.text())
      .then(data => alert(data)); 
  }
</script>
</body></html>)rawliteral";

void setup() {
    //delay(5000);
    Serial.begin(115200);
    SPI.begin(SCK, MISO, MOSI, SS);
    pinMode(SS, OUTPUT);
    
    setup_ADC();
    
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
    digitalWrite(SS, LOW);
    delay(10);
    SPI.transfer16(0b0100000001111001); 
    uint16_t enz = 0;
    enz |= (uint16_t) SPI.transfer(0x00) << 8; 
    enz |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("CH_ENABLE: "); // READING 
    Serial.println(enz, BIN);
  
    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000010000011); 
    uint16_t ena = 0;
    ena |= (uint16_t) SPI.transfer(0x00) << 8; 
    ena |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("CH0: "); // READING 
    Serial.println(ena, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000010000111); 
    uint16_t enb = 0;
    enb |= (uint16_t) SPI.transfer(0x00) << 8; 
    enb |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("CH1: "); // READING 
    Serial.println(enb, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000010001011); 
    uint16_t enc = 0;
    enc |= (uint16_t) SPI.transfer(0x00) << 8; 
    enc |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("CH2: "); // READING 
    Serial.println(enc, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000011000011); 
    uint16_t en = 0;
    en |= (uint16_t) SPI.transfer(0x00) << 8; 
    en |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("AFE0: "); // READING AFE ENABLE
    Serial.println(en, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000011010001); 
    uint16_t en1 = 0;
    en1 |= (uint16_t) SPI.transfer(0x00) << 8; 
    en1 |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("AFE1: "); // READING AFE ENABLE
    Serial.println(en1, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000011011111); 
    uint16_t en2 = 0;
    en2 |= (uint16_t) SPI.transfer(0x00) << 8; 
    en2 |= (uint16_t) SPI.transfer(0x00); 
    digitalWrite(SS, HIGH);
    Serial.print("AFE2: "); // READING AFE ENABLE
    Serial.println(en2, BIN);

    digitalWrite(SS, LOW);
    SPI.transfer32(0b0100000011001010); 
    uint32_t off0 = 0;
    off0 |= (uint32_t)SPI.transfer(0x00) << 16;
    off0 |= (uint32_t)SPI.transfer(0x00) << 8;
    off0 |= (uint32_t)SPI.transfer(0x00);
    Serial.print("OFFSET0: ");
    Serial.println(off0, BIN);
    digitalWrite(SS, HIGH);

    digitalWrite(SS, LOW);
    SPI.transfer16(0b0100000000000011); 
    uint8_t idc = SPI.transfer(0x00);
    digitalWrite(SS, HIGH);
    Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
    Serial.println(idc, HEX);

    SPI.endTransaction();

    delay(2000);

    WiFi.softAP("JAXIS", "Coilcapstone");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ r->send_P(200, "text/html", index_html); });
    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *r){
        String j = "{\"x\":{\"f\":"+String(chX.freq,1)+",\"a\":"+String(chX.amp,4)+",\"o\":"+String(chX.off,4)+"},";
        j += "\"y\":{\"f\":"+String(chY.freq,1)+",\"a\":"+String(chY.amp,4)+",\"o\":"+String(chY.off,4)+"},";
        j += "\"z\":{\"f\":"+String(chZ.freq,1)+",\"a\":"+String(chZ.amp,4)+",\"o\":"+String(chZ.off,4)+"}}";
        r->send(200, "application/json", j);
    });
    server.on("/trigger", HTTP_GET, [](AsyncWebServerRequest *request){
      shouldResetADC = true; // Just set the flag
      request->send(200, "text/plain", "Reset Initiated");
    });

    server.begin();

    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    // Alarm fires 3x faster to maintain 10kHz per channel
    timerAlarm(timer, 1000000 / (SAMPLING_FREQ * 3), true, 0);
}

void setup_ADC() {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  
  uint8_t idn = 0;
  
  while(idn != 7){
  digitalWrite(SS, LOW);
  delay(50);
  SPI.transfer16(0b0100000000000011); 
  idn = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000001111001);
  SPI.transfer16(0b0000000000000111);
  //SPI.transfer(0b00000111);
  digitalWrite(SS, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(idn, HEX);
  }
  
  
  digitalWrite(SS, LOW);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 7; j++) {
      SPI.transfer(0xFF); 
    } 
    SPI.transfer(0xFE);
  }
  delay(1000);
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  delay(50);
  SPI.transfer16(0b0100000000000011); 
  uint8_t idd = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(idd, HEX);

  // SELECT 14-BIT ADDRESSING
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000000000001); // ADDRESS
  SPI.transfer(0b10000000); // DATA
  digitalWrite(SS, HIGH);

  // PERFORM/MAKE SURE PERFORMED RESET
  //digitalWrite(SS, LOW);
  //delay(100);
  //SPI.transfer16(0b0000000000000000);
  //SPI.transfer(0b10010001);
  //digitalWrite(SS, HIGH);

  // SET SPI NO CRC NO STATUS
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000000010000);
  SPI.transfer(0b00110111);
  digitalWrite(SS, HIGH);

  // PROGRAM CONTINUOUS READ MODE
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000001110001); // ADDRESS
  SPI.transfer16(0b0000000000010000); // DATA
  //SPI.transfer(0b00010000); // PROG CONT READ
  digitalWrite(SS, HIGH);

  // CHANNEL_ENABLE 0,1,2
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000001111001);
  SPI.transfer16(0b0000000000000111);
  //SPI.transfer(0b00000111);
  digitalWrite(SS, HIGH);

  // CH_MAP0
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000010000011);
  SPI.transfer16(0b0000000000000001); // AIN0(+)
  //SPI.transfer(0b00000001); // AIN1(-)
  digitalWrite(SS, HIGH);

  // CH_MAP1
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000010000111);
  SPI.transfer16(0b0000001000000011); // AIN2(+)
  //SPI.transfer(0b00000011); // AIN3(-)
  digitalWrite(SS, HIGH);

  // CH_MAP2
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000010001011);
  SPI.transfer16(0b0000010000000101); // AIN4(+)
  //SPI.transfer(0b00000101); // AIN5(-)
  digitalWrite(SS, HIGH);

  // AFE0 Descending
  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000011000011);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(SS, HIGH); 

  // AFE1
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000011010001);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(SS, HIGH);

  // AFE2
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000011011111);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0000000100110101); // VOLTAGE BIAS REG
  SPI.transfer16(0b0000000000000000);
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  delay(10);
  SPI.transfer16(0b0100000000000011); 
  uint8_t idaa = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(idaa, HEX);

  SPI.endTransaction(); 

}

void loop() {
  if (shouldResetADC) {
        // 1. Pause sampling if necessary (e.g., stop the timer)
        timerStop(timer); 

        // 2. Perform the reset
        Serial.println("Resetting ADC...");
        setup_ADC(); 

        // 3. Clear the flag and resume
        shouldResetADC = false;
        timerStart(timer);
    }
  
  if (buffersReady) {
      analyzeSignal(chX); analyzeSignal(chY); analyzeSignal(chZ);
      portENTER_CRITICAL(&timerMux);
      chX.idx = 0; chY.idx = 0; chZ.idx = 0;
      buffersReady = false;
      portEXIT_CRITICAL(&timerMux);
  }

}
