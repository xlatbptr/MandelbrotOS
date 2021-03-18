#ifndef __PMM_H__
#define __PMM_H__

#include <boot/stivale2.h>
#include <stddef.h>

void pmm_init(struct stivale2_mmap_entry *memory_map, size_t memory_entries);

#endif // !__PMM_H__
