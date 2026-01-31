#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <stdint.h>

int tcp_server_run(const char *ip, uint16_t port);
void tcp_server_stop(void);

#endif
