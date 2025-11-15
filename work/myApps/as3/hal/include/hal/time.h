#ifndef TIME_H
#define SPITIME_H_H

#include <stdint.h>
#include <unistd.h>

/*
Header file giving basic time related functionality
*/

long long time_get_ms(void);
void time_sleep_ms(float delayInMs);

#endif // TIME_H