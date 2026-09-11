#include <stdio.h>
#include sigrok_protocol.h
#include capture.h
#include pico/stdlib.h
#include pico/stdio_usb.h

#define SUMP_CMD_RESET        0x00
#define SUMP_CMD_ARM          0x01
#define SUMP_CMD_QUERY_ID     0x02
#define SUMP_CMD_QUERY_META   0x04
#define SUMP_CMD_SET_DIV      0x80
#define SUMP_CMD_SET_READ_LEN 0x81
#define SUMP_CMD_SET_TRIG_MASK 0xC0

static uint32_t s_sump_read_count = 10000;
static uint32_t s_sump_sample_rate = 10000000;

void sigrok_protocol_init(void) {
    stdio_usb_init();
}

void sigrok_protocol_task(void) {
    int c = getchar_timeout_us(0);
    if (c == PICO_ERROR_TIMEOUT || c < 0) return;

    uint8_t cmd = (uint8_t)c;

    switch (cmd) {
        case SUMP_CMD_RESET:
            capture_stop();
            break;

        case SUMP_CMD_QUERY_ID:
            // Sump ID string 1ALS
            putchar_raw('1');
            putchar_raw('A');
            putchar_raw('L');
            putchar_raw('S');
            break;

        case SUMP_CMD_QUERY_META:
            // Metadata: Name, 8 channels, 100MHz max rate, 64KB memory
            putchar_raw(0x01); // Device name
            printf(RP2040 Logic Analyzer\0);
            putchar_raw(0x20); // Max sample rate (100MHz)
            putchar_raw(0x05); putchar_raw(0xF5); putchar_raw(0xE1); putchar_raw(0x00);
            putchar_raw(0x21); // Max sample memory (65536 bytes)
            putchar_raw(0x00); putchar_raw(0x01); putchar_raw(0x00); putchar_raw(0x00);
            putchar_raw(0x40); // Number of probes (8 channels)
            putchar_raw(0x08);
            putchar_raw(0x00); // End of metadata
            break;

        case SUMP_CMD_ARM:
            capture_arm(s_sump_sample_rate, s_sump_read_count, 0, TRIG_NONE);
            while (!capture_is_complete()) {
                tight_loop_contents();
            }
            // Send back captured raw sample bytes
            for (uint32_t i = 0; i < s_sump_read_count; i++) {
                putchar_raw(capture_buffer[i]);
            }
            break;

        default:
            break;
    }
}
