#include "sensors.h"
#include "tcp_server.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t running = 1;

static void handle_signal(int sig) {
    (void)sig;
    running = 0;
    tcp_server_stop();
}

static void print_usage(const char *prog) {
    printf("Usage: %s [--ip IP] [--port PORT]\n", prog);
}

int main(int argc, char **argv) {
    const char *ip = "0.0.0.0";
    int port = 9000;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ip") == 0 && i + 1 < argc) {
            ip = argv[++i];
        } else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--h") == 0 ||
                   strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    signal(SIGINT, handle_signal);

    sensors_init();

    if (running) {
        tcp_server_run(ip, (uint16_t)port);
    }

    return 0;
}
