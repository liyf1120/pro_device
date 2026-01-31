#include "sensors.h"

#include "led.h"
#include "light.h"
#include "temp-hum.h"

#include <stddef.h>

int sensors_init(void) {
    led_init();
    light_init(NULL);
    temp_hum_init(NULL);
    return 0;
}

int sensors_read(float *t, float *h, int16_t *light, int8_t *led) {
    int ok = 0;
    int led_state = 0;

    if (t && temp_read_c(t) != 0) {
        ok = -1;
    }
    if (h && hum_read_percent(h) != 0) {
        ok = -1;
    }
    if (light && light_read_lux(light) != 0) {
        ok = -1;
    }
    if (led && led_get(&led_state) == 0) {
        *led = (int8_t)led_state;
    }

    return ok;
}
