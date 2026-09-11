#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdint.h>
#include <stdbool.h>
#include pico/stdlib.h
#include hardware/pio.h
#include hardware/dma.h

#define MAX_SAMPLE_BUFFER_SIZE (64 * 1024) // 64 KB (65,536 samples of 8 channels)
#define LOGIC_PIN_BASE 0                  // GPIO0 through GPIO7

typedef enum {
    TRIG_NONE = 0,
    TRIG_RISING,
    TRIG_FALLING,
    TRIG_HIGH,
    TRIG_LOW
} trigger_type_t;

typedef struct {
    uint32_t sample_rate_hz;
    uint32_t sample_count;
    uint8_t trigger_channel;
    trigger_type_t trigger_type;
    bool is_running;
    bool capture_complete;
} capture_config_t;

extern uint8_t capture_buffer[MAX_SAMPLE_BUFFER_SIZE];
extern volatile capture_config_t g_capture_cfg;

void capture_init(void);
void capture_arm(uint32_t sample_rate_hz, uint32_t sample_count, uint8_t trig_chan, trigger_type_t trig_type);
bool capture_is_complete(void);
void capture_stop(void);

#endif // CAPTURE_H
