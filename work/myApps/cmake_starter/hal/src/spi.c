#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "hal/spi.h"

/*
My Notes:
generic spi involves the following steps:
Config:
    Open spidev file
    configure mode, bits per word, and speed

Perform transfer:
    define length of transfer
    create arrays of uint8_t[LENGTH] tx and rx, aka transfer bits and recieve bits
    set transfer bits to whatever message you need
    put data into a specific struct
    pass struct reference into ioctl and it will modify rx with the response bits

process recieved data (bit mask and rearange to extract what you need based on datasheet)
Only device specific parts are config values, tx, and post processing of rx

close spi file when done using* depending on what devices are connected may require funkiness 
to access multiple things at once with different config setting
*/

// default values, shouldn't ever be used as must initialze before doing a transfer but idk
static uint8_t mode = 0;        // SPI mode: Mode 0 (CPOL=0, CPHA=0)
static uint8_t bits = 8;        // Bits per word
static uint32_t speed = 250000; // Hz

int spi_initialize(int* fd, char* device, uint8_t spi_mode, uint8_t bits_per_word, uint32_t speed_hz) {
    *fd = open(device, O_RDWR);
    if (*fd < 0)
    {
        perror("open");
        return 1;
    }
    if (ioctl(*fd, SPI_IOC_WR_MODE, &spi_mode) == -1)
    {
        perror("mode");
        return 1;
    }
    mode = spi_mode;
    if (ioctl(*fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1)
    {
        perror("bpw");
        return 1;
    }
    bits = bits_per_word;
    if (ioctl(*fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed_hz) == -1)
    {
        perror("speed");
        return 1;
    }
    speed = speed_hz;

    printf("SPI device opened successfully at %s\n", device);
    printf("SPI Mode: %d, Bits per Word: %d, Speed: %d Hz\n", mode, bits, speed);
    return 0;
}

int spi_transfer(int fd, uint8_t *tx, uint8_t *rx, size_t length)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = length,
        .speed_hz = speed,
        .bits_per_word = bits,
        .cs_change = 0};

    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < -1)
    {
        perror("Can't send SPI message");
    }
    return ret;
}

void spi_disable(int* fd) {
    close(*fd);
}