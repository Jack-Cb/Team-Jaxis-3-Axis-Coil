//Austin Arnold
//4/18/26
//Handles communication between ADC and DSP
//Utulizes SPI1 for communication
#include "axis.h"
#include <math.h>
#include <stdio.h>
#include <cstring>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <linux/spi/spidev.h>
#include <chrono>

#define SPI_DEVICE "/dev/spidev1.1"  //SPI, should be CS1 pin
#define SPI_REGISTER //....? Where do I read from
#define IDENTITY "Frontend Client" 

int main() {
  log(IDENTITY, "Initiating Frontend Client Process...");

  int M = 0;
  int T = 0;
  int H = 0;
  int Th = 0;
  int D10 = 0;
  int D11 = 0;
  int D20 = 0;
  int D21 = 0;

  int clientSocket = CreateSocket(IDENTITY);
  usleep(1000);


  //Configure SPI
  
 


  while(1) {
    //HANDLE SPI 








    //SEND SPI
   // std::string send = buildVector(M,T,H,Th,D10,D11,D20,D1);
    sendSocket(clientSocket, send);
 //   log(IDENTITY, std::to_string(since(start).count()));
    usleep(1000);
  }
  close(clientSocket); 
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
