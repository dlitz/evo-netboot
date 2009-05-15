#ifndef MYNETBOOT_H
#define MYNETBOOT_H

#include <stdint.h>

struct gdtr {
    uint16_t _pad;       // the "limit" field should be odd-aligned, per Intel's specifications
    uint16_t limit;
    void *base;
} __attribute__((packed)) __attribute__((aligned (4)));

struct idtr {
    uint16_t _pad;       // the "limit" field should be odd-aligned, per Intel's specifications
    uint16_t limit;
    void *base;
} __attribute__((packed)) __attribute__((aligned (4)));

/* Segment descriptor */
struct segdesc {
    unsigned int limit0:16;
    unsigned int base0:16;
    unsigned int base1:8;
    unsigned int type:4;
    unsigned int s:1;
    unsigned int dpl:2;
    unsigned int p:1;
    unsigned int limit1:4;
    unsigned int avl:1;
    unsigned int _pad0:1;
    unsigned int db:1;
    unsigned int g:1;
    unsigned int base2:8;
} __attribute__((packed));

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
extern void get_gdtr(struct gdtr *out);
extern void get_idtr(struct idtr *out);
extern void set_gdtr(struct gdtr *out);
extern void set_idtr(struct idtr *out);
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
