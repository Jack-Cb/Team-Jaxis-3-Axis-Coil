#include <SPI.h>
#include <Arduino.h>

#define VSPI_MISO 19
#define VSPI_MOSI 23
#define VSPI_SCLK 18
#define VSPI_SS   5

SPIClass * vspi = new SPIClass(VSPI);

void setup_ADC() {
  
  vspi->beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 7; j++) {
      vspi->transfer(0xFF); 
    } 
    vspi->transfer(0xFE);
  }
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  delay(1000);
  /*
  for (int j = 0; j < 7; j++) {
      vspi->transfer(0xFF); 
  }
  vspi->transfer(0xFE);
  vspi->transfer(0b00);
  digitalWrite(vspi->pinSS(), HIGH);
  */
  
  // SELECT 14-BIT ADDRESSING
  vspi->beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000000000001); // ADDRESS
  vspi->transfer(0b10000000); // DATA
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  

  // SET SPI NO CRC NO STATUS
  vspi->beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000000010000);
  vspi->transfer(0b00110111);
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  

  // CHANNEL_ENABLE 0,1,2
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  //delay(1000);
  vspi->transfer16(0b0000000001111001);
  vspi->transfer16(0b1111111111111111);
  //vspi->transfer(0b00000111);
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  
  // CH_MAP0
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000010000011);
  vspi->transfer16(0b0000000000011000); // AIN0(+)
  //vspi->transfer(0b00000001); // AIN1(-)
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();
  
  // CH_MAP1
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  delay(10);
  vspi->transfer16(0b0000000010000111);
  vspi->transfer16(0b0000001000011000); // AIN2(+)
  //vspi->transfer(0b00000011); // AIN3(-)
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  // CH_MAP2
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000010001011);
  vspi->transfer16(0b0000010000011000); // AIN4(+)
  //vspi->transfer(0b00000101); // AIN5(-)
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  // AFE0 Descending
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  //delay(10);
  vspi->transfer16(0b0000000011000011);
  vspi->transfer16(0b0000000001100000);
  //vspi->transfer(0b01101001); 
  digitalWrite(vspi->pinSS(), HIGH); 
  vspi->endTransaction();

  // AFE1
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000011010001);
  vspi->transfer16(0b0000000001101001);
  //vspi->transfer(0b01101001); 
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  // AFE2
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000011011111);
  vspi->transfer16(0b0000000001101001);
  //vspi->transfer(0b01101001); 
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000100110101);
  vspi->transfer16(0b0000000000000000);
  //vspi->transfer(0b01101001); 
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  /*
  // PROGRAM CONTINUOUS READ MODE
  vspi->beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0000000001110001); // ADDRESS
  vspi->transfer16(0b0000000000000000); // DATA
  //vspi->transfer(0b00010000); // PROG CONT READ
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();
  */



  // READING REGISTERS
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000001111001); 
  uint16_t enz = 0;
  enz |= (uint16_t) vspi->transfer(0x00) << 8; 
  enz |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("CH_ENABLE: "); // READING 
  Serial.println(enz, BIN);
  vspi->endTransaction();
  
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000010000011); 
  uint16_t ena = 0;
  ena |= (uint16_t) vspi->transfer(0x00) << 8; 
  ena |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("CH0: "); // READING 
  Serial.println(ena, BIN);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000010000111); 
  uint16_t enb = 0;
  enb |= (uint16_t) vspi->transfer(0x00) << 8; 
  enb |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("CH1: "); // READING 
  Serial.println(enb, BIN);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000010001011); 
  uint16_t enc = 0;
  enc |= (uint16_t) vspi->transfer(0x00) << 8; 
  enc |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("CH2: "); // READING 
  Serial.println(enc, BIN);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000011000011); 
  uint16_t en = 0;
  en |= (uint16_t) vspi->transfer(0x00) << 8; 
  en |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("AFE0: "); // READING AFE ENABLE
  Serial.println(en, BIN);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000011010001); 
  uint16_t en1 = 0;
  en1 |= (uint16_t) vspi->transfer(0x00) << 8; 
  en1 |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("AFE1: "); // READING AFE ENABLE
  Serial.println(en1, BIN);
  vspi->endTransaction();

  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000011011111); 
  uint16_t en2 = 0;
  en2 |= (uint16_t) vspi->transfer(0x00) << 8; 
  en2 |= (uint16_t) vspi->transfer(0x00); 
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("AFE2: "); // READING AFE ENABLE
  Serial.println(en2, BIN);
  vspi->endTransaction();

  
  delay(1000);
  
}

void setup() {
  Serial.begin(115200);
  vspi->begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_SS);
  pinMode(vspi->pinSS(), OUTPUT);
  delay(1000);

  // AD4170 uses SPI Mode 3 (CPOL=1, CPHA=1)
  //vspi->beginTransaction(SPISettings(1000, MSBFIRST, SPI_MODE3));
  setup_ADC();
  //vspi->endTransaction();

}


void loop() {
  vspi->beginTransaction(SPISettings(125000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  //delay(100);
  vspi->transfer16(0b0100000000000011); 
  uint32_t id = vspi->transfer16(0x00);
  //uint32_t id2 = vspi->transfer(0x00);
  digitalWrite(vspi->pinSS(), HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(id, HEX);
  //Serial.println(id2, BIN);
  vspi->endTransaction();
  
  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000000101010); 
  uint32_t data0 = 0;
  data0 |= (uint32_t)vspi->transfer(0x00) << 16;
  data0 |= (uint32_t)vspi->transfer(0x00) << 8;
  data0 |= (uint32_t)vspi->transfer(0x00);
  Serial.print(data0);
  float voltage0 = (data0 * 5) / 16777216.0;
  Serial.print(", Voltage X: ");
  Serial.println(voltage0, 10);
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();


  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000000101110); 
  uint32_t data1 = 0;
  data1 |= (uint32_t)vspi->transfer(0x00) << 16;
  data1 |= (uint32_t)vspi->transfer(0x00) << 8;
  data1 |= (uint32_t)vspi->transfer(0x00);
  Serial.print(data1);
  float voltage1 = (data1 * 5) / 16777216.0;
  Serial.print(", Voltage Y: ");
  Serial.println(voltage1, 10);
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();


  vspi->beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE3));
  digitalWrite(vspi->pinSS(), LOW);
  vspi->transfer16(0b0100000000110010); 
  uint32_t data2 = 0;
  data2 |= (uint32_t)vspi->transfer(0x00) << 16;
  data2 |= (uint32_t)vspi->transfer(0x00) << 8;
  data2 |= (uint32_t)vspi->transfer(0x00);
  Serial.print(data2);
  float voltage2 = (data2 * 5) / 16777216.0;
  Serial.print(", Voltage Z: ");
  Serial.println(voltage2, 10);
  digitalWrite(vspi->pinSS(), HIGH);
  vspi->endTransaction();

  
}
