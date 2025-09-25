#include "hal/joystick.h"
#include "hal/led.h"
#include "hal/button.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    srand(time(NULL));
    joystick_initialize();
    led_initialize();
    button_initialize();
    led_act_set_off();
    led_pwr_set_off();

    printf("Hello embedded world, from Trevor!\n");
    printf("When the LEDs light up, press the joystick in that direction!\n");
    printf("(Press left or right to exit)\n");

    int best_time = 5000;

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
        led_act_set_off();
        led_pwr_set_off();
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("Please let go of joystick\n");
        }
        usleep(100); // let adc calculate
        while (fabs(joystick_get_y_normalized()) > 0.1)
        {
            usleep(10000);
        }
        int random_sleep_time = 500000 + (rand() % 2500000); // Random between 0.5-2.5 seconds
        usleep(random_sleep_time);
        if (fabs(joystick_get_y_normalized()) > 0.1)
        {
            printf("too soon\n"); // LOOP BACK TO WHILE LOOP
        }

        int dir = (rand() % 2) ? 1 : -1; // Randomly choose 1 or -1 (up or down)
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
        double elapsed = 0;
        while (fabs(joystick_get_y_normalized()) < 0.75)
        {
            usleep(500); // let adc calculate
            if (fabs(joystick_get_x_normalized()) > 0.75)
            {
                break;
            }

            gettimeofday(&current_tv, NULL);
            elapsed = (current_tv.tv_sec - start_tv.tv_sec) +
                      (current_tv.tv_usec - start_tv.tv_usec) / 1000000.0;
            if (elapsed >= 5.0)
                break;   // 5 second timeout
            usleep(500); // let adc calculate
        }
        if (elapsed >= 5.0)
        {
            printf("No input within 5000ms; quitting!\n");
            break; // 5 second timeout
        }
        led_act_set_off();
        led_pwr_set_off();
        usleep(500); // let adc calculate
        float y = joystick_get_y_normalized();
        usleep(500); // let adc calculate
        float x = joystick_get_x_normalized();
        // printf("x: %.2f, y:  %.2f\n", x, y);
        if (fabs(x) > 0.75)
        {
            printf("User selected to quit.\n");
            break;
        }

        gettimeofday(&current_tv, NULL);
        int reaction_time = (int)(1000 * ((current_tv.tv_sec - start_tv.tv_sec) +
                                          (current_tv.tv_usec - start_tv.tv_usec) / 1000000.0));
        if (y / fabs(y) == dir)
        {
            printf("Correct!\n");
            if (reaction_time < best_time)
            {
                best_time = reaction_time;
                printf("New best time!\n");
            }
            printf("Your reaction time was %dms; best so far in game is %dms\n", reaction_time, best_time);
            for (int i = 0; i < 5; i++)
            {
                led_act_set_on();
                usleep(100000);
                led_act_set_off();
                usleep(100000);
            }
        }
        else
        {
            printf("Incorrect.\n");
            for (int i = 0; i < 5; i++)
            {
                led_pwr_set_on();
                usleep(100000);
                led_pwr_set_off();
                usleep(100000);
            }
        }
    }

    joystick_disable(); // idk if this matters
    led_act_set_off();
    led_pwr_set_off();
    return 0;
}