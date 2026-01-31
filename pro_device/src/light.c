#include "light.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static char light_dev_path[256] = "/dev/bh1750";

static void print_usage(void) {
    printf("example :  ./app /dev/bh1750"
           "\n\t'/dev/bh1750' sensor device PATH\n");
}

int light_init(const char *dev_path) {
    if (dev_path && dev_path[0] != '\0') {
        snprintf(light_dev_path, sizeof(light_dev_path), "%s", dev_path);
    }
    return 0;
}

int light_read_lux(int16_t *out_light) {
    int fd;
    int ret;
    unsigned short data = 0;

    if (!out_light) {
        print_usage();
        return -1;
    }

    fd = open(light_dev_path, O_RDWR);
    if (fd < 0) {
        return -1;
    }

    ret = read(fd, &data, sizeof(unsigned short));
    if (ret < 0) {
        perror("read error!\n");
        close(fd);
        return -1;
    }

    close(fd);

    *out_light = (int16_t)((float)data / 1.2f);
    return 0;
}
