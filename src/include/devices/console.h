#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <stdint.h>

void console_init(uint32_t *fb, uint16_t fb_width, uint16_t fb_height, uint8_t *font_, uint16_t font_width_, uint16_t font_height_);
void console_cls(uint32_t color);
void console_putc(int ch, uint32_t foreground_color, uint32_t background_color);
void console_puts(const char *string, uint32_t foreground_color, uint32_t background_color);

#endif
