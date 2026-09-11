#ifndef UI_H
#define UI_H

#include <stdint.h>
#include <stdbool.h>

#define PIN_NAV_UP    16
#define PIN_NAV_DOWN  17
#define PIN_NAV_LEFT  18
#define PIN_NAV_RIGHT 19
#define PIN_NAV_ENTER 28

typedef struct {
    uint8_t selected_channel;
    uint32_t timebase_index; // 0: 1us, 1: 5us, 2: 10us, 3: 50us, 4: 100us, 5: 1ms, 6: 10ms
    int32_t cursor_x;
    bool is_running;
    char decoder_text[64];
} ui_state_t;

extern ui_state_t g_ui;

void ui_init(void);
void ui_poll_buttons(void);
void ui_render_frame(void);

#endif // UI_H
