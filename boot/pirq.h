#ifndef PIRQ_H
#define PIRQ_H

#include <stdint.h>

#define PIRQ_NUM_SLOTS 8

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

extern void create_pirq_table(void);

#endif /* PIRQ_H */
