#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/button.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main()
{
    joystick_initialize();
    led_initialize();
    button_initialize();
    led_act_set_off();
    led_pwr_set_off();

    printf("Hello embedded world, from Trevor!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");

    while (1)
    {
        printf("Get ready...\n");
        for (int i = 0; i < 4; i++)
        {
            led_act_set_on();
            led_pwr_set_off();
            usleep(250000);
            led_act_set_off();
            led_pwr_set_on();
            usleep(250000);
        }
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("Please let go of joystick\n");
        }
        while (fabs(joystick_get_y_normalized()) > 0.1)
        {
            usleep(10000);
        }
        int random_sleep_time = 1000000;
        usleep(random_sleep_time);
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("too soon\n"); // LOOP BACK TO WHILE LOOP
        }

        int dir = 1; // randomly choose up or down
        if (dir == 1)
        {
            printf("Press UP now!\n");
            led_act_set_on();
            led_pwr_set_off();
        }
        else
        {
            printf("Press DOWN now!\n");
            led_act_set_off();
            led_pwr_set_on();
        }
        struct timeval start_tv, current_tv;
        gettimeofday(&start_tv, NULL);

        while (fabs(joystick_get_y_normalized()) < 0.5)
        {
            gettimeofday(&current_tv, NULL);
            double elapsed = (current_tv.tv_sec - start_tv.tv_sec) +
                             (current_tv.tv_usec - start_tv.tv_usec) / 1000000.0;
            if (elapsed >= 5.0)
                break; // 5 second timeout
            usleep(10000);
        }
        // float x = joystick_get_y_normalized();
        gettimeofday(&current_tv, NULL);
        double reaction_time = (current_tv.tv_sec - start_tv.tv_sec) +
                               (current_tv.tv_usec - start_tv.tv_usec) / 1000000.0;
        printf("Your reaction time was %dms\n", (int)(reaction_time * 1000));
    }

    joystick_disable(); // idk if this matters
    led_act_set_off();
    led_pwr_set_off();
    return 0;
}