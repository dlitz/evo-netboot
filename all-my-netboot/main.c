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

static void dump_gdt(void *base, uint16_t limit)
{
    l_print("GDT located at base: 0x%08x", (uint32_t) base);
    l_print(" limit: 0x%04x\r\n", limit);
    struct segdesc *p = (struct segdesc *)base;
    for (int i = 0; i <= limit; i += 8) {
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

static void bzero(void *s, unsigned int n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0) {
        *p = '\0';
        p++;
        n--;
    }
}

struct segdesc real_mode_gdt[5];

static void set_desc_base(struct segdesc *desc, void *base)
{
    uint32_t addr = (uint32_t)base;
    desc->base0 = addr & 0xffff;
    desc->base1 = (addr >> 16) & 0xff;
    desc->base2 = (addr >> 24) & 0xff;
}

static void set_desc_limit(struct segdesc *desc, uint32_t limit)
{
    desc->limit0 = limit & 0xffff;
    desc->limit1 = (limit >> 16) & 0xf;
}

void create_real_mode_gdt(struct segdesc *gdt)
{
    bzero(gdt, sizeof(struct segdesc)*5);
    set_desc_base(&gdt[1], 0x00000000);
    set_desc_limit(&gdt[1], 0xfffff);
    gdt[1].db = 1;
    gdt[1].dpl = 0;
    gdt[1].g = 1;
    gdt[1].p = 1;
    gdt[1].s = 1;
    gdt[1].type = 0xb; /* code */

    set_desc_base(&gdt[2], 0x00000000);
    set_desc_limit(&gdt[2], 0xfffff);
    gdt[2].db = 1;
    gdt[2].dpl = 0;
    gdt[2].g = 1;
    gdt[2].p = 1;
    gdt[2].s = 1;
    gdt[2].type = 0x3; /* data */

    set_desc_base(&gdt[3], 0x00000000);
    set_desc_limit(&gdt[3], 0x0ffff);
    gdt[3].db = 0;      /* 16-bit segment */
    gdt[3].dpl = 0;
    gdt[3].g = 0;   /* byte granularity */
    gdt[3].p = 1;
    gdt[3].s = 1;
    gdt[3].type = 0xb; /* code */

    set_desc_base(&gdt[4], 0x00000000);
    set_desc_limit(&gdt[4], 0x0ffff);
    gdt[4].db = 0;      /* 16-bit segment */
    gdt[4].dpl = 0;
    gdt[4].g = 0;   /* byte granularity */
    gdt[4].p = 1;
    gdt[4].s = 1;
    gdt[4].type = 0x3; /* data */
}

void init_c(void)
{
    int x = 1;
    l_print("init_c starting\r\n", 0);
    l_print("stack is in the ballpark of 0x%x\r\n", (uint32_t) &x);
    l_print("test_return returned 0x%x\r\n", test_return(5));
    dump_regs();

    struct gdtr gdtr;
    get_gdtr(&gdtr);
    dump_gdt(gdtr.base, gdtr.limit);

    l_print("Creating real-mode GDT\r\n", 0);
    create_real_mode_gdt(real_mode_gdt);
    dump_gdt(real_mode_gdt, 5*8-1);
}
