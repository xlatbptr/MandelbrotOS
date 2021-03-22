#include <string.h>
#include <kernel/idt.h>

idt_entry_t idt[256];
idt_ptr_t idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
  idt[num].base_low = (uint32_t)(base & 0xFFFF);
  idt[num].base_mid = (uint32_t)((base >> 16) & 0xFFFF);
  idt[num].base_high = (uint32_t)(base >> 32);

  idt[num].padding_1 = 0;
  idt[num].padding_2 = 0;

  idt[num].sel = sel;
  idt[num].flags = flags;
}

int init_idt() {
  idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
  idtp.base = (uint64_t)(idt);

  memset(idt, 0, sizeof(idt_entry_t) * 256);

  idt_load();

  return 0;
}
