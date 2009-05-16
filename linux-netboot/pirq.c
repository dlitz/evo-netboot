#include "pirq.h"
#include "memory.h"
#include "portio.h"
#include "printf.h"
#include <stdbool.h>

#define CFGINT 0x3c

static uint8_t pci_config_in8(unsigned int busno, unsigned int devfunc,
    unsigned int index)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    return inb(0xcfc | (index & 3));
}

static uint16_t pci_config_in16(unsigned int busno, unsigned int devfunc,
    unsigned int index)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    return inw(0xcfc | (index & 2));
}

static uint32_t pci_config_in32(unsigned int busno, unsigned int devfunc,
    unsigned int index)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    return inl(0xcfc);
}

static void pci_config_out8(unsigned int busno, unsigned int devfunc,
    unsigned int index, uint8_t value)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    outb(value, 0xcfc | (index & 3));
}

static void pci_config_out16(unsigned int busno, unsigned int devfunc,
    unsigned int index, uint16_t value)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    outw(value, 0xcfc | (index & 2));
}

static void pci_config_out32(unsigned int busno, unsigned int devfunc,
    unsigned int index, uint32_t value)
{
    outl(0x80000000
            | ((busno & 0xff) << 16)
            | ((devfunc & 0xff) << 8)
            | (index & 0xfc), 0xcf8);
    outl(value, 0xcfc);
}

#define PCI_F0_IN8(index) pci_config_in8(0, 0x12 << 3, index)
#define PCI_F0_IN16(index) pci_config_in16(0, 0x12 << 3, index)
#define PCI_F0_IN32(index) pci_config_in32(0, 0x12 << 3, index)
#define PCI_F0_OUT8(index, value) pci_config_out8(0, 0x12 << 3, index, value)
#define PCI_F0_OUT16(index, value) pci_config_out16(0, 0x12 << 3, index, value)
#define PCI_F0_OUT32(index, value) pci_config_out32(0, 0x12 << 3, index, value)

// irq must be 1,3-7,9-15
static void isa_set_irq_edge(unsigned int irq, bool edge)
{
    if (irq == 0) return;
    uint8_t mask, value;
    uint16_t port;
    if (irq == 1 || (irq >= 2 && irq <= 7)) {
        mask = (1 << irq);
        port = 0x4d0;
    } else if (irq >= 9 && irq <= 15) {
        mask = (1 << (irq - 8));
        port = 0x4d1;
    } else {
        // XXX: Do nothing (we should warn/panic here)
    }
    if (edge) {
        value = mask;
        mask = 0xff;
    } else {
        value = 0;
        mask = ~mask;
    }
    outb((inb(port) & mask) | value, port);
}

static bool isa_get_irq_edge(unsigned int irq)
{
    uint8_t mask;
    uint16_t port;
    if (irq == 1 || (irq >= 2 && irq <= 7)) {
        mask = (1 << irq);
        port = 0x4d0;
    } else if (irq >= 9 && irq <= 15) {
        mask = (1 << (irq - 8));
        port = 0x4d1;
    } else {
        // XXX: Do nothing (we should warn/panic here)
    }
    if (inb(port) & mask) {
        return false;
    } else {
        return true;
    }
}

static void pci_set_inta_irq(unsigned int irq) {
    isa_set_irq_edge(irq, false);
    PCI_F0_OUT8(0x5c, (PCI_F0_IN8(0x5c) & 0xf0) | irq);
}
static void pci_set_intb_irq(unsigned int irq) {
    isa_set_irq_edge(irq, false);
    PCI_F0_OUT8(0x5c, (PCI_F0_IN8(0x5c) & 0x0f) | (irq << 4));
}
static void pci_set_intc_irq(unsigned int irq) {
    isa_set_irq_edge(irq, false);
    PCI_F0_OUT8(0x5d, (PCI_F0_IN8(0x5d) & 0xf0) | irq);
}
static void pci_set_intd_irq(unsigned int irq) {
    isa_set_irq_edge(irq, false);
    PCI_F0_OUT8(0x5d, (PCI_F0_IN8(0x5d) & 0x0f) | (irq << 4));
}

static unsigned int pci_get_inta_irq(void) {
    unsigned int irq = PCI_F0_IN8(0x5c) & 0x0f;
    printf("INTA(%d) EDGE: %d\n", irq, isa_get_irq_edge(irq));
    return irq;
}

static unsigned int pci_get_intb_irq(void) {
    unsigned int irq = PCI_F0_IN8(0x5c) >> 4;
    printf("INTB(%d) EDGE: %d\n", irq, isa_get_irq_edge(irq));
    return irq;
}

static unsigned int pci_get_intc_irq(void) {
    unsigned int irq = PCI_F0_IN8(0x5d) & 0x0f;
    printf("INTC(%d) EDGE: %d\n", irq, isa_get_irq_edge(irq));
    return irq;
}

static unsigned int pci_get_intd_irq(void) {
    unsigned int irq = PCI_F0_IN8(0x5d) >> 4;
    printf("INTD(%d) EDGE: %d\n", irq, isa_get_irq_edge(irq));
    return irq;
}


// See http://www.microsoft.com/whdc/archive/pciirq.mspx
// PCI IRQ Routing Table Specification
// Microsoft Corporation, Version 1.0, February 27, 1998
// (Updated: December 4, 2001)
void create_pirq_table(void)
{
    pci_get_inta_irq();
    pci_get_intb_irq();
    pci_get_intc_irq();
    pci_get_intd_irq();

    // Clear existing IRQ settings to force Linux to configure them
    pci_set_inta_irq(0);
    pci_set_intb_irq(0);
    pci_set_intc_irq(0);
    pci_set_intd_irq(0);

    struct pirq_table *t = (struct pirq_table *)0xf0000;
    bzero(t, sizeof(struct pirq_table));
    t->signature = '$' | ('P' << 8) | ('I' << 16) | ('R' << 24);    // $PIR
    t->version = 0x0100;
    t->table_size = 32 + PIRQ_NUM_SLOTS*16;
    t->irq_router_bus = 0;
    t->irq_router_devfunc = 0x90; // CS5530A: F0 Bridge Configuration
    t->exclusive_irq = (1<<5)|(1<<10)|(1<<11); // IRQs exclusive to PCI: 5, 10, 11
    //t->exclusive_irq = 0;  // IRQs exclusive to PCI: none
    t->irq_router_compat_vendor_id = 0x1078;    // PCI_VENDOR_ID_CYRIX
    t->irq_router_compat_device_id = 0x0002;    // PCI_DEVICE_ID_CYRIX_5520
    t->miniport_data = 0;

    for (unsigned int i = 0; i < PIRQ_NUM_SLOTS; i++) {
        bzero(&t->slots[i], sizeof(struct pirq_slot));
        t->slots[i].pci_bus = 0;
        t->slots[i].inta_link = 2;
        t->slots[i].inta_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intb_link = 1;
        t->slots[i].intb_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intc_link = 4;
        t->slots[i].intc_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].intd_link = 3;
        t->slots[i].intd_bitmap = 0xDEFA; // IRQs 1,3,4,5,6,7,9,10,11,12,14,15
        t->slots[i].slot_number = 0;
        switch (i) {
        // See the CS5530A documentation,
        // Table 5-1 "PCI Configuration Address Register"
        case 0: // Function 0: Bridge Configuration
            t->slots[i].pci_devfunc = 0x90;
            break;
        case 1: // Function 1: SMI Status and ACPI Timer
            t->slots[i].pci_devfunc = 0x91;
            break;
        case 2: // Function 2: IDE Controller
            t->slots[i].pci_devfunc = 0x92;
            break;
        case 3: // Function 3: XpressAUDIO Subsystem
            t->slots[i].pci_devfunc = 0x93;
            break;
        case 4: // Function 4: Video Controller
            t->slots[i].pci_devfunc = 0x94;
            break;
        case 5: // USB Controller
            t->slots[i].pci_devfunc = 0x98;     // 0000:00:13.0
            break;
        case 6: // TI PCI1410APGE CardBus controller
            t->slots[i].pci_devfunc = 0x0e << 3;
            // INTA# on this device triggers INTD# (PIRQ3) at the host
            t->slots[i].inta_link = 3;  // determined through trial-and-error
            // NB: We only use IRQs that Linux's yenta_socket.c will probe for.
            t->slots[i].inta_bitmap = 0x0EF8; // IRQs 3,4,5,6,7,9,10,11
            t->slots[i].intb_link = 0;
            t->slots[i].intb_bitmap = 0;
            t->slots[i].intc_link = 0;
            t->slots[i].intc_bitmap = 0;
            t->slots[i].intd_link = 0;
            t->slots[i].intd_bitmap = 0;
            break;
        case 7: // DP83815 Ethernet Controller  // 0000:00:0f.0
            t->slots[i].pci_devfunc = 0x0f << 3;
            // INTA# on this device triggers INTC# (PIRQ4) at the host
            t->slots[i].inta_link = 4;  // determined through trial-and-error
            t->slots[i].intb_link = 0;
            t->slots[i].intb_bitmap = 0;
            t->slots[i].intc_link = 0;
            t->slots[i].intc_bitmap = 0;
            t->slots[i].intd_link = 0;
            t->slots[i].intd_bitmap = 0;
            break;
        }
    }

    // Compute the PIRQ table checksum.
    t->checksum = 0;
    uint8_t checksum = 0;
    for (int i = 0; i < t->table_size; i++) {
        checksum += ((unsigned char *)t)[i];
    }
    t->checksum = -checksum;

    // WORKAROUND: Zero the "Interrupt Line" (3Ch) registers of all the PCI
    // devices we listed in the PIRQ table.  These registers are for
    // informational purposes only, but if we don't clear them, some drivers
    // (e.g. natsemi, ohci_hcd, ehci_hcd) will claim whatever non-zero IRQ
    // value is stored in the register, rather than the interrupt that
    // actually gets triggered by the hardware.  Setting these to zero seems
    // to force Linux to determine the IRQs via the interrupt steering registers.
    for (unsigned int i = 0; i < PIRQ_NUM_SLOTS; i++) {
        pci_config_out8(t->slots[i].pci_bus, t->slots[i].pci_devfunc, CFGINT, 0);
    }
}

