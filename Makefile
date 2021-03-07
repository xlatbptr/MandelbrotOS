OS = mandelbrotos.hdd
KERNEL = mandelbrotos.elf

AS = nasm
ASFLAGS = -f elf64

CC = x86_64-elf-gcc
CFLAGS := \
	-mcmodel=kernel \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pic \
	-mno-mmx \
	-mno-80387 \
	-mno-3dnow \
	-mno-sse \
	-mno-sse2 \
	-mno-red-zone \
	-Wall \
	-Wextra \
	-O2 \
	-Isrc/include

LD = x86_64-elf-ld
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

$(KERNEL): $(OFILES)
	$(LD) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

clean:
	rm -rf $(OFILES) $(KERNEL) $(OS)

run:
	qemu-system-x86_64 -hda $(OS) -enable-kvm
