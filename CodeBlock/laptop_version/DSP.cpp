//Austin Arnold
//4/18/2026
//Handles values from frontend, frequency selections from transmitter, and packages vectors for output to the transmitter
#include "axis.h"
#include <cstring>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sstream>
#include <math.h>

#define BUFFER 1024
#define pi 3.14159265358979
#define e 2.71828182846
#define IDENTITY "DSP Server"

std::string globalBuffer;

int main() {
  log(IDENTITY, "Initiating DSP server process...");
  int listening_socket = 0;
  int max_threads = 3;
  int thread_count = 0;
  listening_socket = configureServer();

  //Get connections
  while(1) {
    int connecting_socket = accept(listening_socket, nullptr, nullptr);
    std::string CLIENTID = IDClient(connecting_socket);
    log(IDENTITY, "DSP MAIN THREAD ID: " + IDThread());
    if(thread_count <= max_threads && connecting_socket >= 0) {
      if(strcmp(CLIENTID.c_str(), "Transmit Client") == 0) {
          log(IDENTITY, "Transmit Client Connected...");
          thread_count++;
          std::thread Transmit(handleTransmit, connecting_socket);
          Transmit.detach();
      }  
      else if(strcmp(CLIENTID.c_str(), "Frontend Client") == 0) {
          log(IDENTITY, "Frontend Client Connected...");
          thread_count++;
          std::thread Frontend(handleFrontend, connecting_socket);
          Frontend.detach();
      }
      else if(strcmp(CLIENTID.c_str(), "Simulation Client") == 0) {
          log(IDENTITY, "Simulation Client Connected...");
          thread_count++;
          std::thread Simulation(handleSimulator, connecting_socket);
          Simulation.detach();
      }

      else {
          log(IDENTITY, "Unknown Connection");
          log(IDENTITY, readSocket(connecting_socket));  
       }
    }
    else if(thread_count > max_threads) {
        std::cerr << "Max threads reached" << std::endl;
    }
  }


  std::cout << "DSP Socket Status: " << listening_socket << std::endl;
  return 0;
  
  close(listening_socket);
}


std::string dsp(std::string frequency, std::string vector) {
    return "Implement";
}

void handleTransmit(int socket) {
    log(IDENTITY, "Created Transmit thread at " + IDThread());
    while(1) {
        std::string frequency = readSocket(socket);
        std::string vector = dsp(frequency, globalBuffer);
        sendSocket(socket, vector);
    }
}
void handleFrontend(int socket) {
    log(IDENTITY, "Created Frontend thread at " + IDThread());
    while(1) {
        globalBuffer = readSocket(socket);
    }
}
void handleSimulator(int socket) {
    log(IDENTITY, "implement...");
}

std::string IDThread() {
    std::ostringstream id;
    id << std::this_thread::get_id();
    return id.str();
}



std::string IDClient(int socket) {
  char buffer[100] = {0};
  int returned = recv(socket, buffer, sizeof(buffer) -1, 0);
  if(returned > 0) {
    std::string name(buffer, returned);
    return name; 
  } else {
    return "Failed to recieve a proper client ID";
  } 
}

//Create a socket to listen at incoming connections
int configureServer() {
    int DSPserver = socket(AF_INET, SOCK_STREAM, 0);
    if (DSPserver < 0) { std::cerr << "DSP socket failed" << std::endl; exit(EXIT_FAILURE);}
   
    int opt = 1;
    setsockopt(DSPserver, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in connections{};
    connections.sin_family = AF_INET;
    connections.sin_addr.s_addr = INADDR_ANY;
    connections.sin_port = htons(PORT);

    bind(DSPserver, (struct sockaddr*)&connections, sizeof(connections));
    listen(DSPserver, 5);
    return DSPserver;
}
