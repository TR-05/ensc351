#include <stdio.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include "hal/led.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "hal/joystick.h"
#include "hal/spi.h"

/*
simple joystick reading via the MCP3208 ADC over spi
*/

// JOYSTICK CONFIGURATION:
#define X_CHANNEL 6
#define Y_CHANNEL 5
#define DEVICE_PATH "/dev/spidev0.0"
#define SPI_MODE 0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED 250000

// modifiable joystick calibration values, used for normaling output from -1 to 1
static int x_max_val = 3071;
static int x_mid_val = 2550;
static int x_min_val = 2048;
static int y_max_val = 3071;
static int y_mid_val = 2556;
static int y_min_val = 2048;

// JOYSTICK methods
static int fd;
int joystick_initialize(void)
{
    spi_initialize(&fd, DEVICE_PATH, SPI_MODE, SPI_BITS_PER_WORD, SPI_SPEED);
    printf("Reading MCP3208 channels\n");
    return 0;
}

static int read_mcp3208_channel(uint8_t channel)
{
    if (channel > 7)
    {
        fprintf(stderr, "Invalid channel specified. MCP3208 has 8 channels (0-7).\n");
        return -1;
    }

    uint8_t tx[3] = {(uint8_t)(0x06 | ((channel & 0x04) >> 2)),
                     (uint8_t)((channel & 0x03) << 6),
                     0x00};
    uint8_t rx[3] = {0};

    int bytes_read = spi_transfer(fd, tx, rx, sizeof(tx));

    if (bytes_read > 0)
    {
        // The 12-bit value is split across the two received bytes (rx_buffer[1] and rx_buffer[2]).
        // The upper 4 bits of the value are in rx_buffer[1], and the lower 8 bits are in rx_buffer[2].
        // The `& 0x0F` masks off the junk bits in the first data byte.
        return ((rx[1] & 0x0F) << 8) | rx[2];
    }

    return -1; // Return -1 on failure
}

// gets the raw 12 bit data from the joystick
float joystick_get_x_raw(void)
{
    usleep(500); // let adc calculate
    return (float)(read_mcp3208_channel(X_CHANNEL));
}
float joystick_get_y_raw(void)
{
    usleep(500); // let adc calculate
    return (float)(read_mcp3208_channel(Y_CHANNEL));
}

// gets joystick data normalized from -1 to 1, assumes that the rest position is exactly halfway between max and min for simplicity
float joystick_get_x_normalized(void)
{
    return (joystick_get_x_raw() - x_mid_val) / ((x_max_val - x_min_val) / 2.0f);
}
float joystick_get_y_normalized(void)
{
    return (joystick_get_y_raw() - y_mid_val) / ((y_max_val - y_min_val) / 2.0f);
}

// override the default calibration values, may be useful?
void joystick_configure_x(int max, int mid, int min)
{
    x_max_val = max;
    x_mid_val = mid;
    x_min_val = min;
}
void joystick_configure_y(int max, int mid, int min)
{
    y_max_val = max;
    y_mid_val = mid;
    y_min_val = min;
}

void joystick_disable(void)
{
    spi_disable(&fd);
}
