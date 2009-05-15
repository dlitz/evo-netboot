#include <stdint.h>
#include <stddef.h>

extern void l_print(const char *fmt, uint32_t arg);
extern void bzero(void *s, unsigned int n);
extern void boot_linux(void);
void *bzImage_start = (void *)0x01080000;
void *kernel32_entry_point = (void *)0x00100000;
extern void serial_outstr(const char *s);

struct e820entry {
    uint64_t addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

struct boot_params {    // a.k.a. the "zero-page"
    uint8_t screen_info[0x40];          /* 0x000 */
    uint8_t apm_bios_info[0x14];        /* 0x040 */
    uint8_t _pad00[12];                 /* 0x054 */
    uint8_t ist_info[0x10];             /* 0x060 */
    uint8_t _pad01[0x30];               /* 0x070 */
    uint8_t sys_desc_table[0x10];       /* 0x0a0 */
    uint8_t _pad02[0x90];               /* 0x0b0 */
    uint8_t edid_info[0x80];            /* 0x140 */
    uint8_t efi_info[0x20];             /* 0x1c0 */
    uint32_t alt_mem_k;                 /* 0x1e0 */
    uint32_t scratch;                   /* 0x1e4 */
    uint8_t e820_entries;               /* 0x1e8 */
    uint8_t eddbuf_entries;             /* 0x1e9 */
    uint8_t edd_mbr_sig_buf_entries;    /* 0x1ea */
    uint8_t _pad03[6];                  /* 0x1eb */

    // struct setup_header
    uint8_t  setup_sects;       /* 0x1f1 */
    uint16_t root_flags;        /* 0x1f2 */
    uint32_t syssize;           /* 0x1f4 */
    uint8_t  _pad1[2];          /* 0x1f8 */
    uint16_t vid_mode;          /* 0x1fa */
    uint16_t root_dev;          /* 0x1fc */
    uint16_t boot_flag;         /* 0x1fe */
    uint16_t jump;              /* 0x200 */
    uint8_t  _pad2[4];          /* 0x202 */
    uint16_t version;           /* 0x206 */
    uint8_t  _pad3[8];          /* 0x208 */
    uint8_t  type_of_loader;    /* 0x210 */
    uint8_t  loadflags;         /* 0x211 */
    uint8_t  _pad4[0x16];       /* 0x212 */
    uint32_t cmd_line_ptr;      /* 0x228 */
    uint8_t  _pad5[0xa4];       /* 0x22c */
    struct e820entry e820_map[128]; /* 0x2d0 */
} __attribute__((packed));

struct boot_params boot_params;

static void *memcpy(void *dest, const void *src, unsigned int n)
{
    char *d = dest;
    const char *s = src;
    while (n-- > 0) {
        *d++ = *s++;
    }
    return dest;
}

//const char *kernel_command_line = "auto";
const char *kernel_command_line =
    "console=ttyS0,115200 console=tty0 earlyprintk=serial,ttyS0,115200"
    //" pci=nobios"
    //" pci=earlydump,noacpi,routeirq,nobios,assign-busses"
    " pci=earlydump"
    //" irqpoll"
    //" video=640x480@60m"
    //" pci=biosirq"
    //" pirq=8,9,10,11"
    //" gxfb.mode_option=1024x768@60"
    //" video=gxfb:1024x768@60"
    //" rootwait root=LABEL=geo.green ro"
    " rootwait root=/dev/sda1 ro"
    " debug loglevel=7"
//    " acpi=off noapm"
//    " no-hlt"
//    " nolapic"
//    " idle=poll"
//    " clocksourcfe=tsc"
//    " mfgptfix"
//    " memmap=exactmap"
//    " memmap=1M$0"
//    " memmap=15M@1M"
    " memtest=4";

extern void d_print(const char *fmt, uint32_t arg)
{
    serial_outstr(fmt);
    l_print(fmt, arg);
}

void load_linux(void)
{
    // Load boot_params
    d_print("Copying boot_params...\r\n", 0);
    struct boot_params *bp = (struct boot_params *)bzImage_start;
    unsigned int bp_size = 0x202 + (bp->jump >> 8);
    d_print("bp_size = 0x%x\r\n", bp_size);
    memcpy(&boot_params, bp, bp_size);
    bzero(&boot_params, 0x1f1);     /* clear everything up to the setup_header */
    bp = &boot_params;

    // Clear the kernel
    const char *kernel32_start = bzImage_start + (bp->setup_sects+1)*512;
    unsigned int kernel32_size = 16 * bp->syssize;
    d_print("Clearing buffer for 32-bit kernel...\r\n", 0);
    d_print("kernel32_entry_point = 0x%08x\r\n", (uint32_t) kernel32_entry_point);
    d_print("kernel32_size = 0x%08x\r\n", kernel32_size);
    d_print("bzImage_start = 0x%08x\r\n", (uint32_t) bzImage_start);
    bzero(kernel32_entry_point, kernel32_size);

    {
        // Dump first few bytes of Linux code
        unsigned char *p = (unsigned char *)kernel32_entry_point;
        d_print("DUMP[0x%08x]:", (uint32_t) p);
        for (int i = 0; i < 16; i++) {
            d_print(" %02x", p[i]);
        }
        d_print("\r\n", 0);
    }

    // Copy kernel
    d_print("Copying 32-bit kernel code...\r\n", 0);
    memcpy(kernel32_entry_point, kernel32_start, kernel32_size);
    d_print("bp->setup_sects = 0x%02x\r\n", bp->setup_sects);
    d_print("bp->syssize = 0x%08x\r\n", bp->syssize);
    d_print("offsetof setup_sects: 0x%x\r\n", offsetof(struct boot_params, setup_sects));
    d_print("offsetof syssize: 0x%x\r\n", offsetof(struct boot_params, syssize));
    d_print("offsetof cmd_line_ptr: 0x%x\r\n", offsetof(struct boot_params, cmd_line_ptr));

    // Set up zero-page
    d_print("Setting up zero-page...\r\n", 0);
    //bp->alt_mem_k = 16*1024;    /* support 16 MiB memory (XXX) */
    bp->e820_entries = 0;

    /* 0x00000000 - 0x0009efff (636 KiB) usable */
    bp->e820_map[0].addr   = 0x00000000;
    bp->e820_map[0].length = 0x0009f000;
    bp->e820_map[0].type   = 1;
    bp->e820_entries++;

    /* 0x0009f000 - 0x0009ffff (4 KiB) reserved */
    /* Linux ignores e820 maps with only 1 entry, so we need some kind of
     * discontinuity. */
    bp->e820_map[1].addr   = 0x0009f000;
    bp->e820_map[1].length = 0x00001000;
    bp->e820_map[1].type   = 2;
    bp->e820_entries++;

    /* 0x000a0000 - 0x00FFffff (16 MiB less 640 KiB) usable */
    bp->e820_map[2].addr   = 0x000a0000;
    bp->e820_map[2].length = 0x00f60000;
    bp->e820_map[2].type   = 1;
    bp->e820_entries++;

    // Set boot_params
    d_print("Setting boot_params...\r\n", 0);
    bp->vid_mode = 0xffff;      /* vga=normal */
    bp->type_of_loader = 0xff;  /* other */
    bp->loadflags = 0x01;  /* LOADED_HIGH, !QUIET_FLAG, !KEEP_SEGMENTS, !CAN_USE_HEAP */
    bp->cmd_line_ptr = (uint32_t) kernel_command_line;

    {
        // Dump first few bytes of Linux code
        unsigned char *p = (unsigned char *)kernel32_entry_point;
        d_print("DUMP[0x%08x]:", (uint32_t) p);
        for (int i = 0; i < 16; i++) {
            d_print(" %02x", p[i]);
        }
        d_print("\r\n", 0);
    }

    // Boot Linux
    d_print("Booting Linux...\r\n", 0);
    boot_linux();
}
