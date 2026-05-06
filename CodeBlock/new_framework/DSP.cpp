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
#define NUMSAMPLES 1000

const float FS = 1000000; //Sample rate of the ADC, depends on the socket rate too.... Timing is going to a big focus here
const float alpha = 0.01; //Controls the low pass filter
float F01 = 0; //Set by the frequency select
float phase1 = 0;
float phi1 = 2.0f*pi*F01/FS;
float IXLP1 = 0;
float QXLP1 = 0;
float IYLP1 = 0;
float QYLP1 = 0;
float IZLP1 = 0;
float QZLP1 = 0;
float F02 = 0; 
float phase2 = 0;
float phi2 = 2.0f*pi*F02/FS;
float IXLP2 = 0;
float QXLP2 = 0;
float IYLP2 = 0;
float QYLP2 = 0;
float IZLP2 = 0;
float QZLP2 = 0;
//really bad but we are running out of time
std::string outputBuffer;
std::string inputBuffer1;
std::string inputBuffer2;

int main() {
  log(IDENTITY, "Initiating DSP server process...");
  int listening_socket = 0;
  int max_threads = 5;
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
      else if(strcmp(CLIENTID.c_str(), "Frontend Client 1") == 0) {
          log(IDENTITY, "Frontend Client 1 Connected...");
          thread_count++;
          std::thread Frontend1(handleFrontend1, connecting_socket);
          Frontend1.detach();
      }
      else if(strcmp(CLIENTID.c_str(), "Frontend Client 2") == 0) {
          log(IDENTITY, "Frontend Client 2 Connected...");
          thread_count++;
          std::thread Frontend2(handleFrontend2, connecting_socket);
          Frontend2.detach();
      }
      else if(strcmp(CLIENTID.c_str(), "Calculator") == 0) {
          log(IDENTITY, "Calculator Connected...");
          thread_count++;
          std::thread Calculator(handleCalculator, connecting_socket);
          Calculator.detach();
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


void setF01(std::string frequency) {
    F01=std::stof(frequency);
}
void setF02(std::string frequency) {
    F02=std::stof(frequency);
}

void unpackVector(float * X, float * Y, float * Z, std::string vector) {
    



}


void handleCalculator(int socket) {
    std::string vector1 = dsp1(inputBuffer1);
    std::string vector2 = dsp2(inputBuffer2);

    sendSocket(socket, vector1);
    sendSocket(socket, vector2);
    std::string outputVector = readSocket(socket);
    outputBuffer = package(outputVector);
}

std::string package(std::string outputVector) {
    //Need to send back a string in the form
    //[0] Magnitude
    //[1] tens place
    //[2] hundreds place
    //[3] thousandths place 
    //[4] Degree p1 Azimuth = p1 + p2
    //[5] Degree p2 
    //[6] Degree p3 Altitude = p3 + p4
    //[7] Degree p4 
};


void handleTransmit(int socket) {
    log(IDENTITY, "Created Transmit thread at " + IDThread());
    while(1) {
        std::string frequency = readSocket(socket);
        setF01(frequency);
        setF02(frequency);
        sendSocket(socket, outputBuffer);
        log(IDENTITY, "Frequency: " + frequency);
    }
}

void handleFrontend1(int socket) {
    log(IDENTITY, "Created Frontend 1 thread at " + IDThread());
    while(1) {
        inputBuffer1 = readSocket(socket);
        //log(IDENTITY, "Vector: " + globalBuffer);
    }
}
void handleFrontend2(int socket) {
    log(IDENTITY, "Created Frontend 2 thread at " + IDThread());
    while(1) {
        inputBuffer2 = readSocket(socket);
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
std::string dsp1(std::string vector) {

    std::string DSPvector;
    float X = 0;
    float Y = 0;
    float Z = 0;
    
    //convert strings to floats
    //F01 = stof(frequency);   
    phi1 = 2.0f*pi*F01/FS;

    //create X Y Z samples
    unpackVector(&X, &Y, &Z, vector);

    //create a reference signal
    float cosine = cos(phase1);
    float sine = sin(phase1);
 
    //mix 
    float IXL1 = X * cosine;
    float QXL1 = X * sine;
    float IYL1 = Y * cosine;
    float QYL1 = Y * sine;
    float IZL1 = Z * cosine;
    float QZL1 = Z * sine;
    
    //low pass filter
    IXLP1 += alpha * (IXL1 - IXLP1);
    QXLP1 += alpha * (QXL1 - QXLP1); 
    IYLP1 += alpha * (IYL1 - IYLP1);
    QYLP1 += alpha * (QYL1 - QYLP1);
    IZLP1 += alpha * (IZL1 - IZLP1);
    QZLP1 += alpha * (QZL1 - QZLP1);

    //Add phase
    phase1 += phi1;
    if(phase1 > 2.0f * pi){ phase1 -= 2.0f * pi; }
    
    //Compute amplitude
    float AX = sqrt(IXLP1*IXLP1 + QXLP1*QXLP1);
    float AY = sqrt(IYLP1*IYLP1 + QYLP1*QYLP1);
    float AZ = sqrt(IZLP1*IZLP1 + QZLP1*QZLP1);
    //package back into transmit format


    return DSPvector;
}


std::string dsp2(std::string vector) {

    std::string DSPvector;
    float X = 0;
    float Y = 0;
    float Z = 0;
    
    //convert strings to floats
   //Do this elsewhere
   //F02 = stof(frequency);   
    phi2 = 2.0f*pi*F02/FS;

    //create X Y Z samples
    unpackVector(&X, &Y, &Z, vector);

    //create a reference signal
    float cosine = cos(phase2);
    float sine = sin(phase2);
 
    //mix 
    float IXL2 = X * cosine;
    float QXL2 = X * sine;
    float IYL2 = Y * cosine;
    float QYL2 = Y * sine;
    float IZL2 = Z * cosine;
    float QZL2 = Z * sine;
    
    //low pass filter
    IXLP2 += alpha * (IXL2 - IXLP2);
    QXLP2 += alpha * (QXL2 - QXLP2); 
    IYLP2 += alpha * (IYL2 - IYLP2);
    QYLP2 += alpha * (QYL2 - QYLP2);
    IZLP2 += alpha * (IZL2 - IZLP2);
    QZLP2 += alpha * (QZL2 - QZLP2);

    //Add phase
    phase2 += phi2;
    if(phase2 > 2.0f * pi){ phase2 -= 2.0f * pi; }
    
    //Compute amplitude
    float AX = sqrt(IXLP2*IXLP2 + QXLP2*QXLP2);
    float AY = sqrt(IYLP2*IYLP2 + QYLP2*QYLP2);
    float AZ = sqrt(IZLP2*IZLP2 + QZLP2*QZLP2);
    //package back into transmit format


    return DSPvector;
}

