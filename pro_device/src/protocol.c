#include "protocol.h"

#include <string.h>

uint16_t proto_read_u16_be(const uint8_t *buf) {
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

void proto_write_u16_be(uint8_t *buf, uint16_t value) {
    buf[0] = (uint8_t)(value >> 8);
    buf[1] = (uint8_t)(value & 0xff);
}

void proto_write_u32_be(uint8_t *buf, uint32_t value) {
    buf[0] = (uint8_t)(value >> 24);
    buf[1] = (uint8_t)((value >> 16) & 0xff);
    buf[2] = (uint8_t)((value >> 8) & 0xff);
    buf[3] = (uint8_t)(value & 0xff);
}

void proto_write_float_be(uint8_t *buf, float value) {
    uint32_t raw = 0;
    memcpy(&raw, &value, sizeof(raw));
    proto_write_u32_be(buf, raw);
}

uint16_t proto_crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

int proto_parse_request(const uint8_t *buf, size_t len, proto_request_t *req) {
    size_t offset = 0;
    uint16_t calc_crc;
    uint16_t pkt_crc;
    uint16_t payload_len;
    uint32_t header;
    uint32_t tail;

    if (!buf || !req) {
        return -1;
    }

    if (len < 4 + 1 + 1 + 1 + 2 + 4 + 2 + 4) {
        return -1;
    }

    header = (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 |
             (uint32_t)buf[2] << 8 | (uint32_t)buf[3];
    if (header != PROTO_HEADER) {
        return -1;
    }
    offset += 4;

    req->dev_addr = buf[offset++];
    req->type = buf[offset++];
    req->func = buf[offset++];
    req->len = proto_read_u16_be(&buf[offset]);
    offset += 2;

    if (req->len < 6) {
        return -1;
    }

    payload_len = req->len - 2;
    if (len < 4 + 1 + 1 + 1 + 2 + payload_len + 2 + 4) {
        return -1;
    }

    req->start_addr = proto_read_u16_be(&buf[offset]);
    req->reg_count = proto_read_u16_be(&buf[offset + 2]);
    offset += 4;

    if (req->func == PROTO_FUNC_QUERY) {
        if (payload_len != 4) {
            return -1;
        }
    } else if (req->func == PROTO_FUNC_LED) {
        if (payload_len < 5) {
            return -1;
        }
        req->led_value = buf[offset];
    } else {
        return -1;
    }

    offset = 4 + 1 + 1 + 1 + 2 + payload_len;
    pkt_crc = proto_read_u16_be(&buf[offset]);
    calc_crc = proto_crc16(&buf[4], 1 + 1 + 1 + 2 + payload_len);
    if (pkt_crc != calc_crc) {
        return -2;
    }

    offset += 2;
    tail = (uint32_t)buf[offset] << 24 | (uint32_t)buf[offset + 1] << 16 |
           (uint32_t)buf[offset + 2] << 8 | (uint32_t)buf[offset + 3];
    if (tail != PROTO_TAIL) {
        return -1;
    }

    return 0;
}

static size_t build_header(uint8_t *buf, size_t buf_len, uint8_t dev_addr,
                           uint8_t func, uint16_t data_len) {
    uint16_t len_field = (uint16_t)(data_len + 2);

    if (buf_len < 4 + 1 + 1 + 1 + 2 + data_len + 2 + 4) {
        return 0;
    }

    proto_write_u32_be(buf, PROTO_HEADER);
    buf[4] = dev_addr;
    buf[5] = PROTO_TYPE_RESP;
    buf[6] = func;
    proto_write_u16_be(&buf[7], len_field);

    return 9;
}

static size_t finish_packet(uint8_t *buf, size_t buf_len, size_t payload_len) {
    size_t crc_offset = 4 + 1 + 1 + 1 + 2 + payload_len;
    uint16_t crc;

    if (buf_len < crc_offset + 2 + 4) {
        return 0;
    }

    crc = proto_crc16(&buf[4], 1 + 1 + 1 + 2 + payload_len);
    proto_write_u16_be(&buf[crc_offset], crc);
    proto_write_u32_be(&buf[crc_offset + 2], PROTO_TAIL);

    return crc_offset + 2 + 4;
}

size_t proto_build_query_response(uint8_t *buf, size_t buf_len, uint8_t dev_addr,
                                  float temp, float hum, int16_t light,
                                  int8_t led) {
    size_t offset;
    size_t data_len = 2 + 2 + 4 + 4 + 2 + 1;

    offset = build_header(buf, buf_len, dev_addr, PROTO_FUNC_QUERY, data_len);
    if (offset == 0) {
        return 0;
    }

    proto_write_u16_be(&buf[offset], 0x0001);
    proto_write_u16_be(&buf[offset + 2], 4);
    proto_write_float_be(&buf[offset + 4], temp);
    proto_write_float_be(&buf[offset + 8], hum);
    proto_write_u16_be(&buf[offset + 12], (uint16_t)light);
    buf[offset + 14] = (uint8_t)led;

    return finish_packet(buf, buf_len, data_len);
}

size_t proto_build_led_response(uint8_t *buf, size_t buf_len, uint8_t dev_addr,
                                int8_t led) {
    size_t offset;
    size_t data_len = 2 + 2 + 1;

    offset = build_header(buf, buf_len, dev_addr, PROTO_FUNC_LED, data_len);
    if (offset == 0) {
        return 0;
    }

    proto_write_u16_be(&buf[offset], 0x0004);
    proto_write_u16_be(&buf[offset + 2], 1);
    buf[offset + 4] = (uint8_t)led;

    return finish_packet(buf, buf_len, data_len);
}
