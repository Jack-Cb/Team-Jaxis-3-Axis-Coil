#include <SPI.h>

#define SS 10
#define MOSI 11
#define MISO 13
#define SCK 12

void setup() {
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
  SPI.begin(SCK, MISO, MOSI, SS);
  
  // AD4170 typically uses SPI Mode 3 (CPOL=1, CPHA=1)
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  
  Serial.begin(115200);
  setup_ADC();

  digitalWrite(SS, LOW);
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


  delay(1000);
  
}
// FOR PROGRAMMING: HAD 5V CONNECTED TO SUPPLY, 3.3V TO THE ESP'S 3.3V OUTPUT, GND OF THE 3.3 SUPPLY CONNECTED TO GND NET
void setup_ADC() {
  digitalWrite(SS, LOW);
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
  digitalWrite(SS, HIGH);

  // SELECT 14-BIT ADDRESSING
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000000000001); // ADDRESS
  SPI.transfer(0b10000000); // DATA
  digitalWrite(SS, HIGH);

  // PERFORM/MAKE SURE PERFORMED RESET
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000000000000);
  SPI.transfer(0b10010001);
  digitalWrite(SS, HIGH);

  // SET SPI NO CRC NO STATUS
  digitalWrite(SS, LOW);
  SPI.transfer16(0b0000000000010000);
  SPI.transfer(0b00110111);
  digitalWrite(SS, HIGH);

  // PROGRAM CONTINUOUS READ MODE
  digitalWrite(SS, LOW);
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
  SPI.endTransaction(); 

}

void loop() {

  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000000011); 
  uint8_t id = SPI.transfer(0x00);
  digitalWrite(SS, HIGH);
  Serial.print("ID_VAL: "); // SHOULD BE 7 (CHIP_TYPE)
  Serial.println(id, HEX);

  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000101010); 
  uint32_t data0 = 0;
  data0 |= (uint32_t)SPI.transfer(0x00) << 16;
  data0 |= (uint32_t)SPI.transfer(0x00) << 8;
  data0 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data0);
  float voltage0 = (data0 * 5) / 16777216.0;
  Serial.print(", Voltage X: ");
  Serial.println(voltage0, 10);
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000101110); 
  uint32_t data1 = 0;
  data1 |= (uint32_t)SPI.transfer(0x00) << 16;
  data1 |= (uint32_t)SPI.transfer(0x00) << 8;
  data1 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data1);
  float voltage1 = (data1 * 5) / 16777216.0;
  Serial.print(", Voltage Y: ");
  Serial.println(voltage1, 10);
  digitalWrite(SS, HIGH);

  digitalWrite(SS, LOW);
  SPI.transfer16(0b0100000000110010); 
  uint32_t data2 = 0;
  data2 |= (uint32_t)SPI.transfer(0x00) << 16;
  data2 |= (uint32_t)SPI.transfer(0x00) << 8;
  data2 |= (uint32_t)SPI.transfer(0x00);
  Serial.print(data2);
  float voltage2 = (data2 * 5) / 16777216.0;
  Serial.print(", Voltage Z: ");
  Serial.println(voltage2, 10);
  digitalWrite(SS, HIGH);

}
