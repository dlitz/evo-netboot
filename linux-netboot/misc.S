.section .data
.global p_syscall
p_syscall:
    .int 0xcafebabe

.section .rodata
a_aborted:
    .asciz "abort() (EIP=0x%08x)\r\n"
a_halted:
    .asciz "*** Bootloader halted ***\r\n"

.section .text

.global abort
abort:
        push %esp      # push the return address to the stack
        push $a_aborted
        call printf
        add $8, %esp    # pop args off the stack
        jmp halt

.global halt
halt:
        push $a_halted
        call printf
        add $4, %esp    # pop arg off the stack
1:
        jmp 1b

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

# get_eip_reg() just returns the return address that is already on the stack.
.global get_eip_reg
get_eip_reg:
        mov (%esp), %eax
        ret
