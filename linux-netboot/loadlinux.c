#include <stddef.h>

#include "loadlinux.h"
#include "memory.h"
#include "printf.h"

void *bzImage_start = NULL;
void *initrd_start = NULL;
uint32_t initrd_size = 0;
char *kernel_command_line = "auto";
void *kernel32_entry_point = (void *)0x00100000;

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
    uint8_t  _pad4[6];          /* 0x212 */
    uint32_t  ramdisk_image;    /* 0x218 */
    uint32_t  ramdisk_size;     /* 0x21c */
    uint8_t  _pad5[8];          /* 0x220 */
    uint32_t cmd_line_ptr;      /* 0x228 */
    uint8_t  _pad6[0xa4];       /* 0x22c */
    struct e820entry e820_map[128]; /* 0x2d0 */
} __attribute__((packed));

struct boot_params boot_params;

void load_linux(void)
{
    // Load boot_params
    struct boot_params *bp = (struct boot_params *)bzImage_start;
    unsigned int bp_size = 0x202 + (bp->jump >> 8);
    printf(" Copying boot_params (%u bytes)...\n", bp_size);
    memcpy(&boot_params, bp, bp_size);
    bzero(&boot_params, 0x1f1);     /* clear everything up to the setup_header */
    bp = &boot_params;

    // Copy kernel
    unsigned int kernel32_size = 16 * bp->syssize;
    const char *kernel32_start = bzImage_start + (bp->setup_sects+1)*512;
    printf(" Copying 32-bit kernel code to 0x%08x...\n", (uint32_t) kernel32_start);
    memcpy(kernel32_entry_point, kernel32_start, kernel32_size);

    // Dump the first few bytes of Linux code
    if (0) {
        unsigned char *p = (unsigned char *)kernel32_entry_point;
        printf(" DUMP[0x%08x]:", (uint32_t) p);
        for (int i = 0; i < 16; i++) {
            printf(" %02x", p[i]);
        }
        printf("\n");
    }

    // Set boot_params
    printf(" Setting boot_params...\n");
    bp->vid_mode = 0xffff;      /* vga=normal */
    bp->type_of_loader = 0xff;  /* other */
    bp->loadflags = 0x01;  /* LOADED_HIGH, !QUIET_FLAG, !KEEP_SEGMENTS, !CAN_USE_HEAP */
    bp->cmd_line_ptr = (uint32_t) kernel_command_line;
    bp->ramdisk_image = (uint32_t) initrd_start;
    bp->ramdisk_size = initrd_size;

    // Set up zero-page
    printf(" Setting up fake e820 memory map...\n");
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

    /* 0x000a0000 - 0x01d7ffff (29.5 MiB less 640 KiB) usable */
    bp->e820_map[2].addr   = 0x000a0000;
    bp->e820_map[2].length = 0x01ce0000;
    bp->e820_map[2].type   = 1;
    bp->e820_entries++;
}
