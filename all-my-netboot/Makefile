CC=gcc
AS=as

WARNINGS = -Wall -W

CFLAGS = $(WARNINGS) -Winit-self -std=c99 -g -O2 -nostdlib -fomit-frame-pointer
ASFLAGS = $(WARNINGS)
LDFLAGS = -nostdlib

boot.elf: boot.bin
	../bin2elf.sh boot.bin boot.elf

boot.bin: kernel.bin nbiheader.bin
	dd if=/dev/zero of=boot.bin bs=512 count=578
	dd if=nbiheader.bin of=boot.bin bs=512 count=1 conv=notrunc
	dd if=kernel.bin of=boot.bin bs=512 seek=1 conv=notrunc

nbiheader.bin: kernel.elf
	objcopy -O binary -j .nbiheader kernel.elf nbiheader.bin

kernel.bin: kernel.elf
	objcopy -O binary -j .text kernel.elf kernel.bin

kernel.elf: mynetboot.o main.o i386-netboot.ld
	$(CC) mynetboot.o main.o -o kernel.elf -T i386-netboot.ld $(LDFLAGS)

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f mynetboot.o main.o kernel.elf kernel.bin boot.bin boot.elf nbiheader.bin

.PHONY: clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
