#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

int light_init(const char *dev_path);
int light_read_lux(int16_t *out_light);

#endif
