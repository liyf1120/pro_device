#include "temp-hum.h"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define SHT20_READ_TEMPERTURE 98
#define SHT20_READ_HUMIDITY 99

#define CRC_MODEL 0x131

static char temp_hum_dev_path[256] = "/dev/sht20";

static unsigned char CRC_Check(unsigned char *ptr, unsigned char len,
                               unsigned char checksum) {
    unsigned char i;
    unsigned char crc = 0x00; // 计算的初始crc值

    while (len--) {
        crc ^= *ptr++; // 每次先与需要计算的数据异或,计算完指向下一数据

        for (i = 8; i > 0; --i) // 下面这段计算过程与计算一个字节crc一样
        {
            if (crc & 0x80) {
                crc = (crc << 1) ^ CRC_MODEL;
            } else {
                crc = (crc << 1);
            }
        }
    }

    if (checksum == crc) {
        return 0;
    }
    return 1;
}

static int read_sht20(int cmd, float *out_value, int is_temp) {
    int ret;
    int fd;
    unsigned char buf[3];
    unsigned int temp;

    if (!out_value) {
        return -1;
    }

    fd = open(temp_hum_dev_path, O_RDONLY);
    if (fd < 0) {
        printf("can't open file %s\r\n", temp_hum_dev_path);
        return -1;
    }

    ret = ioctl(fd, cmd, buf);
    if (ret < 0) {
        printf("sht20 read %s fail!\r\n", is_temp ? "temperture" : "humidity");
        close(fd);
        return -1;
    }

    /* buf[2] stores CRC byte; original buf[3] access was out-of-bounds. */
    if (!CRC_Check(buf, 2, buf[2])) {
        printf("CRC check error!");
        close(fd);
        return -1;
    }

    temp = (buf[0] << 8) | buf[1];
    if (is_temp) {
        *out_value = (float)((temp * 175.72) / 65535.0 - 46.85);
    } else {
        *out_value = (float)((temp * 125.0) / 65536.0 - 6.0);
    }

    close(fd);
    return 0;
}

int temp_hum_init(const char *dev_path) {
    if (dev_path && dev_path[0] != '\0') {
        snprintf(temp_hum_dev_path, sizeof(temp_hum_dev_path), "%s", dev_path);
    }
    return 0;
}

int temp_read_c(float *t) {
    return read_sht20(SHT20_READ_TEMPERTURE, t, 1);
}

int hum_read_percent(float *h) {
    return read_sht20(SHT20_READ_HUMIDITY, h, 0);
}
