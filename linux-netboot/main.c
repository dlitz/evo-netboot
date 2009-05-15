#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "portio.h"
#include "mynetboot.h"

extern void *bzImage_start;
extern void load_linux(void);

#define PIRQ_NUM_SLOTS 7

struct pirq_slot {
    uint8_t pci_bus;
    uint8_t pci_devfunc;
    uint8_t inta_link;
    uint16_t inta_bitmap;
    uint8_t intb_link;
    uint16_t intb_bitmap;
    uint8_t intc_link;
    uint16_t intc_bitmap;
    uint8_t intd_link;
    uint16_t intd_bitmap;
    uint8_t slot_number;
    uint8_t _reserved[1];
} __attribute__((packed));

struct pirq_table {
    uint32_t signature;
    uint16_t version;
    uint16_t table_size;
    uint8_t irq_router_bus;
    uint8_t irq_router_devfunc;
    uint16_t exclusive_irq;
    uint16_t irq_router_compat_vendor_id;
    uint16_t irq_router_compat_device_id;
    uint32_t miniport_data;
    uint8_t _reserved[11];
    uint8_t checksum;   /* mod 256 must equal 0 */
    struct pirq_slot slots[PIRQ_NUM_SLOTS];
} __attribute__((packed));

/* Invoke the loader's printf-like function */
void l_print(const char *fmt, uint32_t arg)
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

void bzero(void *s, unsigned int n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0) {
        *p = '\0';
        p++;
        n--;
    }
}

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

void create_linux_gdt(struct segdesc *gdt)
{
    bzero(gdt, sizeof(struct segdesc)*4);
    set_desc_base(&gdt[2], 0x00000000);
    set_desc_limit(&gdt[2], 0xfffff);
    gdt[2].db = 1;
    gdt[2].dpl = 0;
    gdt[2].g = 1;
    gdt[2].p = 1;
    gdt[2].s = 1;
    gdt[2].type = 0xb; /* code */

    set_desc_base(&gdt[3], 0x00000000);
    set_desc_limit(&gdt[3], 0xfffff);
    gdt[3].db = 1;
    gdt[3].dpl = 0;
    gdt[3].g = 1;
    gdt[3].p = 1;
    gdt[3].s = 1;
    gdt[3].type = 0x3; /* data */
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

/* out must point to a buffer of 13 bytes (4*3 regs + 1 NUL) */
static void dump_cpuid(void)
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t cpuid_buf[5];

    // CPUID 0
    eax = 0;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        );
    cpuid_buf[0] = ebx;
    cpuid_buf[1] = edx;
    cpuid_buf[2] = ecx;
    cpuid_buf[3] = 0;
    l_print("CPUID0: EAX=0x%08x", eax);
    l_print(" \"%s\"\r\n", (uint32_t) cpuid_buf);

    // CPUID 1
    eax = 1;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        );
    l_print("CPUID1: EAX=0x%08x", eax);
    l_print(" EBX=0x%08x", ebx);
    l_print(" ECX=0x%08x", ecx);
    l_print(" EDX=0x%08x\r\n", edx);

    // CPUID 2
    eax = 2;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(eax)
        );
    l_print("CPUID2: EAX=0x%08x", eax);
    l_print(" EBX=0x%08x", ebx);
    l_print(" ECX=0x%08x", ecx);
    l_print(" EDX=0x%08x\r\n", edx);
}

static inline void serial_putc(unsigned char c)
{
    // Wait for transmitter ready (TXRDY) (bit 5 of LSR)
    while ((inb(0x3f8+5) & (1<<5)) == 0);

    // Send the byte
    outb(c, 0x3f8);
}

void serial_outstr(const char *s)
{
    while (*s != '\0') {
        serial_putc((unsigned char) *s);
        s++;
    }
}

static const unsigned char hex[17] = "0123456789abcdef";

#define ROL32(x) (((x) << 1) | (((x) >> 31) & 1))

void dump_flash(void)
{
    unsigned char c, *p;
    uint32_t addr;
    uint32_t cksum = 0;
    int i;
    p = (unsigned char *)(0x00000000);
    //for (i = 0; i < 0x01000000; i++) {
    i = 0;
    do {
        if ((i & 0x3f) == 0) {
            serial_putc(';');
            //serial_putc(hex[(cksum >> 28) & 0xf]);
            //serial_putc(hex[(cksum >> 24) & 0xf]);
            //serial_putc(hex[(cksum >> 20) & 0xf]);
            //serial_putc(hex[(cksum >> 16) & 0xf]);
            serial_putc(hex[(cksum >> 12) & 0xf]);
            serial_putc(hex[(cksum >> 8) & 0xf]);
            serial_putc(hex[(cksum >> 4) & 0xf]);
            serial_putc(hex[cksum & 0xf]);
            serial_putc('\n');
            addr = (uint32_t) p;
            serial_putc(hex[(addr >> 28) & 0xf]);
            serial_putc(hex[(addr >> 24) & 0xf]);
            serial_putc(hex[(addr >> 20) & 0xf]);
            serial_putc(hex[(addr >> 16) & 0xf]);
            serial_putc(hex[(addr >> 12) & 0xf]);
            serial_putc(hex[(addr >> 8) & 0xf]);
            serial_putc(hex[(addr >> 4) & 0xf]);
            serial_putc(hex[addr & 0xf]);
            serial_putc(':');
        }
        c = *p;
        //serial_putc(hex[(c >> 4) & 0xf]);
        //serial_putc(hex[c & 0xf]);
        serial_putc(c);
        cksum = ROL32(cksum) ^ (uint32_t) c;
        p++;
    //}
        i++;
    } while (p != 0);
}

// See http://www.microsoft.com/whdc/archive/pciirq.mspx
// PCI IRQ Routing Table Specification
// Microsoft Corporation, Version 1.0, February 27, 1998
// (Updated: December 4, 2001)
extern void create_pirq_table(void)
{
    struct pirq_table *t = (struct pirq_table *)0xf0000;
    bzero(t, sizeof(struct pirq_table));
    t->signature = '$' | ('P' << 8) | ('I' << 16) | ('R' << 24);
    t->version = 0x0100;
    t->table_size = 32 + PIRQ_NUM_SLOTS*16;
    t->irq_router_bus = 0;
    t->irq_router_devfunc = 0x90; // CS5530A: F0 Bridge Configuration
    t->exclusive_irq = 0;  // No IRQs are exclusive to PCI (XXX: probably wrong)
    t->irq_router_compat_vendor_id = 0x1078;    // PCI_VENDOR_ID_CYRIX
    t->irq_router_compat_device_id = 0x0002;    // PCI_DEVICE_ID_CYRIX_5520
    t->miniport_data = 0;

    for (unsigned int i = 0; i < PIRQ_NUM_SLOTS; i++) {
        bzero(&t->slots[i], sizeof(struct pirq_slot));
        t->slots[i].pci_bus = 0;
        switch (i) {
        // See the CS5530A documentation,
        // Table 5-1 "PCI Configuration Address Register"
        case 0: // Function 0: Bridge Configuration
            t->slots[i].pci_devfunc = 0x90;
        case 1: // Function 1: SMI Status and ACPI Timer
            t->slots[i].pci_devfunc = 0x91;
        case 2: // Function 2: IDE Controller
            t->slots[i].pci_devfunc = 0x92;
        case 3: // Function 3: XpressAUDIO Subsystem
            t->slots[i].pci_devfunc = 0x93;
        case 4: // Function 4: Video Controller
            t->slots[i].pci_devfunc = 0x94;
        case 5: // USB Controller
            t->slots[i].pci_devfunc = 0x98;
        case 6: // TI PCI1410APGE CardBus controller
            t->slots[6].pci_devfunc = 0x0e << 3;
        }
        t->slots[i].inta_link = 2;
        t->slots[i].inta_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intb_link = 1;
        t->slots[i].intb_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intc_link = 4;
        t->slots[i].intc_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intd_link = 3;
        t->slots[i].intd_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].slot_number = 0;
    }

    t->checksum = 0;
    uint8_t checksum = 0;
    for (int i = 0; i < t->table_size; i++) {
        checksum += ((unsigned char *)t)[i];
    }
    t->checksum = -checksum;
}

void memtest(void)
{
    uint32_t volatile * volatile p = 0x01d70000;
    for (;;) {
        //l_print("@0x%08x: ", (uint32_t) p);
        //l_print("0x%08x", (uint32_t) *p);
        *p = ~(uint32_t) p;
        //l_print(" 0x%08x", ~(uint32_t) p);
        //l_print(" 0x%08x", *p);
        if (*p != ~(uint32_t)p) {
            l_print("@0x%08x: ", (uint32_t) p);
            l_print(" FAIL\r\n", 0);
            for(;;);    // halt
        } else {
            //l_print(" ok\r\n", 0);
        }
        p += 1;
        //for (volatile uint32_t j=0; j < 0x00100000; j++);  // delay
    }
}

void init_c(void)
{
    int x = 1;
    l_print("init_c starting\r\n", 0);
    l_print("stack is in the ballpark of 0x%x\r\n", (uint32_t) &x);
    l_print("test_return returned 0x%x\r\n", test_return(5));
    dump_regs();

    l_print("@0x0400: 0x%04x\r\n", *(uint16_t *)0x0400);
    l_print("@0x0402: 0x%04x\r\n", *(uint16_t *)0x0402);
    l_print("@0x0404: 0x%04x\r\n", *(uint16_t *)0x0404);
    l_print("@0x0406: 0x%04x\r\n", *(uint16_t *)0x0406);

    // PC97307 Super I/O
#define SUPERIO_PORT 0x15c
    outb(0x20, SUPERIO_PORT);
    l_print("SID: 0x%02x\r\n", inb(SUPERIO_PORT+1));

    outb(0x27, SUPERIO_PORT);
    l_print("SRID: 0x%02x\r\n", inb(SUPERIO_PORT+1));

    outb(0x22, SUPERIO_PORT);
    l_print("Config Reg 2: 0x%02x\r\n", inb(SUPERIO_PORT+1));

    dump_cpuid();

    // Enable  PC97307 UART1
    uint8_t b;
    outb(0x7, SUPERIO_PORT);  // 0x07: logical device number
    outb(6, SUPERIO_PORT+1);  // logical device 6 (UART1)

    outb(0x30, SUPERIO_PORT);   // UART1.0x30: Activate
    b = inb(SUPERIO_PORT+1);
    b |= 1;     // activate
    outb(b, SUPERIO_PORT+1);

    // Serial
    l_print("Setting up serial port 115200 8N1\r\n", 0);
    //outb(0x0b, 0x3f8+4);
    // Set baud rate to 115200 bps (divisor=1)
    outb(0x80 | inb(0x3f8+3), 0x3f8+3);     // Set LCR:DLAB
    outb(0x01, 0x3f8);   // low byte
    outb(0x00, 0x3f8+1);   // high byte
    outb(~0x80 & inb(0x3f8+3), 0x3f8+3);     // Clear LCR:DLAB
    // Set 8N1
    outb(0x03, 0x3f8+3);    // LCR

    // Enable FIFO
    outb(0xc1, 0x3f8+2);    // FCR

    serial_putc('*');
    serial_putc('*');
    serial_putc('*');
    serial_outstr("Serial port enabled\r\n");

    //l_print("Dumping contents of flash to serial port\r\n", 0);
    //serial_outstr("== FLASH START ==\r\n");
    //dump_flash();
    //serial_outstr("\r\n== FLASH DONE ==\r\n");

    struct gdtr gdtr;
    get_gdtr(&gdtr);
//    dump_gdt(gdtr.base, gdtr.limit);

    l_print("Creating Linux GDT\r\n", 0);
    create_linux_gdt(linux_gdt);
    gdtr.base = linux_gdt;
    gdtr.limit = 4*8-1;
    dump_gdt(gdtr.base, gdtr.limit);
    set_gdtr(&gdtr);

    l_print("bzImage_start = 0x%08x\r\n", (uint32_t) bzImage_start);

    l_print("Testing writing to low addresses\r\n", 0);
    uint32_t volatile * volatile p = (uint32_t *)0x90000;
    l_print("*p = 0x%08x\r\n", *p);
    *p = 0xcafebabe;
    l_print("*p = 0x%08x\r\n", *p);

    l_print("Creating PCI IRQ table...\r\n", 0);
    create_pirq_table();

//    l_print("Memory test...\r\n", 0);
//    memtest();

    l_print("Loading Linux...\r\n", 0);
    load_linux();

//    l_print("Calling test_16bit\r\n", 0);
//    test_16bit();
}
