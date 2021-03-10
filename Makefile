KVM=0

LD = cross/bin/x86_64-elf-ld
CC = cross/bin/x86_64-elf-gcc
AS = nasm
LIBGCC=cross/lib/gcc/x86_64-elf/9.2.0/libgcc.a

ifeq ($(KVM), 1)
	QEMU = qemu-system-x86_64 -hda $(OS) -enable-kvm
else
	QEMU = qemu-system-x86_64 -hda $(OS)
endif

OS = mandelbrotos.hdd
KERNEL = mandelbrotos.elf

ASFLAGS = -f elf64

CFLAGS := \
	-mcmodel=kernel \
	-ffreestanding \
	-Isrc/include \
	-Wall \
	-Wextra \

LDFLAGS := \
	-static \
	-no-pie \
	-Tlinker.ld \
	-nostdlib

CFILES := $(shell find src/ -name '*.c')
ASFILES := $(shell find src/ -name '*.asm')
OFILES := $(CFILES:.c=.o) $(ASFILES:.asm=.o)

.PHONY: all clean

all: $(OS)

$(OS): $(KERNEL)
	dd if=/dev/zero of=$@ bs=1M seek=64 count=0
	parted -s $@ mklabel gpt
	parted -s $@ mkpart primary 2048s 100%
	echfs-utils -g -p0 $@ quick-format 512
	echfs-utils -g -p0 $@ import resources/limine.cfg boot/limine.cfg
	echfs-utils -g -p0 $@ import $< boot/$<
	limine-install $@

$(KERNEL): $(OFILES) $(LIBGCC)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(OFILES) $(KERNEL) $(OS)

qemu:
	$(QEMU)
