CC=gcc
AS=as

WARNINGS = -Wall -W

CFLAGS = $(WARNINGS) -Winit-self -std=c99 -g -O2 -nostdlib -fomit-frame-pointer
ASFLAGS = $(WARNINGS)
LDFLAGS = -nostdlib

boot.elf: boot.bin
	../bin2elf.sh boot.bin boot.elf

boot.bin: kernel.bin nbiheader.bin lowmem.bin bzImage
	dd if=/dev/zero of=boot.bin bs=512 count=8321
	dd if=nbiheader.bin of=boot.bin bs=512 count=1 conv=notrunc
	dd if=lowmem.bin of=boot.bin bs=512 seek=1 count=64 conv=notrunc
	dd if=kernel.bin of=boot.bin bs=512 seek=65 count=64 conv=notrunc
	dd if=bzImage of=boot.bin bs=512 seek=129 conv=notrunc

nbiheader.bin: kernel.elf
	objcopy -O binary -j .nbiheader kernel.elf nbiheader.bin

lowmem.bin: kernel.elf
	objcopy -O binary -j .lowmem kernel.elf lowmem.bin

kernel.bin: kernel.elf
	objcopy -O binary -j .text kernel.elf kernel.bin

kernel.elf: segment.o memory.o pirq.o mynetboot.o main.o loadlinux.o bootlinux.o i386-netboot.ld
	$(CC) segment.o memory.o pirq.o mynetboot.o main.o loadlinux.o bootlinux.o -o kernel.elf -T i386-netboot.ld $(LDFLAGS)

%.o: %.S
	$(CC) $(ASFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f segment.o memory.o pirq.o mynetboot.o main.o loadlinux.o bootlinux.o kernel.elf kernel.bin boot.bin boot.elf nbiheader.bin lowmem.bin

.PHONY: clean

# vim:set ts=8 sw=8 sts=8 noexpandtab:
