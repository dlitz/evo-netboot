#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdint.h>

struct gdtr {
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

extern void get_gdtr(struct gdtr *out);
extern void set_gdtr(const struct gdtr *out);
extern void dump_gdt(void *base, uint16_t limit);
extern void dump_segment_descriptor(struct segdesc *desc);
extern void set_desc_base(struct segdesc *desc, void *base);
extern void set_desc_limit(struct segdesc *desc, uint32_t limit);

#endif /* SEGMENT_H */
