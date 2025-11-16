#include "hal/encoder.h"
#include "hal/gpio.h"
#include "hal/time.h"

#include <pthread.h>
#include <stdio.h>

static pthread_t EncoderThread;
static pthread_mutex_t EncoderMutex = PTHREAD_MUTEX_INITIALIZER;
static int Apin = 0;
static int Bpin = 0;
static int Buttonpin = 0;
static int count = 0;
static int pressing = 0;
static void *Encoder_update_loop()
{
    bool lastA = 1;
    int pulses = 0;

    while (1)
    {
        bool A = gpio_read(Apin);
        bool B = gpio_read(Bpin);
        bool button = gpio_read(Buttonpin);

        if (A != lastA && A == 1)
        {
            if (B != A) // clockwise
            {
                pulses++;
            }
            else // counter clockwise;
            {
                pulses--;
            }
        }
        lastA = A;
        pthread_mutex_lock(&EncoderMutex);
        count = pulses;
        pressing = !button;
        pthread_mutex_unlock(&EncoderMutex);
        time_sleep_ms(.5);
    }
    Encoder_cleanup();
    return 0;
}

int Encoder_init(int pinA, int pinB, int pinButton)
{
    Apin = pinA;
    Bpin = pinB;
    Buttonpin = pinButton;
    gpio_initialize(Apin);
    gpio_initialize(Bpin);
    gpio_initialize(Buttonpin);
    pthread_create(&EncoderThread, NULL, Encoder_update_loop, NULL);

    return 0;
}

int Encoder_read(void)
{
    int val = 0;
    pthread_mutex_lock(&EncoderMutex);
    val = count;
    pthread_mutex_unlock(&EncoderMutex);
    return val;
}

int Encoder_button_pressing(void)
{
    int val = 0;
    pthread_mutex_lock(&EncoderMutex);
    val = pressing;
    pthread_mutex_unlock(&EncoderMutex);
    return val;
}

int Encoder_cleanup(void)
{
    pthread_mutex_destroy(&EncoderMutex);
    pthread_cancel(EncoderThread);
    return 0;
}
