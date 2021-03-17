#include <kernel/device.h>
#include <fs/flatfs.h>
#include <stdint.h>
#include <string.h>

flatfs_t flatfs_get_fs(device_t device) {
  flatfs_t fs;
  device.read(device, 512 - sizeof(flatfs_t), &fs, sizeof(flatfs_t));

  return fs;
}

flatfs_header_t flatfs_find(device_t device, flatfs_t fs, uint64_t dir, const char *name) {

}

int flatfs_read(device_t device, flatfs_t fs, flatfs_header_t header, uint8_t *buffer) {

}

uint64_t flatfs_get_size(device_t device, flatfs_t fs, flatfs_header_t header) {

}
