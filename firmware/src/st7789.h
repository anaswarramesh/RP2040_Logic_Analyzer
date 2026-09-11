#ifndef ST7789_H
#define ST7789_H

#include <stdint.h>
#include <stdbool.h>
#include pico/stdlib.h
#include hardware/spi.h

#define ST7789_WIDTH   320
#define ST7789_HEIGHT  240

// Hardware Pin Definitions
#define PIN_LCD_SCK    10
#define PIN_LCD_MOSI   11
#define PIN_LCD_CS     13
#define PIN_LCD_DC     14
#define PIN_LCD_RST    15
#define PIN_LCD_BL     27
#define LCD_SPI_PORT   spi1

// Standard 16-bit RGB565 Colors
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_NAVY        0x000F
#define COLOR_DARKGREY    0x39E7
#define COLOR_GREY        0x7BEF
#define COLOR_LIGHTGREY   0xC618
#define COLOR_CYAN        0x07FF
#define COLOR_YELLOW      0xFFE0
#define COLOR_GREEN       0x07E0
#define COLOR_MAGENTA     0xF81F
#define COLOR_RED         0xF800
#define COLOR_ORANGE      0xFD20
#define COLOR_BLUE        0x001F

void st7789_init(void);
void st7789_set_backlight(uint8_t brightness);
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void st7789_fill_screen(uint16_t color);
void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7789_draw_fast_h_line(int16_t x, int16_t y, int16_t w, uint16_t color);
void st7789_draw_fast_v_line(int16_t x, int16_t y, int16_t h, uint16_t color);
void st7789_fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void st7789_draw_char(int16_t x, int16_t y, char c, uint16_t color, uint16_t bg, uint8_t size);
void st7789_draw_string(int16_t x, int16_t y, const char *str, uint16_t color, uint16_t bg, uint8_t size);

#endif // ST7789_H
