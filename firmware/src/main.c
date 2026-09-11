#include <stdio.h>
#include pico/stdlib.h
#include pico/multicore.h
#include hardware/vreg.h
#include hardware/clocks.h

#include capture.h
#include st7789.h
#include ui.h
#include sigrok_protocol.h

//
// Core 1 Entry: High-Speed Display Refresh & User Interaction Loop
//
void core1_entry(void) {
    st7789_init();
    st7789_fill_screen(COLOR_BLACK);
    ui_init();

    while (true) {
        ui_poll_buttons();
        ui_render_frame();
        sleep_ms(30); // ~33 FPS UI refresh rate
    }
}

//
// Core 0 Entry: System Initialization, Sampling & USB Logic Engine
//
int main(void) {
    // Set system clock to 133.0 MHz (Standard fast clock for RP2040)
    vreg_set_voltage(VREG_VOLTAGE_1_15);
    set_sys_clock_khz(133000, true);

    stdio_init_all();
    sigrok_protocol_init();
    capture_init();

    // Start initial continuous capture for standalone display
    capture_arm(2000000, 10000, 0, TRIG_NONE); // 2 MSPS, 10k samples

    // Launch UI engine on Core 1
    multicore_launch_core1(core1_entry);

    // Core 0 Main Loop
    while (true) {
        // Handle USB CDC / Sigrok PC commands
        sigrok_protocol_task();

        // If in standalone mode and previous frame capture finished, re-arm capture
        if (g_ui.is_running && capture_is_complete()) {
            capture_arm(g_capture_cfg.sample_rate_hz, g_capture_cfg.sample_count, g_ui.selected_channel, TRIG_NONE);
        }

        tight_loop_contents();
    }

    return 0;
}
