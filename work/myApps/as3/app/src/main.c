#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "hal/audioMixer.h"
#include "hal/time.h"
#include "hal/encoder.h"
#include "hal/adc.h"

#include "beatbox.h"
#include "periodTimer.h"
#include "udp.h"

static int stopping = 0;
void* printLoop() {
    while (1)
    {
        if (stopping) {
            break;
        }
        Period_statistics_t b;
        Period_getStatisticsAndClear(PERIOD_EVENT_FILL_BUFFER, &b);
        Period_statistics_t a;
        Period_getStatisticsAndClear(PERIOD_EVENT_READ_ACCELEROMETER, &a);

        printf("M%d %dbpm vol:%d", beatbox_track(), beatbox_bpm(), beatbox_volume());
        printf(" Audio[%.3f, %.3f] avg %.3f/%d", b.minPeriodInMs, b.maxPeriodInMs, b.avgPeriodInMs, b.numSamples);
        printf(" Accel[%.3f, %.3f] avg %.3f/%d\n",  a.minPeriodInMs, a.maxPeriodInMs, a.avgPeriodInMs, a.numSamples);
        time_sleep_ms(1000);
    }
    return 0;
}

static pthread_t printLoopThread;


int main(void)
{
    Period_init();
    beatbox_init();
    ADC_init(7, 4, 5, 6);
    Encoder_init(27, 4, 17);
    UDP_init();
    pthread_create(&printLoopThread, NULL, printLoop, NULL);
    UDP_join_thread();

    beatbox_cleanup();
    stopping = 1;
    pthread_join(printLoopThread, NULL);
    ADC_cleanup();
    Encoder_cleanup();
    return 0;
}