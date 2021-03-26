#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <stdint.h>

typedef struct device_t device_t;

struct device_t {
  void *data;

  int (*read)(device_t device, uint64_t address, uint8_t *buffer,
              uint64_t size);
  int (*write)(device_t device, uint64_t address, uint8_t *buffer,
               uint64_t size);
  uint64_t (*get_size)(device_t device);
};

#endif // !__DEVICE_H__
