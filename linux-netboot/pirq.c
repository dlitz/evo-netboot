#include "pirq.h"

// See http://www.microsoft.com/whdc/archive/pciirq.mspx
// PCI IRQ Routing Table Specification
// Microsoft Corporation, Version 1.0, February 27, 1998
// (Updated: December 4, 2001)
void create_pirq_table(void)
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

