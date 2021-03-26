#include <kernel/idt.h>

// Variable definitions
idt_entry_t idt[256];
idt_ptr_t idtp;

// Set IDT gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  idt[num].base_lo = base & 0xFFFF;
  idt[num].base_hi = (base >> 16) & 0xFFFF;
  idt[num].always0 = 0;
  idt[num].sel = sel;
  idt[num].flags = flags;
}

// Initialize IDT
int init_idt() {
  idtp.limit = (sizeof(idt_entry_t) * 256) - 1;
  idtp.base = (uint64_t)&idt;

  __asm__ volatile("lidtq (%0)" ::"r"(&idtp));

  return 0;
}