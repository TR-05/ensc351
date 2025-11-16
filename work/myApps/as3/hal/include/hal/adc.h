#ifndef ADC_H
#define ADC_H

#include <stdbool.h>

/*
Header file giving access to ADC data
*/

void ADC_init(int ADC_channel, int adxl_x, int adxl_y, int adxl_z);
float ADC_read_joystick(void);
float ADC_read_acel_x(void);
float ADC_read_acel_y(void);
float ADC_read_acel_z(void);

void ADC_cleanup(void);

#endif // ADC_H