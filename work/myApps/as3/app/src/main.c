#include <stdio.h>
#include <unistd.h>
#include "hal/audioMixer.h"
#include "hal/time.h"
#include "hal/encoder.h"
#include "hal/adc.h"

#include "beatbox.h"

int main(void)
{
    ADC_init(7, 4, 5, 6);
    Encoder_init(27, 4, 17);
    beatboxInit();

    while(1) {
        time_sleep_ms(10);
    }

    
    ADC_cleanup();
    Encoder_cleanup();
    return 0;
}