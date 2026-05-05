//This is a server that sends data taken from the SPI interface with the analog frontend
//Take in SPI and send it over TCP to the DSP code
//Just using SPI0 for simplicity

#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
