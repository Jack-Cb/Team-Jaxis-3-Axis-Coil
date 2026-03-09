#include <SPI.h>

const int chipSelectPin = 5;

void setup() {
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);
  SPI.begin();
  
  // AD4170 typically uses SPI Mode 3 (CPOL=1, CPHA=1)
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE3));
  
  Serial.begin(115200);
  
}
// MAKE SURE THAT ALL GROUNDS ARE SHARED/SINGLE GROUND FOR ALL
void readChipID() {
  digitalWrite(chipSelectPin, LOW);
  /*for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 7; j++) {
      SPI.transfer(0xFF); 
    } 
    SPI.transfer(0xFE);
  }
  */
  for (int j = 0; j < 7; j++) {
      SPI.transfer(0xFF); 
  }
  SPI.transfer(0xFE);

  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x01);
  SPI.transfer(0b10001000);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x00);
  SPI.transfer(0x10);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x10);
  SPI.transfer(0b00110111);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer(0x01);
  SPI.transfer(0b10000000);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000001110001);
  SPI.transfer(0b00000000);
  SPI.transfer(0b10000000); // PROG CONT READ
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000001111001);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00000001);
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000010000011);
  SPI.transfer(0b00000000);
  SPI.transfer(0b00011000); // DGND
  digitalWrite(chipSelectPin, HIGH);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0000000011000011);
  SPI.transfer(0b00000000);
  SPI.transfer(0b01100000); // DGND
  digitalWrite(chipSelectPin, HIGH);

  // 0x40 (Read bit) | 0x03 (Register address) = 0x43
  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000000011); 
  // Read the 16-bit ID value
  uint8_t msb = SPI.transfer(0x00);
  uint8_t lsb = SPI.transfer(0x00);
  
  digitalWrite(chipSelectPin, HIGH);
  
  uint16_t chipID = (msb << 8) | lsb;
  
  Serial.print("AD4170 Chip ID: 0x");
  Serial.println(chipID, HEX);

  digitalWrite(chipSelectPin, LOW);
  SPI.transfer16(0b0100000000100011); 
  // Read the 16-bit ID value

  uint32_t data = 0;

  data |= (uint32_t)SPI.transfer(0x00) << 16;
  data |= (uint32_t)SPI.transfer(0x00) << 8;
  data |= (uint32_t)SPI.transfer(0x00);
  
  //Serial.println(data);
  float voltage = (data * 5) / 16777216.0;
  Serial.println(voltage);


  digitalWrite(chipSelectPin, HIGH);
  
  // Expected ID for AD4170-4 is typically 0xXXXX (Check datasheet for exact silicon revision code)
}

void loop() {
  readChipID();
}
