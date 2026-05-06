#ifndef AXIS_H
#define AXIS_H
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 20001

//Helpers
inline void log(const std::string & identity, const std::string & message) {
   std::cout << identity << "[PID: " << getpid() << "] Message: " << message << std::endl;
   std::cout.flush();
}
std::string bufferString(const volatile uint8_t *buffer);
void stringBuffer(volatile uint8_t * buffer, std::string& string);

//Create a socket
int CreateSocket(const std::string & identity);
//Handle SPI
int config(int fd, int mode, int bits, int speed);
int transrecv(int fd, volatile uint8_t *tx, volatile uint8_t *rx, size_t length);
void printbuffer(volatile uint8_t *buffer, size_t length);


//IPC
void handleTransmit(int socket);
void handleFrontend(int socket);
void handleSimulator(int socket); //Could also just tie this to frontend
std::string IDClient(int socket);
std::string IDThread();
int configureServer();
std::string readSocket(int socket);
void sendSocket(int socket, std::string buffer);

//threads
void acceptConnection(void);

//calculations
std::string dsp(std::string frequency, std::string vector);
void vector(void);

std::string readSocket(int socket) {
  char buffer[72] = {0};
  ssize_t returned = recv(socket, buffer, sizeof(buffer)-1, 0);
  if(returned > 0) {
    std::string package(buffer, returned);
    return package;
  } else if (returned == 0) {
    return "Connection Closed";
  } else {
    return "Socket unreadable";
  }
}
void sendSocket(int socket, std::string package) {
    ssize_t flag = send(socket, package.c_str(), package.size(), 0);
    if(flag < 0) {
        perror("Send failed");
    }
}


#endif

