CC=gcc
AS=as

WARNINGS = -Wall -W

CFLAGS = $(WARNINGS) -Winit-self -std=c99 -g -O2 -nostdlib -fomit-frame-pointer
ASFLAGS = $(WARNINGS)
LDFLAGS = -nostdlib

KERNEL_CMDLINE = \
	console=ttyS0,115200 console=tty0 earlyprintk=serial,ttyS0,115200 \
	pci=earlydump \
	rootwait root=/dev/sda1 ro \
	debug loglevel=7 \
	memtest=4

OBJS = \
	startup.o \
	misc.o \
	bootlinux.o \
	led.o \
	loadlinux.o \
	memory.o \
	pcspkr.o \
	pirq.o \
	printf.o \
	propaganda.o \
	segment.o \
	serial.o \
	superio.o \
	main.o

boot.bin: loader.bin bzImage
	./mkloader -o $@ -L loader.bin -c "$(KERNEL_CMDLINE)" bzImage

loader.bin: loader.elf
	objcopy -O binary -j .text loader.elf loader.bin

loader.elf: $(OBJS) i386-netboot.ld
	$(CC) -o $@ $(OBJS) -T i386-netboot.ld $(LDFLAGS)

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) loader.elf loader.bin boot.bin nbiheader.bin

.PHONY: clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
