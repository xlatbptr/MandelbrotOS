#include <drivers/ata.h>
#include <drivers/serial.h>
#include <font.h>
#include <kernel/alloc.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <kernel/isr.h>
#include <kernel/kbd.h>
#include <kernel/kshell.h>
#include <kernel/pit.h>
#include <kernel/power.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <macros.h>
#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

int do_checks(unsigned long magic, void *mbi) {
  if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
    serial_writestring("Invalid multiboot2 magic\r\n");
    return 1; // return to boot.s and halt
  }
  if (init_vbe(mbi) != 0) {
    serial_writestring("Failed to find framebuffer tag");
    return 1;
  }
  return 0;
}

int kernel_main(unsigned long magic, unsigned long addr) {

  // Variable definitions
  inited_funs_no = 0;
  void *mbi = (void *)addr;

  // Init functions
  init_serial();
  do_checks(magic, mbi);
  init_color(0xff0000, 0x990000, 0x00ff00, 0x009900, 0xffff00, 0x999900,
             0x0000ff, 0x000099, 0xff00ff, 0x990099, 0x00ffff, 0x009999,
             0xffffff, 0x000000, 0x999999, 0x444444, 0x161925, 0xdedede);
  init_text(5);
  init_check(init_gdt(), "GDT", true);
  init_check(init_idt(), "IDT", true);
  init_check(init_isr(), "ISR", true);
  init_check(init_irq(), "IRQ", true);
  init_check(enable_interrupts(), "Interrupts", true);
  init_check(init_timer(), "Programmable interrupt timer", true);
  init_check(init_heap(0x00f00000), "Memory allocator", true);
  init_check(kbd_init(), "Keyboard", true);
  set_kbd();

  printf("\r\n");
  printf("The keyboard layout is: %s\r\n", kbd);

  serial_writestring("Serial inited!\r\n");

  printf("reading...\r\n");

  uint32_t *target;

  read_sectors_ATA_PIO(target, 0x0, 1);

  int i;
  i = 0;
  while (i < 128) {
    printf("%x ", target[i] & 0xFF);
    printf("%x ", (target[i] >> 8) & 0xFF);
    i++;
  }

  printf("\r\n");
  printf("writing 0...\r\n");
  char bwrite[512];
  bwrite[0] = 0xDE;
  bwrite[1] = 0xAD;
  bwrite[2] = 0xBE;
  bwrite[3] = 0xEF;
  bwrite[4] = 0x12;
  bwrite[5] = 0x34;
  bwrite[6] = 0x56;
  bwrite[7] = 0x78;
  for (i = 8; i < 512; i++) {
    bwrite[i] = 0x0;
  }
  write_sectors_ATA_PIO(0x0, 0, bwrite);

  printf("reading...\r\n");
  read_sectors_ATA_PIO(target, 0x0, 0);
  i = 0;
  while (i < 128) {
    printf("%x ", target[i] & 0xFF);
    printf("%x ", (target[i] >> 8) & 0xFF);
    i++;
  }

  // kshell(mbi, magic);

  return 0;
}
