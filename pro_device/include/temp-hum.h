#ifndef TEMP_HUM_H
#define TEMP_HUM_H

int temp_hum_init(const char *dev_path);
int temp_read_c(float *t);
int hum_read_percent(float *h);

#endif
