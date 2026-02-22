#include <SPI.h>

#define SS 10
#define MOSI 11
#define MISO 13
#define SCK 12

void setup() {
  // Initialize SPI with default VSPI pins
  SPI.begin(SCK, MISO, MOSI, SS); 
  pinMode(SS, OUTPUT);
  digitalWrite(SS, HIGH);
}

void loop() {
  // 1. Define clock speed (e.g., 1 MHz), bit order, and SPI mode
  // 2. Start transaction to begin generating the signal
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  
  digitalWrite(SS, LOW);    // Select device
  SPI.transfer(0x00);       // Trigger SCLK for 8 clock cycles
  digitalWrite(SS, HIGH);   // Deselect device
  
  SPI.endTransaction();     // Release the bus         // Wait 1 second
}