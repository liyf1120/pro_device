CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11 -Iinclude

SRCS = \
	src/main.c \
	src/led.c \
	src/light.c \
	src/temp-hum.c \
	src/sensors.c \
	src/protocol.c \
	src/tcp_server.c

OBJS = $(SRCS:.c=.o)

TARGET = pro_device.bin

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
