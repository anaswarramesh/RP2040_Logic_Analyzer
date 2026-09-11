#include <stdio.h>
#include <string.h>
#include ui.h
#include st7789.h
#include capture.h
#include hardware/gpio.h

static const uint16_t channel_colors[8] = {
    COLOR_CYAN,    // CH0 (SDA)
    COLOR_YELLOW,  // CH1 (SCL)
    COLOR_GREEN,   // CH2 (SCK)
    COLOR_MAGENTA, // CH3 (MOSI)
    COLOR_ORANGE,  // CH4 (MISO)
    COLOR_WHITE,   // CH5 (CS)
    COLOR_RED,     // CH6 (TRIG)
    COLOR_LIGHTGREY// CH7 (DAT)
};

static const char *timebase_labels[] = {
    1us/div, 5us/div, 10us/div, 50us/div, 100us/div, 1ms/div, 10ms/div
};

static const uint32_t sample_rates[] = {
    100000000, 20000000, 10000000, 2000000, 1000000, 100000, 10000
};

ui_state_t g_ui = {
    .selected_channel = 0,
    .timebase_index = 3, // 50us/div
    .cursor_x = 160,
    .is_running = true,
    .decoder_text = I2C: Addr 0x48 [ACK] Data: 0x3F [ACK]
};

void ui_init(void) {
    uint8_t btns[] = {PIN_NAV_UP, PIN_NAV_DOWN, PIN_NAV_LEFT, PIN_NAV_RIGHT, PIN_NAV_ENTER};
    for (int i = 0; i < 5; i++) {
        gpio_init(btns[i]);
        gpio_set_dir(btns[i], GPIO_IN);
        gpio_pull_up(btns[i]);
    }
}

void ui_poll_buttons(void) {
    static uint32_t last_poll_ms = 0;
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (now - last_poll_ms < 150) return; // Debounce
    last_poll_ms = now;

    if (!gpio_get(PIN_NAV_UP)) {
        if (g_ui.selected_channel > 0) g_ui.selected_channel--;
    }
    if (!gpio_get(PIN_NAV_DOWN)) {
        if (g_ui.selected_channel < 7) g_ui.selected_channel++;
    }
    if (!gpio_get(PIN_NAV_LEFT)) {
        if (g_ui.timebase_index < 6) {
            g_ui.timebase_index++; // Zoom out
            capture_arm(sample_rates[g_ui.timebase_index], 10000, g_ui.selected_channel, TRIG_NONE);
        }
    }
    if (!gpio_get(PIN_NAV_RIGHT)) {
        if (g_ui.timebase_index > 0) {
            g_ui.timebase_index--; // Zoom in
            capture_arm(sample_rates[g_ui.timebase_index], 10000, g_ui.selected_channel, TRIG_NONE);
        }
    }
    if (!gpio_get(PIN_NAV_ENTER)) {
        g_ui.is_running = !g_ui.is_running;
        if (g_ui.is_running) {
            capture_arm(sample_rates[g_ui.timebase_index], 10000, g_ui.selected_channel, TRIG_NONE);
        } else {
            capture_stop();
        }
    }
}

void ui_render_frame(void) {
    // 1. Render Top Header Bar (0..20 px)
    st7789_fill_rect(0, 0, ST7789_WIDTH, 20, COLOR_NAVY);
    st7789_draw_string(5, 6, RP2040 LOGIC, COLOR_WHITE, COLOR_NAVY, 1);

    if (g_ui.is_running) {
        st7789_fill_rect(110, 4, 60, 12, COLOR_GREEN);
        st7789_draw_string(115, 6, RUNNING, COLOR_BLACK, COLOR_GREEN, 1);
    } else {
        st7789_fill_rect(110, 4, 50, 12, COLOR_RED);
        st7789_draw_string(115, 6, STOPPED, COLOR_WHITE, COLOR_RED, 1);
    }

    char time_buf[32];
    snprintf(time_buf, sizeof(time_buf), Time: %s, timebase_labels[g_ui.timebase_index]);
    st7789_draw_string(210, 6, time_buf, COLOR_YELLOW, COLOR_NAVY, 1);

    // 2. Clear Waveform Area (20..200 px)
    st7789_fill_rect(0, 20, ST7789_WIDTH, 185, COLOR_BLACK);

    // Draw Grid Lines (every 40 px horizontally)
    for (int x = 40; x < ST7789_WIDTH; x += 40) {
        for (int y = 20; y < 205; y += 4) {
            st7789_draw_pixel(x, y, COLOR_DARKGREY);
        }
    }

    // 3. Render 8 Waveform Traces
    const int start_x = 45;
    const int plot_w = ST7789_WIDTH - start_x - 5;

    for (int ch = 0; ch < 8; ch++) {
        int base_y = 36 + (ch * 21);
        int high_y = base_y - 12;

        // Draw Channel Label (CH0..CH7)
        char lbl[8];
        snprintf(lbl, sizeof(lbl), D%d, ch);
        uint16_t lbl_col = (ch == g_ui.selected_channel) ? COLOR_WHITE : COLOR_GREY;
        st7789_draw_string(5, high_y + 2, lbl, channel_colors[ch], COLOR_BLACK, 1);

        // Render digital trace from sample buffer
        uint8_t last_state = 0;
        int last_y = base_y;

        for (int px = 0; px < plot_w; px++) {
            uint32_t sample_idx = (px * g_capture_cfg.sample_count) / plot_w;
            if (sample_idx >= MAX_SAMPLE_BUFFER_SIZE) sample_idx = MAX_SAMPLE_BUFFER_SIZE - 1;

            uint8_t state = (capture_buffer[sample_idx] >> ch) & 0x01;
            int cur_y = state ? high_y : base_y;

            if (px > 0 && state != last_state) {
                // Vertical transition edge
                st7789_draw_fast_v_line(start_x + px, high_y, 12, channel_colors[ch]);
            }
            // Horizontal level pixel
            st7789_draw_pixel(start_x + px, cur_y, channel_colors[ch]);

            last_state = state;
            last_y = cur_y;
        }
    }

    // 4. Draw Center Timing Cursor
    for (int y = 20; y < 205; y += 2) {
        st7789_draw_pixel(g_ui.cursor_x, y, COLOR_WHITE);
    }

    // 5. Render Bottom Decoder Banner (205..240 px)
    st7789_fill_rect(0, 206, ST7789_WIDTH, 34, COLOR_NAVY);
    st7789_draw_fast_h_line(0, 205, ST7789_WIDTH, COLOR_LIGHTGREY);

    st7789_draw_string(5, 210, g_ui.decoder_text, COLOR_CYAN, COLOR_NAVY, 1);
    st7789_draw_string(5, 224, Trig: CH0 Edge | D0..D7 -> GPIO0..GPIO7, COLOR_LIGHTGREY, COLOR_NAVY, 1);
}
