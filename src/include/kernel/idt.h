#ifndef __IDT_H__
#define __IDT_H__

#include <stdint.h>

typedef struct idt_entry {
  uint16_t base_low;
  uint16_t sel;

  uint8_t padding_1;

  uint8_t flags;
  uint16_t base_mid;

  // 64-bit extension
  uint32_t base_high;
  uint32_t padding_2;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) idt_ptr_t;

extern idt_entry_t idt[256];
extern idt_ptr_t idtp;

extern void idt_load();

int init_idt();
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);

#endif // !__IDT_H__
