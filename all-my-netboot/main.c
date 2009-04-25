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
static void l_print(const char *fmt, void *arg)
{
    (*p_syscall)(1, 1, fmt, arg, (void*)0);
}

static void dump_segment_descriptor(struct segdesc *desc)
{
    uint32_t avl, base, db, dpl, g, limit, p, s, type;
    limit = desc->limit0 | (desc->limit1) << 16;
    base = desc->base0 | (desc->base1 << 16) | (desc->base2 << 24);
    g = desc->g;
    db = desc->db;
    avl = desc->avl;
    p = desc->p;
    dpl = desc->dpl;
    s = desc->s;
    type = desc->type;
    l_print("[%08x]", &desc);
    l_print(" base=0x%08x", &base);
    l_print(" limit=0x%05x", &limit);
    l_print(" AVL=%x", &avl);
    l_print(" D/B=%x", &db);
    l_print(" DPL=%x", &dpl);
    l_print(" G=%x", &g);
    l_print(" P=%x", &p);
    l_print(" S=%x", &s);
    l_print(" TYPE=0x%x", &type);
    l_print("\r\n", 0);
}

static void dump_gdt(void)
{
    struct gdtr gdtr;
    get_gdtr(&gdtr);
    l_print("GDT located at base: 0x%08x", &gdtr.base);
    unsigned int d = gdtr.limit;
    l_print(" limit: 0x%04x\r\n", &d);
    struct segdesc *p = (struct segdesc *)gdtr.base;
    for (int i = 0; i <= gdtr.limit; i += 8) {
        dump_segment_descriptor(p);
        p++;
    }
}

static void dump_regs(void)
{
    uint32_t v;
    v = get_cs_reg(); l_print("CS: 0x%08x\r\n", &v);
    v = get_ds_reg(); l_print("DS: 0x%08x\r\n", &v);
    v = get_ss_reg(); l_print("SS: 0x%08x\r\n", &v);
    v = get_es_reg(); l_print("ES: 0x%08x\r\n", &v);
    v = get_fs_reg(); l_print("FS: 0x%08x\r\n", &v);
    v = get_gs_reg(); l_print("GS: 0x%08x\r\n", &v);
    v = get_eflags_reg(); l_print("EFLAGS: 0x%08x\r\n", &v);
    v = get_eip_reg(); l_print("EIP (approx): 0x%08x\r\n", &v);
    v = get_cr0_reg(); l_print("CR0: 0x%08x\r\n", &v);
    v = get_cr2_reg(); l_print("CR2: 0x%08x\r\n", &v);
    v = get_cr3_reg(); l_print("CR3: 0x%08x\r\n", &v);
    v = get_cr4_reg(); l_print("CR4: 0x%08x\r\n", &v);
}

void init_c(void)
{
    int x = 1;
    int *y = &x;
    l_print("init_c starting\r\n", (void*)0);
    l_print("stack is in the ballpark of 0x%x\r\n", &y);
    static unsigned int d;
    d = test_return(5);
    l_print("test_return returned 0x%x\r\n", &d);
    dump_regs();
    dump_gdt();
}
