#include <boot/stivale2.h>
#include <mm/pmm.h>
#include <printf.h>
#include <stddef.h>

// Simple watermark allocator. Works 99% of the time until you run out of memory (aka. never).

void * bottom;

void * pmalloc(size_t size) {
  void * ptr = bottom;
  bottom = (void *)((uintptr_t)bottom + size);
  return ptr;
}

void * prealloc(void * ptr, size_t size) {
  if(ptr == NULL) {
    return pmalloc(size);
  }
  
  // We just allocate again...
  void * new_ptr = pmalloc(size);
  memcpy(ptr,new_ptr,size);
  return new_ptr;
}

void pfree(void * ptr) {
  // We don't actually free!
  return;
}

// Init physical memory management
void pmm_init(struct stivale2_mmap_entry *memory_map, size_t memory_entries) {
  uintptr_t highest_page = 0;
  for (size_t i = 0; i < memory_entries; i++) {
    struct stivale2_mmap_entry entry = memory_map[i];

    if (entry.type != STIVALE2_MMAP_USABLE &&
        entry.type != STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE &&
        entry.type != STIVALE2_MMAP_KERNEL_AND_MODULES)
      continue;

    uintptr_t top = entry.base + entry.length;

    if (top > highest_page)
      highest_page = top;
    bottom = entry.base;
  }
}
