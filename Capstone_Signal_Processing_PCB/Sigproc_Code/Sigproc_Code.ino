#include <SPI.h>

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_SCLK 18
#define VSPI_CS 5

SPIClass * hspi = NULL;

// AD4170 Register Addresses


// AD4170 Commands
#define COMM_WRITE_MASK 0x00
#define COMM_READ_MASK 0x40

// Function prototypes
void writeAD4170Register(uint16_t address, uint8_t value);
long readAD4170Data();
void AD4170_Init_AIN0_Diff();

void setup() {
  Serial.begin(115200);
  SPI.begin(VSPI_SCLK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

  pinMode(VSPI_CS, OUTPUT);
  digitalWrite(VSPI_CS, HIGH);

  AD4170_Init_AIN0_Diff();

  Serial.println("AD4170 Initialized for AIN0 Differential Continuous Read.");
}

void loop() {
  // Wait for the Data Ready signal to go LOW

  // Data is ready, read the value
  long adc_value = readAD4170Data();

  //Serial.print("ADC Value: ");
  delay(10);
  Serial.println(adc_value);
}

void writeAD4170Register(uint16_t address, uint8_t value) {
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  digitalWrite(VSPI_CS, LOW);

  //SPI.transfer(0b0);
  //SPI.transfer(0b0);
  // Send the command byte: WRITE operation + register address
  /*uint8_t command_first = COMM_WRITE_MASK | ((address >> 8) & 0x3F);
  if(((address >> 8) & 0x3F) != 0){
    SPI.transfer(command_first);
  }
  uint8_t command_second = (address & 0xFF);
  SPI.transfer(command_second);
  */
  SPI.transfer16((0b0011111111111111 & address));
  // Send the data byte
  SPI.transfer(value);

  digitalWrite(VSPI_CS, HIGH);
  SPI.endTransaction();
}

long readAD4170Data() {
  long data = 0;
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  digitalWrite(VSPI_CS, LOW);
  
  byte b1 = SPI.transfer(0x00);
  byte b2 = SPI.transfer(0x00);
  byte b3 = SPI.transfer(0x00);

  digitalWrite(VSPI_CS, HIGH);
  SPI.endTransaction();

  data = (long)b1 << 16 | (long)b2 << 8 | b3;

  // Sign extension for 24-bit value to 32-bit long
  if (data & 0x800000) {
    data |= 0xFF000000;
  }
  
  return data;
}

void AD4170_Init_AIN0_Diff() {
  SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE3));
  digitalWrite(VSPI_CS, LOW);
  for (int i = 0; i < 3; i++) {
    for (int i = 0; i < 7; i++) {
      SPI.transfer(0xFF); 
    } 
    SPI.transfer(0xFE);
  }
  digitalWrite(VSPI_CS, HIGH);
  SPI.endTransaction();
  
  delay(100); // Wait for reset to complete
  writeAD4170Register(0x0070, 0x10);

  writeAD4170Register(0x00C2, 0x60);

  writeAD4170Register(0x006A, 0x01);

}

