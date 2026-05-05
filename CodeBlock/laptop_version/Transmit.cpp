//Austin Arnold
//4/18/2026
//Handles communication between transmitter and DSP code
#include "axis.h"
#include <math.h>
#include <stdio.h>
#include <cstring>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/spi/spidev.h>
#include <chrono>
#include <vector>

#define BUFFER
#define ESP32SPI
#define IDENTITY "Transmit Client"
#define TRNUMBYTES 16
#define RXNUMBYTES 16
#define SPIDEV "/dev/spidev0.0"
#define BITS 8
#define SPEED 1000000

template <
  class result_t = std::chrono::seconds,
  class clock_t = std::chrono::steady_clock,
  class duration_t = std::chrono::seconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
  return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}



using namespace std;

int main() {

  log(IDENTITY, "Initiating Transmit Client Process...");
  int clientSocket = CreateSocket(IDENTITY);

  int count = 0;
  std::string VectorCount;
  auto start = std::chrono::steady_clock::now();

  for(int i = 0; i < 100; i++) {
    std::string count = std::to_string(i);
    std::string buffer = readSocket(clientSocket);
    sendSocket(clientSocket, count);
    log(IDENTITY, buffer);
    usleep(100000);
  }

  log("TIMER", std::to_string(since(start).count()) + " Seconds, Total Vectors Sent: " + VectorCount);
//REMOVING SPI FRAMEWORK
#if 0
  log(IDENTITY, "Initiating Transmit Client Process...");
  int clientSocket = CreateSocket(IDENTITY);

  int count = 0;
  std::string VectorCount;
  auto start = std::chrono::steady_clock::now();

  int fd = -1;
  int mode = SPI_MODE_0;
  int bits = BITS;
  int speed = SPEED;

  volatile uint8_t tx[TRNUMBYTES] = {0};
  volatile uint8_t rx[RXNUMBYTES] = {0};

  fd = open(SPIDEV, O_RDWR);
    if(fd < 0) {
      cout << "Could not open " << SPIDEV << endl;
    }
  cout << "fd: " << fd << endl;
  config(fd, mode, bits, speed);
  

  for(int i = 0; i < 100; i++) {
    std::string count = std::to_string(i);
    std::string buffer = readSocket(clientSocket);
    sendSocket(clientSocket, count);
    log(IDENTITY, buffer);
    usleep(100000);
  }

 // while(1) {

    //HANDLE SPI
for(int i = 0; i < 10000; i++) {
    transrecv(fd, tx, rx, TRNUMBYTES);

    VectorCount = std::to_string(count);
    std::string frequency = bufferString(rx);
    sendSocket(clientSocket, frequency);
    count++;
    std::string buffer = readSocket(clientSocket);
    stringBuffer(tx, buffer);
    log(IDENTITY, buffer);
    log("TIMER", std::to_string(since(start).count()) + " Seconds, Total Vectors Sent: " + VectorCount);

    usleep(100);
  //}
}

#endif
  close(clientSocket);
  return 0;

}

std::string bufferString(const volatile uint8_t *buffer) {
    std::string recieved;
    recieved.reserve(RXNUMBYTES);

    for(int i = 0; i < RXNUMBYTES; i++) {
        recieved.push_back(static_cast<char>(buffer[i]));
    }
    return recieved;
}

void stringBuffer(volatile uint8_t * buffer, std::string& package) {
    for(int i = 0; i< TRNUMBYTES; i++) {
        buffer[i] = static_cast<uint8_t>(package[i]);
    }
}
#if 0
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
#endif

#if 0
int config(int fd, int mode, int bits, int speed) {
  if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) { close(fd); }
  return 0;
}
#endif



int CreateSocket(const std::string & identity) {
   int clientSocket = socket(AF_INET, SOCK_STREAM, 0);   
   fcntl(clientSocket, F_SETFL, O_NONBLOCK);
   if(clientSocket <0) { log(IDENTITY, "Failed to create socket."); exit(EXIT_FAILURE); }
 
   log(IDENTITY, "Frontend attempting to connect...");
   sockaddr_in server;
   server.sin_family = AF_INET;
   server.sin_port = htons(PORT);
   server.sin_addr.s_addr = INADDR_ANY;

   connect(clientSocket, (struct sockaddr*)&server, sizeof(server));
   send(clientSocket, IDENTITY, sizeof(IDENTITY), 0);
   return clientSocket;
}
