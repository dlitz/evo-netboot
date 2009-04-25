#ifndef MYNETBOOT_H
#define MYNETBOOT_H

#include <stdint.h>

struct gdtr {
    uint16_t _pad;       // the "limit" field should be odd-aligned, per Intel's specifications
    uint16_t limit;
    void *base;
} __attribute__((packed)) __attribute__((aligned (4)));

extern void (*p_syscall)(unsigned int a, unsigned int b, const void *p, const void *q, const void *r);

extern unsigned int test_return(unsigned int a);
extern void get_gdtr(struct gdtr *out);
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

#endif
