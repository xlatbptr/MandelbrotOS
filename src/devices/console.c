#include <stdint.h>
#include <devices/console.h>

static uint32_t *framebuffer;
static uint16_t framebuffer_width;
static uint16_t framebuffer_height;

static uint8_t *font;
static uint16_t font_width;
static uint16_t font_height;

static int x_pos_pixel = 0;
static int y_pos_pixel = 0;

static void console_plot_pixel(int x, int y, uint32_t color) {
    framebuffer[y * framebuffer_width + x] = color;
}

void console_init(uint32_t *fb, uint16_t fb_width, uint16_t fb_height, uint8_t *font_, uint16_t font_width_, uint16_t font_height_) {
    framebuffer = fb;
    framebuffer_width = fb_width;
    framebuffer_height = fb_height;
    font = font_;
    font_width = font_width_;
    font_height = font_height_;
}

void console_cls(uint32_t color) {
    x_pos_pixel = 0;
    y_pos_pixel = 0;
    for (int i = 0; i < framebuffer_width * framebuffer_height; i++) {
        framebuffer[i] = color;
    }
}

void console_putc(int ch, uint32_t foreground_color, uint32_t background_color) {
    if (ch == '\n') {
        goto newline;
    }
    uint8_t *font_glyph = &font[ch * font_height];
    for (int i = 0; i < font_height; i++) {
        for (int j = font_width; j > 0; j--) {
            if (font_glyph[i] & (1 << j)) {
                console_plot_pixel(x_pos_pixel++, y_pos_pixel, foreground_color);
            } else {
                console_plot_pixel(x_pos_pixel++, y_pos_pixel, background_color);
            }
        }
        y_pos_pixel++;
        x_pos_pixel -= font_width;
    }
    y_pos_pixel -= font_height;
    x_pos_pixel += font_width;
    if (x_pos_pixel >= framebuffer_width) {
newline:
        x_pos_pixel = 0;
        y_pos_pixel += font_height;
    }
}

void console_puts(const char *string, uint32_t foreground_color, uint32_t background_color) {
    while (*string) {
        console_putc(*string++, foreground_color, background_color);
    }
}
