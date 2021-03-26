#ifndef __PMM_H__
#define __PMM_H__

#include <boot/stivale2.h>
#include <stddef.h>

enum {
  HBLOCK_FREE = 0,
  HBLOCK_USED = 1,
  HBLOCK_NOT_PRESENT = 2,
};

struct heap_master {
  size_t n_blocks;
  struct heap_block blocks[];
};

struct heap_block {
  size_t size;
  unsigned char type;
  struct heap_block * next;
};

void pmm_init(struct stivale2_mmap_entry *memory_map, size_t memory_entries);

#endif // !__PMM_H__
