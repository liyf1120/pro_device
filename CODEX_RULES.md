# Codex Rules for pro_device (MUST FOLLOW)

## Branch workflow
- Always work on branch: `feature/pro_device`
- Never create/switch to other branches
- Every task must end with exactly one commit (unless explicitly asked not to)

## No unnecessary rewrites
- Do not rewrite untouched files
- Do not reformat the whole repository
- Do not rename/move files unless requested

## Directory structure (must match)
pro_device/
  include/
    light.h
    temp-hum.h
    led.h
    sensors.h
    protocol.h
    tcp_server.h
  src/
    main.c
    light.c
    temp-hum.c
    led.c
    sensors.c
    protocol.c
    tcp_server.c
  Makefile

## TCP private protocol (DO NOT CHANGE)
Network byte order (big-endian):
- header: 0xaaaaaaaa (4B)
- dev_addr: 1B
- type: 1B (0x01 req / 0x02 resp)
- func: 1B (0x01 query / 0x02 led control)
- len: 2B (data + crc16 length, excluding header/tail)
- data: start_addr(2B) + reg_count(2B) + reg_values(N)
- crc16: 2B (CRC over all fields except header/tail/crc field itself)
- tail: 0xbbbbbbbb (4B)

CRC16: CRC-16/IBM (Modbus), poly 0xA001, init 0xFFFF.

## Register map (DO NOT CHANGE)
- 0x0001 temperature float32 4B RO (IEEE754, big-endian bytes on wire)
- 0x0002 humidity    float32 4B RO
- 0x0003 light       int16   2B RO
- 0x0004 led         int8    1B RW (1 on, 0 off)

## Output requirements for each task
At the end of the task:
- show: changed files list (added/modified/deleted)
- run: `make clean && make`
- provide: how to run + at least 3 tests (query, led control, crc error discard)
- create exactly one git commit with the requested message
