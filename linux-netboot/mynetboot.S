.arch i486
.section .text, "ax"
.global _start
_start:
        popl    entry_arg1
        popl    p_nbi_header
        mov     p_nbi_header, %ebx
        mov     (%ebx), %ebx
        mov     %ebx, p_syscall

        mov     $p_nbi_header, %eax
        mov     $value, %ebx
        call    xprintf
        mov     p_nbi_header, %eax
        mov     $value, %ebx
        call    xprintf

        call    init_c

        mov     $0, %eax
        mov     $returned, %ebx
        call    xprintf

.global halt
halt:
        mov     $0, %eax
        mov     $halting, %ebx
        call    xprintf
real_halt:
        jmp     real_halt

.global get_gdtr
get_gdtr:
        mov         0x4(%esp), %eax
        add         $2, %eax    # skip the _pad member
        sgdt        (%eax)
        ret

.global set_gdtr
set_gdtr:
        mov         0x4(%esp), %eax
        add         $2, %eax    # skip the _pad member
        lgdt        (%eax)
        ret

.global get_cs_reg
get_cs_reg:
        mov         %cs, %eax
        ret

.global get_ds_reg
get_ds_reg:
        mov         %ds, %eax
        ret

.global get_ss_reg
get_ss_reg:
        mov         %ss, %eax
        ret

.global get_es_reg
get_es_reg:
        mov         %es, %eax
        ret

.global get_fs_reg
get_fs_reg:
        mov         %fs, %eax
        ret

.global get_gs_reg
get_gs_reg:
        mov         %gs, %eax
        ret

.global get_eflags_reg
get_eflags_reg:
        pushf
        pop         %eax
        ret

.global get_cr0_reg
get_cr0_reg:
        mov         %cr0, %eax
        ret

.global get_cr2_reg
get_cr2_reg:
        mov         %cr2, %eax
        ret

.global get_cr3_reg
get_cr3_reg:
        mov         %cr3, %eax
        ret

.global get_cr4_reg
get_cr4_reg:
        mov         %cr4, %eax
        ret

.global get_eip_reg
get_eip_reg:
        call    1f
1:
        pop     %eax
        ret

.global test_return
test_return:
        mov 0x4(%esp), %eax
        inc %eax
        ret

xprintf:
        push %ebx
        push %ecx
        push %edx
        pushl $0
        push %eax
        push %ebx
        pushl $1
        pushl $1
        call *p_syscall
        add $0x14, %esp
        pop %edx
        pop %ecx
        pop %ebx
        ret

.section .data
entry_arg1:
        .int 0xcafebabe
p_nbi_header:
        .int 0xcafebabe

.global p_syscall
p_syscall:
        .int 0x8888f00d

blah:
        .asciz "blah\r\n"

value:
        .asciz "Value: 0x%x\r\n"

halting:
        .asciz "Halting\r\n"

returned:
        .asciz "Returned from init_c\r\n"

.global linux_gdt
linux_gdt:
    .fill 4, 8, 0x00    # There is room for 4 GDT entries here. Increase if necessary

.section .nbiheader, "ax"
# NBI format.  See "Draft Net Boot Image Proposal 0.3, June 15, 1997"

# Header
.int    0x1b031336      # magic
.int    0x4             # flags and length
.int    0xc00d          # load address for the 512-byte header (ds:bx format)
.int    0xc200          # execute address (cs:ip format)

# "Loading information records" follow:

# lowmem (64 sectors)
.int    0x00004104   # FIXME: tags
.int    0x00001000  # Load address
.int    0x00008000  # Image length (32768 bytes)
.int    0x00008000  # Memory length

# ram0 (64 sectors)
.int    0x00004104   # FIXME: tags
.int    0x01000000  # Load address
.int    0x00008000  # Image length (32768 bytes)
.int    0x00008000  # Memory length

# bzImage (8192 sectors)
.int    0x04004104  # FIXME: tags (last entry)
.int    0x01080000  # Load address
.int    0x00400000  # Image length (4194304 bytes)
.int    0x00400000  # Memory length

.fill   435, 1, 0x00

hdr_entry_point:
    # jump far, absolute, to immediate address 08:01000000
    ljmp $0x8, $0x01000000

.fill   6, 1, 0x00

.section .lowmem, "ax"
.code32

.global real_mode_gdt
real_mode_gdt:
    .fill 5, 8, 0x00    # There is room for 5 GDT entries here. Increase if necessary

.global test_16bit
test_16bit:
#        cli
#        pushf       # EFLAGS register
#        #pushl $0x08 # CS register GDT[1]
#        pushl $0x18 # CS register GDT[3]
#        push $1f    # push EIP register
#        iret        # set CS:EIP and EFLAGS to what's on the stack
#1:
#        pushf       # EFLAGS register
#        pushl $0x08 # CS register GDT[1]
#        push $2f    # push EIP register
#        iret        # set CS:EIP and EFLAGS to what's on the stack
#2:
#        mov     $p_nbi_header, %eax
#        mov     $value, %ebx
#        call    xprintf
#        jmp 2b
#        #ret

    cli
    pushf       # EFLAGS register
    pushl $0x18 # CS register GDT[3]
    push $1f    # push EIP register
    iret        # set CS:EIP and EFLAGS to what's on the stack
1:
    mov $0x20, %eax     # GDT[4]
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs
    
    # Clear the PE (Paging Enabled) flag in CR0
    mov %cr0, %eax
    and $0xfffffffe, %eax   # Clear the PE bit
    mov %eax, %cr0

    # Execute a far jump into real-mode
    ljmp $0x18,$real_mode_entry_point

done:
    mov $0x10, %eax     # GDT[2]
    mov %eax, %ds
    mov %eax, %es
    mov %eax, %fs
    mov %eax, %gs

        mov     $p_nbi_header, %eax
        mov     $value, %ebx
        call    xprintf
        ret

.code16
real_mode_entry_point:

    # Make the PC speaker start beeping
    mov $182, %al
    out %al, $0x43

    mov $0xd0, %al
    out %al, $0x42
    mov $0x10, %al
    out %al, $0x42

    in $0x61, %al
    or $0x03, %al
    out %al, $0x61

    # Make a BIOS call to output 'A' to the screen
#    mov $0x0e, %ah
#    mov $0x41, %al
#    int $0x10

    # Make the PC speaker stop beeping
    mov $182, %al
    out %al, $0x43

    mov $0xd0, %al
    out %al, $0x42
    mov $0x10, %al
    out %al, $0x42

    in $0x61, %al
    and $0xfc, %al
    out %al, $0x61

    # Set the PE flag in CR0 and jump back to 32-bit mode
    mov %cr0, %eax
    or $0x1, %eax
    mov %eax, %cr0

    ljmp $0x08, $done

#    # Reboot
#    ljmp $0xffff,$0x0000

#    mov $0x00, %ah
#    mov $0x01, %al
#    int $0x10

1:
    mov $0x0e, %ah
    mov $0x41, %al
    int $0x10
    jmp 1b
