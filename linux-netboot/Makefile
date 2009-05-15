CC=gcc
AS=as

WARNINGS = -Wall -W

CFLAGS = $(WARNINGS) -Winit-self -std=c99 -g -O2 -nostdlib -fomit-frame-pointer
ASFLAGS = $(WARNINGS)
LDFLAGS = -nostdlib

OBJS = \
	startup.o \
	misc.o \
	bootlinux.o \
	loadlinux.o \
	memory.o \
	nbiheader.o \
	pcspkr.o \
	pirq.o \
	printf.o \
	propaganda.o \
	segment.o \
	serial.o \
	superio.o \
	main.o

boot.elf: boot.bin
	../bin2elf.sh boot.bin boot.elf

boot.bin: kernel.bin nbiheader.bin lowmem.bin bzImage
	dd if=/dev/zero of=boot.bin bs=512 count=8321
	dd if=nbiheader.bin of=boot.bin bs=512 count=1 conv=notrunc
	dd if=lowmem.bin of=boot.bin bs=512 seek=1 count=64 conv=notrunc
	dd if=kernel.bin of=boot.bin bs=512 seek=65 count=64 conv=notrunc
	dd if=bzImage of=boot.bin bs=512 seek=129 conv=notrunc

nbiheader.bin: nbiheader.o
	objcopy -O binary -j .nbiheader nbiheader.o nbiheader.bin

lowmem.bin: kernel.elf
	objcopy -O binary -j .lowmem kernel.elf lowmem.bin

kernel.bin: kernel.elf
	objcopy -O binary -j .text kernel.elf kernel.bin

kernel.elf: $(OBJS) i386-netboot.ld
	$(CC) -o kernel.elf $(OBJS) -T i386-netboot.ld $(LDFLAGS)

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) kernel.elf kernel.bin boot.bin boot.elf nbiheader.bin lowmem.bin

.PHONY: clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
