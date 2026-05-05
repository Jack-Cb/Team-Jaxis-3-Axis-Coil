//Testing SPI interface 
#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>


#define DEVICE_PATH "/dev/spidev1.1" //Pretty sure this is wired up to CS 1, but check if there is issues

using namespace std;

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if(fd < 0) {
        cerr << "SPIDEV1.1 failed to open" << endl;
        return -1;
    }


    //For writing
    uint8_t wr_spimode = SPI_MODE_0; //THIS NEEDS TO MATCH THE OTHER DEVICE, using 0 for now //SPI_mode 3 for adc, spi_mode 0 for transmitter
    if(ioctl(fd, SPI_IOC_WR_MODE, &wr_spimode) < 0) {
        cerr << "Failed to set wr_SPI_MODE" << endl;
    }
    uint8_t wr_bits = 8; 
    if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &wr_bits) < 0) {
        cerr << "Failed to set wr_Bits_Per_Word" << endl;
    }
    uint32_t wr_speed = 1000000;
    if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &wr_speed) < 0) {
        cerr << "Failed to set wr_speed" << endl;
    }

    //For reading
    uint8_t rd_spimode = SPI_MODE_0;
    if(ioctl(fd, SPI_IOC_RD_MODE, &rd_spimode) < 0) {
        cerr << "Failed to set rd_SPI_MODE" << endl;
    }
    uint8_t rd_bits = 8;
    if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &rd_bits) < 0) {
        cerr << "Failed to set rd_bits" << endl;
    }
    uint32_t rd_speed = 1000000;
    if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &rd_speed) < 0) {
        cerr << "Failed to set rd_speed" << endl;
    }

    //uint8_t length = 2; //Set for package size
    //uint8_t transmit[length];
    //uint8_t recieve[length];
    //memset(transmit, 0, sizeof(transmit));
    //memset(recieve, 0, sizeof(recieve));
//Need to set the WHOAMI register as the first byte.......

    uint8_t ATMEGA_SPI = 0x00;
    uint8_t package = 0xf2;

    while(true) {
        write(fd, ATMEGA_SPI, package);
        sleep(0.5);
    }
}


int read(int fd, uint8_t read_register) {
    struct spi_ioc_transfer recieve[1] = {0};

    uint8_t reg = read_register | 0x80;
    uint8_t package[2];
    package[0] = reg;
    package[1] = 0x00;
    recieve[0].tx_buf = (__u64)package;
    recieve[0].rx_buf = (__u64)package;
    recieve[0].len = (__u32)sizeof(package);

    int d = ioctl(fd, SPI_IOC_MESSAGE(1), &recieve);
    if(d < 0) { cerr << "Error in read"  << endl;}
    
    return package[1];
}

int write(int fd, uint8_t write_register, uint8_t data) {
    struct spi_ioc_transfer transfer[1] = {0};

    uint8_t package[2];
    package[0] = write_register;
    package[1] = data;

    transfer[0].tx_buf = (__u64)package;
    transfer[0].rx_buf = (__u64)package;
    transfer[0].len = (__u32)sizeof(package);
    //might need to include speed, bits, etc

    int d = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
    if(d < 0) { cerr << "Error in write" << endl; }

    return d;
}
