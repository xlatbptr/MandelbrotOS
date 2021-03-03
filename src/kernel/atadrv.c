#include <font.h>
#include <kernel/alloc.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <kernel/isr.h>
#include <kernel/kbd.h>
#include <kernel/kshell.h>
#include <kernel/pit.h>
#include <kernel/power.h>
#include <kernel/serial.h>
#include <kernel/text.h>
#include <kernel/vbe.h>
#include <kernel/atadrv.h>
#include <hw.h>
#include <macros.h>
#include <multiboot.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

struct DEVICE{          
    int base;
    int dev_ctl;
};


//Copied from osdev.org
int detect_devtype(int slavebit, struct DEVICE *ctrl){      //Value should be
    ata_soft_reset(ctrl->dev_ctl);                          //ctrl->base =0x1F0, ctrl->dev_ctl =0x3F6
    outb(ctrl->base + REG_DEVSEL, 0xA0 | slavebit<<4);      //On a primary bus
    inb(ctrl->dev_ctl);
    inb(ctrl->dev_ctl);
    inb(ctrl->dev_ctl);
    inb(ctrl->dev_ctl);
    unsigned cl=inb(ctrl->base + REG_CYL_LO);
    unsigned ch=inb(ctrl->base + REG_CYL_HI);

    if(cl == 0x14 && ch == 0xEB) return ATADEV_PATAPI;
    if(cl == 0x69 && ch == 0x96) return ATADEV_SATAPI;
    if(cl == 0 && ch == 0) return ATADEV_PATA;
    if(cl == 0x3c && ch == 0xc3) return ATADEV_SATA;
    return ATADEV_UNKNOWN;
}