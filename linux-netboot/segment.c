#include "segment.h"
#include "printf.h"

void get_gdtr(struct gdtr *out)
{
    __asm__ volatile (
        "sgdt (%0)"
        : /* no result */
        : "a"(&out->limit)
        );
}

void set_gdtr(const struct gdtr *out)
{
    __asm__ volatile (
        "lgdt (%0)"
        : /* no result */
        : "a"(&out->limit)
        );
}

void dump_gdt(void *base, uint16_t limit)
{
    printf("GDT located at base: 0x%08x limit: 0x%04x\n", (uint32_t) base, limit);
    struct segdesc *p = (struct segdesc *)base;
    for (int i = 0; i <= limit; i += 8) {
        dump_segment_descriptor(p);
        p++;
    }
}

void dump_segment_descriptor(struct segdesc *desc)
{
    uint32_t base, limit;
    limit = desc->limit0 | (desc->limit1) << 16;
    base = desc->base0 | (desc->base1 << 16) | (desc->base2 << 24);
    printf("[%08x]", (uint32_t) desc);
    printf(" base=0x%08x", base);
    printf(" limit=0x%05x", limit);
    printf(" AVL=%x", desc->avl);
    printf(" D/B=%x", desc->db);
    printf(" DPL=%x", desc->dpl);
    printf(" G=%x", desc->g);
    printf(" P=%x", desc->p);
    printf(" S=%x", desc->s);
    printf(" TYPE=0x%x", desc->type);
    printf("\n");
}

void set_desc_base(struct segdesc *desc, void *base)
{
    uint32_t addr = (uint32_t)base;
    desc->base0 = addr & 0xffff;
    desc->base1 = (addr >> 16) & 0xff;
    desc->base2 = (addr >> 24) & 0xff;
}

void set_desc_limit(struct segdesc *desc, uint32_t limit)
{
    desc->limit0 = limit & 0xffff;
    desc->limit1 = (limit >> 16) & 0xf;
}
