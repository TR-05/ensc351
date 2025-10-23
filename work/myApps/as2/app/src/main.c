#include "hal/gpio.h"
#include "hal/time.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int main()
{
    //button_initialize();
    gpio_initialize(4);
    gpio_initialize(17);
    gpio_initialize(27);
    bool lastA = 1;
    int pulses = 0;
    while (1) {
        bool A = gpio_read(27);
        bool B = gpio_read(17);

        if (A!= lastA && A == 1) {
            if (B != A) // clockwise
            {
                pulses++;
            } else // counter clockwise;
            {
                pulses--;
            }
        }
        lastA = A;

        printf("4: %d, 17: %d, 27: %d, p: %.2f\n", gpio_read(4),gpio_read(17),gpio_read(27), pulses / 48.0f );
        sleep_for_ms(1);
    }
    return 0;
}