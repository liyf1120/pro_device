#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#define PROTO_HEADER 0xaaaaaaaa
#define PROTO_TAIL 0xbbbbbbbb

#define PROTO_TYPE_REQ 0x01
#define PROTO_TYPE_RESP 0x02

#define PROTO_FUNC_QUERY 0x01
#define PROTO_FUNC_LED 0x02

typedef struct {
    uint8_t dev_addr;
    uint8_t type;
    uint8_t func;
    uint16_t len;
    uint16_t start_addr;
    uint16_t reg_count;
    uint8_t led_value;
} proto_request_t;

uint16_t proto_crc16(const uint8_t *data, size_t len);
int proto_parse_request(const uint8_t *buf, size_t len, proto_request_t *req);
size_t proto_build_query_response(uint8_t *buf, size_t buf_len, uint8_t dev_addr,
                                  float temp, float hum, int16_t light,
                                  int8_t led);
size_t proto_build_led_response(uint8_t *buf, size_t buf_len, uint8_t dev_addr,
                                int8_t led);

uint16_t proto_read_u16_be(const uint8_t *buf);
void proto_write_u16_be(uint8_t *buf, uint16_t value);
void proto_write_u32_be(uint8_t *buf, uint32_t value);
void proto_write_float_be(uint8_t *buf, float value);

#endif
