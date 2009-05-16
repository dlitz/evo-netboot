#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "portio.h"
#include "pirq.h"
#include "memory.h"
#include "serial.h"
#include "segment.h"
#include "superio.h"
#include "misc.h"
#include "pcspkr.h"
#include "printf.h"
#include "led.h"
#include "loadlinux.h"
#include "bootlinux.h"
#include "propaganda.h"

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

// NBI format.  See "Draft Net Boot Image Proposal 0.3, June 15, 1997"
struct nbi_header {
    // magic number 0x1b031336 is replaced by pointer to loader 
    void (*p_syscall)(unsigned int a, unsigned int b, const void *p, const void *q, const void *r);
    uint32_t flags_and_length;
    uint32_t header_load_address;   // real-mode address (ds:bx format)
    uint32_t header_exec_address;   // real-mode address (cs:ip format)
    struct nbi_entry {
        uint32_t ftl;   // flags, tags, and lengths
        uint32_t load_address;  // 32-bit linear load address
        uint32_t image_length;  // length within the image file
        uint32_t memory_length; // length in memory
    } __attribute__((packed)) entries[31];
} __attribute__((packed));

void c_main(struct nbi_header *nbi_header)
{
    struct gdtr gdtr;

    // The built-in NETXFER program on the Evo T30 provides several services
    // to the netboot image via a service routine.  This sets up the function
    // pointer needed to access the service routine.
    // NB: The only service we support is printf(), and the following
    // assignment is necessary before printf() will work.
    p_syscall = nbi_header->p_syscall;

    // Show a greeting and some propaganda
    printf("%s", propaganda);

    // Parse nbi_header
    for (int i = 0; i < 31; i++) {
        // FIXME: Use vendor flags rather than position
        switch(i) {
        case 0: // loader.bin
            break;
        case 1: // cmdline
            kernel_command_line = (char *)nbi_header->entries[i].load_address;
            printf("cmdline: %s\n", kernel_command_line);   // DEBUG FIXME
            break;
        case 2: // bzImage
            bzImage_start = (void *)nbi_header->entries[i].load_address;
            printf("bzImage_start: 0x%08x\n", (uint32_t) bzImage_start);   // DEBUG FIXME
            break;
        case 3: // initrd
            initrd_start = (void *)nbi_header->entries[i].load_address;
            initrd_size = nbi_header->entries[i].memory_length;
            printf("initrd: %d bytes at 0x%08x\n", initrd_size, (uint32_t) initrd_start);   // DEBUG FIXME
            break;
        default:
            ; // do nothing
        }

        // Stop if this is the last record.
        if (nbi_header->entries[i].ftl & 0x04000000) {
            break;
        }
    }

    // Dump some information about the environment we're running in.
    dump_regs();
    dump_cpuid();
    dump_superio();

    // Initialize SuperI/O devices (serial, parallel)
    superio_init();

    // Set the power-button LED to amber (it should be this colour already)
    led_set(LED_AMBER);

    // Dump the build-in global descriptor table (GDT)
//    printf("Built-in GDT:\n");
//    get_gdtr(&gdtr);
//    dump_gdt(gdtr.base, gdtr.limit);

    printf("Setting up Linux-compatible GDT...\n");
    create_linux_gdt(linux_gdt);
    gdtr.base = linux_gdt;
    gdtr.limit = 4*8-1;
    dump_gdt(gdtr.base, gdtr.limit);
    set_gdtr(&gdtr);

    // Set up PCI IRQ table (normally provided by a BIOS)
    printf("Creating PCI IRQ table...\n");
    create_pirq_table();

    // Copy Linux to its proper location in memory
    printf("Loading Linux...\n");
    load_linux();

    // Boot Linux
    printf("Booting Linux...\n");
    pcspkr_boot_tune();
    led_set(LED_GREEN);
    boot_linux();

    // We should never get here, but if we do, print something.
    led_set(LED_AMBER);
    printf("Linux not booted.  c_main() returning.\n");
    pcspkr_error_tune();
}
