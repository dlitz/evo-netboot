#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "portio.h"
#include "pirq.h"
#include "memory.h"
#include "serial.h"
#include "segment.h"
#include "superio.h"
#include "mynetboot.h"
#include "printf.h"

extern void *bzImage_start;
extern void load_linux(void);

struct segdesc linux_gdt[4];

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

static void dump_regs(void)
{
    int dummy = 1;
    printf("CS=%08Xh DS=%08Xh SS=%08Xh ES=%08Xh FS=%08Xh GS=%08Xh\n",
        get_cs_reg(), get_ds_reg(), get_ss_reg(),
        get_es_reg(), get_fs_reg(), get_gs_reg());
    printf("EFLAGS=%08Xh EIP=%08Xh CR0=%08Xh CR3=%08Xh CR4=%08Xh\n",
        get_eflags_reg(), get_eip_reg(),
        get_cr0_reg(), get_cr3_reg(), get_cr4_reg());
    printf("Stack is in the ballpark of 0x%x\n", (uint32_t) &dummy);
}

static void dump_cpuid(void)
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t cpuid_buf[4];
    uint32_t i, max_std_cpuid_level;

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
    cpuid_buf[3] = 0;   // { '\0', '\0', '\0', '\0' }
    max_std_cpuid_level = eax;
    printf("CPUID0: EAX=0x%08x \"%s\"\n", eax, (char *) cpuid_buf);

    // Other CPUID info
    for (i = 1; i <= max_std_cpuid_level; i++) {
        eax = i;
        __asm__ volatile (
            "cpuid"
            : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
            : "a"(eax)
            );
        printf("CPUID%d: EAX=0x%08x EBX=0x%08x ECX=0x%08x EDX=0x%08x\n", i, eax, ebx, ecx, edx);
    }
}

void init_c(void)
{
    printf("init_c starting\n");
    dump_regs();

    printf("@0x0400: 0x%04x\n", *(uint16_t *)0x0400);
    printf("@0x0402: 0x%04x\n", *(uint16_t *)0x0402);
    printf("@0x0404: 0x%04x\n", *(uint16_t *)0x0404);
    printf("@0x0406: 0x%04x\n", *(uint16_t *)0x0406);

    dump_cpuid();

    // PC97307 Super I/O
    printf("SID(20h)=0x%02x SRID(27h)=0x%02x CR2(22h)=0x%02x\n", superio_inb(0x20), superio_inb(0x27), superio_inb(0x22));

    // Enable  PC97307 UART1
    superio_select_logical_device(6); // logical device 6 (UART1)
    superio_outb(superio_inb(0x30) | 1, 0x30);  // UART1.0x30: Activate

    // Serial
    printf("Setting up serial port 115200 8N1\n");
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

    printf("Creating Linux GDT\n");
    create_linux_gdt(linux_gdt);
    gdtr.base = linux_gdt;
    gdtr.limit = 4*8-1;
    dump_gdt(gdtr.base, gdtr.limit);
    set_gdtr(&gdtr);

    printf("bzImage_start = 0x%08x\n", (uint32_t) bzImage_start);

    printf("Testing writing to low addresses\n");
    uint32_t volatile * volatile p = (uint32_t *)0x90000;
    printf("*p = 0x%08x\n", *p);
    *p = 0xcafebabe;
    printf("*p = 0x%08x\n", *p);

    printf("Creating PCI IRQ table...\n");
    create_pirq_table();

    printf("Loading Linux...\r\n", 0);
    load_linux();
}
