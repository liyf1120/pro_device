#ifndef LED_H
#define LED_H

#include <stdint.h>

int led_init(void);
int led_set(int on);
int led_get(int *on);
void led_set_color(const char *color);

#endif
