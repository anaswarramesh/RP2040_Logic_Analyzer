#include st7789.h
#include hardware/pwm.h
#include hardware/gpio.h

// Basic 5x7 ASCII Font table subset (Space to ~)
static const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // Space
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // 
 0x14, 0x7F, 0x14, 0x7F, 0x14, // #
 0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
 0x23, 0x13, 0x08, 0x64, 0x62, // %
 0x36, 0x49, 0x55, 0x22, 0x50, // &
 0x00, 0x05, 0x03, 0x00, 0x00, // '
 0x00, 0x1C, 0x22, 0x41, 0x00, // (
 0x00, 0x41, 0x22, 0x1C, 0x00, // )
 0x14, 0x08, 0x3E, 0x08, 0x14, // *
 0x08, 0x08, 0x3E, 0x08, 0x08, // +
 0x00, 0x50, 0x30, 0x00, 0x00, // ,
 0x08, 0x08, 0x08, 0x08, 0x08, // -
 0x00, 0x60, 0x60, 0x00, 0x00, // .
 0x20, 0x10, 0x08, 0x04, 0x02, // /
 0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
 0x00, 0x42, 0x7F, 0x40, 0x00, // 1
 0x42, 0x61, 0x51, 0x49, 0x46, // 2
 0x21, 0x41, 0x45, 0x4B, 0x31, // 3
 0x18, 0x14, 0x12, 0x7F, 0x10, // 4
 0x27, 0x45, 0x45, 0x45, 0x39, // 5
 0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
 0x01, 0x71, 0x09, 0x05, 0x03, // 7
 0x36, 0x49, 0x49, 0x49, 0x36, // 8
 0x06, 0x49, 0x49, 0x29, 0x1E, // 9
 0x00, 0x36, 0x36, 0x00, 0x00, // :
 0x00, 0x56, 0x36, 0x00, 0x00, // ;
 0x08, 0x14, 0x22, 0x41, 0x00, // <
 0x14, 0x14, 0x14, 0x14, 0x14, // =
 0x00, 0x41, 0x22, 0x14, 0x08, // >
 0x02, 0x01, 0x51, 0x09, 0x06, // ?
 0x32, 0x49, 0x79, 0x41, 0x3E, // @
 0x7E, 0x11, 0x11, 0x11, 0x7E, // A
 0x7F, 0x49, 0x49, 0x49, 0x36, // B
 0x3E, 0x41, 0x41, 0x41, 0x22, // C
 0x7F, 0x41, 0x41, 0x22, 0x1C, // D
 0x7F, 0x49, 0x49, 0x49, 0x41, // E
 0x7F, 0x09, 0x09, 0x09, 0x01, // F
 0x3E, 0x41, 0x49, 0x49, 0x7A, // G
 0x7F, 0x08, 0x08, 0x08, 0x7F, // H
 0x00, 0x41, 0x7F, 0x41, 0x00, // I
 0x20, 0x40, 0x41, 0x3F, 0x01, // J
 0x7F, 0x08, 0x14, 0x22, 0x41, // K
 0x7F, 0x40, 0x40, 0x40, 0x40, // L
 0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
 0x7F, 0x04, 0x08, 0x10, 0x7F, // N
 0x3E, 0x41, 0x41, 0x41, 0x3E, // O
 0x7F, 0x09, 0x09, 0x09, 0x06, // P
 0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
 0x7F, 0x09, 0x19, 0x29, 0x46, // R
 0x46, 0x49, 0x49, 0x49, 0x31, // S
 0x01, 0x01, 0x7F, 0x01, 0x01, // T
 0x3F, 0x40, 0x40, 0x40, 0x3F, // U
 0x1F, 0x20, 0x40, 0x20, 0x1F, // V
 0x3F, 0x40, 0x38, 0x40, 0x3F, // W
 0x63, 0x14, 0x08, 0x14, 0x63, // X
 0x07, 0x08, 0x70, 0x08, 0x07, // Y
 0x61, 0x51, 0x49, 0x45, 0x43, // Z
 0x00, 0x7F, 0x41, 0x41, 0x00, // [
 0x02, 0x04, 0x08, 0x10, 0x20, // \
 0x00, 0x41, 0x41, 0x7F, 0x00, // ]
 0x04, 0x02, 0x01, 0x02, 0x04, // ^
 0x40, 0x40, 0x40, 0x40, 0x40 // _
};

static inline void lcd_write_cmd(uint8_t cmd) {
 gpio_put(PIN_LCD_DC, 0);
 gpio_put(PIN_LCD_CS, 0);
 spi_write_blocking(LCD_SPI_PORT, &cmd, 1);
 gpio_put(PIN_LCD_CS, 1);
}

static inline void lcd_write_data(const uint8_t *data, size_t len) {
 gpio_put(PIN_LCD_DC, 1);
 gpio_put(PIN_LCD_CS, 0);
 spi_write_blocking(LCD_SPI_PORT, data, len);
 gpio_put(PIN_LCD_CS, 1);
}

static inline void lcd_write_data_byte(uint8_t data) {
 lcd_write_data(&data, 1);
}

void st7789_set_backlight(uint8_t brightness) {
 gpio_set_function(PIN_LCD_BL, GPIO_FUNC_PWM);
 uint slice_num = pwm_gpio_to_slice_num(PIN_LCD_BL);
 pwm_set_wrap(slice_num, 255);
 pwm_set_chan_level(slice_num, pwm_gpio_to_channel(PIN_LCD_BL), brightness);
 pwm_set_enabled(slice_num, true);
}

void st7789_init(void) {
 // 62.5 MHz SPI Clock (RP2040 clk_peri / 2)
 spi_init(LCD_SPI_PORT, 62500000);
 gpio_set_function(PIN_LCD_SCK, GPIO_FUNC_SPI);
 gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);

 gpio_init(PIN_LCD_CS);
 gpio_set_dir(PIN_LCD_CS, GPIO_OUT);
 gpio_put(PIN_LCD_CS, 1);

 gpio_init(PIN_LCD_DC);
 gpio_set_dir(PIN_LCD_DC, GPIO_OUT);

 gpio_init(PIN_LCD_RST);
 gpio_set_dir(PIN_LCD_RST, GPIO_OUT);

 // Hardware Reset
 gpio_put(PIN_LCD_RST, 0);
 sleep_ms(50);
 gpio_put(PIN_LCD_RST, 1);
 sleep_ms(120);

 lcd_write_cmd(0x11); // Sleep Out
 sleep_ms(120);

 lcd_write_cmd(0x36); // Memory Data Access Control (Orientation: Landscape 320x240)
 lcd_write_data_byte(0x70); 

 lcd_write_cmd(0x3A); // Color format: 16-bit RGB565
 lcd_write_data_byte(0x05);

 lcd_write_cmd(0xB2); // Porch Control
 const uint8_t porch[] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
 lcd_write_data(porch, sizeof(porch));

 lcd_write_cmd(0xB7); // Gate Control
 lcd_write_data_byte(0x35);

 lcd_write_cmd(0xBB); // VCOM Setting
 lcd_write_data_byte(0x19);

 lcd_write_cmd(0xC0); // LCM Control
 lcd_write_data_byte(0x2C);

 lcd_write_cmd(0xC2); // VDV and VRH Command Enable
 lcd_write_data_byte(0x01);

 lcd_write_cmd(0xC3); // VRH Set
 lcd_write_data_byte(0x12);

 lcd_write_cmd(0xC4); // VDV Set
 lcd_write_data_byte(0x20);

 lcd_write_cmd(0xC6); // Frame Rate Control in Normal Mode
 lcd_write_data_byte(0x0F);

 lcd_write_cmd(0xD0); // Power Control 1
 const uint8_t pwr[] = {0xA4, 0xA1};
 lcd_write_data(pwr, sizeof(pwr));

 lcd_write_cmd(0x21); // Display Inversion ON
 lcd_write_cmd(0x29); // Display ON
 sleep_ms(20);

 st7789_set_backlight(220); // 85% brightness
}

void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
 lcd_write_cmd(0x2A); // CASET
 uint8_t caset[4] = { (uint8_t)(x0 >> 8), (uint8_t)(x0 & 0xFF), (uint8_t)(x1 >> 8), (uint8_t)(x1 & 0xFF) };
 lcd_write_data(caset, 4);

 lcd_write_cmd(0x2B); // RASET
 uint8_t raset[4] = { (uint8_t)(y0 >> 8), (uint8_t)(y0 & 0xFF), (uint8_t)(y1 >> 8), (uint8_t)(y1 & 0xFF) };
 lcd_write_data(raset, 4);

 lcd_write_cmd(0x2C); // RAMWR
}

void st7789_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
 if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT || w <= 0 || h <= 0) return;
 if (x + w > ST7789_WIDTH) w = ST7789_WIDTH - x;
 if (y + h > ST7789_HEIGHT) h = ST7789_HEIGHT - y;

 st7789_set_window(x, y, x + w - 1, y + h - 1);

 uint8_t buf[64];
 uint8_t hi = color >> 8;
 uint8_t lo = color & 0xFF;
 for (int i = 0; i < 64; i += 2) {
 buf[i] = hi;
 buf[i+1] = lo;
 }

 uint32_t total_pixels = w * h;
 gpio_put(PIN_LCD_DC, 1);
 gpio_put(PIN_LCD_CS, 0);
 while (total_pixels > 0) {
 uint32_t chunk = (total_pixels > 32) ? 32 : total_pixels;
 spi_write_blocking(LCD_SPI_PORT, buf, chunk * 2);
 total_pixels -= chunk;
 }
 gpio_put(PIN_LCD_CS, 1);
}

void st7789_fill_screen(uint16_t color) {
 st7789_fill_rect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void st7789_draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint16_t color) {
 st7789_fill_rect(x, y, w, 1, color);
}

void st7789_draw_fast_v_line(int16_t x, int16_t y, int16_t h, uint16_t color) {
 st7789_fill_rect(x, y, 1, h, color);
}

void st7789_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size) {
 if (c < 32 || c > 126) c = '?';
 const uint8_t *glyph = &font5x7[(c - 32) * 5];

 for (int i = 0; i < 5; i++) {
 uint8_t line = glyph[i];
 for (int j = 0; j < 8; j++) {
 if (line & 0x01) {
 if (size == 1) st7789_fill_rect(x + i, y + j, 1, 1, color);
 else st7789_fill_rect(x + (i * size), y + (j * size), size, size, color);
 } else if (bg != color) {
 if (size == 1) st7789_fill_rect(x + i, y + j, 1, 1, bg);
 else st7789_fill_rect(x + (i * size), y + (j * size), size, size, bg);
 }
 line >>= 1;
 }
 }
}

void st7789_draw_string(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size) {
 while (*str) {
 st7789_draw_char(x, y, *str, color, bg, size);
 x += (6 * size);
 str++;
 }
}
