#include "hal/joystick.h"
#include "hal/led.h"
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "reaction_methods.h"

int main()
{
    // setup leds and joystick, print intro text to player terminal
    reaction_initialize();

    float best_time = 5000;
    // main game loop
    while (true)
    {
        // print Get Ready and blink LEDs
        reaction_prep_user();

        // repeat random generation if player inputs too soon
        int dir = reaction_gen_rng();
        if (dir == 0)
        {
            printf("User selected to quit.\n");
            fflush(stdout);
            break;
        }
        reaction_start_timer();
        // wait for player input or break if player is inactive
        while (!(fabs(joystick_get_x_normalized()) > 0.75 || fabs(joystick_get_y_normalized()) > 0.75 || reaction_read_timer_ms() >= 5000))
            ;

        // save loop time before anything else for accuracy
        float reaction_time = reaction_read_timer_ms();

        // exit main game loop if player is inactive
        if (reaction_read_timer_ms() >= 5000)
        {
            printf("No input within 5000ms; quitting!\n");
            fflush(stdout);
            break; // 5 second timeout
        }

        // turn off leds once reaction test is finished
        led_act_set_off();
        led_pwr_set_off();

        // save joystick state immediately after loop exits to determine outcome
        float y = joystick_get_y_normalized();
        float x = joystick_get_x_normalized();

        // exit main game loop if player wants to quit (or just fumbles that badly)
        if (fabs(x) > 0.75)
        {
            printf("User selected to quit.\n");
            fflush(stdout);
            break;
        }

        // if user moved the joystick in the right direction readout their time + highscore
        if (y / fabs(y) == dir)
        {
            printf("Correct!\n");
            fflush(stdout);
            if (reaction_time < best_time)
            {
                best_time = reaction_time;
                printf("New best time!\n");
                fflush(stdout);
            }
            printf("Your reaction time was %dms; best so far in game is %dms\n", (int)reaction_time, (int)best_time);
            fflush(stdout);
            for (int i = 0; i < 5; i++)
            {
                led_act_set_on();
                usleep(100000);
                led_act_set_off();
                usleep(100000);
            }
        }
        // if user moved the joystick in the wrong direction make fun of them
        else
        {
            printf("Incorrect.\n");
            fflush(stdout);
            for (int i = 0; i < 5; i++)
            {
                led_pwr_set_on();
                usleep(100000);
                led_pwr_set_off();
                usleep(100000);
            }
        }
    }

    // disable spi cleanly and turn off leds to return to default state when program ends
    joystick_disable();
    led_act_set_off();
    led_pwr_set_off();
    return 0;
}