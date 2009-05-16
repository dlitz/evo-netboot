OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

MEMORY
{
  ram0 (wx)     : ORIGIN = 0x01000000, LENGTH = 0x01000000
}

SECTIONS
{
  .text : {
    *(.startup)
    *(.text)
    *(.rodata)
    *(.rodata.*)
    *(.data)
    *(.bss)
    *(COMMON)
  } > ram0
}
