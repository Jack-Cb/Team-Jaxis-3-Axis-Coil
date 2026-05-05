#include <math.h>
#include <stdio.h>
#include <cstring>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdint>
#include <linux/spi/spidev.h>
#include <chrono>

#define PORT 20001
#define TRNUMBYTES 8
#define RXNUMBYTES 8
#define SPIDEV "/dev/spidev0.0"
#define BITS 8
#define SPEED 10000000 //20000000 works...
using namespace std;
//since
template <
  class result_t = chrono::seconds,
  class clock_t = chrono::steady_clock,
  class duration_t = chrono::seconds
>
auto since(chrono::time_point<clock_t, duration_t> const& start)
{
  return chrono::duration_cast<result_t>(clock_t::now() - start);
}

int config(int fd, uint8_t mode, uint8_t bits, uint32_t speed);
int transrecv(int fd, volatile uint8_t *tx,  volatile uint8_t *rx, size_t length);
void printbuffer(volatile uint8_t *buffer, size_t length);

int main () {
  int fd = -1;
  uint8_t mode = SPI_MODE_0;
  uint8_t bits = BITS;
  uint32_t speed = SPEED;

  auto start = chrono::steady_clock::now();

  volatile uint8_t tx[TRNUMBYTES] = {0};
  volatile uint8_t rx[RXNUMBYTES] = {0};

  int count = 0;
  int theta = 0;
  int phi = 0;
  //create some test values
  tx[0] = 0x00;

#if 0
  tx[0] = 10;
  tx[1] = 11;
  tx[2] = 0;
  tx[3] = 0;
  tx[4] = 0;
  tx[5] = 20;
  tx[6] = 0;
  tx[7] = 30;
#endif
  fd = open(SPIDEV, O_RDWR);
    if(fd < 0) {
      cout << "Could not open " << SPIDEV << endl;
    }
  cout << "fd: " << fd << endl;
  config(fd, mode, bits, speed);
  

  int vectors = 0;
  while(true) {
    count = count % 30;
    count++;
    //theta = theta % 255;
    phi = phi % 255;
    //theta++;
    //phi++;
  for(int i = 0; i < TRNUMBYTES; i++) {
    tx[i] = 30;
  }
  tx[4] = 0;
  tx[5] = theta;
  tx[6] = 0;
  tx[7] = phi;
   transrecv(fd, tx, rx, TRNUMBYTES);
  // cout << "TX: " << printbuffer(tx , TRNUMBYTES) << endl;
  
  //cout << "RX: " << printbuffer(rx, RXNUMBYTES) << endl; 
#if 0
    if(rx[0] != 0xAA) {
        usleep(1000);
        continue;
    }
#endif
    vectors++;

    if(1) {
    //if(vectors % 100 == 1) {

      cout << "Vectors Output: " << vectors << endl;
      cout << "Elapsed Time: " << since(start).count() << endl;
 
      cout << "TX: "; printbuffer(tx , TRNUMBYTES); cout << endl;
      cout << "RX: "; printbuffer(rx, RXNUMBYTES); cout << endl;
    }
    usleep(10000);
  }
}
#if 0 
void packageTx(int magnitude, int azimuth, int altitude, uint8_t * Tx) {
    tx[0] = 
    tx[3] = azimuth - 255;
    tx[4] = 
    tx[5] =
    tx[6] = 
    tx[7] =  
}
#endif
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


int config(int fd, uint8_t mode, uint8_t bits, uint32_t speed) {
  if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
    perror("SPI_IOC_WR_MODE"); 
    close(fd);
    return -1;
  }
/*
  if(ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) {

    perror("SPI_IOC_RD_MODE"); 
    close(fd);
    return -1;
  }
*/
  if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) {

    perror("SPI_IOC_WR_BITS"); 
    close(fd);
    return -1;
  }
/*
  if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) {

    perror("SPI_IOC_RD_BITS"); 
    close(fd);
    return -1;
  }
*/
  if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {

    perror("SPI_IOC_WR_SPEED"); 
    close(fd);
    return -1;
  }
/*
  if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
    perror("SPI_IOC_RD_SPEED"); 
    close(fd);
    return -1;
  }
*/
  return 0;
}

void printbuffer(volatile uint8_t *buffer, size_t length) {
  for(int i = 0; i < length; i++) {
    cout << static_cast<int>(buffer[i]) << " " ; 
  }
  return ;
}
