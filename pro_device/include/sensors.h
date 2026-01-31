#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

int sensors_init(void);
int sensors_read(float *t, float *h, int16_t *light, int8_t *led);

#endif
