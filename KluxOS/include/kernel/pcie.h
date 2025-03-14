#ifndef _PCIE_H
#define _PCIE_H

#include <kernel/types.h>

// PCIe yapılandırma alanları
#define PCIE_CONFIG_ADDRESS 0xE0000000
#define PCI_CONFIG_ADDRESS  0xCF8
#define PCI_CONFIG_DATA    0xCFC

// PCIe cihaz sınıfları
#define PCIE_CLASS_NETWORK     0x02
#define PCIE_SUBCLASS_WIFI     0x80

struct pcie_device {
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t prog_if;
    uint8_t revision;
    uint32_t bar[6];        // bar = Base Address Registers
};

// PCIe fonksiyonları
void pcie_init(void);
struct pcie_device* pcie_find_wifi_card(void);
uint32_t pcie_read_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pcie_write_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);

#endif 