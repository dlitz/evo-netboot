#ifndef MYNETBOOT_H
#define MYNETBOOT_H

#include <stdint.h>

/* Interrupt-gate descriptor */
struct intdesc {
    unsigned int offset0:16;
    unsigned int segment:16;
    unsigned int flags:13;
    unsigned int dpl:2;
    unsigned int p:1;
    unsigned int offset1:16;
} __attribute__((packed));

extern void (*p_syscall)(unsigned int a, unsigned int b, const void *p, const void *q, const void *r);

extern unsigned int test_return(unsigned int a);
extern uint32_t get_cs_reg(void);
extern uint32_t get_ds_reg(void);
extern uint32_t get_ss_reg(void);
extern uint32_t get_es_reg(void);
extern uint32_t get_fs_reg(void);
extern uint32_t get_gs_reg(void);
extern uint32_t get_eflags_reg(void);
extern uint32_t get_cr0_reg(void);
extern uint32_t get_cr2_reg(void);
extern uint32_t get_cr3_reg(void);
extern uint32_t get_cr4_reg(void);
extern uint32_t get_eip_reg(void);
extern struct segdesc real_mode_gdt[5];
extern struct segdesc linux_gdt[4];

extern void test_16bit(void);


#endif
