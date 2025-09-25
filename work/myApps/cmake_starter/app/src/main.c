#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "badmath.h"
#include "hal/button.h"
#include "hal/led.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// SPI device path
static const char *device = "/dev/spidev0.0";
// SPI mode: Mode 0 (CPOL=0, CPHA=0)
static uint8_t mode = 0;
// Bits per word
static uint8_t bits = 8;
// SPI speed in Hz
static uint32_t speed = 1000000; // 1 MHz

// Function to perform an SPI transfer
static int spi_transfer(int fd, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t length) {
    // Structure for the SPI transfer message
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx_buffer, // Transmit buffer
        .rx_buf = (unsigned long)rx_buffer, // Receive buffer
        .len = length,                      // Length of the transfer
        .speed_hz = speed,                  // SPI speed
        .bits_per_word = bits,              // Bits per word
    };

    // Perform the transfer using ioctl
    int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("Can't send SPI message");
    }
    return ret;
}

// Function to read a single channel from the MCP3208 ADC
static int read_mcp3208_channel(int fd, uint8_t channel) {
    if (channel > 7) {
        fprintf(stderr, "Invalid channel specified. MCP3208 has 8 channels (0-7).\n");
        return -1;
    }

    // The MCP3208 command for single-ended mode is a 3-byte transfer.
    // Tx: [Start bit], [SGL/DIF bit + Channel], [Dummy byte]
    // Rx: [Junk], [Data MSB], [Data LSB]
    uint8_t tx_buffer[3] = {0};
    uint8_t rx_buffer[3] = {0};
    int bytes_read;
    int adc_value;

    // Byte 1: Start bit (0x01)
    tx_buffer[0] = 0x01;
    
    // Byte 2: Single-ended bit (0x80) and channel bits
    // The single-ended bit `0x80` (0b10000000) is combined with the channel
    // number shifted to the most significant nibble.
    tx_buffer[1] = 0x80 | (channel << 4);

    // Byte 3: Dummy byte to clock out the remaining data
    tx_buffer[2] = 0x00;

    // Perform the transfer
    bytes_read = spi_transfer(fd, tx_buffer, rx_buffer, sizeof(tx_buffer));

    if (bytes_read > 0) {
        // The 12-bit value is split across the two received bytes (rx_buffer[1] and rx_buffer[2]).
        // The upper 4 bits of the value are in rx_buffer[1], and the lower 8 bits are in rx_buffer[2].
        // The `& 0x0F` masks off the junk bits in the first data byte.
        adc_value = ((rx_buffer[1] & 0x0F) << 8) | rx_buffer[2];
        return adc_value;
    }

    return -1; // Return -1 on failure
}

int main() {
    int fd;
    int ret = 0;

    // Open the SPI device file
    fd = open(device, O_RDWR);
    if (fd < 0) {
        perror("Error opening SPI device");
        return 1;
    }

    // Set SPI mode
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        perror("Can't set SPI mode");
        close(fd);
        return 1;
    }

    // Set bits per word
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        perror("Can't set bits per word");
        close(fd);
        return 1;
    }

    // Set SPI speed
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        perror("Can't set SPI speed");
        close(fd);
        return 1;
    }

    printf("SPI device opened successfully at %s\n", device);
    printf("SPI Mode: %d, Bits per Word: %d, Speed: %d Hz\n", mode, bits, speed);
    printf("Reading MCP3208 channels 6 and 7. Press Ctrl+C to exit...\n");

    // Main loop to read ADC data
    while (1) {
        int ch6_value = read_mcp3208_channel(fd, 5);
        usleep(5000); // 50ms delay
        int ch7_value = read_mcp3208_channel(fd, 6);


        // y =  2556, 3071, 2048
        int max = 3071;
        int mid = 2557;
        int min = 2048;
        float range = (max-min)/2.0f;
        float y = (ch6_value-mid) / range;
        //float x = (ch7_value-mid) / range;
        if (ch6_value != -1 && ch7_value != -1) {
            printf("Y: %.2f, X: %d\n", y, ch7_value);
        }

        // Wait a bit before the next read
        usleep(50000); // 50ms delay
    }

    // Close the SPI device file
    close(fd);
    return 0;
}
