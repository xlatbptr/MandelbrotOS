#include <drivers/ata.h>
#include <drivers/serial.h>
#include <hw.h>
#include <kernel/pit.h>
#include <kernel/text.h>
#include <macros.h>

struct IDEChannelRegisters {
  unsigned short base;  // I/O Base.
  unsigned short ctrl;  // Control Base
  unsigned short bmide; // Bus Master IDE
  unsigned char nIEN;   // nIEN (No Interrupt);
} channels[2];

struct ide_device {
  unsigned char Reserved;      // 0 (Empty) or 1 (This Drive really exists).
  unsigned char Channel;       // 0 (Primary Channel) or 1 (Secondary Channel).
  unsigned char Drive;         // 0 (Master Drive) or 1 (Slave Drive).
  unsigned short Type;         // 0: ATA, 1:ATAPI.
  unsigned short Signature;    // Drive Signature
  unsigned short Capabilities; // Features.
  unsigned int CommandSets;    // Command Sets Supported.
  unsigned int Size;           // Size in Sectors.
  unsigned char Model[41];     // Model in string.
} ide_devices[4];

unsigned char ide_buf[2048] = {0};
unsigned static char ide_irq_invoked = 0;

void ide_write(unsigned char channel, unsigned char reg, unsigned char data) {
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
  if (reg < 0x08)
    outb(channels[channel].base + reg - 0x00, data);
  else if (reg < 0x0C)
    outb(channels[channel].base + reg - 0x06, data);
  else if (reg < 0x0E)
    outb(channels[channel].ctrl + reg - 0x0A, data);
  else if (reg < 0x16)
    outb(channels[channel].bmide + reg - 0x0E, data);
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_read(unsigned char channel, unsigned char reg) {
  unsigned char result;
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
  if (reg < 0x08)
    result = inb(channels[channel].base + reg - 0x00);
  else if (reg < 0x0C)
    result = inb(channels[channel].base + reg - 0x06);
  else if (reg < 0x0E)
    result = inb(channels[channel].ctrl + reg - 0x0A);
  else if (reg < 0x16)
    result = inb(channels[channel].bmide + reg - 0x0E);
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
  return result;
}

void ide_read_buffer(unsigned char channel, unsigned char reg,
                     unsigned int *buffer, unsigned int quads) {
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, 0x80 | channels[channel].nIEN);
  __asm__("pushw %es; pushw %ax; movw %ds, %ax; movw %ax, %es; popw %ax;");
  if (reg < 0x08)
    insl(channels[channel].base + reg - 0x00, buffer, quads);
  else if (reg < 0x0C)
    insl(channels[channel].base + reg - 0x06, buffer, quads);
  else if (reg < 0x0E)
    insl(channels[channel].ctrl + reg - 0x0A, buffer, quads);
  else if (reg < 0x16)
    insl(channels[channel].bmide + reg - 0x0E, buffer, quads);
  __asm__("popw %es;");
  if (reg > 0x07 && reg < 0x0C)
    ide_write(channel, ATA_REG_CONTROL, channels[channel].nIEN);
}

unsigned char ide_polling(unsigned char channel, unsigned int advanced_check) {

  // (I) Delay 400 nanosecond for BSY to be set:
  // -------------------------------------------------
  for (int i = 0; i < 4; i++)
    ide_read(channel, ATA_REG_ALTSTATUS); // Reading the Alternate Status port
                                          // wastes 100ns; loop four times.

  // (II) Wait for BSY to be cleared:
  // -------------------------------------------------
  while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
    ; // Wait for BSY to be zero.

  if (advanced_check) {
    unsigned char state =
        ide_read(channel, ATA_REG_STATUS); // Read Status Register.

    // (III) Check For Errors:
    // -------------------------------------------------
    if (state & ATA_SR_ERR)
      return 2; // Error.

    // (IV) Check If Device fault:
    // -------------------------------------------------
    if (state & ATA_SR_DF)
      return 1; // Device Fault.

    // (V) Check DRQ:
    // -------------------------------------------------
    // BSY = 0; DF = 0; ERR = 0 so we should check for DRQ now.
    if ((state & ATA_SR_DRQ) == 0)
      return 3; // DRQ should be set
  }

  return 0; // No Error.
}

unsigned char ide_print_error(unsigned int drive, unsigned char err) {
  if (err == 0)
    return err;

  serial_writestring("IDE:");
  if (err == 1) {
    serial_writestring("- Device Fault\r\n     ");
    err = 19;
  } else if (err == 2) {
    unsigned char st = ide_read(ide_devices[drive].Channel, ATA_REG_ERROR);
    if (st & ATA_ER_AMNF) {
      serial_writestring("- No Address Mark Found\r\n     ");
      err = 7;
    }
    if (st & ATA_ER_TK0NF) {
      serial_writestring("- No Media or Media Error\r\n     ");
      err = 3;
    }
    if (st & ATA_ER_ABRT) {
      serial_writestring("- Command Aborted\r\n     ");
      err = 20;
    }
    if (st & ATA_ER_MCR) {
      serial_writestring("- No Media or Media Error\r\n     ");
      err = 3;
    }
    if (st & ATA_ER_IDNF) {
      serial_writestring("- ID mark not Found\r\n     ");
      err = 21;
    }
    if (st & ATA_ER_MC) {
      serial_writestring("- No Media or Media Error\r\n     ");
      err = 3;
    }
    if (st & ATA_ER_UNC) {
      serial_writestring("- Uncorrectable Data Error\r\n     ");
      err = 22;
    }
    if (st & ATA_ER_BBK) {
      serial_writestring("- Bad Sectors\r\n     ");
      err = 13;
    }
  } else if (err == 3) {
    serial_writestring("- Reads Nothing\r\n     ");
    err = 23;
  } else if (err == 4) {
    serial_writestring("- Write Protected\r\n     ");
    err = 8;
  }

  return err;
}

void init_ide(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2,
              unsigned int BAR3, unsigned int BAR4) {
  int j, k, i, count = 0;

  // 1- Detect I/O Ports which interface IDE Controller:
  channels[ATA_PRIMARY].base = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
  channels[ATA_PRIMARY].ctrl = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
  channels[ATA_SECONDARY].base = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
  channels[ATA_SECONDARY].ctrl = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
  channels[ATA_PRIMARY].bmide = (BAR4 & 0xFFFFFFFC) + 0;   // Bus Master IDE
  channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

  // 2- Disable IRQs:
  ide_write(ATA_PRIMARY, ATA_REG_CONTROL, 2);
  ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

  // 3- Detect ATA-ATAPI Devices:
  for (i = 0; i < 2; i++)
    for (j = 0; j < 2; j++) {

      unsigned char err = 0, type = IDE_ATA, status;
      ide_devices[count].Reserved = 0; // Assuming that no drive here.

      // (I) Select Drive:
      ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4)); // Select Drive.
      sleep(1); // Wait 1ms for drive select to work.

      // (II) Send ATA Identify Command:
      ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
      sleep(1); // This function should be implemented in your OS. which waits
                // for 1 ms. it is based on System Timer Device Driver.

      // (III) Polling:
      if (ide_read(i, ATA_REG_STATUS) == 0)
        continue; // If Status = 0, No Device.

      while (1) {
        status = ide_read(i, ATA_REG_STATUS);
        if ((status & ATA_SR_ERR)) {
          err = 1;
          break;
        } // If Err, Device is not ATA.
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
          break; // Everything is right.
      }

      // (IV) Probe for ATAPI Devices:

      if (err != 0) {
        unsigned char cl = ide_read(i, ATA_REG_LBA1);
        unsigned char ch = ide_read(i, ATA_REG_LBA2);

        if (cl == 0x14 && ch == 0xEB)
          type = IDE_ATAPI;
        else if (cl == 0x69 && ch == 0x96)
          type = IDE_ATAPI;
        else
          continue; // Unknown Type (may not be a device).

        ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
        sleep(1);
      }

      // (V) Read Identification Space of the Device:
      ide_read_buffer(i, ATA_REG_DATA, (unsigned int)ide_buf, 128);

      // (VI) Read Device Parameters:
      ide_devices[count].Reserved = 1;
      ide_devices[count].Type = type;
      ide_devices[count].Channel = i;
      ide_devices[count].Drive = j;
      ide_devices[count].Signature =
          *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
      ide_devices[count].Capabilities =
          *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
      ide_devices[count].CommandSets =
          *((unsigned int *)(ide_buf + ATA_IDENT_COMMANDSETS));

      // (VII) Get Size:
      if (ide_devices[count].CommandSets & (1 << 26))
        // Device uses 48-Bit Addressing:
        ide_devices[count].Size =
            *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
      else
        // Device uses CHS or 28-bit Addressing:
        ide_devices[count].Size =
            *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA));

      // (VIII) String indicates model of device (like Western Digital HDD and
      // SONY DVD-RW...):
      for (k = 0; k < 40; k += 2) {
        ide_devices[count].Model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
        ide_devices[count].Model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
      }
      ide_devices[count].Model[40] = 0; // Terminate String.

      count++;
    }

  // 4- Print Summary:
  for (i = 0; i < 4; i++)
    if (ide_devices[i].Reserved == 1) {
      printf(" Found %s Drive %fGB - %s\r\n",
             (const char *[]){"ATA", "ATAPI"}[ide_devices[i].Type], /* Type */
             ide_devices[i].Size,                                   /* Size */
             ide_devices[i].Model);
    }
}

unsigned char ide_ata_access(unsigned char direction, unsigned char drive,
                             unsigned int lba, unsigned char numsects,
                             unsigned short selector, unsigned int edi) {

  unsigned char lba_mode /* 0: CHS, 1:LBA28, 2: LBA48 */,
      dma /* 0: No DMA, 1: DMA */, cmd;
  unsigned char lba_io[6];
  unsigned int channel = ide_devices[drive].Channel; // Read the Channel.
  unsigned int slavebit =
      ide_devices[drive].Drive; // Read the Drive [Master/Slave]
  unsigned int bus =
      channels[channel].base; // Bus Base, like 0x1F0 which is also data port.
  unsigned int words =
      256; // Almost every ATA drive has a sector-size of 512-byte.
  unsigned short cyl, i;
  unsigned char head, sect, err;

  ide_write(channel, ATA_REG_CONTROL,
            channels[channel].nIEN = (ide_irq_invoked = 0x0) + 0x02);

  // (I) Select one from LBA28, LBA48 or CHS;
  if (lba >= 0x10000000) { // Sure Drive should support LBA in this case, or you
                           // are giving a wrong LBA.
    // LBA48:
    lba_mode = 2;
    lba_io[0] = (lba & 0x000000FF) >> 0;
    lba_io[1] = (lba & 0x0000FF00) >> 8;
    lba_io[2] = (lba & 0x00FF0000) >> 16;
    lba_io[3] = (lba & 0xFF000000) >> 24;
    lba_io[4] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
    lba_io[5] = 0; // LBA28 is integer, so 32-bits are enough to access 2TB.
    head = 0;      // Lower 4-bits of HDDEVSEL are not used here.
  } else if (ide_devices[drive].Capabilities & 0x200) { // Drive supports LBA?
    // LBA28:
    lba_mode = 1;
    lba_io[0] = (lba & 0x00000FF) >> 0;
    lba_io[1] = (lba & 0x000FF00) >> 8;
    lba_io[2] = (lba & 0x0FF0000) >> 16;
    lba_io[3] = 0; // These Registers are not used here.
    lba_io[4] = 0; // These Registers are not used here.
    lba_io[5] = 0; // These Registers are not used here.
    head = (lba & 0xF000000) >> 24;
  } else {
    // CHS:
    lba_mode = 0;
    sect = (lba % 63) + 1;
    cyl = (lba + 1 - sect) / (16 * 63);
    lba_io[0] = sect;
    lba_io[1] = (cyl >> 0) & 0xFF;
    lba_io[2] = (cyl >> 8) & 0xFF;
    lba_io[3] = 0;
    lba_io[4] = 0;
    lba_io[5] = 0;
    head = (lba + 1 - sect) % (16 * 63) /
           (63); // Head number is written to HDDEVSEL lower 4-bits.
  }
  // (II) See if drive supports DMA or not;
  dma = 0; // We don't support DMA
  while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY)
    ; // Wait if busy.
      // (IV) Select Drive from the controller;
  if (lba_mode == 0)
    ide_write(channel, ATA_REG_HDDEVSEL,
              0xA0 | (slavebit << 4) | head); // Drive & CHS.
  else
    ide_write(channel, ATA_REG_HDDEVSEL,
              0xE0 | (slavebit << 4) | head); // Drive & LBA
                                              // (V) Write Parameters;
  if (lba_mode == 2) {
    ide_write(channel, ATA_REG_SECCOUNT1, 0);
    ide_write(channel, ATA_REG_LBA3, lba_io[3]);
    ide_write(channel, ATA_REG_LBA4, lba_io[4]);
    ide_write(channel, ATA_REG_LBA5, lba_io[5]);
  }
  ide_write(channel, ATA_REG_SECCOUNT0, numsects);
  ide_write(channel, ATA_REG_LBA0, lba_io[0]);
  ide_write(channel, ATA_REG_LBA1, lba_io[1]);
  ide_write(channel, ATA_REG_LBA2, lba_io[2]);

  if (lba_mode == 0 && dma == 0 && direction == 0)
    cmd = ATA_CMD_READ_PIO;
  if (lba_mode == 1 && dma == 0 && direction == 0)
    cmd = ATA_CMD_READ_PIO;
  if (lba_mode == 2 && dma == 0 && direction == 0)
    cmd = ATA_CMD_READ_PIO_EXT;
  if (lba_mode == 0 && dma == 1 && direction == 0)
    cmd = ATA_CMD_READ_DMA;
  if (lba_mode == 1 && dma == 1 && direction == 0)
    cmd = ATA_CMD_READ_DMA;
  if (lba_mode == 2 && dma == 1 && direction == 0)
    cmd = ATA_CMD_READ_DMA_EXT;
  if (lba_mode == 0 && dma == 0 && direction == 1)
    cmd = ATA_CMD_WRITE_PIO;
  if (lba_mode == 1 && dma == 0 && direction == 1)
    cmd = ATA_CMD_WRITE_PIO;
  if (lba_mode == 2 && dma == 0 && direction == 1)
    cmd = ATA_CMD_WRITE_PIO_EXT;
  if (lba_mode == 0 && dma == 1 && direction == 1)
    cmd = ATA_CMD_WRITE_DMA;
  if (lba_mode == 1 && dma == 1 && direction == 1)
    cmd = ATA_CMD_WRITE_DMA;
  if (lba_mode == 2 && dma == 1 && direction == 1)
    cmd = ATA_CMD_WRITE_DMA_EXT;
  ide_write(channel, ATA_REG_COMMAND, cmd); // Send the Command.

  if (dma)
    if (direction == 0)
      ;
    // DMA Read.
    else
      ;
  // DMA Write.
  else if (direction == 0)
    // PIO Read.
    for (i = 0; i < numsects; i++) {
      if (err = ide_polling(channel, 1))
        return err; // Polling, set error and exit if there is.
      __asm__("pushw %es");
      __asm__("mov %%ax, %%es" : : "a"(selector));
      __asm__("rep insw" : : "c"(words), "d"(bus), "D"(edi)); // Receive Data.
      __asm__("popw %es");
      edi += (words * 2);
    }
  else {
    // PIO Write.
    for (i = 0; i < numsects; i++) {
      ide_polling(channel, 0); // Polling.
      __asm__("pushw %ds");
      __asm__("mov %%ax, %%ds" ::"a"(selector));
      __asm__("rep outsw" ::"c"(words), "d"(bus), "S"(edi)); // Send Data
      __asm__("popw %ds");
      edi += (words * 2);
    }
    ide_write(channel, ATA_REG_COMMAND,
              (char[]){ATA_CMD_CACHE_FLUSH, ATA_CMD_CACHE_FLUSH,
                       ATA_CMD_CACHE_FLUSH_EXT}[lba_mode]);
    ide_polling(channel, 0); // Polling.
  }

  return 0; // Easy, isn't it?
}

int ide_read_sectors(unsigned char drive, unsigned char numsects,
                     unsigned int lba, unsigned short es, unsigned int edi) {

  // 1: Check if the drive presents:
  // ==================================
  if (drive > 3 || ide_devices[drive].Reserved == 0)
    return 0x1; // Drive Not Found!

  // 2: Check if inputs are valid:
  // ==================================
  else if (((lba + numsects) > ide_devices[drive].Size) &&
           (ide_devices[drive].Type == IDE_ATA))
    return 0x2; // Seeking to invalid position.

  // 3: Read in PIO Mode through Polling & IRQs:
  // ============================================
  else {
    unsigned char err;
    if (ide_devices[drive].Type == IDE_ATA)
      err = ide_ata_access(ATA_READ, drive, lba, numsects, es, edi);
    return ide_print_error(drive, err);
  }
}

int ide_write_sectors(unsigned char drive, unsigned char numsects,
                      unsigned int lba, unsigned short es, unsigned int edi) {

  // 1: Check if the drive presents:
  // ==================================
  if (drive > 3 || ide_devices[drive].Reserved == 0)
    return 0x1; // Drive Not Found!
  // 2: Check if inputs are valid:
  // ==================================
  else if (((lba + numsects) > ide_devices[drive].Size) &&
           (ide_devices[drive].Type == IDE_ATA))
    return 0x2; // Seeking to invalid position.
  // 3: Read in PIO Mode through Polling & IRQs:
  // ============================================
  else {
    unsigned char err;
    if (ide_devices[drive].Type == IDE_ATA)
      err = ide_ata_access(ATA_WRITE, drive, lba, numsects, es, edi);
    return ide_print_error(drive, err);
  }
}