#!/bin/sh
exec objcopy --adjust-vma 0x100000 --rename .data=.text,contents,alloc,load,code -I binary -O elf32-i386 -B i386 "$1" "$2"
