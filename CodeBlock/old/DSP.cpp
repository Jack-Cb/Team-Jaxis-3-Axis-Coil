#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>


#define PORT 20001
#define BUFFER 1024
#define pi 3.14159265358979
#define e 2.71828182846

using namespace std;

void handleSim(int socket) {
  
}

//Need mutex
void handleFrontend(int socket) {

}

//Need mutex
void handleTransmit(int socket) {

}

//1 is simulation, 2 is frontend, 3 is transrecv
int IDCLient(int socket) {
    char identity[4];
    recv(socket, identity, 4, 0);

    int v = static_cast<int>(identity);
    return v;
}

int main() {

    
//this is from when I was managing a single socket    
#if 0  
    // Create a socket
    int DSPSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (DSPSocket < 0) { perror("DSP socket failed"); exit(EXIT_FAILURE);}
//    char buffer[BUFFER] = {0};


    //Address
    struct sockaddr_in DSP_address;
    socklen_t DSP_len = sizeof(DSP_address);
    DSP_address.sin_family = AF_INET;
    DSP_address.sin_port = htons(PORT);
    DSP_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    //Configure options
    int opt = 1;
    setsockopt(DSPSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(DSPSocket, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    //Binding socket to address
    if(bind(DSPSocket, (struct sockaddr*)&DSP_address, sizeof(DSP_address)) < 0) { perror("bind failed"); exit(EXIT_FAILURE); }


    //Listen to the socket, maybe change the backlog parameter if necessary
    if(listen(DSPSocket, 16) < 0) { perror("listen failure"); exit(EXIT_FAILURE); }
    
    //Accept
    struct sockaddr_in signal_address;
    socklen_t signal_len = sizeof(signal_address);

    int clientSocket = accept(DSPSocket, (struct sockaddr*)&DSP_address, &DSP_len); 
    if (clientSocket < 0) { perror("Accepting failed"); exit(EXIT_FAILURE);}
#endif

//multiple sockets + multithreading
//Need a socket for frontend and one for transmit
    
    int DSPserver = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in connections{};
    connections.sin_family = AF_INET;
    connections.sin_addr.s_addr = INADDR_ANY;
    connections.port = htons(PORT);

    bind(DSPserver, (sockaddr*)connections, sizeof(connections));
    listen(DSPserver, 5);

    cout << "Launching DSP server" << endl;

    while(true) {
        int client = accept(DSPserver, nullptr, nullptr);

        switch (IDClient) {
            //Simulation Client
            case 1:
                cout << "Creating simulation thread" << endl;
                thread sim(handleSim, client).detach();
            //Frontend Client
            case 2:
                cout << "Creating frontend thread" << endl;
                thread front(handleFrontend, client).detach();
            //Transmit Client
            case 3:
                cout << "Creating transmit thread" << endl;
                thread transmit(handleTransmit, client).detach();
        }

    }






//DSP CODE

    //this could totally be cleaned up but I'm exhausted. come back later
    float data[5] = {0}; //0:X 1:Y 2:Z 3:n 4:TickPeriod
    //float data_1[4] = {0};
   // float data_2[4] = {0};
    //float output[4] = {0};
    //float output_1[4] = {0};
    //float output_2[4] = {0};
    float Ioutput[3] = {0};
    float Qoutput[3] = {0};
    float Amplitude[3] = {0};
    float Phase[3] = {0};
    int readBytes = sizeof(data);
    char * i = (char*)&data;
    float frequency =  50000; //this will be input from the user through spi
                            
    float cutoff = 0.0005; //set cutoff 1/25th of carrier
    float outputFrequency = 0.01; //5 vectors per second...
    float d = pow(e, -2 * pi * cutoff);
//    float b = 1-d;
    float b = 0.03;
    float counter = 0;
                                   //
    /* Come back for second order later
    w_0 = 2*pi*frequency/sampleRate;
    q   =  
    b_0 = (1-cos(w_0))/2;
    b_1 = (1-;
    b_2 = ;
    a_0 = ;
    a_1 = ;
    a_2 = ;
    */
    while(true) {
    /* Come back for second order later
        for(int i = 0;i <3;i++) {
            data_2[i]=data_1[i];
        }
        for(int i = 0;i <3;i++) {
            data_1[i]=data[i];
        }
    */
     
/*
        int count = 0;        
        while(count < readBytes) {
            ssize_t numChars = read(clientSocket, i + count, readBytes - count); //Read from the socket :>)
           // data[numChars] = '\0';
            if(numChars == 0) { cout << "Connection Closed" << endl; break; } 
            if(numChars < 0) { perror("Lost connection"); exit(EXIT_FAILURE); break; } 
            count += numChars; //read a char
        }

        if(count < readBytes) {break;}
*/
        //Chunk keeps a history of the past 5 states for FIR filtering
        //Shift
       // cout << "X: " << data[0] << " n: " << data[3] - 2 << endl;
       // cout << "Y: " << data[1] << " n: " << data[3] - 1 << endl;
       // cout << "Z: " << data[2] << " n: " << data[3] << endl;


        //Lock in detection is what I need I think...
        //generate reference signals
        //Maybe make these doubles...
    

        //sin (in phase I think but not sure)
        //Shoud data[3] be divided by sample rate   
        cout << endl;
        cout << "Size of X Input: " << sizeof(data[0]) << endl;
        cout << "Size of Y Input: " << sizeof(data[1]) << endl;
        cout << "Size of Z Input: " << sizeof(data[2]) << endl;
        float sampleRate = 1/data[4];
        float DownsampleRate = sampleRate / outputFrequency;
        float ref_sinX = sin(2*pi*frequency/sampleRate *data[3]);   
        //cos (out of phase)
        //This could be done with an array but I'm on four hours of sleep and I can't get this to sync
        
        float ref_cosX = cos(2*pi*frequency/sampleRate *data[3]);
        float ref_sinY = sin(2*pi*frequency/sampleRate *(data[3]+1)); 
        float ref_cosY = cos(2*pi*frequency/sampleRate *(data[3]+1));
        float ref_sinZ = sin(2*pi*frequency/sampleRate *(data[3]+2));  
        float ref_cosZ = cos(2*pi*frequency/sampleRate *(data[3]+2));
        Ioutput[0] = data[0] * ref_sinX;
        Ioutput[1] = data[1] * ref_sinY;
        Ioutput[2] = data[2] * ref_sinZ;
        Qoutput[0] = data[0] * ref_cosX;
        Qoutput[1] = data[1] * ref_cosY;
        Qoutput[2] = data[2] * ref_cosZ;


        //IIR Low pass filter
        Ioutput[0] += (b * (ref_cosX - Ioutput[0]));
        Ioutput[1] += (b * (ref_cosY - Ioutput[1]));
        Ioutput[2] += (b * (ref_cosZ - Ioutput[2]));
        Qoutput[0] += (b * (ref_sinX - Qoutput[0]));
        Qoutput[1] += (b * (ref_sinY - Qoutput[1]));
        Qoutput[2] += (b * (ref_sinZ - Qoutput[2]));

#if 1
        //downsample
//        if(counter > DownsampleRate) {
  //         counter = 0;
            //calculate a vector A /_ phusing some triangulation
            //float magnitude = sqrt(pow(Amplitude[0],2) + pow(Amplitude[1],2) + pow(Amplitude[2],2)
            Amplitude[0] = sqrt(pow(Ioutput[0],2) + pow(Qoutput[0],2));
            Amplitude[1] = sqrt(pow(Ioutput[1],2) + pow(Qoutput[1],2));
            Amplitude[2] = sqrt(pow(Ioutput[2],2) + pow(Qoutput[2],2));
            Phase[0] = atan2(Qoutput[0], Ioutput[0]);
            Phase[1] = atan2(Qoutput[1], Ioutput[1]);
            Phase[2] = atan2(Qoutput[2], Ioutput[2]);

            for(int i = 0; i <3; i++) {
                cout << "Server side Amplitude (Magnitude) : " << Amplitude[i] << " @ n = " << data[3]+i << endl;
            }
             for(int i = 0; i <3; i++) {
                cout << "Server side Phase : " << Phase[i] << " @ n = " << data[3]+i << endl;
            }

            float Z = sqrt(pow(Amplitude[0],2) + pow(Amplitude[1],2) + pow(Amplitude[2],2));
            float phi =(Phase[0] - Phase[2]); //idk 
            float theta = (Phase[1] - Phase[2]);
             cout << "Output Rate: " << 1/(3*data[4] * 1000000) << " MBps"  << endl;
             cout << "Output Vector Polar Coordinatez <Z, phi, theta>" << endl;
             cout << "< " << Z << " , " << phi << " , " << theta << " >" << endl;
             cout << "Size of Z: " << sizeof(Z) << endl;
             cout << "Size of Theta: " << sizeof(theta)<< endl;
             cout << "Size of Phi: " << sizeof(phi) << endl << endl << endl;

       // } 
    //    counter++;
#endif



#if 0
        cout << "SigX: " << data[0] << endl;
        cout << "SigY: " << data[1] << endl;
        cout << "SigZ: " << data[2] << endl;
        cout << "n: " << data[3] << endl;
        cout << "b: " << b << endl;
        cout << "sample rate: " << sampleRate << endl;
        cout << "frequency: " << frequency << endl;
        cout << "DownsampleRate: " << DownsampleRate << endl;
        cout << "XSin reference: " << ref_sinX << endl;
        cout << "YSin reference: " << ref_sinY << endl;
        cout << "ZSin reference: " << ref_sinZ << endl;
        cout << "XCos reference: " << ref_cosX << endl;
        cout << "YCos reference: " << ref_cosY << endl;
        cout << "ZCos reference: " << ref_cosZ << endl;
        cout << "AmplitudeX: " << Amplitude[0] << endl;
        cout << "AmplitudeY: " << Amplitude[1] << endl;
        cout << "AmplitudeZ: " << Amplitude[2] << endl;

#endif

//SO all this is almost there, but the issue is that it's not sinced up with n, i.e X, Y, Z all occur at n instead of n, n+1, n+2
//I am too tired to fix it so coming back later

        /* Save this for later when you switch to an array
        //multiply the signal by the reference at the selected frequency
        for(int i = 0; i<3; i++) {
            Ioutput[i] = data[i] * ref_sin;
            Qoutput[i] = data[i] * ref_cos;
            cout << "n:  " << data[3] << endl;
            cout << "ref_sin: " << ref_sin << endl;
            cout << "ref_cos: " << ref_cos << endl;
            cout << "Data: " << data[0] << endl;
            cout << "Ioutput: " << Ioutput[0] << endl;
            cout << "Qoutput: " << Qoutput[0] << endl;
        }
*/
        //See if you can make a better filter later, but right now an IIR will suffice
     /* 
        //Second order IIR filter has the difference equation y[n] = b_0x[n]+b_1x[n-1]+b_2x[n-2]-a1y[n-1]-a2y[n-2]
        for(int i = 0; i <=3; i++) {
            output[i] = b_0*data[i]+b_1*data_1[i]+b_2*data_2[i]+a_1*output_1[i]-a_2*output_2[i];     
        }
        //too complicated for tonight
    */
        //First order IIR low pass filter has the difference equation y[n] = y[n-1] +b(x[n] - y[n-1]) 
        //b = d - 1 where d is the decay factor, d = e^-2pifc
        //set cutoff below carrier frequency, maybe 1/30th of the carrier. This is a total guess...
        //
    /*Save this for later when you switch to an array
        for(int i = 0; i <3; i++) {
            Ioutput[i] += (b * (ref_cos - Ioutput[i]));
            Qoutput[i] += (b * (ref_sin - Qoutput[i]));

            //find amplitude and phase
            //Move this later, it's not optimal here
            Amplitude[i] = sqrt(pow(Ioutput[i],2) + pow(Qoutput[i],2));
            Phase[i] = atan2(Qoutput[i], Ioutput[i]);
           // cout << "Amplitude: " << Amplitude[i] << endl;
           // cout << "Phase: " << Phase[0] << endl;
        }
*/
#if 0
        ++counter;
        //downsample
        if(counter > DownsampleRate) {
           counter = 0;
            //calculate a vector A /_ phusing some triangulation
            //float magnitude = sqrt(pow(Amplitude[0],2) + pow(Amplitude[1],2) + pow(Amplitude[2],2)
            for(int i = 0; i <3; i++) {
                cout << "Amplitude!! : " << Amplitude[i] << endl;
                cout << "Phase!! : " << Phase[i] << endl;
            }
      
        }
#endif
    }
    
    //read
    //Can block everything if not careful, needs more error handling
#if 0
    while(1) { 
        ssize_t numChars = read(clientSocket, buffer, BUFFER); //Read from the socket :>)
        buffer[numChars] = '\0';
        if(numChars == 0) { cout << "Connection Closed" << endl; break; } 
        data_1[0] = data[0]; data_1[1] = data[1]; data_1[2] = data[2]; data_1[3] = data[3];        if(numChars < 0) { perror("Lost connection"); exit(EXIT_FAILURE); break; } 
    
        Sample _Sample = arrangeBuffer(buffer);
        
        cout << "Message from client: " << buffer << endl;
        
    }
#endif
    close(clientSocket);
    close(DSPSocket);
    return 0;
}

