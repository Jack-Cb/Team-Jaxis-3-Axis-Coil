#include <SPI.h>

const int chipSelectPin = 5;

void setup() {
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);
  SPI.begin();
  
  // AD4170 typically uses SPI Mode 3 (CPOL=1, CPHA=1)
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  
  Serial.begin(115200);
  setup_ADC();

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000001111001); 
  uint16_t enz = 0;
  enz |= (uint16_t) SPI.transfer(0x00) << 8; 
  enz |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("CH_ENABLE: "); // READING 
  Serial.println(enz, BIN);
  
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000010000011); 
  uint16_t ena = 0;
  ena |= (uint16_t) SPI.transfer(0x00) << 8; 
  ena |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("CH0: "); // READING 
  Serial.println(ena, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000010000111); 
  uint16_t enb = 0;
  enb |= (uint16_t) SPI.transfer(0x00) << 8; 
  enb |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("CH1: "); // READING 
  Serial.println(enb, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000010001011); 
  uint16_t enc = 0;
  enc |= (uint16_t) SPI.transfer(0x00) << 8; 
  enc |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("CH2: "); // READING 
  Serial.println(enc, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000011000011); 
  uint16_t en = 0;
  en |= (uint16_t) SPI.transfer(0x00) << 8; 
  en |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("AFE0: "); // READING AFE ENABLE
  Serial.println(en, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000011010001); 
  uint16_t en1 = 0;
  en1 |= (uint16_t) SPI.transfer(0x00) << 8; 
  en1 |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("AFE1: "); // READING AFE ENABLE
  Serial.println(en1, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000011011111); 
  uint16_t en2 = 0;
  en2 |= (uint16_t) SPI.transfer(0x00) << 8; 
  en2 |= (uint16_t) SPI.transfer(0x00); 
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("AFE2: "); // READING AFE ENABLE
  Serial.println(en2, BIN);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer32(0b0100000011001010); 
  uint32_t off0 = 0;
  off0 |= (uint32_t)SPI.transfer(0x00) << 16;
  off0 |= (uint32_t)SPI.transfer(0x00) << 8;
  off0 |= (uint32_t)SPI.transfer(0x00);
  Serial.print("OFFSET0: ");
  Serial.println(off0, BIN);
  digitalWrite(chipSelectPin, HIGH);


  delay(1000);
  
}
// FOR PROGRAMMING: HAD 5V CONNECTED TO SUPPLY, 3.3V TO THE ESP'S 3.3V OUTPUT, GND OF THE 3.3 SUPPLY CONNECTED TO GND NET
void setup_ADC() {
  digitalWrite(chipSelectPin, LOW);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 7; j++) {
      SPI.transfer(0xFF); 
    } 
    SPI.transfer(0xFE);
  }
  /*for (int j = 0; j < 7; j++) {
      SPI.transfer(0xFF); 
  }
  SPI.transfer(0xFE);
  SPI.transfer(0b00);*/
  digitalWrite(chipSelectPin, HIGH);

  // SELECT 14-BIT ADDRESSING
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000000000001); // ADDRESS
  SPI.transfer(0b10000000); // DATA
  digitalWrite(chipSelectPin, HIGH);

  // PERFORM/MAKE SURE PERFORMED RESET
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000000000000);
  SPI.transfer(0b10010001);
  digitalWrite(chipSelectPin, HIGH);

  // SET SPI NO CRC NO STATUS
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000000010000);
  SPI.transfer(0b00110111);
  digitalWrite(chipSelectPin, HIGH);

  // PROGRAM CONTINUOUS READ MODE
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000001110001); // ADDRESS
  SPI.transfer16(0b0000000000010000); // DATA
  //SPI.transfer(0b00010000); // PROG CONT READ
  digitalWrite(chipSelectPin, HIGH);

  // CHANNEL_ENABLE 0,1,2
  digitalWrite(chipSelectPin, LOW);
  delay(10);
  SPI.transfer16(0b0000000001111001);
  SPI.transfer16(0b0000000000000111);
  //SPI.transfer(0b00000111);
  digitalWrite(chipSelectPin, HIGH);

  // CH_MAP0
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000010000011);
  SPI.transfer16(0b0000000000011000); // AIN0(+)
  //SPI.transfer(0b00000001); // AIN1(-)
  digitalWrite(chipSelectPin, HIGH);

  // CH_MAP1
  digitalWrite(chipSelectPin, LOW);
  delay(10);
  SPI.transfer16(0b0000000010000111);
  SPI.transfer16(0b0000001000011000); // AIN2(+)
  //SPI.transfer(0b00000011); // AIN3(-)
  digitalWrite(chipSelectPin, HIGH);

  // CH_MAP2
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000010001011);
  SPI.transfer16(0b0000010000011000); // AIN4(+)
  //SPI.transfer(0b00000101); // AIN5(-)
  digitalWrite(chipSelectPin, HIGH);

  /* AFE0 Ascending
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011000010);
  SPI.transfer16(0b0110100100000000);
  //SPI.transfer(0b01101001); 
  digitalWrite(chipSelectPin, HIGH); */

  // AFE0 Descending
  digitalWrite(chipSelectPin, LOW);
  delay(10);
  SPI.transfer16(0b0000000011000011);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(chipSelectPin, HIGH); 

  // AFE1
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011010001);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(chipSelectPin, HIGH);

  // AFE2
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011011111);
  SPI.transfer16(0b0000000001101001);
  //SPI.transfer(0b01101001); 
  digitalWrite(chipSelectPin, HIGH);

  // OFFSET0
  digitalWrite(chipSelectPin, LOW);
  delay(10);
  SPI.transfer16(0b0000000011001010);
  SPI.transfer(0b00110000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  digitalWrite(chipSelectPin, HIGH);

  // OFFSET1
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011011000);
  SPI.transfer(0b11010000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  digitalWrite(chipSelectPin, HIGH);

  // OFFSET2
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011100110);
  SPI.transfer(0b11010000);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000000);
  digitalWrite(chipSelectPin, HIGH);

}

void loop() {

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000000011); 
  uint8_t id = SPI.transfer(0x00);
  digitalWrite(chipSelectPin, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(id, HEX);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000101010); 
  uint32_t data0 = 0;
  data0 |= (uint32_t)SPI.transfer(0x00) << 16;
  data0 |= (uint32_t)SPI.transfer(0x00) << 8;
  data0 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data0);
  float voltage0 = (data0 * 5) / 16777216.0;
  Serial.print(", Voltage X: ");
  Serial.println(voltage0, 4);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000101110); 
  uint32_t data1 = 0;
  data1 |= (uint32_t)SPI.transfer(0x00) << 16;
  data1 |= (uint32_t)SPI.transfer(0x00) << 8;
  data1 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data1);
  float voltage1 = (data1 * 5) / 16777216.0;
  Serial.print(", Voltage Y: ");
  Serial.println(voltage1, 4);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000110010); 
  uint32_t data2 = 0;
  data2 |= (uint32_t)SPI.transfer(0x00) << 16;
  data2 |= (uint32_t)SPI.transfer(0x00) << 8;
  data2 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data2);
  float voltage2 = (data2 * 5) / 16777216.0;
  Serial.print(", Voltage Z: ");
  Serial.println(voltage2, 4);
  digitalWrite(chipSelectPin, HIGH);

}
