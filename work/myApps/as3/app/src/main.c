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
    while(0) {
        printf("%d, %d, %.2f, X %5.2f, Y %5.2f, Z: %5.2f\n", Encoder_read(), Encoder_button_pressing(), ADC_read_joystick(), ADC_read_acel_x(), ADC_read_acel_y(), ADC_read_acel_z());
        time_sleep_ms(10);
    }
    beatboxInit();

    
    ADC_cleanup();
    Encoder_cleanup();
    return 0;
}