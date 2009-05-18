.section .startup, "ax"
.global _start
_start:
        # The stack is already set up and NETXFER already uses GCC's default
        # calling convention, so just jump to the C code.
        jmp c_main
