OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

MEMORY
{
  nbiheader (rx): ORIGIN = 0x0000c00d, LENGTH = 0x00000200
  ram0 (wx)     : ORIGIN = 0x00100200, LENGTH = 0x00008000
  ram1 (wx)     : ORIGIN = 0x00108200, LENGTH = 0x00000400
}

SECTIONS
{
  .nbiheader : { *(.nbiheader) } > nbiheader
  .text : {
    *(.text)
    *(.rodata)
    *(.rodata.*)
    *(.data)
    *(.bss)
    *(COMMON)
  } > ram0
}
