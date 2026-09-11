#include capture.h
#include hardware/clocks.h
#include logic_analyzer.pio.h

// Dedicated DMA Capture Buffer isolated to prevent crossbar bus contention with Core 1 / Display
uint8_t capture_buffer[MAX_SAMPLE_BUFFER_SIZE] __attribute__((aligned(4), section(".uninitialized_data")));
volatile capture_config_t g_capture_cfg = {
    .sample_rate_hz = 10000000, // Default 10 MSPS
    .sample_count = 10000,
    .trigger_channel = 0,
    .trigger_type = TRIG_NONE,
    .is_running = false,
    .capture_complete = false
};

static PIO capture_pio = pio0;
static uint capture_sm = 0;
static uint pio_offset = 0;
static int dma_chan = -1;

void capture_init(void) {
    pio_offset = pio_add_program(capture_pio, &logic_analyzer_program);
    dma_chan = dma_claim_unused_channel(true);
}

void capture_arm(uint32_t sample_rate_hz, uint32_t sample_count, uint8_t trig_chan, trigger_type_t trig_type) {
    if (sample_count > MAX_SAMPLE_BUFFER_SIZE) {
        sample_count = MAX_SAMPLE_BUFFER_SIZE;
    }
    
    g_capture_cfg.sample_rate_hz = sample_rate_hz;
    g_capture_cfg.sample_count = sample_count;
    g_capture_cfg.trigger_channel = trig_chan;
    g_capture_cfg.trigger_type = trig_type;
    g_capture_cfg.capture_complete = false;
    g_capture_cfg.is_running = true;

    // Stop PIO and clear FIFOs
    pio_sm_set_enabled(capture_pio, capture_sm, false);
    pio_sm_clear_fifos(capture_pio, capture_sm);
    pio_sm_restart(capture_pio, capture_sm);

    // Calculate clock divider for PIO
    float div = (float)clock_get_hz(clk_sys) / (float)sample_rate_hz;
    if (div < 1.0f) div = 1.0f;

    logic_analyzer_program_init(capture_pio, capture_sm, pio_offset, LOGIC_PIN_BASE, div);

    // Setup DMA transfer from PIO RX FIFO to capture_buffer (32-bit words)
    dma_channel_config dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, pio_get_dreq(capture_pio, capture_sm, false));
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_32);

    uint32_t word_count = (sample_count + 3) / 4;
    dma_channel_configure(
        dma_chan,
        &dma_cfg,
        capture_buffer,
        &capture_pio->rxf[capture_sm],
        word_count,
        true // Start immediately
    );

    // Enable PIO state machine
    pio_sm_set_enabled(capture_pio, capture_sm, true);
}

bool capture_is_complete(void) {
    if (!g_capture_cfg.is_running) {
        return g_capture_cfg.capture_complete;
    }

    if (!dma_channel_is_busy(dma_chan)) {
        pio_sm_set_enabled(capture_pio, capture_sm, false);
        g_capture_cfg.is_running = false;
        g_capture_cfg.capture_complete = true;
        return true;
    }
    return false;
}

void capture_stop(void) {
    pio_sm_set_enabled(capture_pio, capture_sm, false);
    if (dma_chan >= 0) {
        dma_channel_abort(dma_chan);
    }
    g_capture_cfg.is_running = false;
    g_capture_cfg.capture_complete = false;
}
