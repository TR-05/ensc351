#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdbool.h>

/*
Header file giving access to joysitck data
*/

void Joystick_init(int adc_y_channel);
float Joystick_read(void);
void Joystick_cleanup(void);

#endif // JOYSTICK_H