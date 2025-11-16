#include <stdio.h>
#include <unistd.h>
#include "hal/audioMixer.h"
#include "hal/time.h"
#include "hal/encoder.h"
#include "hal/joystick.h"

#include "beatbox.h"

int main(void)
{
    Joystick_init(7);
    Encoder_init(27, 17, 4);
    while(1) {
        printf("%d, %d\n", Encoder_read(), Encoder_button_pressing());
        time_sleep_ms(10);
    }
    beatboxInit();

    
    Joystick_cleanup();
    Encoder_cleanup();
    return 0;
}