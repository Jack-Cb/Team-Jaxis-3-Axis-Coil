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
#include <sstream>

#define BUFFER
#define ESP32SPI
#define IDENTITY "Transmit Client"
#define SPIDEV "/dev/spidev0.0"
#define BITS 8
#define SPEED 10000000

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
  long long VectorCount = 0;
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
  


  while(1) {
    VectorCount++;
    std::string vectors = to_string(VectorCount);
    log(IDENTITY, "Vectors sent: " + vectors);
    std::string time = to_string(since(start).count());
    log(IDENTITY, "Time Elapsed: " + time);

    //Read Socket
    std::string buffer = readSocket(clientSocket); 
     
    log(IDENTITY, buffer);
    //Package MOSI
    
    cout << "TX: "; printbuffer(tx , TRNUMBYTES); cout << endl;
    stringBuffer(tx, buffer);
      
    transrecv(fd, tx, rx, TRNUMBYTES);
    
    //Package MISO
    std::string recieved = bufferString(rx);

    cout << "RX: "; printbuffer(rx, RXNUMBYTES); cout << endl;
    //log(IDENTITY, "RECIEVED: " + recieved);

    //Send Socket
    sendSocket(clientSocket, recieved); 
    usleep(1000);
  }


  close(clientSocket);
  return 0;

}

void printbuffer(volatile uint8_t *buffer, size_t length) {
  for(int i = 0; i < length; i++) {
    cout << static_cast<int>(buffer[i]) << " "; 
    //cout << buffer[i] << " ";
  }
  return ;
}


std::string bufferString(const volatile uint8_t *buffer) {
    std::string recieved;
    recieved.reserve(RXNUMBYTES);

   

    for(int i = 0; i < 4; i++) {
        char c = static_cast<char>(buffer[i]) + 48;    
        recieved.push_back(c);
    }
    return recieved;
#if 0
    for(int i = 0; i < RXNUMBYTES; i++) {
        recieved.push_back(static_cast<int>(buffer[i]));
    }
    return recieved;
#endif
    
    
}

void stringBuffer(volatile uint8_t * buffer, const std::string& package) {

    std::stringstream ss(package);
    std::string delim;

    int i = 0;

    while(std::getline(ss,delim, ',') && i < TRNUMBYTES) {

        if(!delim.empty() && delim.back() == ';') {
            delim.pop_back();
        }

        if(delim.empty()) {
            continue;
        }

        try {
            int value = std::stoi(delim);
            buffer[i] = static_cast<uint8_t>(value);
            i++;
        } catch(...) {

        }

    }
    /*
    for(int i = 0; i< TRNUMBYTES; i++) {
        buffer[i] = static_cast<uint8_t>(package[i]) - '0';
    }
    */


    //Package integers between delimeters
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


int config(int fd, int mode, int bits, int speed) {
  if(ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_MODE, &mode) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) { close(fd); }
  if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) { close(fd); }
  return 0;
}



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
