#include <stdint.h>
#include "mynetboot.h"

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

/* Invoke the loader's printf-like function */
static void l_print(const char *fmt, uint32_t arg)
{
    (*p_syscall)(1, 1, fmt, &arg, (void*)0);
}

static void dump_segment_descriptor(struct segdesc *desc)
{
    uint32_t base, limit;
    limit = desc->limit0 | (desc->limit1) << 16;
    base = desc->base0 | (desc->base1 << 16) | (desc->base2 << 24);
    l_print("[%08x]", (uint32_t) desc);
    l_print(" base=0x%08x", base);
    l_print(" limit=0x%05x", limit);
    l_print(" AVL=%x", desc->avl);
    l_print(" D/B=%x", desc->db);
    l_print(" DPL=%x", desc->dpl);
    l_print(" G=%x", desc->g);
    l_print(" P=%x", desc->p);
    l_print(" S=%x", desc->s);
    l_print(" TYPE=0x%x", desc->type);
    l_print("\r\n", 0);
}

static void dump_gdt(void)
{
    struct gdtr gdtr;
    get_gdtr(&gdtr);
    l_print("GDT located at base: 0x%08x", (uint32_t) gdtr.base);
    l_print(" limit: 0x%04x\r\n", gdtr.limit);
    struct segdesc *p = (struct segdesc *)gdtr.base;
    for (int i = 0; i <= gdtr.limit; i += 8) {
        dump_segment_descriptor(p);
        p++;
    }
}

static void dump_regs(void)
{
    l_print("CS: 0x%08x\r\n", get_cs_reg());
    l_print("DS: 0x%08x\r\n", get_ds_reg());
    l_print("SS: 0x%08x\r\n", get_ss_reg());
    l_print("ES: 0x%08x\r\n", get_es_reg());
    l_print("FS: 0x%08x\r\n", get_fs_reg());
    l_print("GS: 0x%08x\r\n", get_gs_reg());
    l_print("EFLAGS: 0x%08x\r\n", get_eflags_reg());
    l_print("EIP (approx): 0x%08x\r\n", get_eip_reg());
    l_print("CR0: 0x%08x\r\n", get_cr0_reg());
    l_print("CR2: 0x%08x\r\n", get_cr2_reg());
    l_print("CR3: 0x%08x\r\n", get_cr3_reg());
    l_print("CR4: 0x%08x\r\n", get_cr4_reg());
}

void init_c(void)
{
    int x = 1;
    l_print("init_c starting\r\n", 0);
    l_print("stack is in the ballpark of 0x%x\r\n", (uint32_t) &x);
    l_print("test_return returned 0x%x\r\n", test_return(5));
    dump_regs();
    dump_gdt();
}
