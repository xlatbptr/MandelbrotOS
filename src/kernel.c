#include <stdint.h>
#include <font.h>
#include <boot/stivale2.h>
#include <devices/console.h>

void kmain(struct stivale2_struct *bootloader_info) {
    // First get the framebuffer
    struct stivale2_tag *tag;
    if ((tag = stivale2_get_tag(bootloader_info, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID))) {
        struct stivale2_struct_tag_framebuffer *framebuffer_info = (struct stivale2_struct_tag_framebuffer *) tag;
        console_init(
            (uint32_t *) framebuffer_info->framebuffer_addr,
            framebuffer_info->framebuffer_width,
            framebuffer_info->framebuffer_height,
            (uint8_t *) font,
            GLYPH_WIDTH,
            GLYPH_HEIGHT
        );
    } else {
        for (;;) {} // No framebuffer at all
    }
    console_puts("Welcome to MandelbrotOS!", 0xffffff, 0);
}