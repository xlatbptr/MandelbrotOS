#include <drivers/ata.h>
#include <hw.h>

void ata_wait_bsy(drive_t *device) {
  while (inb(device->base_port + ATA_PORT_STATUS) & ATA_STATUS_BSY)
    ;
}

bool ata_wait_drq(drive_t *device) {
  while (true) {
    uint8_t status = inb(device->base_port + ATA_PORT_STATUS);
    if (status & ATA_STATUS_DRQ)
      return true;
    if (status & ATA_STATUS_ERR || status & ATA_STATUS_DF)
      return false;
  }
}

void ata_pio_send_lba48(drive_t *device, uint64_t lba, uint16_t sectors) {
  outb(device->base_port + ATA_PORT_SECTOR_COUNT, sectors >> 8);
  outb(device->base_port + ATA_PORT_LBA_LO, lba >> 24);
  outb(device->base_port + ATA_PORT_LBA_MID, lba >> 32);
  outb(device->base_port + ATA_PORT_LBA_HI, lba >> 40);
  outb(device->base_port + ATA_PORT_SECTOR_COUNT, sectors);
  outb(device->base_port + ATA_PORT_LBA_LO, lba);
  outb(device->base_port + ATA_PORT_LBA_MID, lba >> 8);
  outb(device->base_port + ATA_PORT_LBA_HI, lba >> 16);
}

// FIXME: This is completely broken
bool has_ata_drive(drive_t *device, bool slave) {
  // FIXME: This piece breaks it
  // if (! slave) outb(device->base_port + ATA_PORT_DRIVE_HEAD, 0xA0);
  // else outb(device->base_port + ATA_PORT_DRIVE_HEAD, 0xB0);

  for (int i = 0; i < 4; ++i) {
    inb(device->base_control_port + ATA_CONTROL_STATUS);
  }

  // FIXME: This one breaks it too
  // outb(device->base_port + ATA_PORT_SECTOR_COUNT, 0);
  // outb(device->base_port + ATA_PORT_LBA_LO, 0);
  // outb(device->base_port + ATA_PORT_LBA_MID, 0);
  // outb(device->base_port + ATA_PORT_LBA_HI, 0);
  // outb(device->base_port + ATA_PORT_COMMAND, ATA_COMMAND_IDENTIFY);

  uint8_t status = inb(device->base_port + ATA_PORT_STATUS);
  return status != 0;
}

int ata_pio_read(uint16_t *target, uint64_t lba, uint16_t sectors,
                 drive_t *device) {
  ata_wait_bsy(device);
  ata_pio_send_lba48(device, lba, sectors);

  outb(device->base_port + ATA_PORT_COMMAND, ATA_COMMAND_READ_SECTORS_EXT);

  for (unsigned int i = 0; i < sectors; i++) {
    ata_wait_bsy(device);
    if (!ata_wait_drq(device)) {
      uint8_t status = inb(device->base_port + ATA_PORT_STATUS);
      if (status & ATA_STATUS_ERR) {
        return 1;
      }
      if (status & ATA_STATUS_DF) {
        return 2;
      }
    }
    for (int j = 0; j < 512; j+=2){
      uint16_t value = inw(device->base_port + ATA_PORT_DATA);
      target[j] = value & 0xFF;
      target[j+1] = (value >> 8) & 0xFF;
    }
    target += 512;
  }
  return 0;
}

int ata_pio_write(uint16_t *bytes, uint64_t lba, uint16_t sectors,
                  drive_t *device) {
  ata_wait_bsy(device);
  ata_pio_send_lba48(device, lba, sectors);

  outb(device->base_port + ATA_PORT_COMMAND, ATA_COMMAND_WRITE_SECTORS_EXT);

  for (unsigned int i = 0; i < sectors; ++i) {
    ata_wait_bsy(device);
    if (!ata_wait_drq(device)) {
      uint8_t status = inb(device->base_port + ATA_PORT_STATUS);
      if (status & ATA_STATUS_ERR) {
        return 1;
      }
      if (status & ATA_STATUS_DF) {
        return 2;
      }
    }
    for (int j = 0; j < 256; ++j) {
      outw(device->base_port + ATA_PORT_DATA, bytes[j]);
    }
    bytes += 256;
  }
  return 0;
}