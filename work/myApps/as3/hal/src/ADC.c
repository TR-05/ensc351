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
#include "hal/adc.h"
#include "hal/spi.h"
#include "hal/time.h"
#include <pthread.h>
/*
simple ADC reading via the MCP3208 over spi
*/

static pthread_t ADCThread;
static pthread_mutex_t ADCMutex = PTHREAD_MUTEX_INITIALIZER;

// JOYSTICK CONFIGURATION:
#define DEVICE_PATH "/dev/spidev0.0"
#define SPI_MODE 0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED 250000

static int read_mcp3208_channel(uint8_t channel);

static int JOYSTICK_Y_CHANNEL = 0;
static int ACEL_X_CHANNEL = 0;
static int ACEL_Y_CHANNEL = 0;
static int ACEL_Z_CHANNEL = 0;

static float Joystick_y = 0;
static float Acel_x = 0;
static float Acel_y = 0;
static float Acel_z = 0;

static void *ADC_update_loop()
{

    while (1)
    {
        int raw = read_mcp3208_channel(JOYSTICK_Y_CHANNEL);
        float joystick_y = (raw / 4095.0 - 0.5) * 2;
        time_sleep_ms(0.5);

        raw = read_mcp3208_channel(ACEL_X_CHANNEL);
        float acel_x = (((raw-1320) / 4095.0) * 16); // magic numbers always do the trick :D
        time_sleep_ms(0.5);

        raw = read_mcp3208_channel(ACEL_Y_CHANNEL);
        float acel_y = (((raw-1320) / 4095.0) * 16);
        time_sleep_ms(0.5);

        raw = read_mcp3208_channel(ACEL_Z_CHANNEL);
        float acel_z = (((raw-1320) / 4095.0) * 16);

        pthread_mutex_lock(&ADCMutex);
        Joystick_y = joystick_y;
        Acel_x = acel_x;
        Acel_y = acel_y;
        Acel_z = acel_z;
        pthread_mutex_unlock(&ADCMutex);
        time_sleep_ms(8.5);
    }
    
    ADC_cleanup();
    return 0;
}

static int fd;
void ADC_init(int joystick_channel, int adxl_x, int adxl_y, int adxl_z)
{
    JOYSTICK_Y_CHANNEL = joystick_channel;
    ACEL_X_CHANNEL = adxl_x;
    ACEL_Y_CHANNEL = adxl_y;
    ACEL_Z_CHANNEL = adxl_z;

    spi_initialize(&fd, DEVICE_PATH, SPI_MODE, SPI_BITS_PER_WORD, SPI_SPEED);
    pthread_create(&ADCThread, NULL, ADC_update_loop, NULL);
}

float ADC_read_joystick(void)
{
    float val = 0;
    pthread_mutex_lock(&ADCMutex);
    val = Joystick_y;
    pthread_mutex_unlock(&ADCMutex);
    return val;
}

float ADC_read_acel_x(void)
{
    float val = 0;
    pthread_mutex_lock(&ADCMutex);
    val = Acel_x;
    pthread_mutex_unlock(&ADCMutex);
    return val;
}

float ADC_read_acel_y(void)
{
    float val = 0;
    pthread_mutex_lock(&ADCMutex);
    val = Acel_y;
    pthread_mutex_unlock(&ADCMutex);
    return val;
}

float ADC_read_acel_z(void)
{
    float val = 0;
    pthread_mutex_lock(&ADCMutex);
    val = Acel_z;
    pthread_mutex_unlock(&ADCMutex);
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

void ADC_cleanup(void)
{
    spi_disable(&fd);
    pthread_mutex_destroy(&ADCMutex);
    pthread_cancel(ADCThread);
}
