//this is a client/server for sending out distance vectors on SPI to the transmit recieve block
//It also takes input from SPI for a frequency selection
//Using SPI1 for simplicity
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

#define PORT 20001
#define BUFFER
#define ESP32SPI
#define SPIDEV "/dev/spidev0.1"

using namespace std;

int main() {
    
    //Socket
    int TransRecv = socket(AF_INET, SOCK_STREAM, 0);
    if(TransRecv < 0) { perror("TransRecv socket failed"); exit(EXIT_FAILURE);}

    //Address
    struct sockaddr_in DSPaddress;
    TRaddress.sin_family = AF_INET;
    TRaddress.sin_port = htons(PORT);
    TRaddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    //Configure 
    //int opt = 1;
    //setsockopt(TransRecv, SOL_SOCKET, SO_REUSEADRR, &opt, sizeof(opt));
    //setsockopt(TransRecv, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));

    //Connect socket
    connect(TransRecv, (struct sockaddr*)&DSPaddress, sizeof(DSPaddress));


    //Configure SPI
    //SPI
    int fd = open(SPI_DEVICE, O_RDWR);
    if(fd<0) {
        std::cerr << "SPIDEV1.1 failed to open" << std::endl;
        return -1;
    }

    //For writing
    uint8_t wr_spimode = SPI_MODE_0; //THIS NEEDS TO MATCH THE OTHER DEVICE, using 0 for now //SPI_mode 3 for adc, spi_mode 0 for transmitter
    if(ioctl(fd, SPI_IOC_WR_MODE, &wr_spimode) < 0) {
        std::cerr << "Failed to set wr_SPI_MODE" << std::endl;
    }
    uint8_t wr_bits = 8; 
    if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &wr_bits) < 0) {
      std::cerr << "Failed to set wr_Bits_Per_Word" << std::endl;
    }
    uint32_t wr_speed = 1000000;
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &wr_speed) < 0) {
        std::cerr << "Failed to set wr_speed" << std::endl;
    }

    //For reading
    uint8_t rd_spimode = SPI_MODE_0;
    if(ioctl(fd, SPI_IOC_RD_MODE, &rd_spimode) < 0) {
        std::cerr << "Failed to set rd_SPI_MODE" << std::endl;
    }
    uint8_t rd_bits = 8;
    if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &rd_bits) < 0) {
        std::cerr << "Failed to set rd_bits" << std::endl;
    }
    uint32_t rd_speed = 1000000;
    if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &rd_speed) < 0) {
        std::cerr << "Failed to set rd_speed" << std::endl;
    }

    int frequency = 0;
    int vector = 0; //THIS DEFINITELY WILL BE A DIFFERENT TYPE, NEEDS TO MATCH WITH THE HANDLER FROM THE DSP SERVER

    bool Toggle = true;
    while(Toggle) {
        //Get SPI frequency info
        frequency = read(fd, ESP32SPI); //This returns raw bytes
                                        //
                                        //Need to probably convert to string
        //Send frequency info
        send(TransRecv, frequency, strlen(frequency), 0);
        //recieve vector data
        recv(TransRecv, vector, strlen(vector), 0);
        //Send SPI vector data
        write(fd, ESP32SPI, vector);
    }
    close(TransRecv);
    return 0;
}
    
int read(int fd, uint8_t read_register) {
    struct spi_ioc_transfer recieve[1] = {0};

    uint8_t reg = read_register | 0x80;
    uint8_t package[8];
    package[0] = reg;
    //dataframe is 8 bytes. so register, then 8? should the array be [9]...
    for(int i = 1; i <=7; i++) {
        package[i] = 0x00;
    }
    recieve[0].tx_buf = (__u64)package;
    recieve[0].rx_buf = (__u64)package;
    recieve[0].len = (__u32)sizeof(package);

    int d = ioctl(fd, SPI_IOC_MESSAGE(1), &recieve);
    if(d < 0) { std::cerr << "Error in read" << std::endl;}

    return package[1];
}

int write(int fd, uint8_t write_register, uint8_t &data) {
    struct spi_ioc_transfer transfer[1] = {0};

    uint8_t package[2];
    package[0] = write_register;
    for(int i = 1; i<= 7, i++) {
        package[i] = data[i-1];
    }
    transfer[0].tx_buf = (__u64)package;
    transfer[0].rx_buf = (__u64)package;
    transfer[0].len = (__u32)sizeof(package);

    int d = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if(d < 0) { std::cerr << "Error in write" << std::endl;}

    return d;
}

