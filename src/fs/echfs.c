#include <fs/echfs.h>
#include <kernel/device.h>
#include <stdint.h>
#include <string.h>

echfs_t echfs_get_fs(device_t device) {
  echfs_table_t table;
  device.read(device, 0, (uint8_t *)(&table), sizeof(echfs_table_t));

  echfs_t fs = (echfs_t){
      table.block_cnt, table.dir_block_cnt, table.block_size, 0, 0, 0};
  fs.alloc_offset = 16 * fs.block_size;
  fs.dir_offset = ((fs.alloc_offset + fs.block_cnt * sizeof(uint64_t) +
                    (fs.block_size - 1)) /
                   fs.block_size) *
                  fs.block_size;
  fs.dir_cnt = (fs.dir_block_cnt * fs.block_size) / sizeof(echfs_entry_t);

  return fs;
}

uint64_t echfs_load_block(device_t device, echfs_t fs, uint8_t *buffer,
                          uint64_t block) {
  uint64_t next_block;
  device.read(device, fs.alloc_offset + block * sizeof(uint64_t),
              (uint8_t *)(&next_block), sizeof(uint64_t));

  device.read(device, block * fs.block_size, buffer, fs.block_size);

  return next_block;
}

echfs_entry_t echfs_find(device_t device, echfs_t fs, uint64_t dir,
                         const char *name) {
  echfs_entry_t entry;

  for (uint64_t i = 0; i < fs.dir_cnt; i++) {
    device.read(device, fs.dir_offset + i * sizeof(echfs_entry_t),
                (uint8_t *)(&entry), sizeof(echfs_entry_t));

    if (!entry.parent_id)
      break;
    else if (entry.parent_id == dir && !strcmp(name, entry.name))
      return entry;
  }

  entry.type = 0xFF;
  return entry;
}

int echfs_read(device_t device, echfs_t fs, echfs_entry_t file,
               uint8_t *buffer) {
  if (!file.type || !(file.perms & ECHFS_READ_MASK))
    return 0;

  uint64_t block = file.blk_id;

  while (block && block < 0xFFFFFFFFFFFFFFF0) {
    block = echfs_load_block(device, fs, buffer, block);
    buffer += fs.block_size;
  }

  return 1;
}

uint64_t echfs_get_size(device_t device, echfs_t fs, echfs_entry_t file) {
  return file.size;
}
