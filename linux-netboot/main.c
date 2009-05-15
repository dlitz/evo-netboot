#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "portio.h"
#include "pirq.h"
#include "memory.h"
#include "segment.h"
#include "mynetboot.h"

extern void *bzImage_start;
extern void load_linux(void);

struct segdesc linux_gdt[4];

/* Invoke the loader's printf-like function */
void l_print(const char *fmt, uint32_t arg)
{
    (*p_syscall)(1, 1, fmt, &arg, (void*)0);
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

// Set up a flat memory model for Linux, per the requireents in the "32-bit
// BOOT PROTOCOL" specified in linux-2.6/Documentation/x86/boot.txt.
// Linux needs a 4-entry global descriptor table, as follows:
//   Entry    Description
//   -------+-------------------------------------------------
//   GDT[0]   Standard IA-32 empty GDT entry
//   GDT[1]   *don't care*
//   GDT[2]   __BOOT_CS(%cs=0x10): 4 GiB flat segment, execute/read perms
//   GDT[3]   __BOOT_DS(%ds=%es=%ss=0x18): 4 GiB flat segment, read/write perms
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

void init_c(void)
{
    int x = 1;
    l_print("init_c starting\r\n", 0);
    l_print("stack is in the ballpark of 0x%x\r\n", (uint32_t) &x);
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

    l_print("Loading Linux...\r\n", 0);
    load_linux();
}
