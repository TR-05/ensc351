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
#include "hal/joystick.h"
#include "hal/spi.h"
#include "hal/time.h"
#include <pthread.h>
/*
simple joystick reading via the MCP3208 ADC over spi
*/

static pthread_t JoystickThread;
static pthread_mutex_t JoystickMutex = PTHREAD_MUTEX_INITIALIZER;

// JOYSTICK CONFIGURATION:
#define DEVICE_PATH "/dev/spidev0.0"
#define SPI_MODE 0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED 250000

static int read_mcp3208_channel(uint8_t channel);

static int Y_CHANNEL = 0;
static float Joystick_y = 0;
static void *Joystick_update_loop()
{

    while (1)
    {
        int raw = read_mcp3208_channel(Y_CHANNEL);
        float y = (raw / 4095.0 - 0.5) * 2;
        pthread_mutex_lock(&JoystickMutex);
        Joystick_y = y;
        pthread_mutex_unlock(&JoystickMutex);
        time_sleep_ms(10);
    }
    Joystick_cleanup();
    return 0;
}

static int fd;
void Joystick_init(int adc_y_channel)
{
    Y_CHANNEL = adc_y_channel;
    spi_initialize(&fd, DEVICE_PATH, SPI_MODE, SPI_BITS_PER_WORD, SPI_SPEED);
    pthread_create(&JoystickThread, NULL, Joystick_update_loop, NULL);
}

float Joystick_read(void)
{
    float val = 0;
    pthread_mutex_lock(&JoystickMutex);
    val = Joystick_y;
    pthread_mutex_unlock(&JoystickMutex);
    return val;
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

void Joystick_cleanup(void)
{
    spi_disable(&fd);
    pthread_mutex_destroy(&JoystickMutex);
    pthread_cancel(JoystickThread);
}
