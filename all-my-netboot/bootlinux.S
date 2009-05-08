.arch i486
.section .text, "ax"
.global boot_linux
boot_linux:
    # Set DS, ES, SS registers to __BOOT_DS(0x18) i.e. GDT[3]
    mov $0x18, %eax
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %ss

    # Set ESI to point to the boot_params structure
    mov $boot_params, %esi

    # Clear EBP, EDI, EBX
    xor %ebp, %ebp
    xor %edi, %edi
    xor %ebx, %ebx
    ljmp $0x10, $0x00100000     # __BOOT_CS(0x10) i.e. GDT[2] : kernel32_entry_point
