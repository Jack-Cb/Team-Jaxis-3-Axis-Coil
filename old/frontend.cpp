//This is a client that sends data taken from the SPI interface with the analog frontend
//Take in SPI and send it over TCP to the DSP code
//Just using SPI0 for simplicity
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


#define PORT 20001 //Reference server port
#define SPI_DEVICE "/dev/spidev1.1"  //SPI, should be CS1 pin
#define SPI_REGISTER //....? Where do I read from

int main() {
    
    //Create a socket
    int FrontSocket = socket(AF_INET, SOCK_STREAM, 0);

    //Addressing
    sockaddr_in DSPAddress;
    DSPAddress.sin_family = AF_INET;
    DSPAddress.sin_port = htons(PORT);
    DSPAddress.sin_addr.s_addr = INADDR_ANY;

    //Connect
    connect(FrontSocket, (struct sockaddr*)&DSPAddress, sizeof(DSPAddress));

    //SPI
    int fd = open(SPI_DEVICE, O_RDWR);
    if(fd<0) {
        std::cerr << "SPIDEV1.1 failed to open" << std::endl;
        return -1;
    }

    //For writing
    uint8_t wr_spimode = SPI_MODE_3; //THIS NEEDS TO MATCH THE OTHER DEVICE, using 0 for now //SPI_mode 3 for adc, spi_mode 0 for transmitter
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


    //Send commands to instruct the ADC
    uint8_t data[8]; //Need to find the size of the data frame
    uint8_t command;
    uint8_t writeRegister;
    //Confirm ID?
    command=0b00000000; //Depends on instructions and data frame
    writeRegister=0b00000000;
        //write(fd, writeRegister, data);
    instruct(fd, writeRegister, command);
    //Probably reset
    command=0b00000000; //Depends on instructions and data frame
    writeRegister=0b00000000;
        //write(fd, writeRegister, data);
    instruct(fd, writeRegister, command);
    //Set Continuous Read
    command=0b00000000;
    writeRegister=0b00000000;
        //write(fd, writeRegister, data);
    instruct(fd, writeRegister, command);
    //Set ADC Conversion Mode
    command=0b00000000;
    writeRegister=0b00000000;
    instruct(fd, writeRegister, command);
    //Data is 8 bytes.... 


    //Read Data and send through socket
    while(fcntl(fd, F_GETFD) > 0) {
        read(fd, SPI_REGISTER);
        send(FrontSocket, package, sizeof(package), 0); 
    }

    //Send data
    close(FrontSocket);
}

int instruct(int fd, uint8_t write_register, uint8_t instruction) {
    struct spi_ioc_transfer transfer[1] = {0};

    uint8_t package[2];
    package[0] = write_register;
    package[1] = instruction;

    transfer[0].tx_buf = (__u64)package;
    transfer[0].rx_buf = (__u64)package;
    transfer[0].len = (__u32)sizeof(package);

    int d = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if(d < 0) { std::cerr << "Error in write" << std::endl;}

    return d;
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

