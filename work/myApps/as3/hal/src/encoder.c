#include "hal/encoder.h"
#include "hal/gpio.h"
#include "hal/time.h"
#include "../../app/include/beatbox.h"
#include <pthread.h>
#include <stdio.h>

static pthread_t EncoderThread;
static pthread_mutex_t EncoderMutex = PTHREAD_MUTEX_INITIALIZER;
static int Apin = 0;
static int Bpin = 0;
static int Buttonpin = 0;
static int count = 0;
static int pressing = 0;
static int stopping = 0;
static void *Encoder_update_loop()
{
    bool lastA = 1;
    int pulses = 0;
    int lastPressing[5] = {0};
    while (1)
    {
        if (stopping) {
            break;
        }
        bool A = gpio_read(Apin);
        bool B = gpio_read(Bpin);
        bool buttonPressing = !gpio_read(Buttonpin);
        int bpm = 0;

        if (A != lastA && A == 1)
        {
            if (B != A) // clockwise
            {
                pulses++;
                bpm += 5;
            }
            else // counter clockwise;
            {
                pulses--;
                bpm -= 5;
            }
        }
        lastA = A;

        if (bpm != 0)
        {
            bpm += beatbox_bpm();
            if (bpm < 40)
            {
                bpm = 40;
            }
            if (bpm > 300)
            {
                bpm = 300;
            }
            if (bpm != beatbox_bpm())
            {
                beatbox_set_bpm(bpm);
            }
        }

        if (buttonPressing && !lastPressing[0] && !lastPressing[1] && !lastPressing[2] && !lastPressing[3] && !lastPressing[4])
        {
            int track = beatbox_track();
            track++;
            if (track > 2)
            {
                track = 0;
            }
            beatbox_set_track(track);
        }

        lastPressing[4] = lastPressing[3];
        lastPressing[3] = lastPressing[2];
        lastPressing[2] = lastPressing[1];
        lastPressing[1] = lastPressing[0];
        lastPressing[0] = buttonPressing;

        pthread_mutex_lock(&EncoderMutex);
        count = pulses;
        pressing = buttonPressing;
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
    stopping = 1;
    pthread_join(EncoderThread, NULL);
    pthread_mutex_destroy(&EncoderMutex);
    return 0;
}
