CC=gcc
AS=as

WARNINGS = -Wall -W

CFLAGS = $(WARNINGS) -m32 -Winit-self -std=c99 -g -O2 -nostdlib -fomit-frame-pointer
ASFLAGS = $(WARNINGS) -m32
LDFLAGS = -nostdlib

OBJS = \
	startup.o \
	misc.o \
	bootlinux.o \
	gprintf/gprintf.o \
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

all: loader.bin

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

.PHONY: all clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
