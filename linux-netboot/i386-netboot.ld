OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

MEMORY
{
  nbiheader (rx): ORIGIN = 0x0000c00d, LENGTH = 0x00000200
  lowmem (wx)   : ORIGIN = 0x00001000, LENGTH = 0x00008000
  ram0 (wx)     : ORIGIN = 0x01000000, LENGTH = 0x00008000
  bzImage (wx)  : ORIGIN = 0x01080000, LENGTH = 0x00400000
}

SECTIONS
{
  .nbiheader : { *(.nbiheader) } > nbiheader
  .lowmem : { *(.lowmem) } > lowmem
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
