#include "tcp_server.h"

#include "led.h"
#include "protocol.h"
#include "sensors.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_BACKLOG 5
#define SERVER_BUF_SIZE 2048

static volatile int server_running = 1;

void tcp_server_stop(void) {
    server_running = 0;
}

static ssize_t find_header(const uint8_t *buf, size_t len) {
    for (size_t i = 0; i + 3 < len; i++) {
        uint32_t value = (uint32_t)buf[i] << 24 | (uint32_t)buf[i + 1] << 16 |
                         (uint32_t)buf[i + 2] << 8 | (uint32_t)buf[i + 3];
        if (value == PROTO_HEADER) {
            return (ssize_t)i;
        }
    }
    return -1;
}

static int handle_frame(const uint8_t *frame, size_t frame_len, uint8_t *resp,
                        size_t resp_len, size_t *out_len) {
    proto_request_t req;
    int8_t led_state = 0;
    float temp = 0.0f;
    float hum = 0.0f;
    int16_t light = 0;
    int parse_ret = proto_parse_request(frame, frame_len, &req);

    if (parse_ret == -2) {
        printf("CRC mismatch, discard frame\n");
        return 0;
    }
    if (parse_ret != 0) {
        return 0;
    }

    if (req.type != PROTO_TYPE_REQ) {
        return 0;
    }

    if (req.func == PROTO_FUNC_QUERY) {
        sensors_read(&temp, &hum, &light, &led_state);
        *out_len = proto_build_query_response(resp, resp_len, req.dev_addr, temp,
                                              hum, light, led_state);
        return *out_len > 0 ? 1 : 0;
    }

    if (req.func == PROTO_FUNC_LED) {
        led_set(req.led_value ? 1 : 0);
        led_state = req.led_value ? 1 : 0;
        *out_len = proto_build_led_response(resp, resp_len, req.dev_addr, led_state);
        return *out_len > 0 ? 1 : 0;
    }

    return 0;
}

static void handle_client(int client_fd) {
    uint8_t buffer[SERVER_BUF_SIZE];
    uint8_t resp[SERVER_BUF_SIZE];
    size_t used = 0;

    while (server_running) {
        ssize_t n = recv(client_fd, buffer + used, sizeof(buffer) - used, 0);
        if (n == 0) {
            break;
        }
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("recv");
            break;
        }
        used += (size_t)n;

        while (used > 0) {
            ssize_t hdr_index = find_header(buffer, used);
            if (hdr_index < 0) {
                used = 0;
                break;
            }
            if (hdr_index > 0) {
                memmove(buffer, buffer + hdr_index, used - (size_t)hdr_index);
                used -= (size_t)hdr_index;
            }
            if (used < 4 + 1 + 1 + 1 + 2) {
                break;
            }

            uint16_t len_field = proto_read_u16_be(&buffer[7]);
            size_t frame_len = 4 + 1 + 1 + 1 + 2 + len_field + 4;
            if (frame_len > sizeof(buffer)) {
                used = 0;
                break;
            }
            if (used < frame_len) {
                break;
            }

            size_t out_len = 0;
            if (handle_frame(buffer, frame_len, resp, sizeof(resp), &out_len)) {
                send(client_fd, resp, out_len, 0);
            }

            if (used > frame_len) {
                memmove(buffer, buffer + frame_len, used - frame_len);
            }
            used -= frame_len;
        }
    }
}

int tcp_server_run(const char *ip, uint16_t port) {
    int server_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip == NULL || ip[0] == '\0') {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(server_fd);
        return -1;
    }

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, SERVER_BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    printf("TCP server listening on %s:%u\n", ip ? ip : "0.0.0.0", port);

    while (server_running) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            break;
        }
        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
