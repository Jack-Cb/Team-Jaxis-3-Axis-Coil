#include <linux/spi/spidev.h> 
#include <chrono>

#define SPI_DEVICE "/dev/spidev1.0"
#define IDENTITY "ADC tester"

void setup();
void transrecv(int fd, volatile uint8_t * tx, volatile uint8_t * rx, size_t length);
void setupx

int main () {
    usleep(8000); //not sure why, following ESP32 code
                

    
    return 0;
}

int transrecv(int fd, volatile uint8_t *tx, volatile uint8_t *rx, size_t length) {
  
    struct spi_ioc_transfer transmit{};
    transmit.tx_buf = reinterpret_cast<uintptr_t>(tx);
    transmit.rx_buf = reinterpret_cast<uintptr_t>(rx);
    transmit.len = static_cast<uint32_t>(length);
    transmit.speed_hz =SPEED;
    transmit.bits_per_word = BITS;

    if(ioctl(fd, SPI_IOC_MESSAGE(1), &transmit) < 0) {
      perror("SPI failed to read");
      return -1;
    }
    return 0;
}


