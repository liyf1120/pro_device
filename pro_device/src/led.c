#include "led.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *led_color = "blue";
static int led_state = 0;

static int write_to_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    size_t len = strlen(value);
    ssize_t written = write(fd, value, len);
    if (written < 0 || (size_t)written != len) {
        perror("write");
        close(fd);
        return -1;
    }

    if (close(fd) < 0) {
        perror("close");
        return -1;
    }

    return 0;
}

static void set_led_trigger(const char *color, const char *trigger) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/leds/%s/trigger", color);
    write_to_file(path, trigger);
}

static void set_led_brightness(const char *color, int value) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/leds/%s/brightness", color);
    write_to_file(path, value ? "1" : "0");
}

int led_init(void) {
    set_led_trigger(led_color, "none");
    led_state = 0;
    return led_set(0);
}

int led_set(int on) {
    led_state = on ? 1 : 0;
    set_led_brightness(led_color, led_state);
    return 0;
}

int led_get(int *on) {
    if (!on) {
        return -1;
    }
    *on = led_state;
    return 0;
}

void led_set_color(const char *color) {
    if (color && color[0] != '\0') {
        led_color = color;
    }
}
