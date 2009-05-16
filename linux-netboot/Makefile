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
	led.o \
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

boot.bin: loader.bin nbiheader.bin bzImage
	dd if=/dev/zero of=boot.bin bs=512 count=8321
	dd if=nbiheader.bin of=boot.bin bs=512 count=1 conv=notrunc
	dd if=loader.bin of=boot.bin bs=512 seek=65 count=64 conv=notrunc
	dd if=bzImage of=boot.bin bs=512 seek=129 conv=notrunc

nbiheader.bin: nbiheader.o
	objcopy -O binary -j .nbiheader nbiheader.o nbiheader.bin

loader.bin: loader.elf
	objcopy -O binary -j .text loader.elf loader.bin

loader.elf: $(OBJS) i386-netboot.ld
	$(CC) -o $@ $(OBJS) -T i386-netboot.ld $(LDFLAGS)

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) loader.elf loader.bin boot.bin boot.elf nbiheader.bin

.PHONY: clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
